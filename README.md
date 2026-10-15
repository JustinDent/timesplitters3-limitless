# TimeSplitters: Future Perfect PC port workspace

Target: the original Xbox US release, title ID `45410066`, XDK 5849.
This workspace uses static recompilation: translate the original x86 executable
into C and link it to a native Windows runtime. This does not recover the original
C/C++ source. A build is an experimental port until gameplay has been verified.

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
`config/pc-controls.json.bak`. F1, F2, F9–F11 and the Windows keys are reserved.

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
| Fullscreen | Alt+Enter |

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

For a fresh checkout, install Visual Studio 2022 with its C++ workload and Windows
SDK, supply a Python 3.12+ executable, and run:

```powershell
git clone https://github.com/TheDoffman/timesplitters-future-perfect-pc.git
cd timesplitters-future-perfect-pc
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

`patches/pc-controls.patch` connects the window/input layer to `src/pc_controls.c`
and `src/pc_input.c`, and adds a manual wrapper around the US build's
`sub_000D2B80`. Its generated original body is retained as
`sub_000D2B80_gen`; the generated C is never hand-edited. Run Generate after
applying this patch to an older generated tree, then Build. The native
`tests/pc_controls.c` suite runs during Build.

The camera-field research also consulted the open-source
[MouseInjector TS3 driver](https://github.com/regilopes/MouseInjectorDolphinDuck/blob/master/games/ts3.c).
All hook addresses used here were checked against the supplied Xbox executable.

An independent [tsfp-recomp project](https://github.com/NeilSeligmann/tsfp-recomp)
supports the same executable hash, but its documented tested host is Linux.
Its status claims do not establish the behavior of our Windows build.

See `STATUS.md` for results measured in this workspace.
