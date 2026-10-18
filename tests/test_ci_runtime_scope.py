"""Keep heavyweight patch upgrades scoped without missing combined changes."""
import importlib.util
import json
from pathlib import Path

import pytest

spec = importlib.util.spec_from_file_location('ci_runtime_scope', Path(__file__).resolve().parents[1] / 'tools/ci-runtime-scope.py')
scope = importlib.util.module_from_spec(spec)
spec.loader.exec_module(scope)


@pytest.mark.parametrize('paths, expected', [
    (['tools/launcher/MainWindow.cs'], {'launcher'}),
    (['tests/launcher_maintenance.cs'], {'launcher'}),
    (['tests/launcher_downloads.cs'], {'launcher'}),
    (['tests/launcher_news.cs'], {'launcher'}),
    (['tools/build-game-icon.ps1', 'tests/test_game_icon.py'], {'launcher'}),
    (['tools/publish-release.py'], {'release'}),
    (['tools/publish-release.py', 'docs/releasing.md', 'STATUS.md'], {'release'}),
    (['src/pc_display.c'], {'native'}),
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
