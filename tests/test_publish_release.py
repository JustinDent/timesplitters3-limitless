"""Release orchestration checks use tiny fixtures and a simulated GitHub CLI.

No real tag, release, game data or personal saves are touched.
"""
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import sys
import zipfile

import pytest

spec = importlib.util.spec_from_file_location('publish_release', Path(__file__).resolve().parents[1]/'tools/publish-release.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
HEAD, PREVIOUS = 'a'*40, 'b'*40


class FakePublisher(module.Publisher):
    def __init__(self, root):
        super().__init__(root)
        (root/'VERSION').write_text('0.2.0-alpha.3')
        self.calls = []
        self.branch, self.dirty, self.remote = 'main', False, HEAD
        self.releases = [dict(tag_name='v0.2.0-alpha.2', draft=False, prerelease=True, published_at='2026-10-15T01:00:00Z')]
        self.bad_upload = False
        self.tag_created = False
        self.builds = 0

    def git(self, *args):
        self.calls.append(('git', args))
        if args[0] == 'status':
            return ' M source.c' if self.dirty else ''
        if args[:2] == ('branch', '--show-current'):
            return self.branch
        if args[:2] == ('remote', 'get-url'):
            return 'https://github.com/owner/repo.git'
        if args[0] == 'fetch':
            return ''
        if args[0] == 'rev-parse':
            return PREVIOUS if 'refs/tags/' in args[-1] else HEAD
        if args[0] == 'ls-remote':
            tag = HEAD+'\trefs/tags/'+self.tag if self.tag_created else ''
            return tag if len(args) == 3 else self.remote+'\trefs/heads/main'+ ('\n'+tag if tag else '')
        if args[0] == 'log':
            return HEAD+'\tAdd Online hosting and <diagnostics>'
        raise AssertionError(args)

    def gh(self, *args, payload=None):
        self.calls.append(('gh', args, payload))
        if args[:2] == ('auth', 'status'):
            return ''
        if args[:2] == ('api', '--paginate'):
            return json.dumps([self.releases])
        if 'generate-notes' in str(args):
            assert payload['previous_tag_name'] == 'v0.2.0-alpha.2'
            assert payload['target_commitish'] == HEAD
            return json.dumps(dict(body='## What\u2019s Changed\n\n* Online multiplayer #12'))
        if 'git/refs' in str(args):
            assert payload == dict(ref='refs/tags/'+self.tag, sha=HEAD)
            self.tag_created = True
            return json.dumps(dict(ref=payload['ref'], object=dict(sha=HEAD)))
        if args[:2] in (('release', 'create'), ('release', 'edit')):
            return ''
        if args[:2] == ('release', 'view'):
            files = (self.archive, self.checksum, self.metadata)
            return json.dumps(dict(isDraft=True, targetCommitish=HEAD, url='https://github.com/owner/repo/releases/tag/'+self.tag,
                assets=[dict(name=p.name, size=p.stat().st_size+(1 if self.bad_upload else 0)) for p in files]))
        raise AssertionError(args)

    def recycle(self, path):
        # Only this test's own fixture stage, never a real release tree.
        assert path == self.stage and path.parent == self.output
        shutil.rmtree(path)

    def build_package(self):
        self.builds += 1
        self.stage.mkdir()
        binary = self.stage/'Bin/game.exe'
        binary.parent.mkdir()
        binary.write_bytes(b'fixture executable, no original game data')
        build = dict(source_commit=HEAD, version=self.tag, source_changes=[], files={'Bin/game.exe':module.digest(binary)})
        (self.stage/'BUILD.json').write_text(json.dumps(build))
        with zipfile.ZipFile(self.archive, 'x') as archive:
            for path in (binary, self.stage/'BUILD.json'):
                archive.write(path, self.name+'/'+path.relative_to(self.stage).as_posix())
        self.checksum.write_text(module.digest(self.archive)+'  '+self.archive.name+'\n')
        self.verify_package()
        shutil.copyfile(self.stage/'BUILD.json', self.metadata)


@pytest.fixture
def publisher(tmp_path, monkeypatch):
    monkeypatch.setattr(module.shutil, 'which', lambda name: name)
    return FakePublisher(tmp_path)


def publication_calls(publisher):
    return [call for call in publisher.calls if call[0]=='gh' and call[1][:2] in (('release','create'), ('release','edit'))]


def test_prepare_then_publish_reuses_exact_package(publisher):
    publisher.preflight()
    publisher.prepare()
    assert not publication_calls(publisher)
    assert not publisher.stage.exists()
    notes = publisher.notes.read_text(encoding='utf-8')
    assert 'v0.2.0-alpha.2' in notes and 'What\u2019s Changed' in notes and 'Add Online hosting and &lt;diagnostics&gt;' in notes
    publisher.prepare()
    assert publisher.builds == 1
    publisher.publish()
    create, edit = publication_calls(publisher)
    assert '--draft' in create[1] and '--prerelease' in create[1] and '--latest=false' in create[1]
    assert create[1][create[1].index('--target')+1] == HEAD
    assert '--notes-file' in create[1] and '--draft=false' in edit[1]
    assert '--verify-tag' in create[1]


@pytest.mark.parametrize('problem', ['branch', 'dirty', 'remote', 'published', 'version'])
def test_preflight_blocks_wrong_or_already_published_source(publisher, problem):
    if problem == 'branch': publisher.branch = 'feature'
    if problem == 'dirty': publisher.dirty = True
    if problem == 'remote': publisher.remote = PREVIOUS
    if problem == 'published': publisher.releases[0]['tag_name'] = 'v0.2.0-alpha.3'
    if problem == 'version': (publisher.root/'VERSION').write_text('0.2.0-alpha.1')
    with pytest.raises(ValueError):
        publisher.preflight()
    assert not publication_calls(publisher) and publisher.builds == 0


def test_changed_prepared_asset_is_not_uploaded(publisher):
    publisher.preflight(); publisher.prepare()
    publisher.archive.write_bytes(b'changed after tests')
    with pytest.raises(ValueError, match='assets changed'):
        publisher.prepare()
    assert not publication_calls(publisher)


def test_upload_failure_leaves_draft_unpublished(publisher):
    publisher.preflight(); publisher.prepare()
    publisher.bad_upload = True
    with pytest.raises(ValueError, match='not published'):
        publisher.publish()
    assert len(publication_calls(publisher)) == 1


def test_draft_option_never_publishes(publisher):
    publisher.preflight(); publisher.prepare(); publisher.publish(draft=True)
    assert len(publication_calls(publisher)) == 1


def test_package_manifest_rejects_changed_stage(publisher):
    publisher.preflight(); publisher.output.mkdir(parents=True); publisher.build_package()
    (publisher.stage/'Bin/game.exe').write_bytes(b'changed after packaging')
    with pytest.raises(ValueError, match='Staged file changed'):
        publisher.verify_package()


def test_published_baseline_includes_prereleases_and_ignores_drafts():
    stable = dict(tag_name='v0.1.0', draft=False, published_at='2026-01-01T00:00:00Z')
    preview = dict(tag_name='v0.2.0-alpha.2', draft=False, published_at='2026-10-01T00:00:00Z')
    draft = dict(tag_name='v9.0.0', draft=True, published_at=None)
    assert module.last_published([[stable], [preview, draft]])[1] == preview


def test_semver_order_and_origin_validation():
    versions = ['0.2.0-alpha.2', '0.2.0-alpha.10', '0.2.0-beta.1', '0.2.0', '0.2.1']
    assert sorted(versions, key=module.version_key) == versions
    with pytest.raises(ValueError): module.version_key('0.2.0-alpha.01')
    assert module.github_repository('git@github.com:owner/repo.git') == 'owner/repo'
    with pytest.raises(ValueError): module.github_repository('https://token@github.com/owner/repo')


def test_failed_build_prevents_packaging_and_publication(publisher, monkeypatch):
    publisher.preflight()
    commands = []
    def fail_build(arguments, **kwargs):
        commands.append(arguments)
        if arguments[0] == 'powershell.exe':
            raise RuntimeError('build failed')
        return ''
    monkeypatch.setattr(publisher, 'run', fail_build)
    with pytest.raises(RuntimeError, match='build failed'):
        module.Publisher.build_package(publisher)
    assert len(commands) == 2 and not publication_calls(publisher)


def test_failed_cleanup_preserves_original_test_error(publisher, monkeypatch, capsys):
    publisher.preflight()
    def run(arguments, **kwargs):
        if '-m' in arguments and 'pytest' in arguments:
            (publisher.root / 'build/release-tests').mkdir(parents=True)
            raise RuntimeError('tests failed')
        return ''
    def fail_cleanup(path):
        raise RuntimeError('cleanup unavailable')
    monkeypatch.setattr(publisher, 'run', run)
    monkeypatch.setattr(publisher, 'recycle', fail_cleanup)
    with pytest.raises(RuntimeError, match='^tests failed$'):
        module.Publisher.build_package(publisher)
    assert 'cleanup unavailable' in capsys.readouterr().err


@pytest.mark.skipif(sys.platform != 'win32', reason='Windows cleanup script')
@pytest.mark.parametrize('relative', [
    'build/release-tests',
    'build/releases/TimeSplitters-Future-Perfect-PC-v0.2.0-alpha.2-windows-x64',
    'private/saves',
])
def test_release_cleanup_targets_only_disposable_work(tmp_path, relative):
    script = tmp_path / 'tools/recycle-release-work.ps1'
    script.parent.mkdir()
    shutil.copyfile(module.ROOT / 'tools/recycle-release-work.ps1', script)
    target = tmp_path / relative
    target.mkdir(parents=True)
    (target / 'fixture.txt').write_text('disposable fixture')
    archive = tmp_path / 'build/releases/keep.zip'
    archive.parent.mkdir(parents=True, exist_ok=True)
    archive.write_bytes(b'keep')
    result = subprocess.run(['powershell.exe', '-NoProfile', '-File', str(script),
                             '-LiteralPath', str(target)], capture_output=True, text=True)
    if relative == 'private/saves':
        assert result.returncode != 0
        assert (target / 'fixture.txt').exists()
    else:
        assert result.returncode == 0, result.stderr
        assert not target.exists()
    assert archive.read_bytes() == b'keep'


def test_release_lock_prevents_overlapping_builds(tmp_path):
    with module.release_lock(tmp_path/'release.lock'):
        with pytest.raises(ValueError, match='already running'):
            with module.release_lock(tmp_path/'release.lock'):
                pytest.fail('Second release acquired the lock')


def test_main_moving_after_preparation_prevents_upload(publisher):
    publisher.preflight(); publisher.prepare()
    publisher.remote = PREVIOUS
    with pytest.raises(ValueError, match='main changed'):
        publisher.publish()
    assert not publication_calls(publisher)


def test_changed_asset_between_prepare_and_publish_is_blocked(publisher):
    publisher.preflight(); publisher.prepare()
    publisher.archive.write_bytes(b'changed immediately before upload')
    with pytest.raises(ValueError, match='changed before upload'):
        publisher.publish()
    assert not publication_calls(publisher)
