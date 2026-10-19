"""Pinned, checkout-local Vulkan SDK for the native common-renderer adapter."""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import urllib.request
import zipfile

ROOT = Path(__file__).resolve().parents[1]
CRITICAL = (
    'VERSIONS.txt', 'macOS/include/dxc/dxcapi.h', 'macOS/include/dxc/WinAdapter.h',
    'macOS/lib/libdxcompiler.dylib', 'macOS/lib/libvulkan.1.dylib',
    'macOS/lib/libMoltenVK.dylib', 'macOS/lib/libvulkan_kosmickrisp.dylib',
    'macOS/lib/libVkLayer_khronos_validation.dylib',
    'macOS/share/vulkan/icd.d/MoltenVK_icd.json',
    'macOS/share/vulkan/icd.d/libkosmickrisp_icd.json',
    'macOS/share/vulkan/explicit_layer.d/VkLayer_khronos_validation.json',
)


def sha256(path):
    with Path(path).open('rb') as source:
        return hashlib.file_digest(source, 'sha256').hexdigest()


def pin(root=ROOT):
    return json.loads((root / 'config/vulkan-sdk.json').read_text())


def install_path(root=ROOT):
    return root / 'external/vulkan-sdk' / pin(root)['version']


def verify_archive(path, spec):
    if path.stat().st_size != spec['bytes'] or sha256(path) != spec['sha256']:
        raise RuntimeError('Vulkan SDK archive does not match the shared size/checksum pin')
    # ditto preserves the signed installer bundle. Check all names before extract.
    with zipfile.ZipFile(path) as archive:
        for name in archive.namelist():
            if name.startswith('/') or '..' in Path(name).parts:
                raise RuntimeError('Unsafe path in Vulkan SDK archive')


def record_install(root=ROOT):
    """Only called after the verified installer succeeds."""
    installed = install_path(root)
    files = {name: sha256(installed / name) for name in CRITICAL}
    receipt = dict(version=pin(root)['version'], archive_sha256=pin(root)['sha256'], files=files)
    (installed / 'tsfp-receipt.json').write_text(json.dumps(receipt, indent=2)+'\n')


def verify_install(root=ROOT):
    installed = install_path(root)
    if installed.is_symlink() or installed.parent.is_symlink():
        raise RuntimeError('Refusing symlinked SDK installation')
    receipt_path = installed / 'tsfp-receipt.json'
    if not receipt_path.is_file():
        raise RuntimeError('Pinned Vulkan SDK missing/unverified; run tools/test-rhi-mac.py --setup-sdk')
    receipt = json.loads(receipt_path.read_text())
    spec = pin(root)
    if receipt.get('version') != spec['version'] or receipt.get('archive_sha256') != spec['sha256']:
        raise RuntimeError('Vulkan SDK receipt differs from the shared pin')
    if set(receipt.get('files', {})) != set(CRITICAL):
        raise RuntimeError('Vulkan SDK receipt has an incomplete file set')
    for name, expected in receipt['files'].items():
        path = installed / name
        if not path.resolve().is_relative_to(installed.resolve()) or sha256(path) != expected:
            raise RuntimeError('Vulkan SDK installed file changed: '+name)
    if not (installed / 'Licenses').is_dir():
        raise RuntimeError('Vulkan SDK licenses missing')
    return installed / 'macOS'


def setup(root=ROOT):
    installed = install_path(root)
    if installed.exists():
        return verify_install(root)  # Never overwrite an unrecognized installation.
    if shutil.disk_usage(root).free < 5 * 1024**3:
        raise RuntimeError('Vulkan SDK setup needs 5 GiB free for installation and temporary download')
    spec = pin(root)
    download = root / 'external/vulkan-sdk/download'
    if download.is_symlink() or download.parent.is_symlink():
        raise RuntimeError('Refusing symlinked SDK download directory')
    download.mkdir(parents=True, exist_ok=True)
    archive = download / spec['archive']
    if archive.is_symlink():
        raise RuntimeError('Refusing symlinked SDK archive')
    if not archive.exists():
        partial = archive.with_suffix('.partial')
        if partial.is_symlink():
            raise RuntimeError('Refusing symlinked SDK partial download')
        # LunarG's CDN rejects urllib's generic Python client (403/1010).
        # Identify this downloader; retain the exact URL and archive integrity pins.
        request = urllib.request.Request(spec['url'], headers={'User-Agent': 'TSFP-SDK-Setup/1.0'})
        with urllib.request.urlopen(request, timeout=60) as response, partial.open('wb') as output:
            shutil.copyfileobj(response, output)
        partial.replace(archive)
    verify_archive(archive, spec)
    bundle = f'vulkansdk-macOS-{spec["version"]}.app'
    if (download / bundle).exists() or (download / bundle).is_symlink():
        raise RuntimeError('Existing SDK installer bundle preserved; inspect before retrying setup')
    subprocess.run(['ditto', '-x', '-k', str(archive), str(download)], check=True, timeout=120)
    installer = download / bundle / 'Contents/MacOS' / bundle[:-4]
    subprocess.run([str(installer), '--root', str(installed), '--accept-licenses',
                    '--default-answer', '--confirm-command', 'install',
                    'com.lunarg.vulkan.core', 'copy_only=1'], check=True, timeout=600)
    record_install(root)
    sdk = verify_install(root)
    # Only the verified installer and archive created by this setup are disposable.
    shutil.rmtree(download / bundle)
    archive.unlink()
    return sdk


def probe_environment(sdk, driver, root=ROOT):
    drivers = {'moltenvk': 'MoltenVK_icd.json', 'kosmickrisp': 'libkosmickrisp_icd.json'}
    if driver not in drivers:
        raise ValueError('Unknown pinned Vulkan driver')
    # No inherited user cache, driver override, game or config path can redirect this probe.
    env = {key: value for key, value in os.environ.items()
           if not key.startswith(('RECOMP_', 'TSFP_', 'VK_', 'SDL_VULKAN', 'DYLD_'))}
    isolated = root / 'build/macos/rhi-tests/runtime'
    if isolated.is_symlink() or not isolated.resolve().is_relative_to((root / 'build/macos').resolve()):
        raise RuntimeError('Unexpected RHI probe cache/config path')
    isolated.mkdir(parents=True, exist_ok=True)
    env.update(DYLD_LIBRARY_PATH=str(sdk / 'lib'),
               SDL_VULKAN_LIBRARY=str(sdk / 'lib/libvulkan.1.dylib'),
               VK_DRIVER_FILES=str(sdk / 'share/vulkan/icd.d' / drivers[driver]),
               VK_LAYER_PATH=str(sdk / 'share/vulkan/explicit_layer.d'),
               RECOMP_SHADER_CACHE='0',
               XDG_CACHE_HOME=str(isolated / 'cache'),
               XDG_CONFIG_HOME=str(isolated / 'config'), XDG_DATA_HOME=str(isolated / 'data'))
    return env


def with_probe_gpu_environment(existing, sdk, driver, root=ROOT):
    """Add pinned GPU loading without replacing a runner's isolated data paths."""
    env = {key: value for key, value in existing.items()
           if not key.startswith(('VK_', 'SDL_VULKAN', 'DYLD_'))}
    for key, value in probe_environment(sdk, driver, root).items():
        if key.startswith(('VK_', 'SDL_VULKAN', 'DYLD_')) or key=='RECOMP_SHADER_CACHE':
            env[key]=value
    return env


def require_clean_validation(output):
    """The runtime emits errors as [VKVAL], warnings as [VKVAL warning]."""
    if 'validation on' not in output:
        raise RuntimeError('Vulkan validation was not enabled')
    if '[VKVAL]' in output or '[VKVAL error]' in output:
        raise RuntimeError('Vulkan validation reported an error')
