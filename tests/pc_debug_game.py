"""Exercise Debug settings in the real game using isolated settings and saves."""
import ctypes as c
import base64
from ctypes import wintypes as w
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import time

import pc_settings_game as ui

ROOT = Path(__file__).resolve().parents[1]
user = ui.user
user.SetForegroundWindow.argtypes = [w.HWND]
user.GetSubMenu.argtypes = [w.HMENU, c.c_int]
user.GetSubMenu.restype = w.HMENU
user.GetMenuState.argtypes = [w.HMENU, w.UINT, w.UINT]


def text(window):
    buffer = c.create_unicode_buffer(8192)
    user.SendMessageW(window, 0x000D, len(buffer), c.cast(buffer, c.c_void_p).value)
    return buffer.value


def wait_for(condition, description, seconds=10):
    until = time.monotonic() + seconds
    while time.monotonic() < until:
        if condition():
            return
        time.sleep(.1)
    raise AssertionError(description)


def screenshot(window, path):
    # Capture only this window, even when another application covers it.
    script = r'''
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition 'using System; using System.Runtime.InteropServices; public class DebugCapture { [StructLayout(LayoutKind.Sequential)] public struct Rect { public int L,T,R,B; } [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out Rect r); [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint flags); [DllImport("user32.dll")] public static extern bool SetProcessDPIAware(); }'
[DebugCapture]::SetProcessDPIAware() | Out-Null
$rect = New-Object DebugCapture+Rect
[DebugCapture]::GetWindowRect([IntPtr]WINDOW_HANDLE, [ref]$rect) | Out-Null
$bitmap = New-Object Drawing.Bitmap ($rect.R-$rect.L),($rect.B-$rect.T)
$graphics = [Drawing.Graphics]::FromImage($bitmap)
try {
    $dc = $graphics.GetHdc()
    try { if (![DebugCapture]::PrintWindow([IntPtr]WINDOW_HANDLE,$dc,2)) { throw 'Window capture failed' } }
    finally { $graphics.ReleaseHdc($dc) }
    $bitmap.Save('OUTPUT_PATH',[Drawing.Imaging.ImageFormat]::Png)
}
finally { $graphics.Dispose(); $bitmap.Dispose() }
'''.replace('WINDOW_HANDLE', str(window)).replace('OUTPUT_PATH', str(path).replace("'", "''"))
    encoded = base64.b64encode(script.encode('utf-16-le')).decode('ascii')
    result = subprocess.run(['powershell.exe', '-NoProfile', '-EncodedCommand', encoded],
                            creationflags=subprocess.CREATE_NO_WINDOW, capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr


def main():
    tag = str(time.time_ns())
    work = ROOT / 'private/captures/debug-settings' / tag
    work.mkdir(parents=True)
    config = work / 'controls.json'
    config.write_bytes((ROOT / 'config/pc-controls.default.json').read_bytes())
    original = json.loads(config.read_text())
    error_log = work / 'runtime.err'
    save_directory = ROOT / 'private/test-saves/debug-settings' / tag
    environment = {key: value for key, value in os.environ.items()
                   if not key.startswith(('RECOMP_', 'TSFP_'))}
    environment.update(RECOMP_GAME_DIR=str(ROOT / 'private/game'),
                       RECOMP_SAVE_DIR=str(save_directory),
                       RECOMP_INPUT_CONFIG=str(config), RECOMP_LOG_FILE=str(error_log),
                       RECOMP_TRACE_BUDGET='0', RECOMP_PC_MOUSE='1')
    exe = ROOT / 'build/windows/Release/futureperfect_recomp.exe'
    with (work / 'runtime.out').open('wb') as output, error_log.open('wb') as errors:
        process = subprocess.Popen([str(exe)], cwd=exe.parent, env=environment,
                                   stdout=output, stderr=errors,
                                   creationflags=subprocess.CREATE_NO_WINDOW)
        try:
            game = ui.wait_window(process.pid, 'TimeSplitters FP')
            time.sleep(2)
            user.PostMessageW(game, 0x111, 0x7e21, 0)
            debug = ui.wait_window(process.pid, 'Debug -')
            assert not user.IsWindowEnabled(game)
            wait_for(lambda: re.search(r'Game: [1-9][0-9]*\.', text(user.GetDlgItem(debug, 204))),
                     'No live FPS statistics')
            assert 'unavailable' not in text(user.GetDlgItem(debug, 205)).lower()
            for item in range(200, 204):
                user.SendMessageW(user.GetDlgItem(debug, item), 0xF1, 1, 0)
            user.SetForegroundWindow(debug)
            time.sleep(.3)
            screenshot(debug, work / 'debug-dialog.png')
            user.PostMessageW(debug, 0x111, 1, 0)
            wait_for(lambda: user.IsWindowEnabled(game), 'Debug Save did not return game input')
            updated = json.loads(config.read_text())
            assert all(updated['debug'].values())
            assert updated['controllers'] == original['controllers']
            assert updated['mouse'] == original['mouse']
            wait_for(lambda: '[PC-DEBUG]' in error_log.read_text(errors='replace'),
                     'Performance logging did not apply live')
            user.SetForegroundWindow(game)
            time.sleep(.5)
            screenshot(game, work / 'debug-overlay.png')
            menu = user.GetSubMenu(user.GetMenu(game), 0)

            def fullscreen():
                return not (user.GetWindowLongW(game, -16) & 0x00C00000)

            def check_mode(wanted):
                wait_for(lambda: fullscreen() == wanted, 'Fullscreen mode did not change')
                assert bool(user.GetMenuState(menu, 0x7e22, 0) & 8) == wanted

            # Shortcuts are window messages, without synthesizing desktop keyboard input.
            for full in (False, True):
                if full:
                    user.PostMessageW(game, 0x111, 0x7e22, 0)
                    check_mode(True)
                    user.SendMessageW(game, 0x100, 0x7A, 1 << 30)  # held F11
                    time.sleep(.2)
                    assert fullscreen(), 'Held F11 toggled fullscreen again'
                    user.PostMessageW(game, 0x100, 0x7A, 0)  # F11 exits fullscreen
                    check_mode(False)
                    user.PostMessageW(game, 0x100, 0x7A, 0)  # F11 enters fullscreen
                    check_mode(True)
                user.PostMessageW(game, 0x100, 0x72, 0)  # F3 is no longer a shortcut
                user.PostMessageW(game, 0x101, 0x72, 0)
                time.sleep(.2)
                assert not any(title.startswith('Debug -') for _, title in ui.windows(process.pid))
                user.PostMessageW(game, 0x100, 0x7B, 0)  # F12
                debug = ui.wait_window(process.pid, 'Debug -')
                assert not user.IsWindowEnabled(game)
                user.PostMessageW(debug, 0x111, 2, 0)
                wait_for(lambda: user.IsWindowEnabled(game), 'F12 Debug did not release game input')

            # Keyboard & Mouse no longer contains a Debug button.
            user.PostMessageW(game, 0x100, 0x71, 0)
            keyboard = ui.wait_window(process.pid, 'Keyboard & Mouse -')
            assert not user.GetDlgItem(keyboard, 111), 'Removed Debug button is still present'
            user.PostMessageW(keyboard, 0x111, 2, 0)
            wait_for(lambda: user.IsWindowEnabled(game), 'F2 Cancel did not return game input')
            user.PostMessageW(game, 0x100, 0x7B, 0)
            debug = ui.wait_window(process.pid, 'Debug -')
            assert not user.IsWindowEnabled(game)
            before_cancel = config.read_bytes()
            for item in range(200, 204):
                assert user.SendMessageW(user.GetDlgItem(debug, item), 0xF0, 0, 0) == 1
                user.SendMessageW(user.GetDlgItem(debug, item), 0xF1, 0, 0)
            user.PostMessageW(debug, 0x111, 2, 0)
            wait_for(lambda: user.IsWindowEnabled(game), 'Debug Cancel did not return game input')
            assert config.read_bytes() == before_cancel
            user.PostMessageW(game, 0x111, 0x7e22, 0)
            check_mode(False)
            for full in (True, False):
                user.PostMessageW(game, 0x104, 0x0D, 1 << 29)  # Alt+Enter still works
                check_mode(full)
            assert process.poll() is None
            user.PostMessageW(game, 0x10, 0, 0)
            assert process.wait(timeout=10) == 0
        finally:
            if process.poll() is None:
                process.terminate()
                process.wait(timeout=5)
    assert '[CRASH]' not in error_log.read_text(errors='replace')
    if save_directory.is_dir():
        resolved = save_directory.resolve()
        allowed = (ROOT / 'private/test-saves/debug-settings').resolve()
        assert resolved != allowed and resolved.is_relative_to(allowed)
        shutil.rmtree(resolved)
    print('PASS: Debug menu/F12, released F3, removed F2 Debug button, live diagnostics, Save/Cancel, fullscreen shortcuts/checkmarks, and clean exit')
    print(f'Screenshots and isolated test artifacts: {work}')


if __name__ == '__main__':
    main()
