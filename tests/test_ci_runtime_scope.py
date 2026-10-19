"""Keep heavyweight patch upgrades scoped without missing combined changes."""
import importlib.util
import json
import re
from pathlib import Path

import pytest

spec = importlib.util.spec_from_file_location('ci_runtime_scope', Path(__file__).resolve().parents[1] / 'tools/ci-runtime-scope.py')
scope = importlib.util.module_from_spec(spec)
spec.loader.exec_module(scope)


@pytest.mark.parametrize('filename', ['issue-forms.yml', 'issue-resolution.yml', 'runtime-patches.yml'])
def test_required_workflows_report_on_every_pull_request(filename):
    workflow = (Path(__file__).resolve().parents[1] / '.github/workflows' / filename).read_text()
    trigger = re.search(r'^  pull_request:\n(.*?)(?=^  [a-z_]+:|^[^ \n]|\Z)', workflow, re.M | re.S)
    assert trigger, 'Required checks need a pull_request trigger.'
    assert not trigger[1].strip(), 'Required workflows must not skip PRs by path, branch or event type.'


def test_version_refresh_cannot_push_directly_to_protected_main():
    workflow = (Path(__file__).resolve().parents[1] / '.github/workflows/issue-form-versions.yml').read_text()
    versions = workflow.split('\n  versions:\n', 1)[1]
    assert '--pull-request' in versions
    assert 'pull-requests: write' in versions
    assert 'git push' not in versions
    assert 'persist-credentials: false' in versions


@pytest.mark.parametrize('filename, job, check', [
    ('issue-forms.yml', 'validate', 'Issue form tests'),
    ('issue-resolution.yml', 'test', 'Issue resolution tests'),
])
def test_pr_issue_checks_contain_tests_only_and_keep_required_names(filename, job, check):
    workflow = (Path(__file__).resolve().parents[1] / '.github/workflows' / filename).read_text()
    triggers = workflow.split('\non:\n', 1)[1].split('\npermissions:\n', 1)[0]
    assert set(re.findall(r'^  ([a-z_]+):', triggers, re.M)) == {'pull_request', 'push'}
    jobs = workflow.split('\njobs:\n', 1)[1]
    assert re.findall(r'^  ([a-z_-]+):', jobs, re.M) == [job]
    assert f'    name: {check}\n' in jobs
    assert ': write' not in workflow
    assert 'GITHUB_TOKEN' not in workflow


@pytest.mark.parametrize('filename, events', [
    ('issue-labels.yml', {'issues'}),
    ('issue-form-versions.yml', {'push', 'release', 'workflow_dispatch'}),
    ('issue-resolution-comments.yml', {'pull_request_target'}),
])
def test_automation_is_not_scheduled_for_open_prs(filename, events):
    workflow = (Path(__file__).resolve().parents[1] / '.github/workflows' / filename).read_text()
    triggers = workflow.split('\non:\n', 1)[1].split('\npermissions:\n', 1)[0]
    assert set(re.findall(r'^  ([a-z_]+):', triggers, re.M)) == events
    if filename == 'issue-labels.yml':
        assert 'types: [opened, edited, reopened]' in triggers
        assert 'issues: write' in workflow and 'contents: write' not in workflow
        assert 'python tools/issue-forms.py labels' in workflow
    elif filename == 'issue-form-versions.yml':
        assert 'types: [published, edited, deleted, unpublished]' in triggers
        assert 'branches: [main]' in triggers
        assert 'needs: validate' in workflow
        assert 'python -m unittest discover -s tests -p test_issue_forms.py -v' in workflow
        # Release tags (including old/deleted ones) must not choose the code
        # used either to validate the references or to propose the update.
        assert workflow.count('ref: ${{ github.event.repository.default_branch }}') == 2
    else:
        assert 'types: [closed]' in triggers and 'branches: [main]' in triggers
        assert 'github.event.pull_request.merged == true' in workflow
        assert 'ref: ${{ github.event.repository.default_branch }}' in workflow
        assert 'persist-credentials: false' in workflow
        assert 'python tools/issue-resolution.py' in workflow


@pytest.mark.parametrize('paths, expected', [
    (['tools/launcher/MainWindow.cs'], {'launcher'}),
    (['tools/build-mac-launcher.py'], {'launcher'}),
    (['config/mac-launcher-sdk.json'], {'launcher'}),
    (['config/pc-discord.default.ini'], {'launcher'}),
    (['tests/launcher_maintenance.cs'], {'launcher'}),
    (['tests/launcher_downloads.cs'], {'launcher'}),
    (['tests/launcher_startup.cs'], {'launcher'}),
    (['tests/launcher_news.cs'], {'launcher'}),
    (['tests/launcher_crashes.cs', 'tests/launcher_sessions.cs'], {'launcher'}),
    (['tools/crash_helper.c', 'src/pc_crash.c', 'tests/test_crash_capture.py'], {'native'}),
    (['tools/crash-symbols.py', 'tests/test_crash_symbols.py'], {'release'}),
    (['tools/build-game-icon.ps1', 'tests/test_game_icon.py'], {'launcher'}),
    (['tools/publish-release.py'], {'release'}),
    (['tools/publish-release.py', 'docs/releasing.md', 'STATUS.md'], {'release'}),
    (['src/pc_display.c'], {'native'}),
    (['tools/online_theme.py', 'tools/bundle_online.py', 'tools/network-identity.py'], {'native'}),
    (['src/pc_display_modes.inc'], {'native', 'launcher'}),
    (['tools/release/ImportGame.cs', 'config/pc-controls.default.json'], {'launcher'}),
    (['tools/launcher/MainWindow.cs', 'tests/test_audio_timing.py'], {'launcher', 'native'}),
    (['tests/test_publish_release.py', 'tools/recycle-release-work.ps1'], {'release'}),
    (['patches/legacy/old.patch'], set(scope.SUITES)),
    (['tools/apply-patches.py'], set(scope.SUITES)),
    (['tsfp.ps1'], set(scope.SUITES)),
    (['requirements.txt'], set(scope.SUITES)),
    (['.github/workflows/runtime-patches.yml'], set(scope.SUITES)),
    ([], set()),
])
def test_scope(paths, expected):
    assert {name for name, enabled in scope.affected(paths).items() if enabled} == expected


def test_manual_and_new_branch_runs_cover_everything():
    assert all(scope.affected([], full=True).values())


@pytest.mark.parametrize('kind', ['pull_request', 'push', 'workflow_dispatch', 'new_branch'])
def test_event_routing_and_outputs(tmp_path, monkeypatch, kind):
    base = 'a' * 40
    event = {'pull_request': {'base': {'sha': base}}} if kind == 'pull_request' else {'before': base}
    if kind == 'new_branch':
        event['before'] = '0' * 40
    event_file, output = tmp_path / 'event.json', tmp_path / 'output'
    event_file.write_text(json.dumps(event))
    monkeypatch.setenv('GITHUB_EVENT_PATH', str(event_file))
    monkeypatch.setenv('GITHUB_OUTPUT', str(output))
    monkeypatch.setenv('GITHUB_EVENT_NAME', 'push' if kind == 'new_branch' else kind)
    calls = []
    monkeypatch.setattr(scope.subprocess, 'run', lambda args, **kwargs: calls.append(args))
    def diff(args):
        calls.append(args)
        return b'tools/publish-release.py\0docs/releasing.md\0'
    monkeypatch.setattr(scope.subprocess, 'check_output', diff)
    scope.main()
    result = dict(line.split('=') for line in output.read_text().splitlines())
    if kind in ('workflow_dispatch', 'new_branch'):
        assert all(value == 'true' for value in result.values())
        assert not calls
    else:
        assert result == dict(transitions='false', native='false', release='true', launcher='false')
        assert calls[0] == ['git', 'fetch', '--no-tags', '--depth=1', 'origin', base]
        assert calls[1] == ['git', 'diff', '--no-renames', '--name-only', '-z', base, 'HEAD']
