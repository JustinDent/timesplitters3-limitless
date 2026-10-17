"""Exercise real runtime patches on small source-only checkout fixtures."""
from pathlib import Path
import importlib.util
import io
import shutil
import subprocess
import sys
import tarfile

import pytest

ROOT = Path(__file__).resolve().parents[1]
PATCHES = ROOT / 'patches'
RUNTIME = ROOT / 'external/ts3FPrecomp'
spec = importlib.util.spec_from_file_location('apply_patches', ROOT / 'tools/apply-patches.py')
installer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(installer)


@pytest.fixture(scope='session')
def upstream_files():
    # Read pristine bytes once, rather than starting git show for each file in
    # every test. Never reuse a mutable checkout between transition scenarios.
    names = {line[6:] for patch in PATCHES.rglob('*.patch')
             for line in patch.read_text(encoding='utf-8').splitlines()
             if line.startswith('+++ b/')}
    archive = subprocess.check_output(['git', '-C', str(RUNTIME), 'archive', 'HEAD', '--', *sorted(names)])
    with tarfile.open(fileobj=io.BytesIO(archive)) as contents:
        files = {item.name: contents.extractfile(item).read() for item in contents if item.isfile()}
    assert files.keys() == names, 'Upstream archive must include every patched source file'
    return files


@pytest.fixture
def checkout(tmp_path, upstream_files):
    subprocess.run(['git', 'init', '-q', str(tmp_path)], check=True)
    for name, content in upstream_files.items():
        destination = tmp_path / name
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(content)
    return tmp_path


def apply(tree, patch):
    result = installer.git(tree, 'apply', PATCHES / patch)
    assert result.returncode == 0, result.stderr


def snapshot(tree):
    return {p.relative_to(tree): p.read_bytes() for folder in ('src', 'titles', 'tools')
            for p in (tree / folder).rglob('*') if p.is_file()}


def test_fresh_checkout_and_repeated_build(checkout):
    assert installer.apply_patches(checkout, PATCHES)
    complete = snapshot(checkout)
    assert installer.apply_patches(checkout, PATCHES) == []
    assert snapshot(checkout) == complete


def test_upgrade_previous_controls_with_newer_input_features(checkout):
    # Historical state: controls/debug/prompts applied before display/textures
    # and the later Mac bring-up patch. Upgrade it with the complete stack.
    for patch in sorted(PATCHES.glob('*.patch')):
        if patch.name in ('zzzzzzzzzz-decal-projection.patch',
                          'zzzzzzzzzzzzzzzzzzzzzzz-arcade-rank-anchor.patch',
                          'zzzzzzzzzzz-model-projection.patch',
                          'zzzzzzzzzzzzzzzzz-channel-clears.patch',
                          'zzzzzzzzzzzzzzzzzz-ui-layout.patch',
                          'zzzzzzzzzzzzzzzzzz-ui-narrow.patch',
                          'zzzzzzzzzzzzzzzzzzz-tutorial-layout.patch',
                          'zzzzzzzzzzzzzzzzzzzz-fullscreen-effects.patch',
                          'zzzzzzzzzzzzzzzzzzzzz-story-caption.patch',
                          'zzzzzzzzzzzzzzzzzzzzzz-objectives-ui-integration.patch'):
            continue  # Requires the display wrapper, absent in this historical state.
        if patch.name not in ('pc-display.patch', 'texture-pack.patch', 'zz-mac-native-boot.patch', 'zzz-mac-renderer.patch', 'zzzz-mac-input.patch', 'zzzzz-mac-high-refresh.patch', 'zzzzzz-interpolation-layouts.patch', 'zzzzzzz-framebuffer-sampling-passes.patch', 'zzzzzzz-shared-input-model.patch', 'zzzzzzzz-mac-feature-adapters.patch', 'zzzzzzzzz-shared-screen-space.patch', 'zzzzzzzzzz-native-settings-init.patch', 'zzzzzzzzzzz-shared-linear-formats.patch', 'zzzzzzzzzzzz-native-prompts.patch', 'zzzzzzzzzzzzz-native-camera.patch', 'zzzzzzzzzzzzzz-native-debug.patch', 'zzzzzzzzzzzzzzz-cutscene-letterbox.patch', 'zzzzzzzzzzzzzzzzz-terminal-mouse-select.patch', 'zzzzzzzzzzzzzzzzz-story-objectives-health.patch'):
            apply(checkout, patch.name)
    manual = checkout / 'titles/futureperfect/src/recomp_manual.c'
    manual.write_bytes(manual.read_bytes() + b'\r\n/* unrelated local edit */\r\n')
    assert installer.apply_patches(checkout, PATCHES)
    assert b'pc_display_camera' in manual.read_bytes()
    assert b'pc_prompt_guest' in manual.read_bytes()
    assert manual.read_bytes().endswith(b'/* unrelated local edit */\r\n')
    assert installer.apply_patches(checkout, PATCHES) == []


def test_already_integrated_controls_and_display(checkout):
    # Same result as the previously combined pc-controls.patch from main.
    apply(checkout, 'pc-controls.patch')
    apply(checkout, 'pc-display.patch')
    assert installer.apply_patches(checkout, PATCHES)
    assert installer.apply_patches(checkout, PATCHES) == []


def test_upgrade_existing_mac_stack_with_networking(checkout, tmp_path):
    old_patches = tmp_path / 'mac-only-patches'
    old_patches.mkdir()
    for patch in PATCHES.glob('*.patch'):
        if patch.name not in ('zz-pc-network.patch', 'zzz-pc-online.patch',
                              'zzzzzzzzzzzzzzz-pc-network-movement.patch',
                                  'zzzzzzzzzzzzzzzz-pc-network-state-refresh.patch',
                                  'zzzzzzzzzzzzzzzzzzzzzz-pc-network-bots-test.patch',
                                  'zzzzzzzzzzzzzzzzzzzzzzz-pc-network-bot-combat.patch',
                                  'zzzzzzzzzzzzzzzzzzzzzzzz-pc-network-bot-scoreboard.patch',
                                  'zzzzzzzzzzzzzzzzzzzzzzzzz-pc-network-bot-shots.patch',
                                  'zzzzzzzzzzzzzzzzzzzzzzzzzz-pc-network-bot-bounds.patch',
                                  'zzzzzzzzzzzzzzzzzzzzzzzzzzz-pc-network-bot-authority.patch',
                                  'zzzzzzzzzzzzzzzzzzzzzzzzzzzz-pc-network-bot-presentation.patch'):
            shutil.copyfile(patch, old_patches / patch.name)
    assert installer.apply_patches(checkout, old_patches)
    assert installer.apply_patches(checkout, PATCHES)
    cmake = (checkout / 'titles/futureperfect/CMakeLists.txt').read_text()
    assert 'TSFP_NATIVE_DIR' in cmake and 'pc_network.c' in cmake
    assert installer.apply_patches(checkout, PATCHES) == []


def texture_branch_patches(tmp_path):
    patches = tmp_path / 'texture-patches'
    patches.mkdir()
    for patch in PATCHES.glob('*.patch'):
        shutil.copyfile(patch, patches / patch.name)
    shutil.copyfile(PATCHES / 'legacy/texture-toggle.patch', patches / 'texture-toggle.patch')
    return patches


@pytest.mark.parametrize('crlf', [False, True])
def test_switch_from_texture_branch_to_main_and_back(checkout, tmp_path, crlf):
    texture_patches = texture_branch_patches(tmp_path)
    installer.apply_patches(checkout, texture_patches)
    manual = checkout / 'titles/futureperfect/src/recomp_manual.c'
    manual.write_bytes(manual.read_bytes() + b'\n/* unrelated local edit */\n')
    if crlf:
        for name, content in snapshot(checkout).items():
            (checkout / name).write_bytes(content.replace(b'\r\n', b'\n').replace(b'\n', b'\r\n'))
    assert installer.apply_patches(checkout, PATCHES)
    cmake = (checkout / 'src/input/CMakeLists.txt').read_text()
    assert 'texture_mode.c' not in cmake
    assert 'pc_controls.c' in cmake
    assert 'pc_network.c' in (checkout / 'titles/futureperfect/CMakeLists.txt').read_text()
    assert 'tsfp_texture_sync_bindings' not in (checkout / 'src/d3d/d3d8_device.c').read_text()
    assert 'unrelated local edit' in manual.read_text()
    complete = snapshot(checkout)
    assert installer.apply_patches(checkout, PATCHES) == []
    assert snapshot(checkout) == complete
    # The same legacy patch can become active again on the texture branch.
    shutil.copytree(PATCHES / 'legacy', texture_patches / 'legacy')
    assert installer.apply_patches(checkout, texture_patches)
    assert 'texture_mode.c' in (checkout / 'src/input/CMakeLists.txt').read_text()
    assert installer.apply_patches(checkout, texture_patches) == []
    assert installer.apply_patches(checkout, PATCHES)
    assert snapshot(checkout) == complete


def test_modified_legacy_patch_rejects_switch_without_writes(checkout, tmp_path):
    installer.apply_patches(checkout, texture_branch_patches(tmp_path))
    source = checkout / 'src/d3d/d3d8_device.c'
    source.write_bytes(source.read_bytes().replace(b'tsfp_texture_is_4k()',
                                                 b'custom_texture_is_4k()'))
    before = snapshot(checkout)
    with pytest.raises(RuntimeError, match='conflicts with the runtime checkout'):
        installer.apply_patches(checkout, PATCHES)
    assert snapshot(checkout) == before


def test_native_boot_source_conflict_preserves_all_files(checkout):
    installer.apply_patches(checkout, PATCHES)
    cmake = checkout / 'titles/futureperfect/CMakeLists.txt'
    content = cmake.read_bytes()
    assert b'list(APPEND GAME_SOURCES src/main.c)' in content
    cmake.write_bytes(content.replace(b'list(APPEND GAME_SOURCES src/main.c)',
                                     b'list(APPEND GAME_SOURCES custom_main.c)'))
    before = snapshot(checkout)
    with pytest.raises(RuntimeError, match='conflicts with the runtime checkout'):
        installer.apply_patches(checkout, PATCHES)
    assert snapshot(checkout) == before


def test_conflict_preserves_every_runtime_file(checkout):
    apply(checkout, 'pc-controls.patch')
    target = checkout / 'src/input/input_bindings.c'
    target.write_bytes(target.read_bytes().replace(
        b'pc_input_normalize_movement', b'custom_normalize_movement'))
    before = snapshot(checkout)
    with pytest.raises(RuntimeError, match='pc-controls.patch conflicts'):
        installer.apply_patches(checkout, PATCHES)
    assert snapshot(checkout) == before


def test_windows_line_endings(checkout):
    apply(checkout, 'pc-controls.patch')
    for name, content in snapshot(checkout).items():
        (checkout / name).write_bytes(content.replace(b'\r\n', b'\n').replace(b'\n', b'\r\n'))
    assert installer.apply_patches(checkout, PATCHES)
    assert installer.apply_patches(checkout, PATCHES) == []


@pytest.mark.parametrize('already_applied', [False, True])
def test_watermark_beside_native_overlay_guard(tmp_path, already_applied):
    tree, patches = tmp_path / 'runtime', tmp_path / 'patches'
    target = tree / 'src/hle/hle_d3d8.c'
    target.parent.mkdir(parents=True)
    patches.mkdir()
    installer.git(tree, 'init', '-q')
    original = '/* overlay */\nstatic int shown;\nstatic char line[96];\nvoid redraw(void);\n'
    target.write_text(original, encoding='utf-8')
    (patches / 'zz-mac-native-boot.patch').write_text(
        '--- a/src/hle/hle_d3d8.c\n+++ b/src/hle/hle_d3d8.c\n'
        '@@ -1,4 +1,5 @@\n /* overlay */\n+#ifdef _WIN32\n'
        ' static int shown;\n static char line[96];\n void redraw(void);\n', encoding='utf-8')
    watermark = patches / 'zzzz-pc-debug-latency-watermark.patch'
    watermark.write_text(
        '--- a/src/hle/hle_d3d8.c\n+++ b/src/hle/hle_d3d8.c\n'
        '@@ -1,4 +1,4 @@\n /* overlay */\n static int shown;\n'
        '-static char line[96];\n+static char line[192];\n void redraw(void);\n', encoding='utf-8')
    if already_applied:
        assert installer.git(tree, 'apply', watermark).returncode == 0
    assert installer.apply_patches(tree, patches)
    complete = target.read_bytes()
    assert b'#ifdef _WIN32' in complete and b'line[192]' in complete
    assert installer.apply_patches(tree, patches) == []
    # Reduced context must still reject changes to the actual patched code.
    target.write_bytes(complete.replace(b'line[192]', b'line[256]'))
    before = target.read_bytes()
    with pytest.raises(RuntimeError, match='conflicts with the runtime checkout'):
        installer.apply_patches(tree, patches)
    assert target.read_bytes() == before


@pytest.mark.skipif(sys.platform != 'win32', reason='Windows PowerShell stderr behavior')
@pytest.mark.parametrize('exit_code', [0, 7])
def test_checked_command_uses_exit_status_with_redirected_stderr(exit_code, tmp_path):
    def quote(value):
        return "'" + str(value).replace("'", "''") + "'"

    native = tmp_path / 'native warning.py'
    native.write_text('import sys\nsys.stderr.write("warning\\n")\nsys.exit(int(sys.argv[1]))\n',
                      encoding='utf-8')
    script = f"""
$ErrorActionPreference = 'Stop'
$ast = [System.Management.Automation.Language.Parser]::ParseFile(
    {quote(ROOT / 'tsfp.ps1')}, [ref]$null, [ref]$null)
$function = $ast.Find({{ param($node)
    $node -is [System.Management.Automation.Language.FunctionDefinitionAst] -and
    $node.Name -eq 'Invoke-Checked'
}}, $true)
. ([scriptblock]::Create($function.Extent.Text))
try {{
    $output = Invoke-Checked {quote(sys.executable)} @({quote(native)}, '{exit_code}') *>&1
    if ({exit_code} -ne 0) {{ throw 'Nonzero exit was accepted' }}
    if ($ErrorActionPreference -ne 'Stop') {{ throw 'Preference was not restored' }}
    if (($output | Out-String) -notmatch 'warning') {{ throw 'Warning was lost' }}
}} catch {{
    if ({exit_code} -eq 0 -or $_.Exception.Message -notmatch 'failed \\(exit {exit_code}\\)') {{
        throw
    }}
    if ($ErrorActionPreference -ne 'Stop') {{ throw 'Preference was not restored' }}
}}
exit 0
"""
    result = subprocess.run(['powershell.exe', '-NoProfile', '-Command', script],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr
