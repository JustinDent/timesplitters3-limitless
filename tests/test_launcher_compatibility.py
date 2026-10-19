"""Offline contracts for trust, parity gating, patch acceptance and bot follow-up."""
import copy
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('compat', ROOT/'tools/launcher-compatibility.py')
compat = importlib.util.module_from_spec(spec)
spec.loader.exec_module(compat)
HEAD = 'a'*40
BASE = 'b'*40


def pull(number=123):
    return {'number': number, 'state': 'open', 'draft': False, 'changed_files': 1,
            'user': {'login': 'maintainer'},
            'head': {'sha': HEAD, 'ref': 'feature/launcher', 'repo': {'full_name': compat.REPOSITORY}},
            'base': {'sha': BASE, 'ref': 'main', 'repo': {'full_name': compat.REPOSITORY}}}


def report(verdict='pass', patch_text=''):
    return json.dumps({'verdict': verdict, 'summary': 'Both adapters reviewed.', 'findings': [], 'patch': patch_text})


class FakeAPI:
    def __init__(self):
        self.pr = pull()
        self.changed = [{'filename': 'tools/launcher/Ui.cs'}]
        self.permission = 'write'
        self.statuses = []
        self.calls = []
        self.existing = []

    def files(self, number):
        return self.changed

    def status(self, head, state, description):
        self.statuses.append((head, state, description))

    def call(self, method, route, data=None):
        self.calls.append((method, route, data))
        if method == 'GET' and route == '/pulls/123': return copy.deepcopy(self.pr)
        if method == 'GET' and route.startswith('/collaborators/'): return {'permission': self.permission}
        if method == 'GET' and route.startswith('/pulls?'): return self.existing
        if method == 'GET' and route.startswith('/git/commits/'): return {'tree': {'sha': 'c'*40}, 'parents': [{'sha': HEAD}]}
        if method == 'GET' and route.startswith('/git/matching-refs/'): return []
        if method == 'GET' and route == '': return {'default_branch': 'main'}
        if method == 'POST' and route in ('/git/blobs', '/git/trees', '/git/commits'): return {'sha': 'c'*40}
        if method == 'POST' and route == '/pulls':
            return {'number': 124, 'state': 'open', 'html_url': 'https://github.com/fix', 'base': {'ref': self.pr['head']['ref']}}
        if method == 'POST' and route in ('/git/refs', '/actions/workflows/launcher-compatibility.yml/dispatches'): return None
        raise AssertionError((method, route, data))


class ReviewPolicy(unittest.TestCase):
    def setUp(self):
        self.api = FakeAPI()

    def prepare(self, enabled=True, key=True):
        return compat.prepare(self.api, {'number': 123}, enabled, key)

    def test_windows_and_mac_changes_are_equally_relevant(self):
        for path in ['tools/launcher/Launcher.cs', 'tools/launcher/macos/NativeWindow.cs',
                     'tools/launcher/PageTheme.xaml', 'tools/build-launcher.ps1',
                     'tools/build-mac-launcher.py', 'src/pc_display_modes.inc']:
            with self.subTest(path=path):
                self.api.changed = [{'filename': path}]
                self.assertTrue(self.prepare()['run'])

    def test_non_launcher_pr_reports_success_without_key_or_ai(self):
        self.api.changed = [{'filename': 'docs/testing-profile.md'}]
        self.assertFalse(self.prepare(enabled=False, key=False)['run'])
        self.assertEqual(self.api.statuses[-1][1], 'success')
        self.assertFalse(any('/collaborators/' in c[1] for c in self.api.calls))

    def test_removed_launcher_path_in_rename_still_triggers_review(self):
        self.api.changed = [{'filename': 'docs/old-source.txt', 'previous_filename': 'tools/launcher/Ui.cs'}]
        self.assertTrue(self.prepare()['run'])

    def test_missing_key_and_disabled_automation_fail_closed(self):
        for enabled, key in [(False, True), (True, False), (False, False)]:
            with self.subTest(enabled=enabled, key=key):
                self.assertFalse(self.prepare(enabled, key)['run'])
                self.assertEqual(self.api.statuses[-1][1], 'failure')

    def test_draft_fork_deleted_repo_and_untrusted_author_do_not_run_ai(self):
        for change in ('draft', 'fork', 'deleted', 'reader'):
            with self.subTest(change=change):
                self.api = FakeAPI()
                if change == 'draft': self.api.pr['draft'] = True
                if change == 'fork': self.api.pr['head']['repo']['full_name'] = 'outside/fork'
                if change == 'deleted': self.api.pr['head']['repo'] = None
                if change == 'reader': self.api.permission = 'read'
                self.assertFalse(self.prepare()['run'])
                self.assertEqual(self.api.statuses[-1][1], 'failure')

    def test_incomplete_file_list_cannot_skip_review(self):
        self.api.pr['changed_files'] = 301
        self.api.changed = [{'filename': 'README.md'}]
        with self.assertRaises(ValueError): self.prepare()
        self.assertNotEqual(self.api.statuses[-1][1], 'success')

    def test_bot_fix_is_reviewed_but_cannot_spawn_another_fix(self):
        self.api.pr['user']['login'] = 'github-actions[bot]'
        self.api.pr['head']['ref'] = compat.FIX_PREFIX + '123-aaaaaaaaaaaa'
        context = self.prepare()
        self.assertTrue(context['run'])
        self.assertFalse(context['allow_fix'])
        with self.assertRaises(ValueError): compat.parse_report(report('changes_needed', 'patch'), False)

    def test_workflow_dispatch_requires_numeric_pr(self):
        self.assertEqual(compat.pr_number({'inputs': {'pull_request': '42'}}), 42)
        for value in ['1; command', '-2', None, '0']:
            with self.subTest(value=value), self.assertRaises(ValueError):
                compat.pr_number({'inputs': {'pull_request': value}})

    def test_invalid_or_contradictory_agent_output_is_rejected(self):
        examples = [report('pass', 'patch'), report('changes_needed'), 'not json', '{}',
                    report('pass').replace('"findings": []', '"findings": ["missing Mac"]'),
                    report('changes_needed', 'a'*(compat.MAX_PATCH+1))]
        for raw in examples:
            with self.subTest(raw=raw[:100]), self.assertRaises((ValueError, TypeError)):
                compat.parse_report(raw, True)

    def test_pass_requires_actual_success_on_both_hosts(self):
        context = self.prepare()
        for build in ('failure', 'cancelled', 'skipped', 'success'):
            compat.finalize(self.api, context, report(), 'success', build, None)
            self.assertEqual(self.api.statuses[-1][1], 'success' if build == 'success' else 'failure')

    def test_review_failure_and_unfixable_gap_block(self):
        context = self.prepare()
        compat.finalize(self.api, context, '', 'failure', 'success', None)
        self.assertEqual(self.api.statuses[-1][1], 'failure')
        compat.finalize(self.api, context, report('blocked'), 'success', 'success', None)
        self.assertEqual(self.api.statuses[-1][1], 'failure')

    def test_stale_head_base_and_retarget_cannot_publish_results(self):
        context = self.prepare()
        for side, field in [('head', 'sha'), ('base', 'sha'), ('base', 'ref'), ('head', 'ref')]:
            with self.subTest(side=side, field=field):
                self.api.pr = pull()
                self.api.pr[side][field] = 'd'*40 if field == 'sha' else 'different-branch'
                before = len(self.api.statuses)
                with self.assertRaises(ValueError):
                    compat.finalize(self.api, context, report(), 'success', 'success', None)
                self.assertEqual(len(self.api.statuses), before)

    def test_stale_failure_handler_cannot_overwrite_newer_review(self):
        context = self.prepare()
        self.api.pr['base']['sha'] = 'd'*40
        before = len(self.api.statuses)
        compat.fail_current(self.api, context)
        self.assertEqual(len(self.api.statuses), before)
        self.api.pr = pull()
        compat.fail_current(self.api, context)
        self.assertEqual(self.api.statuses[-1][1], 'failure')

    @patch.object(compat, 'apply_patch', return_value=[('tools/launcher/Ui.cs', '100644', b'source')])
    def test_fix_goes_to_separate_branch_and_dispatches_ci_without_auto_merge(self, apply):
        context = self.prepare()
        compat.finalize(self.api, context, report('changes_needed', 'patch'), 'success', 'success', None)
        created = next(c[2] for c in self.api.calls if c[:2] == ('POST', '/pulls'))
        self.assertEqual(created['base'], context['head_ref'])
        self.assertTrue(created['head'].startswith(compat.FIX_PREFIX))
        dispatch = next(c for c in self.api.calls if c[1].endswith('/dispatches'))
        self.assertEqual(dispatch[2], {'ref': 'main', 'inputs': {'pull_request': '124'}})
        self.assertEqual(self.api.statuses[-1][1], 'failure')
        self.assertFalse(any('/merge' in c[1] or c[0] == 'PATCH' for c in self.api.calls))

    @patch.object(compat, 'apply_patch', return_value=[('tools/launcher/Ui.cs', '100644', b'source')])
    def test_retry_reuses_fix_pr_without_overwriting_branch(self, apply):
        context = self.prepare()
        self.api.existing = [{'number': 124, 'state': 'open', 'base': {'ref': context['head_ref']}}]
        compat.finalize(self.api, context, report('changes_needed', 'patch'), 'success', 'success', None)
        self.assertFalse(any(c[0] == 'POST' and c[1] in ('/pulls', '/git/refs', '/git/commits') for c in self.api.calls))
        self.api.existing[0]['state'] = 'closed'
        with self.assertRaises(ValueError):
            compat.finalize(self.api, context, report('changes_needed', 'patch'), 'success', 'success', None)


class PatchPolicy(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        compat.git(self.root, 'init', '-q')
        self.write('tools/launcher/Ui.cs', 'old\n')
        self.write('.github/workflows/guard.yml', 'trusted\n')
        compat.git(self.root, 'add', '.')
        compat.git(self.root, '-c', 'user.name=Test', '-c', 'user.email=test@localhost', 'commit', '-qm', 'base')

    def tearDown(self): self.temp.cleanup()

    def write(self, path, content):
        target = self.root/path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(content, encoding='utf-8', newline='\n')

    def proposal(self, path, text):
        self.write(path, text)
        compat.git(self.root, 'add', '-N', path)
        diff = compat.git(self.root, 'diff', '--binary', 'HEAD').decode()
        compat.git(self.root, 'reset', '--hard', '-q', 'HEAD')
        compat.git(self.root, 'clean', '-fdq')
        return diff

    def test_actual_patch_add_modify_delete(self):
        for path, text in [('tools/launcher/Ui.cs', 'new\n'), ('tests/launcher_new.cs', 'test\n')]:
            with self.subTest(path=path):
                change = compat.apply_patch(self.root, self.proposal(path, text))
                self.assertEqual(change[0][0], path)
                self.assertEqual(change[0][2], text.encode())
                compat.git(self.root, 'reset', '--hard', '-q', 'HEAD')
        (self.root/'tools/launcher/Ui.cs').unlink()
        diff = compat.git(self.root, 'diff', 'HEAD').decode()
        compat.git(self.root, 'reset', '--hard', '-q', 'HEAD')
        self.assertIsNone(compat.apply_patch(self.root, diff)[0][2])

    def test_windows_adapter_patch_preserves_checkout_crlf(self):
        self.write('.gitattributes', '*.ps1 text eol=crlf\n')
        self.write('tools/build-launcher.ps1', 'old\n')
        compat.git(self.root, 'add', '.')
        compat.git(self.root, '-c', 'user.name=Test', '-c', 'user.email=test@localhost', 'commit', '-qm', 'Windows adapter')
        diff = self.proposal('tools/build-launcher.ps1', 'new\n')
        changes = compat.apply_patch(self.root, diff)
        self.assertEqual(changes[0][2], b'new\n')
        self.assertEqual((self.root/'tools/build-launcher.ps1').read_bytes(), b'new\r\n')

    def test_protected_files_and_unrelated_code_rejected(self):
        for path in ['.github/workflows/guard.yml', 'AGENTS.md', 'tools/launcher-compatibility.py',
                     'tests/test_launcher_compatibility.py', 'src/native/main.c', '.env.local']:
            with self.subTest(path=path):
                diff = self.proposal(path, 'changed\n')
                with self.assertRaises(ValueError): compat.apply_patch(self.root, diff)
                compat.git(self.root, 'reset', '--hard', '-q', 'HEAD')

    @unittest.skipIf(os.name == 'nt', 'Windows developer mode is not required for symlink fixtures')
    def test_symlink_rejected(self):
        path = self.root/'tools/launcher/link.cs'
        path.symlink_to('../../.env.local')
        compat.git(self.root, 'add', '-N', 'tools/launcher/link.cs')
        diff = compat.git(self.root, 'diff', 'HEAD').decode()
        compat.git(self.root, 'reset', '--hard', '-q', 'HEAD')
        with self.assertRaises(ValueError): compat.apply_patch(self.root, diff)

    def test_snapshot_excludes_pr_instructions_configuration_and_workflows(self):
        self.write('AGENTS.md', 'untrusted instructions')
        self.write('tools/launcher/.codex/config.toml', 'untrusted config')
        self.write('tools/launcher/AGENTS.md', 'nested instructions')
        compat.git(self.root, 'add', '.')
        compat.git(self.root, '-c', 'user.name=Test', '-c', 'user.email=test@localhost', 'commit', '-qm', 'head')
        head = compat.git(self.root, 'rev-parse', 'HEAD').decode().strip()
        base = compat.git(self.root, 'rev-parse', 'HEAD^').decode().strip()
        destination = self.root/'review'/'source'
        compat.snapshot(self.root, destination, head, base, ROOT)
        self.assertTrue((destination/'tools/launcher/Ui.cs').exists())
        for path in ['AGENTS.md', '.github', 'tools/launcher/.codex', 'tools/launcher/AGENTS.md']:
            self.assertFalse((destination/path).exists())
        self.assertEqual((destination.parent/'trusted-AGENTS.md').read_bytes(), (ROOT/'AGENTS.md').read_bytes())


class ValidationSource(unittest.TestCase):
    def test_explicit_validation_source_matches_checkout_and_cannot_publish(self):
        head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            event = root/'event.json'
            plan = root/'plan.json'
            event.write_text(json.dumps({'repository': {'default_branch': 'main'},
                                         'inputs': {'publish': 'false'}}))
            env = {**os.environ, 'GITHUB_EVENT_PATH': str(event), 'GITHUB_EVENT_NAME': 'workflow_dispatch',
                   'GITHUB_REF': 'refs/heads/main', 'GITHUB_SHA': 'a'*40,
                   'GITHUB_REPOSITORY': compat.REPOSITORY, 'LAUNCHER_VALIDATION_SHA': head}
            env.pop('GITHUB_OUTPUT', None)
            import sys
            command = [sys.executable, str(ROOT/'tools/launcher-release.py'), 'plan', '--plan', str(plan)]
            good = subprocess.run(command, cwd=ROOT, env=env, capture_output=True)
            self.assertEqual(good.returncode, 0, good.stderr.decode())
            self.assertEqual(json.loads(plan.read_text())['source_revision'], head)
            self.assertFalse(json.loads(plan.read_text())['publish'])
            env['LAUNCHER_VALIDATION_SHA'] = 'b'*40
            bad = subprocess.run(command, cwd=ROOT, env=env, capture_output=True)
            self.assertNotEqual(bad.returncode, 0)
            self.assertIn(b'Validation checkout does not match', bad.stderr)


class WorkflowPolicy(unittest.TestCase):
    def test_credential_jobs_are_separate_and_both_host_checks_reused(self):
        text = (ROOT/'.github/workflows/launcher-compatibility.yml').read_text()
        prepare, rest = text.split('\n  review:\n')
        review, rest = rest.split('\n  build:\n')
        build, publish = rest.split('\n  report:\n')
        self.assertNotIn(': write', review)
        self.assertIn('safety-strategy: drop-sudo', review)
        self.assertIn("permission-profile: ':workspace'", review)
        self.assertIn('working-directory: ${{ github.workspace }}/review-input/source', review)
        self.assertNotIn('secrets.', publish)
        self.assertIn('statuses: write', publish)
        self.assertIn('uses: ./.github/workflows/launcher-build.yml', build)
        self.assertNotIn('secrets:', build)
        self.assertIn('source_ref: ${{ needs.prepare.outputs.head }}', build)
        self.assertEqual(text.count('ref: ${{ github.event.repository.default_branch }}'), 3)
        self.assertNotIn('pull_request.title', text)
        self.assertNotIn('pull_request.body', text)
        self.assertNotIn('paths:', prepare)

    def test_reusable_build_is_read_only_and_checks_both_platforms(self):
        text = (ROOT/'.github/workflows/launcher-build.yml').read_text()
        self.assertNotIn(': write', text)
        self.assertIn('runs-on: windows-latest', text)
        self.assertIn('runs-on: macos-15', text)
        self.assertIn('--verify-package', text)
        self.assertIn('needs: [windows, mac]', text)
        self.assertIn('LAUNCHER_VALIDATION_SHA: ${{ inputs.source_ref }}', text)
        self.assertEqual(text.count('ref: ${{ inputs.source_ref || github.sha }}'), 3)
        self.assertEqual(text.count('test_launcher_compatibility.py'), 2)


if __name__ == '__main__': unittest.main()
