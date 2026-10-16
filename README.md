# TimeSplitters: Future Perfect — PC Port

**The Xbox classic, rebuilt for native PC play.** Direct mouse aiming,
customizable controls, in-game resolution settings, ultrawide support, and
experimental Online multiplayer—with a native Apple Silicon port in development.

[Download for Windows](https://github.com/TheDoffman/timesplitters-future-perfect-pc/releases)
· [Multiplayer setup](docs/multiplayer.md)
· [Build from source](docs/building.md)
· [Report a bug](https://github.com/TheDoffman/timesplitters-future-perfect-pc/issues)

This community project statically recompiles the original Xbox executable into
native code. You supply the supported **US Xbox game data**; no emulator is
required to run the Windows release. Original game files are not included.

> **Experimental prerelease.** Windows gameplay has been tested, but a complete
> campaign playthrough and full rendering fidelity remain unverified. Features
> below describe current `main`; check your release notes for what is included
> in a particular download.

## What the port adds

| Feature | What it means for players |
| --- | --- |
| **Direct mouse look** | Raw mouse motion drives the camera, with zoom-aware sensitivity and normalized WASD movement. |
| **Customizable controls** | Rebind keyboard and mouse actions, set sensitivity and inversion, and use up to four bindings per action. |
| **PC button prompts** | Menu and HUD hints follow your bindings and switch back to controller icons when you use a controller. |
| **1080p, 1440p and 4K** | Choose the native rendering resolution from the game's own Settings or paused Options menu. |
| **Ultrawide support** | Monitor-reported resolutions, a wider gameplay view, proportionate menus and HUD, and adaptive in-engine cutscene framing. |
| **Online and LAN multiplayer** | Player-hosted competitive matches, invitations, private join codes and a compatible-build lobby browser. Online is experimental. |
| **Portable Windows setup** | Import your game once, then launch with Play.cmd. Settings and saves stay alongside the release. |
| **Performance tools** | Optional FPS, frame-time and network-latency displays, plus debug information for troubleshooting. |
| **Texture replacement support** | An optional local preview path for the ongoing texture overhaul, with a 6 GiB pack budget. |
| **Native Apple Silicon work** | An ARM64 build with mouse look, native settings and an experimental up-to-120 FPS presentation mode. |

Recent fixes on `main` also improve pickup-range checks, controller sensitivity
adjustment, story audio transitions, Khallos Express scenery rendering, and
multiplayer movement updates. See [measured progress](STATUS.md) for the evidence
and remaining validation.

## Download and play on Windows

**Requirements:** Windows 10/11 x64, about 5 GB free for initial setup, and your
own supported US Xbox ISO or extracted game folder. Optional texture artwork
needs additional space. Python, Visual Studio and Git are not required to play.

1. Download the Windows x64 ZIP from [Releases](https://github.com/TheDoffman/timesplitters-future-perfect-pc/releases).
2. Extract the **entire ZIP** to a writable folder.
3. Double-click **Play.cmd**.
4. On first launch, select your ISO or a folder containing `default.xbe`, `pak`
   and `xmv`. The importer checks the game version and copies the required files.
5. Launch with **Play.cmd** afterward. The imported data stays in `Data`; the
   ISO is no longer needed for play.

**Keep `Data`, `Settings` and `Saves` when upgrading.** Keep the rest of the
extracted package together, including its `Support` directory and licenses.

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
