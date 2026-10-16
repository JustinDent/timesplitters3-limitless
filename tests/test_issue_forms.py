"""Offline checks for release choices and automatic report labels; no live mutations."""
import importlib.util
from pathlib import Path
import shutil
import tempfile
import unittest
import urllib.error
from unittest.mock import Mock

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('issue_forms', ROOT / 'tools/issue-forms.py')
forms = importlib.util.module_from_spec(spec)
spec.loader.exec_module(forms)


def release(tag, **changes):
    return dict(tag_name=tag, draft=False, published_at='2026-10-16T00:00:00Z', **changes)


def report(platform='Windows 11', enhancement=False, legacy=False):
    heading = 'What would you like to add or change?' if enhancement else 'What happened?'
    if enhancement and not legacy:
        return f'### {heading}\n\nAn idea\n\n### Anything else? (optional)\n\n_No response_\n'
    return (f'### Game version\n\nv0.2.0-alpha.4\n\n'
            f'### Launcher version\n\nv1.2.1\n\n'
            f'### Platform\n\n{platform}\n\n### {heading}\n\nDescription\n'
            + ('\n### How would this help?\n\nA reason\n' if enhancement else ''))


def http_error(code):
    return urllib.error.HTTPError('https://api.github.com/', code, 'Fixture error', {}, None)


class VersionTests(unittest.TestCase):
    def test_release_kinds_drafts_and_semantic_order(self):
        records = [release(tag) for tag in (
            'v1.0.0-alpha.2', 'v1.0.0-alpha.10', 'v1.0.0',
            'v2.0.0-beta.1', 'launcher-v1.10.0', 'launcher-v1.2.0',
            'launcher-v1.10.0', 'unrelated', 'v1.0.0-alpha.01', 'v01.0.0',
            'v2.0.0\nInjected heading')]
        records.extend([
            dict(release('v9.0.0'), draft=True),
            dict(release('launcher-v9.0.0'), published_at=None)])
        versions = forms.release_versions(records)
        self.assertEqual(versions['GAME'], [
            'v2.0.0-beta.1', 'v1.0.0', 'v1.0.0-alpha.10', 'v1.0.0-alpha.2'])
        self.assertEqual(versions['LAUNCHER'], ['v1.10.0', 'v1.2.0'])

    def test_options_are_bounded_and_keep_recent_versions(self):
        versions = forms.release_versions([release(f'v1.0.{i}') for i in range(40)])
        self.assertEqual(len(versions['GAME']), 30)
        self.assertEqual(versions['GAME'][0], 'v1.0.39')
        self.assertEqual(versions['GAME'][-1], 'v1.0.10')

    def test_pagination_does_not_miss_older_release_pages(self):
        client = forms.GitHub('owner/repo')
        client.request = Mock(side_effect=[[release('v1.0.0')] * 100,
                                           [release('launcher-v1.0.0')]])
        records = client.releases()
        self.assertEqual(len(records), 101)
        self.assertEqual(client.request.call_args_list[1].args,
                         ('GET', 'releases?per_page=100&page=2'))

    def test_api_failures_are_not_treated_as_an_empty_release_list(self):
        client = forms.GitHub('owner/repo')
        client.request = Mock(side_effect=http_error(403))
        with self.assertRaises(urllib.error.HTTPError):
            client.releases()
        client.request = Mock(return_value={'message': 'rate limited'})
        with self.assertRaises(ValueError):
            client.releases()

    def test_update_is_idempotent_and_preserves_questions_and_fallbacks(self):
        with tempfile.TemporaryDirectory(prefix='tsfp-issue-forms-test-') as directory:
            root = Path(directory)
            for relative in forms.FORMS:
                (root / relative).parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(ROOT / relative, root / relative)
            records = [release('v3.0.0'), release('launcher-v4.0.0')]
            originals = [(root / p).read_bytes() for p in forms.FORMS]
            self.assertEqual(len(forms.refresh_versions(root, records, check=True)), 1)
            self.assertEqual(originals, [(root / p).read_bytes() for p in forms.FORMS])
            self.assertEqual(len(forms.refresh_versions(root, records)), 1)
            self.assertEqual(forms.refresh_versions(root, records), [])
            for relative in forms.FORMS:
                text = (root / relative).read_text()
                self.assertIn('- "v3.0.0"', text)
                self.assertIn('- "v4.0.0"', text)
                self.assertIn('Other / source build', text)
                self.assertIn("I don't know", text)
                self.assertIn('Anything else? (optional)', text)
                self.assertNotIn('default:', text)
            forms.refresh_versions(root, [])
            self.assertNotIn('- "v3.0.0"', (root / forms.FORMS[0]).read_text())
            self.assertIn('Not installed yet', (root / forms.FORMS[0]).read_text())

    def test_bad_launcher_block_does_not_partially_update_game_versions(self):
        with tempfile.TemporaryDirectory(prefix='tsfp-issue-forms-test-') as directory:
            root = Path(directory)
            for relative in forms.FORMS:
                (root / relative).parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(ROOT / relative, root / relative)
            first = root / forms.FORMS[0]
            first.write_text(first.read_text().replace('END AUTO LAUNCHER VERSIONS', 'missing marker'))
            before = first.read_bytes()
            with self.assertRaises(ValueError):
                forms.refresh_versions(root, [release('v3.0.0')])
            self.assertEqual(first.read_bytes(), before)

    def test_enhancement_stays_without_versions_or_platform_after_refresh(self):
        relative = '.github/ISSUE_TEMPLATE/02-enhancement.yml'
        with tempfile.TemporaryDirectory(prefix='tsfp-issue-forms-test-') as directory:
            root = Path(directory)
            for name in (*forms.FORMS, relative):
                (root / name).parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(ROOT / name, root / name)
            path = root / relative
            before = path.read_bytes()
            forms.refresh_versions(root, [release('v3.0.0'), release('launcher-v4.0.0')])
            self.assertEqual(path.read_bytes(), before)
            for removed in ('game-version', 'launcher-version', 'id: platform', 'id: reason', 'AUTO GAME', 'AUTO LAUNCHER'):
                self.assertNotIn(removed, path.read_text())
            self.assertEqual(path.read_text().count('  - type: textarea'), 2)


class LabelTests(unittest.TestCase):
    def test_platform_and_type_come_from_form_answers(self):
        for platform, label in [('Windows 10', 'platform: windows'),
                                ('Windows 11', 'platform: windows'),
                                ('macOS', 'platform: macos')]:
            with self.subTest(platform=platform):
                self.assertEqual(forms.report_labels(report(platform)), {'bug', label})
                self.assertEqual(forms.report_labels(report(platform, True, legacy=True)), {'enhancement', label})
        self.assertEqual(forms.report_labels(report('Other / not sure')), {'bug'})
        self.assertEqual(forms.report_labels(report().replace('\n', '\r\n')),
                         {'bug', 'platform: windows'})

    def test_short_enhancement_gets_type_label_without_versions_or_platform(self):
        self.assertEqual(forms.report_labels(report(enhancement=True)), {'enhancement'})

    def test_manual_reports_and_pull_requests_are_untouched(self):
        client = Mock()
        self.assertFalse(forms.label_report(client, {'number': 1, 'body': 'Plain report'}))
        self.assertFalse(forms.label_report(client, {
            'number': 2, 'body': report(), 'pull_request': {}}))
        client.request.assert_not_called()

    def test_edits_replace_old_platform_but_preserve_manual_labels(self):
        client = Mock()
        issue = dict(number=5, body=report('macOS', True), labels=[{'name': name} for name in (
            'bug', 'platform: windows', 'major', 'accessibility')])
        self.assertTrue(forms.label_report(client, issue))
        mutations = [call.args for call in client.request.call_args_list if call.args[0] != 'GET']
        self.assertIn(('POST', 'issues/5/labels',
                       {'labels': ['enhancement']}), mutations)
        self.assertIn(('DELETE', 'issues/5/labels/bug'), mutations)
        self.assertIn(('DELETE', 'issues/5/labels/platform%3A%20windows'), mutations)
        self.assertFalse(any('major' in str(call) or 'accessibility' in str(call) for call in mutations))

    def test_missing_platform_label_is_created(self):
        client = Mock()
        client.request.side_effect = [http_error(404), {}, {}]
        forms.label_report(client, dict(number=7, body=report(), labels=[{'name': 'bug'}]))
        calls = client.request.call_args_list
        self.assertEqual(calls[1].args[0:2], ('POST', 'labels'))
        self.assertEqual(calls[1].args[2]['name'], 'platform: windows')
        self.assertEqual(calls[2].args, ('POST', 'issues/7/labels',
                                        {'labels': ['platform: windows']}))

    def test_concurrent_label_creation_is_rechecked(self):
        client = Mock()
        client.request.side_effect = [http_error(404), http_error(422), {}, {}]
        forms.label_report(client, dict(number=7, body=report(), labels=[{'name': 'bug'}]))
        self.assertEqual(client.request.call_args_list[2].args,
                         ('GET', 'labels/platform%3A%20windows'))

    def test_label_permission_error_is_reported_without_removing_labels(self):
        client = Mock()
        client.request.side_effect = http_error(403)
        with self.assertRaises(urllib.error.HTTPError):
            forms.label_report(client, dict(number=7, body=report('macOS'),
                                            labels=[{'name': 'bug'}, {'name': 'platform: windows'}]))
        self.assertEqual(client.request.call_count, 1)


if __name__ == '__main__':
    unittest.main()
