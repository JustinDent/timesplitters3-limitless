# TimeSplitters: Future Perfect PC port workspace

Target: the original Xbox US release, title ID `45410066`, XDK 5849.
This workspace uses static recompilation: translate the original x86 executable
into C and link it to a native Windows runtime. This does not recover the original
C/C++ source. A build is an experimental port until gameplay has been verified.

## Download and play

Download the Windows x64 ZIP from [Releases](https://github.com/TheDoffman/timesplitters-future-perfect-pc/releases).
Extract the entire ZIP to a writable folder, then double-click **Play.cmd**.
On first launch, select your US Xbox ISO or an extracted game folder containing
`default.xbe`, `pak`, and `xmv`. Setup verifies the game version and copies the
files once. Afterward, **Play.cmd** launches directly; the ISO is no longer needed.

The release includes the PC executable, movie libraries, and default controls.
It requires Windows 10/11 x64 and about 5 GB free for setup. Python, Visual Studio,
and an emulator are not required to play. Original game data is not bundled.
This is an experimental prerelease; a complete campaign playthrough is unverified.
Keep the release's `Data`, `Settings`, and `Saves` folders when upgrading.

To produce the same public package after building:

```powershell
.\.venv\Scripts\python.exe tools/package-release.py --version v0.1.0-alpha.1
```

This uses an explicit file list and default settings. It excludes all game data,
personal settings, and saves, and writes the ZIP plus SHA-256 under `build/releases`.

## Local workflow

This repository contains the PC input code, runtime patches, and build scripts.
Game data, translated game code, binaries, personal settings, and saves stay local.
The current build uses a separate F2 settings window; the original game menus
are unchanged. See [STATUS.md](STATUS.md) for tested behavior and current limits.

Run in PowerShell from this directory:

```powershell
.\tsfp.ps1 Status
.\tsfp.ps1 Test
.\tsfp.ps1 Generate
.\tsfp.ps1 Build
.\tsfp.ps1 SmokeTest -Seconds 30
.\tsfp.ps1 Run
```

`Run` starts `build/windows/Release/futureperfect_recomp.exe` with
`RECOMP_GAME_DIR` pointing to `private/game`. A smoke test captures output in
`logs` and ends the process after its time limit. Its report records crashes and
unresolved calls; a report command succeeding does not mean the game is playable.
Smoke tests use separate save folders in `private/test-saves` and capture frames
in `private/captures`. Interactive runs store saves in `private/saves`.

Double-click `Play.cmd` for the keyboard-and-mouse setup. Restart the game after
updating a build or editing the configuration file by hand. Changes saved from
the game's PC settings menu apply immediately.
`Play.cmd` closes its command window after starting a hidden launcher; game logs
continue to be written to `logs`, and launch failures appear in a dialog. The
portable release uses the same behavior after first-time game import. For terminal
diagnostics, use `tsfp.ps1 Run` (or `Launch.ps1` in a portable release) directly.

## PC controls

Press **F2**, or choose **PC Settings → Keyboard & Mouse** from the game
window's menu bar, to open the separate settings window. F2 also works in
fullscreen. Pause a mission before editing: the settings window blocks gameplay
input but does not itself pause the simulation.

Double-click a binding, or select an action and a binding slot and choose
**Change binding**. Press a key, or click a mouse button / scroll inside the
capture box. Each action supports four bindings. A duplicate-key prompt lets
you move a binding from another action. **Clear binding** removes one slot;
**Restore defaults** stages the default PC layout. Mouse sensitivity, inversion,
and mouse input can also be changed here. **Save & Close** saves and applies the
settings; **Cancel** discards edits. The previous configuration is backed up to
`config/pc-controls.json.bak`. F1, F2, F9–F12 and the Windows keys are reserved.

Default bindings:

| Action | Control |
| --- | --- |
| Move / strafe | W A S D |
| Look | Mouse |
| Fire | Left mouse button |
| Aim | Right mouse button |
| Use / confirm | E, Space, or Enter |
| Reload | R |
| Crouch | Ctrl or C |
| Melee / menu back | F, middle mouse, mouse button 4, or Backspace |
| Throw grenade | G or mouse button 5 |
| Change weapon | Mouse wheel or left/right arrows |
| Weapon mode / scope zoom in | Q or Up / Page Up |
| Change grenade / scope zoom out | V or Down / Page Down |
| Pause and release cursor | Escape |
| Capture / release cursor | F1 |
| Keyboard & mouse settings | F2 |
| Debug settings | F12 |
| Fullscreen | F11 or Alt+Enter |

**PC Settings > Fullscreen** also toggles fullscreen; its checkmark reflects the
current window mode. Holding F11 does not repeatedly toggle it.

Click the game window to recapture the mouse. The recapture click is consumed
so it does not fire a weapon. Alt-Tab releases the cursor and clears held
keyboard/mouse inputs. Aim and crouch activation follow the game's profile
settings. Space is an action key; the original game has no jump action.

The settings menu saves to `config/pc-controls.json`. Sensitivity is degrees per
raw mouse count (default `0.10`); lower it for
slower aiming. Mouse motion changes camera angles directly, scales down while
zoomed, and does not pass through the controller's stick deadzone or turn-speed
curve. WASD diagonals are normalized. The supplied configuration uses keyboard
input on player 1; it does not require a controller.

The Xbox action mapping was checked against the publisher's original
[Xbox manual, Complete Controls](https://www.gamesdatabase.org/Media/SYSTEM/Microsoft_Xbox//Manual/formated/Timesplitters_-_Future_Perfect_-_2005_-_Electronic_Arts.pdf).

### Debug settings

Press **F12** or choose **PC Settings > Debug**.
The shortcuts also work in fullscreen. Pause a mission first: settings block gameplay input but do not
pause the simulation. The Debug window offers:

- **Show FPS:** game frame rate; when interpolation adds presentations, the
  overlay shows game/shown FPS. F9 toggles the FPS display for the current session.
- **Show average frame time:** the average interval between game frames in
  milliseconds, measured over roughly half a second; this is not GPU execution time.
- **Show error / warning counters:** on-screen E/W counts of matching runtime log
  lines. The window lists the latest 12 matches and counts frames over 50 ms,
  including loading delays.
- **Write performance samples to log:** FPS and average frame interval, about
  twice per second. This is off by default to avoid growing logs unnecessarily.
- **Open logs folder** and **Clear counters / recent:** inspect the original
  logs or reset the session counters without deleting log files.

**Save & Close** persists the display/logging options under `debug` in the same
controls JSON file and applies them immediately. **Cancel** discards checkbox
edits; clearing counters takes effect immediately. All options default to off.
Existing controls files without a `debug` section remain supported.

Diagnostics match logged failures, unresolved calls, warnings, and unsupported
features; they do not detect every gameplay or visual anomaly. Counts cover the
current launch since startup or the last clear. Use `Play.cmd` or `tsfp.ps1 Run`
for live log monitoring; direct launches without `RECOMP_LOG_FILE` show
"Log unavailable". Log reading is bounded and runs outside the render thread.
The runtime's diagnostic frame capture uses **Ctrl+F11**; plain **F11** toggles
fullscreen.

For a fresh checkout, install Git, Visual Studio 2022 with its C++ workload and
Windows SDK, and Python 3.12+, then run:

```powershell
git clone https://github.com/TheDoffman/timesplitters-future-perfect-pc.git
cd timesplitters-future-perfect-pc
.\setup.ps1 -Python 'C:\path\to\python.exe' -Iso 'C:\path\to\TimeSplitters - Future Perfect (USA).xiso.iso'
```

If you already have this checkout, run only the `setup.ps1` command. The script
validates the input paths and Python version, then runs Setup, Extract, Test,
Generate, and Build in order, stopping at the first failure. It requires internet
access for dependency downloads; it does not install Git, Python, or Visual Studio.
The default output is `build/windows/Release/futureperfect_recomp.exe`.
After setup succeeds, double-click `Play.cmd` to play from the repository.
The extracted files in `private/game` must remain; the ISO is no longer needed.
Use `-Configuration Debug` or `-Configuration RelWithDebInfo` for another build
configuration, and pass the same configuration to `tsfp.ps1 Run` to launch it.

The equivalent individual steps are still available for troubleshooting:

```powershell
.\tsfp.ps1 Setup -Python 'C:\path\to\python.exe'
.\tsfp.ps1 Extract -Iso 'C:\path\to\TimeSplitters - Future Perfect (USA).xiso.iso'
.\tsfp.ps1 Test
.\tsfp.ps1 Generate
.\tsfp.ps1 Build
```

Setup installs dependencies into `.venv`, clones the pinned toolkit, initializes
SDL and XbSymbolDatabase, builds the symbol matcher, and fetches FFmpeg 8.1 for
intro movie playback. The FFmpeg download has a pinned SHA-256; if its mutable
upstream URL changes, setup stops for review. All tool downloads stay local.
The launcher copies `config/pc-controls.default.json` to the ignored personal
`config/pc-controls.json` on first use. Existing personal bindings are preserved.

## Inputs and generated files

The supported `default.xbe` is 6,270,976 bytes with SHA-256:

```text
3cfd001a84fc3e08175d6c4b2e42ebf49577a089b5d0bd87ac724f61a41816bc
```

- `private/game/`: extracted game files.
- `private/pipeline/`: disassembly, function classification and translation reports.
- `external/ts3FPrecomp/titles/futureperfect/src/recomp/gen/`: generated C.
- `build/windows/`: MSVC build and executable.
- `logs/`: validation, build and bounded startup logs.

The original ISO is read without modification. Original assets, executable,
generated game code, tool checkouts and binaries are excluded from the outer Git
repository. Supply your own game dump when reproducing this setup.

## Upstream and licensing

The Windows runtime and Future Perfect target come from
[nemonicicon/ts3FPrecomp](https://github.com/nemonicicon/ts3FPrecomp), pinned to
`d90c92988a9f99d0580d2ca3f7efd6ec3a7b7845`. Its checkout includes its GPL-3.0,
MIT, third-party license and attribution files; retain them when distributing
any relevant runtime code. No game data is supplied by this repository.

`patches/save-routing.patch` fixes the runtime's device paths for Xbox profile
storage: Partition1's TDATA and UDATA now use the configured save directory.
Setup and Build apply it idempotently. Build also runs the native
`tests/save_paths.c` regression test against the actual runtime.
After all native build tests pass, Build removes their temporary `build/path-tests`
data. Failed tests retain that directory for diagnosis; `private/saves` is unaffected.

`patches/pc-controls.patch` connects the window/input layer to `src/pc_controls.c`
and `src/pc_input.c`, and adds a manual wrapper around the US build's
`sub_000D2B80`. Its generated original body is retained as
`sub_000D2B80_gen`; the generated C is never hand-edited. Run Generate after
applying this patch to an older generated tree, then Build. The native
`tests/pc_controls.c` suite runs during Build.

`patches/pc-input-vocabulary.patch` keeps the Python input UI's key names in sync
with the mouse-button and F23/F24 names added to the runtime by the PC controls
patch. Setup, Test, and Build apply local patches idempotently.

`patches/pc-debug.patch` connects the renderer's frame statistics and F9 shortcut
to `src/pc_debug.c`. Build runs `tests/pc_debug.c` to check persistence, log
monitoring, live display options, and Debug dialog Save/Cancel behavior.
`tests/pc_debug_game.py` checks the real game's menu and fullscreen access using
isolated settings/saves and captures the dialog and overlay for review.
`patches/pc-window-shortcuts.patch` moves the runtime's frame capture to Ctrl+F11
so it does not conflict with the fullscreen shortcut.

The camera-field research also consulted the open-source
[MouseInjector TS3 driver](https://github.com/regilopes/MouseInjectorDolphinDuck/blob/master/games/ts3.c).
All hook addresses used here were checked against the supplied Xbox executable.

An independent [tsfp-recomp project](https://github.com/NeilSeligmann/tsfp-recomp)
supports the same executable hash, but its documented tested host is Linux.
Its status claims do not establish the behavior of our Windows build.

See `STATUS.md` for results measured in this workspace.
