# TimeSplitters: Future Perfect — PC Port

**The Xbox classic, rebuilt for native PC play.** Direct mouse aiming,
customizable controls, in-game resolution settings, ultrawide support, and
experimental Online multiplayer—with a native Apple Silicon port in development.

[Download the Windows launcher](https://github.com/TheDoffman/timesplitters-future-perfect-pc/releases/tag/launcher-v1.1.2)
· [Multiplayer setup](docs/multiplayer.md)
· [Build from source](docs/building.md)
· [Report a bug](https://github.com/TheDoffman/timesplitters-future-perfect-pc/issues)

This community project statically recompiles the original Xbox executable into
native code. You supply the supported **US Xbox game data**; no emulator is
required to run the Windows release. Original game files are not included.

> **Experimental prerelease.** Windows gameplay has been tested, but a complete
> campaign playthrough and full rendering fidelity remain unverified. Features
> below describe current `main` and the published Windows launcher. Check the
> launcher's **Release notes** for what is included in your installed game version.

## What the port adds

| Feature | What it means for players |
| --- | --- |
| **Direct mouse look** | Raw mouse motion drives the camera, with zoom-aware sensitivity and normalized WASD movement. |
| **Customizable controls** | Rebind keyboard and mouse actions, set sensitivity and inversion, and use up to four bindings per action. |
| **PC button prompts** | Menu and HUD hints follow your bindings and switch back to controller icons when you use a controller. |
| **1080p, 1440p and 4K** | Choose the native rendering resolution from the game's own Settings or paused Options menu. |
| **Ultrawide support** | Monitor-reported resolutions, a wider gameplay view, proportionate menus and HUD, and adaptive in-engine cutscene framing. |
| **Online and LAN multiplayer** | Player-hosted competitive matches, invitations, private join codes and a compatible-build lobby browser. Online is experimental. |
| **Windows launcher** | Install published game releases, import your ISO once, check for updates and launch with **Play Game**. Updates preserve your saves and settings. |
| **Performance tools** | Optional FPS, frame-time and network-latency displays, plus debug information for troubleshooting. |
| **Texture replacement support** | An optional local preview path for the ongoing texture overhaul, with a 6 GiB pack budget. |
| **Native Apple Silicon work** | An ARM64 build with mouse look, native settings and an experimental up-to-120 FPS presentation mode. |

Recent fixes on `main` also improve pickup-range checks, controller sensitivity
adjustment, story audio transitions, Khallos Express scenery rendering, and
multiplayer movement updates. See [measured progress](STATUS.md) for the evidence
and remaining validation.

## Download the launcher and play

The **TimeSplitters Launcher** is the main entry point for Windows: install the
PC runtime, import your game and start playing from one window.

**Requirements:** Windows 10/11 x64 with .NET Framework 4.8, your supported US
Xbox ISO or extracted game data, and internet access for the initial runtime
download. Allow about 5 GB for the game plus extra space for update staging and
rollback. Python, Visual Studio and Git are not required.

1. Download **TimeSplitters-Launcher-v1.1.2-windows-x64.zip** from the
   [launcher release](https://github.com/TheDoffman/timesplitters-future-perfect-pc/releases/tag/launcher-v1.1.2).
2. Extract the whole launcher ZIP to a writable installation folder and open
   **TimeSplitters Launcher.exe**. Keep its `Bin` and `Support` subfolders together.
3. Set **Update channel** to **Preview**, then select **Install release** when
   the game package appears. Current game releases are prereleases; **Stable**
   excludes them. The launcher downloads and verifies the published runtime.
4. Select **Locate ISO** and choose your US Xbox disc image. Alternatively,
   select **Use extracted files** for a folder containing `default.xbe`, `pak`
   and `xmv`. This is a one-time import; the launcher checks the game version.
5. Select **Play Game**. The launcher minimizes once the game window appears
   and returns when the game closes.

For future sessions, open **TimeSplitters Launcher.exe → Play Game**. Once the
runtime and game data are installed, the ISO is no longer needed for play and
installed play remains available offline.

### Updates, settings and saves

- **Check updates** looks for published game releases in your selected channel.
  Select **Update release** to install one after closing the game. Launcher
  updates are checked separately at startup.
- **Restore previous** returns to the retained game release after an update,
  when a rollback is available. Game updates and rollback preserve imported
  data, saves and existing settings.
- **Saves** opens your save folder; **Game files** opens the installation;
  **View logs** and **Support** help with troubleshooting.
- To customize play, launch the game and use **F2** for keyboard/mouse settings
  or the in-game **Display settings** menu for resolution. These settings
  remain available the next time you start through the launcher.

Keep the complete installation together, including `Data`, `Settings`, `Saves`
and the launcher's recovery files. You do not need to manually replace runtime
files for normal updates. See the [launcher guide](docs/launcher.md) for channels,
recovery and older portable-package instructions.

Only the supported US Xbox revision is accepted: title ID `45410066`.
The exact executable hash is in the [build guide](docs/building.md#inputs-and-generated-files).

## Make it feel like a PC game

Click inside the game to capture the mouse. **Escape** pauses and releases it;
click again after resuming. The capture click is consumed so it does not fire.

| Action | Default Windows control |
| --- | --- |
| Move / look | W A S D / mouse |
| Fire / aim | Left / right mouse button |
| Use / confirm | E, Space or Enter |
| Reload | R |
| Crouch | Ctrl or C |
| Melee / menu back | F, middle mouse or Backspace |
| Throw grenade | G or mouse button 5 |
| Change weapon | Mouse wheel or left/right arrows |
| Weapon mode / scope zoom in | Q or Up / Page Up |
| Change grenade / scope zoom out | V or Down / Page Down |
| Keyboard and mouse settings | F2 |
| Fullscreen | Alt+Enter |
| Show / hide the PC menu bar | F1 |
| FPS overlay / debug settings | F9 / F12 |

Press **F2** to change bindings, sensitivity or inversion, then **Save & Close**
to apply them. This is a separate PC settings window; pause before opening it,
as it blocks game input without pausing the simulation. The game has no jump
action—Space is a confirm/action binding. Aim and crouch behavior follows your
in-game profile settings.

Formatted in-game prompts update with your bindings. Controller diagrams and
text baked into artwork retain their original appearance. For all bindings,
controller behavior and troubleshooting, see the [Windows settings guide](docs/windows-settings.md).

### Set your resolution in the game

Open **Settings → Display settings**, or **Options → Display settings** while
paused. Choose a mode, select **Apply resolution**, then **Keep resolution**
within 15 seconds. Letting the timer expire restores the previous mode.

The picker includes **1920×1080, 2560×1440 and 3840×2160**, plus supported
monitor-reported modes, including ultrawide. It changes the 3D rendering
resolution without changing your desktop resolution or refresh rate. Use
**Alt+Enter** for fullscreen.

Gameplay widens to the selected aspect ratio. Menus retain their original
proportions, and in-engine cutscene bars and camera framing adapt together.
Startup videos retain their existing framing. Supported modes are bounded at
7680 pixels wide and 2160 high; not every monitor mode is supported.

## Play with friends

Choose **Online / System Link → Online** for the experimental player-hosted
competitive mode. Automatic hosting starts a directory and creates an invitation
to share; friends can join by invitation or browse compatible public matches.
No Steam account, paid service or centrally operated server is required.

The host must forward **TCP 30820** and **UDP 30821** and allow them through the
firewall. This is not automatic router setup. Everyone needs a matching build
and game data. For local networks, choose **System Link**.

[Follow the multiplayer guide](docs/multiplayer.md) for hosting, joining, LAN
setup and connection limits. Local two-instance matches have reached gameplay;
real internet routing and sustained cross-machine matches still need validation.
Campaign co-op is a separate future effort. Movement fixes reduce stale network
updates; periodic frame-rate drops remain under investigation.

## Texture overhaul

A modern reinterpretation of the game's materials and surfaces is underway,
with **up to 4K for major surfaces** and smaller sizes where appropriate. The
active pack is capped at **6 GiB**; runtime replacement residency is limited to
**512 MiB**.

This is an unfinished, optional local preview. Development artwork is not bundled
with the public release, and much of the game still uses original textures.
Output resolution does not imply native 4K source detail or completed coverage.
See the [texture guide](docs/textures.md) for preview commands, local coverage
checks and storage rules.

## Native Apple Silicon

The experimental Mac build runs natively on ARM64 with SDL/Cocoa and OpenGL.
Menus and visible first-level gameplay have been reached, with movement, mouse
aiming, firing and pause/resume checks. It shares the PC controls model and adds
native settings windows and in-game display selection.

Its optional **up-to-120 FPS presentation** interpolates rendering while keeping
the original 60 Hz game simulation. This is a target, not a guarantee. Movies,
advanced rendering paths, multiplayer and standalone packaging still need work;
full-level completion and save/load remain unverified.

Start with the [Mac setup and controls guide](docs/macos.md). The
[Mac feature backlog](MAC_PORT_BACKLOG.md) tracks implementation and validation
separately; Windows tests do not establish Mac support.

## Build, contribute and troubleshoot

| Guide | Use it for |
| --- | --- |
| [Windows launcher](docs/launcher.md) | Install, import game data, play, update and recover |
| [Build and development](docs/building.md) | Windows prerequisites, setup, everyday commands, patch workflow and source layout |
| [Windows settings](docs/windows-settings.md) | Full control mapping, display settings, cursor capture and diagnostics |
| [Multiplayer](docs/multiplayer.md) | Online invitations, hosting, LAN and compatibility |
| [Native Mac](docs/macos.md) | Apple Silicon setup, controls and bounded validation |
| [Texture overhaul](docs/textures.md) | Local previews, coverage and storage budgets |
| [Release publishing](docs/releasing.md) | Package, validate and publish a Windows release |
| [Measured progress](STATUS.md) | Test evidence, known limitations and remaining work |
| [Project rules](AGENTS.md) | Shared development, reproducibility and data-preservation requirements |

When [reporting a bug](https://github.com/TheDoffman/timesplitters-future-perfect-pc/issues),
include the release version and short build ID shown in-game, your OS/GPU,
level or mode, and steps to reproduce. Include relevant logs or screenshots;
keep personal saves and game data out of public reports.

## Credits and source

Built on [nemonicicon/ts3FPrecomp](https://github.com/nemonicicon/ts3FPrecomp),
with local runtime patches and platform adapters. The pinned upstream revision
and licensing details are in the [build guide](docs/building.md#upstream-and-licensing).
Retain the applicable upstream and third-party licenses when distributing builds.

This repository contains port code, patches and tooling. Original game assets,
the Xbox executable, generated game code, personal settings and saves remain
local. Static recompilation does not recover the game's original C/C++ source.
