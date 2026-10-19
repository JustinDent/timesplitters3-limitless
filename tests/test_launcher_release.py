"""Exercise dual-host publication, incomplete uploads and immutable release recovery offline."""
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import urllib.parse
import zipfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('launcher_release', ROOT/'tools/launcher-release.py')
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)
SHA = 'a'*40


class FakeGitHub:
    def __init__(self):
        self.calls = []
        self.tag = None
        self.release = None
        self.assets = []
        self.fail_upload = None
        self.bad_digest = False
        self.latest = {'tag_name': 'launcher-v1.2.18'}

    def call(self, method, path, data=None, upload=None):
        self.calls.append((method, path, data))
        if '/git/ref/tags/' in path:
            return {'object': {'type': 'commit', 'sha': self.tag}} if self.tag else None
        if path.endswith('/git/refs'):
            self.tag = data['sha']
            return {}
        if '/releases/tags/' in path:
            return copy.deepcopy(self.release)
        if path.endswith('/releases/latest'):
            return self.latest
        if path.endswith('/releases/generate-notes'):
            return {'body': 'Changes'}
        if method == 'POST' and path.endswith('/releases'):
            assert data['draft'], 'The first visible release state must be a draft.'
            self.release = dict(data, id=1)
            return copy.deepcopy(self.release)
        if method == 'GET' and '/assets?' in path:
            return copy.deepcopy(self.assets)
        if upload:
            name = urllib.parse.parse_qs(urllib.parse.urlparse(path).query)['name'][0]
            if name == self.fail_upload:
                raise RuntimeError('Simulated interrupted upload')
            asset = {'name': name, 'id': len(self.calls), 'state': 'uploaded',
                     'size': upload.stat().st_size,
                     'digest': 'sha256:' + ('0'*64 if self.bad_digest else release.digest(upload))}
            self.assets.append(asset)
            return asset
        if method == 'DELETE':
            self.assets = [a for a in self.assets if a['id'] != int(path.rsplit('/', 1)[-1])]
            return None
        if method == 'PATCH':
            assert len(self.assets) == 12, 'Never publish a partial release.'
            self.release.update(data)
            return self.release
        if path.endswith('/dispatches'):
            assert not self.release['draft']
            return None
        raise AssertionError((method, path, data))


class LauncherReleaseTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.directory = Path(self.temp.name)
        self.plan = dict(schema=1, version='1.2.19', tag='launcher-v1.2.19', source_revision=SHA,
                         repository=release.REPOSITORY, publish=True, prerelease=False, default_branch='main')
        self.api = FakeGitHub()
        self.git = patch.object(release, 'git', return_value=SHA).start()
        self.version = patch.object(release, 'source_version', return_value=self.plan['version']).start()
        self.addCleanup(patch.stopall)

    def package(self, platform, *, extra=None, executable=True):
        files = ({'TimeSplitters Launcher.exe': b'bootstrap', 'Bin/TimeSplitters.App.exe': b'launcher'}
                 if platform == 'windows-x64' else {
                     'TimeSplitters Launcher.app/Contents/Info.plist': b'plist',
                     'TimeSplitters Launcher.app/Contents/MacOS/TimeSplitters.Launcher': b'native',
                     'TimeSplitters Launcher.app/Contents/Resources/Licenses/Avalonia.txt': b'license'})
        if extra:
            files.update(extra)
        manifest = {'schema': 1, 'version': self.plan['version'],
                    'files': {name: hashlib.sha256(data).hexdigest() for name, data in files.items()}}
        files['Support/Launcher/manifest.json'] = json.dumps(manifest).encode()
        archive = self.directory/release.package_names(self.plan['version'], platform)[0]
        with zipfile.ZipFile(archive, 'w') as package:
            for name, data in files.items():
                info = zipfile.ZipInfo(name)
                info.external_attr = (0o100755 if executable else 0o100644) << 16
                package.writestr(info, data)
        checksum = release.digest(archive)
        archive.with_suffix('.zip.sha256').write_text(checksum+'  '+archive.name+'\n')
        release.write_json(self.directory/release.manifest_name(platform),
                           dict(schema=1, version=self.plan['version'], asset_name=archive.name,
                                size=archive.stat().st_size, sha256=checksum))
        return archive

    def both(self):
        for platform in release.PLATFORMS:
            self.package(platform)
            release.stamp(self.directory, self.plan, platform)

    def test_complete_release_publishes_only_after_all_twelve_uploads(self):
        self.both()
        url = release.publish(self.directory, self.plan, self.api)
        self.assertTrue(url.endswith('/launcher-v1.2.19'))
        self.assertFalse(self.api.release['draft'])
        self.assertEqual(self.api.release['make_latest'], 'true')
        self.assertEqual(self.api.calls[-1][1].split('/')[-2:], ['issue-form-versions.yml', 'dispatches'])
        self.assertEqual(self.api.tag, SHA)

    def test_missing_platform_does_not_even_contact_github(self):
        self.package('windows-x64')
        release.stamp(self.directory, self.plan, 'windows-x64')
        with self.assertRaises(FileNotFoundError):
            release.publish(self.directory, self.plan, self.api)
        self.assertEqual(self.api.calls, [])

    def test_mixed_revision_or_corrupt_alias_prevents_remote_mutation(self):
        self.both()
        receipt = self.directory/'launcher-build-macos-arm64.json'
        value = json.loads(receipt.read_text())
        value['source_revision'] = 'b'*40
        release.write_json(receipt, value)
        with self.assertRaisesRegex(ValueError, 'same tested source'):
            release.publish(self.directory, self.plan, self.api)
        self.assertEqual(self.api.calls, [])
        release.stamp(self.directory, self.plan, 'macos-arm64')
        (self.directory/'TimeSplitters-Launcher-windows-x64.zip').write_bytes(b'bad')
        with self.assertRaisesRegex(ValueError, 'alias'):
            release.publish(self.directory, self.plan, self.api)
        self.assertEqual(self.api.calls, [])

    def test_wrong_builder_revision_is_rejected(self):
        self.git.return_value = 'b'*40
        with self.assertRaisesRegex(ValueError, 'planned source'):
            release.stamp(self.directory, self.plan, 'macos-arm64')

    def test_corrupt_metadata_and_payload_are_rejected(self):
        archive = self.package('windows-x64')
        archive.write_bytes(archive.read_bytes()+b'corrupt')
        with self.assertRaisesRegex(ValueError, 'manifest mismatch'):
            release.verify_package(self.directory, self.plan, 'windows-x64')

    def test_unsafe_unexpected_and_case_collision_payloads_are_rejected(self):
        for path in ('../escape.exe', '/absolute.exe', 'Bin\\escape.exe', 'Bin/extra.exe', 'bin/timesplitters.app.exe'):
            with self.subTest(path=path):
                self.package('windows-x64', extra={path: b'bad'})
                with self.assertRaises(ValueError):
                    release.verify_package(self.directory, self.plan, 'windows-x64')

    def test_mac_executable_mode_is_required(self):
        self.package('macos-arm64', executable=False)
        with self.assertRaisesRegex(ValueError, 'permissions'):
            release.verify_package(self.directory, self.plan, 'macos-arm64')

    def test_failed_upload_keeps_draft_and_retry_completes_it(self):
        self.both()
        self.api.fail_upload = 'launcher-update-macos-arm64.json'
        with self.assertRaisesRegex(RuntimeError, 'interrupted'):
            release.publish(self.directory, self.plan, self.api)
        self.assertTrue(self.api.release['draft'])
        self.assertFalse(any(c[0] == 'PATCH' for c in self.api.calls))
        self.api.fail_upload = None
        release.publish(self.directory, self.plan, self.api)
        self.assertFalse(self.api.release['draft'])
        self.assertEqual(len(self.api.assets), 12)
        self.assertEqual(sum(c[0] == 'POST' and c[1].endswith('/releases') for c in self.api.calls), 1)

    def test_digest_mismatch_keeps_draft_then_retry_repairs_owned_assets(self):
        self.both()
        self.api.bad_digest = True
        with self.assertRaisesRegex(ValueError, 'stays a draft'):
            release.publish(self.directory, self.plan, self.api)
        self.assertTrue(self.api.release['draft'])
        self.api.bad_digest = False
        release.publish(self.directory, self.plan, self.api)
        self.assertFalse(self.api.release['draft'])

    def test_published_release_can_retry_refresh_but_cannot_replace_assets(self):
        self.both()
        release.publish(self.directory, self.plan, self.api)
        self.api.calls.clear()
        release.publish(self.directory, self.plan, self.api)
        self.assertEqual([c[0] for c in self.api.calls if c[0] != 'GET'], ['POST'])
        self.api.assets[0]['digest'] = 'sha256:'+'0'*64
        self.api.calls.clear()
        with self.assertRaisesRegex(ValueError, 'immutable'):
            release.publish(self.directory, self.plan, self.api)
        self.assertTrue(all(c[0] == 'GET' for c in self.api.calls))

    def test_existing_unowned_release_or_wrong_tag_is_not_modified(self):
        self.both()
        self.api.tag = 'b'*40
        with self.assertRaisesRegex(ValueError, 'different commit'):
            release.publish(self.directory, self.plan, self.api)
        self.api.tag = SHA
        self.api.release = {'body': 'Maintainer release', 'target_commitish': SHA}
        with self.assertRaisesRegex(ValueError, 'not be overwritten'):
            release.publish(self.directory, self.plan, self.api)
        self.assertTrue(all(c[0] == 'GET' for c in self.api.calls))

    def test_older_stable_cannot_replace_latest(self):
        self.both()
        self.api.latest = {'tag_name': 'launcher-v1.2.20'}
        with self.assertRaisesRegex(ValueError, 'newer'):
            release.publish(self.directory, self.plan, self.api)
        self.assertTrue(all(c[0] == 'GET' for c in self.api.calls))

    def test_preview_does_not_replace_latest(self):
        self.plan.update(version='1.2.19-rc.1', tag='launcher-v1.2.19-rc.1', prerelease=True)
        self.version.return_value = self.plan['version']
        self.both()
        release.publish(self.directory, self.plan, self.api)
        self.assertTrue(self.api.release['prerelease'])
        self.assertEqual(self.api.release['make_latest'], 'false')

    def test_validation_only_is_read_only(self):
        self.plan['publish'] = False
        with self.assertRaisesRegex(ValueError, 'validation-only'):
            release.publish(self.directory, self.plan, self.api)
        self.assertEqual(self.api.calls, [])

    @patch.object(release.subprocess, 'run')
    def test_publish_event_policy_checks_main_ancestry_and_matching_tag(self, run):
        event = {'repository': {'default_branch': 'main'}, 'inputs': {'publish': 'true'}}
        for kind, ref in [('push', 'refs/tags/launcher-v1.2.19'), ('workflow_dispatch', 'refs/heads/main')]:
            with self.subTest(kind=kind):
                plan = release.plan_release(event, kind, ref, SHA, release.REPOSITORY)
                self.assertTrue(plan['publish'])
                self.assertEqual(run.call_args.args[0], ['git', 'merge-base', '--is-ancestor', SHA, 'FETCH_HEAD'])
        for kind, ref, repository in [('push', 'refs/tags/launcher-v9.9.9', release.REPOSITORY),
                                     ('workflow_dispatch', 'refs/heads/topic', release.REPOSITORY),
                                     ('push', 'refs/tags/launcher-v1.2.19', 'fork/repository')]:
            with self.subTest(ref=ref, repository=repository), self.assertRaises(ValueError):
                release.plan_release(event, kind, ref, SHA, repository)
        self.assertFalse(release.plan_release(event, 'pull_request', 'refs/pull/1/merge', SHA, release.REPOSITORY)['publish'])
        event['inputs']['publish'] = 'false'
        self.assertFalse(release.plan_release(event, 'workflow_dispatch', 'refs/heads/topic', SHA, release.REPOSITORY)['publish'])

    def test_workflow_keeps_write_token_after_both_host_builds_and_verification(self):
        workflow = (ROOT/'.github/workflows/launcher-release.yml').read_text()
        build, publish = workflow.split('\n  publish:\n')
        self.assertNotIn(': write', build)
        self.assertNotIn('secrets.GITHUB_TOKEN', build)
        self.assertIn('needs: [windows, mac, verify]', publish)
        self.assertIn("github.event_name == 'push' || github.event_name == 'workflow_dispatch'", publish)
        self.assertIn('cancel-in-progress: false', workflow)
        self.assertIn('persist-credentials: false', publish)
        self.assertIn('--verify-package', build)
        self.assertIn('-Test -RequireArtwork', build)
        self.assertIn('--install-sdk --test --require-artwork', build)
        self.assertIn("tags: ['launcher-v*']", workflow)
        self.assertIn('default: false', workflow)


if __name__ == '__main__':
    unittest.main()
