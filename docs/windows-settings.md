# Windows controls and display settings

[Back to the README](../README.md)

## In-game display settings

These instructions describe the current Windows source build. Downloaded releases may predate individual features.

Open **Settings → Display settings** from the main menu, or **Options → Display
settings** while paused. Use the resolution row's left/right arrows to choose
a mode, including **Original** for the game's default. Choose **Apply resolution**, then
**Keep resolution** within 15 seconds. Back, Revert, or letting the countdown
expire restores the previous mode. Only confirmed settings are saved.

The Windows launcher and in-game picker use the primary display's reported modes,
including ultrawide sizes within 640×480–7680×2160 and a maximum 4:1 aspect ratio.
Refresh-rate duplicates are removed, the console default appears first, and saved
custom modes remain available after a display change. The launcher provides
previous/next buttons beside its dropdown, plus editable custom dimensions.
These settings change the native
3D rendering resolution; the window still follows its existing size. Use
**Alt+Enter** for fullscreen. Desktop resolution and refresh rate are unchanged.
Game menus retain their original proportions with bars; gameplay uses the
selected aspect ratio and a wider horizontal camera view. Cutscene bars adapt to
the selected aspect ratio while preserving the original cinematic framing:
wide screens use thinner bars, and screens wider than the cinematic picture
use side bars to fit its full height. The cutscene camera and scripted bar
animations adapt together, preserving the original composition. This affects
in-engine cutscenes; startup video framing is unchanged.

Settings live in `config/pc-display.ini` beside the keyboard/mouse configuration
(`Settings/pc-display.ini` in packaged builds). `TSFP_DISPLAY_CONFIG` can override
the path for isolated tests. Delete that small file to restore the original
rendering resolution. Extremely wide modes are bounded at 7680×2160; modes above
2160 pixels high are outside this release's resolution range.

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
| Show / hide PC Settings menu bar | F1 |
| Keyboard & mouse settings | F2 |
| Debug settings | F12 |
| Fullscreen | Alt+Enter |

**PC Settings > Fullscreen** also toggles fullscreen; its checkmark reflects the
current window mode.

Press **F1** to show or hide the menu bar, including in fullscreen. You can also
choose **PC Settings > Hide Menu Bar** and press F1 to bring it back. The menu bar
starts visible each launch; F2, F12, and Alt+Enter still work while it is hidden.
Escape releases the mouse; click the game to recapture it.

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

Controller sensitivity uses the original in-game slider, with 0.05 steps
between 0 and 1. Older builds could jump straight to an endpoint (issue #24).
The shared float-comparison fix in `837692d` restores intermediate values;
rebuild with `tsfp.ps1 -Action Build` to refresh older generated code and the
executable. This setting is separate from raw mouse sensitivity in F2.

In-game button prompts automatically show the **first assigned keyboard or mouse
binding** for each control, for example **[E] Select** and **[F] Back**. Saving
bindings in F2 updates the prompts immediately. Unassigned controls show
**[Unbound]**. Mouse-look prompts show **[Mouse]** while raw mouse look is enabled.

Press a controller button, move a stick past its deadzone, or squeeze a trigger
to restore the original controller icons. Keyboard presses, mouse clicks, wheel
input, or mouse movement switch back to keyboard/mouse prompts. An unchanged held
button, button releases, and stick drift do not switch the prompts back and forth.
The first connected controller not assigned to another local player can control
player 1 alongside the keyboard profile; plugging it in does not require a restart.
This applies to the game's formatted text prompts, including menu and HUD hints;
controller diagrams and labels baked into images retain their original artwork.

The Xbox action mapping was checked against the publisher's original
[Xbox manual, Complete Controls](https://www.gamesdatabase.org/Media/SYSTEM/Microsoft_Xbox//Manual/formated/Timesplitters_-_Future_Perfect_-_2005_-_Electronic_Arts.pdf).

### Debug settings

For the temporary five-second-stall investigation, **Play-Quiet.cmd** in the
source checkout launches
with diagnostic output discarded and Debug log polling/performance logging
disabled. FPS measurement and your saved debug options remain intact. The next
normal **Play.cmd** launch restores reporting. For terminal/System Link launches,
set `$env:TSFP_DEBUG_QUIET = '1'` before your usual launch command; afterward use
`Remove-Item Env:TSFP_DEBUG_QUIET` to restore normal launches. Restart both game
instances when comparing modes. Quiet mode tests reporting I/O; internal report
formatting still runs, and diagnostic logs/counters are unavailable for that run.

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
- **Show network latency:** gameplay RTT in milliseconds, updated about once a
  second. Online guests measure their host; hosts and LAN players show the highest
  fresh peer RTT. Offline and waiting states are labelled.
- **Open logs folder** and **Clear counters / recent:** inspect the original
  logs or reset the session counters without deleting log files.

**Save & Close** persists the display/logging options under `debug` in the same
controls JSON file and applies them immediately. **Cancel** discards checkbox
edits; clearing counters takes effect immediately. All options default to off.
Existing controls files without a `debug` section remain supported.

A lower-right watermark always shows the PC version and a short multiplayer
build identifier, including when Debug options are off. `VERSION` is the shared
version source for builds and the default release package name.

Diagnostics match logged failures, unresolved calls, warnings, and unsupported
features; they do not detect every gameplay or visual anomaly. Counts cover the
current launch since startup or the last clear. Use `Play.cmd` or `tsfp.ps1 Run`
for live log monitoring; direct launches without `RECOMP_LOG_FILE` show
"Log unavailable". Log reading is bounded and runs outside the render thread.
The runtime's diagnostic frame capture uses **Ctrl+F11**; plain **F11** does nothing.
