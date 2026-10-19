"""SDK pin, integrity and isolated probe environment; no download/install."""
from pathlib import Path
import sys
import zipfile
import pytest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
import vulkan_sdk as sdk


def root_fixture(tmp_path):
    (tmp_path/'config').mkdir()
    (tmp_path/'config/vulkan-sdk.json').write_bytes((ROOT/'config/vulkan-sdk.json').read_bytes())
    return tmp_path


def test_sdk_receipt_detects_changed_installed_file(tmp_path):
    root=root_fixture(tmp_path)
    installed=sdk.install_path(root)
    for name in sdk.CRITICAL:
        path=installed/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(name.encode())
    (installed/'Licenses').mkdir()
    sdk.record_install(root)
    assert sdk.verify_install(root)==installed/'macOS'
    (installed/'macOS/lib/libdxcompiler.dylib').write_bytes(b'changed')
    with pytest.raises(RuntimeError,match='installed file changed'):
        sdk.verify_install(root)


def test_unknown_existing_install_is_preserved(tmp_path):
    root=root_fixture(tmp_path);installed=sdk.install_path(root);installed.mkdir(parents=True)
    keep=installed/'keep';keep.write_bytes(b'preserve')
    with pytest.raises(RuntimeError,match='missing/unverified'):
        sdk.setup(root)
    assert keep.read_bytes()==b'preserve'


@pytest.mark.parametrize('member',['safe/data','../escape','/absolute'])
def test_archive_pin_and_paths(tmp_path,member):
    archive=tmp_path/'sdk.zip'
    with zipfile.ZipFile(archive,'w') as z:z.writestr(member,b'data')
    spec=dict(bytes=archive.stat().st_size,sha256=sdk.sha256(archive))
    if member=='safe/data':sdk.verify_archive(archive,spec)
    else:
        with pytest.raises(RuntimeError,match='Unsafe path'):sdk.verify_archive(archive,spec)
    spec['sha256']='0'*64
    with pytest.raises(RuntimeError,match='checksum pin'):sdk.verify_archive(archive,spec)


def test_probe_environment_is_local_and_pinned(tmp_path,monkeypatch):
    for key in ('VK_DRIVER_FILES','VK_LAYER_PATH','RECOMP_SAVE_DIR','TSFP_DISPLAY_CONFIG',
                'DYLD_INSERT_LIBRARIES','SDL_VULKAN_LIBRARY'):
        monkeypatch.setenv(key,'/personal/override')
    env=sdk.probe_environment(tmp_path/'sdk','moltenvk',tmp_path)
    assert env['VK_DRIVER_FILES']==str(tmp_path/'sdk/share/vulkan/icd.d/MoltenVK_icd.json')
    assert env['SDL_VULKAN_LIBRARY']==str(tmp_path/'sdk/lib/libvulkan.1.dylib')
    assert 'RECOMP_SAVE_DIR' not in env and 'TSFP_DISPLAY_CONFIG' not in env
    assert 'DYLD_INSERT_LIBRARIES' not in env
    assert env['RECOMP_SHADER_CACHE']=='0'
    assert Path(env['XDG_CONFIG_HOME']).is_relative_to(tmp_path/'build/macos/rhi-tests')
    with pytest.raises(ValueError):sdk.probe_environment(tmp_path/'sdk','unknown',tmp_path)
