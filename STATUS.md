# Measured progress

## Launcher v1.2.6 release preparation (2026-10-16)

Prepared in PR #73 on `release/launcher-v1.2.6`, based on main `293295b` and
merged with main `9c87069` (PR #72 FOV fix). The release includes
PR #71 README/launcher copy cleanup and PR #70 resolution/aspect filters.
Version and assembly metadata are 1.2.6. Release compatibility review found
that the published alpha.4 game accepts heights only through 2160. The launcher
now filters presets and rejects unsupported custom dimensions before any settings
write for alpha.4 and earlier (or an unrecognized version); newer/unpackaged
games retain heights through 4320. Compatible landscape 4K and ultrawide modes
remain available. The game release remains v0.2.0-alpha.4.

Windows x64 / .NET Framework 4.8 / pinned Python 3.12.14: both launcher EXEs
compiled; all 98 updater/bootstrap/cleanup checks, 25 settings checks and five
startup/packaging Python checks passed. Coverage includes atomic rejection of
unsupported legacy resolutions, legacy preset filtering, retained 4K and newer
8K model saves. Home, keyboard/mouse settings, graphics settings and bootstrap
splash previews rendered successfully; Home and graphics were visually inspected.
Tests used disposable fixtures; no actual game session was launched or personal
settings changed. Native Mac launcher execution is unverified; adaptation and
acceptance checks remain queued in MAC-021. Publication is pending exact-main
build, hosted checks and public-download/previous-client verification.
## Falling gameplay FOV (issue #66) — 2026-10-16

Implemented on `66-bug-fov-returns-to-default-when-falling-from-a-ledge`,
based on main `2608580`. Windows x64, MSVC 14.44.35207, D3D11, pinned runtime
and complete patch stack; supported US XBE SHA-256 verified as
`3cfd001a84fc3e08175d6c4b2e42ebf49577a089b5d0bd87ac724f61a41816bc`.

The FOV camera hook accepted grounded player state 2 only. The original US
`sub_00258F50` switches to state 4 on lost ground contact (call at `0x0025954D`),
and `sub_000D4100` restores state 2 on landing. `src/pc_display_menu.c` now
accepts both states for the owned gameplay camera. Existing projection-call,
cinematic and camera-ownership exclusions remain. GPU projection and CPU frustum
continue to scale together without rewriting the guest weapon/zoom lens.

Validation:

- Added grounded/falling/repeated-falling/landing checks at both gameplay camera
  call sites in `tests/cutscene_letterbox.c`. The new regression failed against
  the old hook, then passed with the fix.
- Default and reported 90-degree falling FOV, unchanged guest state/lens,
  non-accumulating projection and excluded terminal/non-gameplay states covered.
- `test_cutscene_letterbox.py`, `test_decal_projection.py` and
  `test_fov_settings.py`: 9 passed, including the lifted original US camera path.
- Complete patch stack verified; `tsfp.ps1 Build` and all build-script checks passed.
- Isolated Windows test game opened at 3440x1440 and 90-degree FOV, with
  separate save/input/display paths, 30-minute limit and capped logs. The user
  confirmed FOV stays consistent through falling and landing on 2026-10-16.
  No native Mac execution performed.
  Shared hook needs no separate Mac adapter; follow-up is recorded in MAC-022.

## README and launcher copy cleanup (2026-10-16)

Copy-only implementation `b9a05b9` on `cleanup/readme-launcher-copy`, based on
main `2608580`.
Removed slogans, game-themed quips and promotional headings from the README,
launcher home screen, default banner, startup splash and runtime status messages.
Visible navigation now uses Home, Game and Updates; stable element IDs and event
handlers remain unchanged. The settings draft hint and launcher guide use Home.
The README screenshot was replaced with the actual updated development UI, with
its source-preview status and executable hash recorded in docs/assets/README.md.
Game branding, artwork, functional instructions and prerequisite notices remain.

Windows x64 / .NET Framework 4.8 / Python 3.12.14: both themed launcher EXEs
compiled. Main, keyboard/mouse settings, graphics settings and startup-splash
preview commands succeeded. A separate build without private banner/logo assets
compiled and its fallback preview rendered successfully. Main and fallback
previews were visually inspected. A source scan found no remaining removed
phrases; the README reference to the game having no jump action is functional
controls documentation and remains. No new tests were added for these text-only
edits; no game build, game execution, update or personal settings changes were
performed. Native Mac UI wording is unverified. Publication is not part of this
change.

## Resolution groups and adaptive UI (issues #27/#31) — 2026-10-16

Implemented on `27-add-aspect-ratio-169-and-more-resolutions`, originally based on main
`4124557`. Windows x64, MSVC 14.44.35207, D3D11, pinned runtime and complete
patch stack, supported US XBE SHA-256
`3cfd001a84fc3e08175d6c4b2e42ebf49577a089b5d0bd87ac724f61a41816bc`.

The guest menu and Windows launcher share 41 common render presets and six
aspect groups, supplemented by reported monitor modes. Bounds are 7680×4320,
33,177,600 pixels and 4:1; saved custom modes and V1/V2 FOV files remain supported.
Aspect selection filters/stages a resolution rather than stretching it. The guest
refresh restores choice indices by row ID, so the new filtered index is reapplied
after refresh to prevent accidental resets. Desktop display modes are unchanged.

Menu backgrounds use the complete output canvas. Text/buttons fit proportionally
on both wide and narrow screens. Single-player ammo groups retain their original
left/right handedness; radar placement starts after world target markers; status
and scope controls receive scoped placement tags. Guest sprite batches flush at
placement changes, and viewport/scissor transforms agree. Split-screen retains
its existing layout. Native Mac shares the catalog and layout rules; its separate
Cocoa aspect selector is deferred in MAC-024 and native execution is unverified.

Health/armor follow-up: the gauges were still using the centred canvas, and
opening a larger submenu increased their authored scale beyond its horizontal
bounds. `sub_0013AB60` now fits the full curve mesh with a four-percent inset.
The primitive-allocation hook separates the three left/right mesh passes before
they are batched, retaining the centred directional hit marker and restoring the
previous placement afterward. Single-player layout is shared with native Mac;
split-screen keeps its previous behavior. A user screenshot at 3440×1440
identified the clipped health/armor curves on Audio/Video Options.

Tutorial follow-up: the original card reserved 40 units for controller icons,
so translated mouse/key labels overlapped the action descriptions. The shared
title hook now measures the current bindings in each row's actual font, uses
the widest label plus padding for all normal action rows, and expands the panel
within the viewport. The original text engine wraps descriptions in the remaining
space and computes their height. A follow-up reduces single-player tutorial
text and glyphs to 90% in both axes, before measurement and wrapping. The font
scale is restored afterward, leaving other HUD/menu text unchanged. Controller
column placement, composite stick rows and split-screen retain their previous behavior; geometry/font/register state is
restored after the card. No literal tutorial strings or generated game C changed.

Shading-seam follow-up: the user capture isolated an animated textured overlay
(draw 327, a 128x128 noise texture over a 640x480 canvas). AUTO placement treated
it as a proportional picture because it was neither flat color nor a framebuffer
copy. The `fullscreen-effects` patch recognizes a complete first quad at the
canvas corners, including inclusive pixel bounds, while retaining proportional
placement for clipped panels and separate corner HUD widgets. It also measures
original fan/quad vertex counts rather than the expanded triangle-list count,
avoiding an out-of-bounds read during placement classification. Explicit placement
scopes still take precedence. The effect is preserved across the full output.

Edge-caption follow-up: the mission intro's date/title (`sub_0013FC50`) and
single-player tutorial cards (`sub_001476F0`) now use the same LEFT placement
as edge HUD widgets on wide displays. Each scope flushes existing sprite batches
before drawing, keeps the full text/outline or panel/key-label group together,
and restores previous placement afterward. Authored insets, vertical positions,
animations, tutorial 90% font sizing and measured columns are retained. Original
4:3/narrow and split-screen placement remains unchanged. The title hooks are
shared with native Mac, whose runtime validation remains pending.

Validation so far:

- PR #70 CI follow-up: Windows run `38981957451` passed patch tracking,
  application and UI checks, then failed compiling the standalone FOV fixture.
  Its production menu include now also compiles tutorial hooks, requiring the
  runtime input-header directory and five guest drawing functions plus the prompt
  device query. The fixture now supplies that include path and fail-fast stubs
  for the unrelated tutorial paths; production runtime behavior is unchanged.
  The exact failing display/FOV step and shared UI suite pass locally (7 cases).
  The original macOS CI job passed its shared tests; this does not establish
  native game compilation or visual support. The correction only changes the
  Windows fixture and requires no Mac adapter change.

- Main integration: feature commit `2d69459` was combined with main `9e08fa3`.
  Both branches wrapped `sub_0013AB60`; a subsequent reproducible patch now
  routes Objectives fitting and ordinary health draws through the shared
  placement helper exactly once, preserving the upstream panel and split-view
  sizing. The combined Windows Release build and all eight build checks pass;
  twenty-six Objectives/UI/patch-transition cases pass. The launcher rebuild
  under Windows PowerShell 5.1 passes 98 regressions and 24 settings checks.
  Logs: `build/issue-27-display/build-main-integration.log`, `merged-tests.log`
  and `launcher-main-integration.log`. Earlier isolated visual captures below
  precede this integration; the combined build has automated validation only.
  Native Mac compilation/playtesting and actual 8K GPU allocation remain unrun.

- Edge captions: the Release rebuild and all eight build checks pass, as do all
  nineteen UI/patch-transition cases. A bounded isolated 3440x1440 Windows run
  reached first-mission gameplay. Reviewed `caption-edge-0.png` and
  `tutorial-edge-2.png` in `build/issue-27-display`: the date/title stays at the
  lower-left inset, while the complete weapon tutorial panel occupies the upper
  left with separated key/action columns, wrapped text and a clear central view.
  Render tags confirm LEFT placement; the mission caption spans x=20..155.6 in
  the guest canvas. Controller/narrow/split guards and placement restoration are
  covered by shared tests; native Mac visual validation remains pending.

- Shading seam: the user's frame 1540 (350 draws) replays successfully at
  3440x1440 with zero malformed/unmapped/failed commands. Stretching only draw
  327 removes the boundary; the fixed renderer matches that diagnostic image
  within one color value in 121 channels. Mission text retains its position and
  proportions. Selected evidence: `build/issue-27-display/seam-before.png` and
  `seam-fixed.png`; capture/checksums are recorded in `seam-validation.json`.
  The Release build, all eight build checks, five shared UI cases and fourteen
  patch-transition cases pass. The historical-controls fixture initially failed
  because it included this display-dependent patch before display support; its
  old-state patch selection was corrected and the affected test passed on rerun.
  The user confirmed the transparent rectangle is gone in the rebuilt isolated
  game during the first-mission opening at 3440x1440. Native Mac validation remains
  pending.

- Tutorial follow-up: all eight Release build checks, four shared UI cases
  (including production tutorial hooks) and fourteen patch-transition cases pass.
  The isolated first-mission run at 3440×1440 shows distinct key/action columns
  and a fully contained two-line fire-mode description. Reviewed capture:
  `build/issue-27-display/tutorial-fixed-3440x1440.png`. Longer rebound labels,
  bounded panel growth, wrapping space, original controller/split-screen geometry
  and guest stack/font/argument restoration are covered by the regression fixture.
  Temporary prompt diagnostics were removed. The subsequent 90% tutorial-size
  adjustment passes all eight build checks and four UI cases, including scoped
  scale restoration; its
  smaller appearance awaits user review. Native Mac remains unverified.

- Health/armor follow-up: Release compilation succeeded. Four UI/FOV cases
  and fourteen patch upgrade/transition cases passed. Seven of eight build
  checks passed; the virtual-controller prompt fixture twice failed its
  `GetForegroundWindow` acceptance assertion (line 55). Input code was unchanged;
  those intermediate runs were not all-green. The subsequent tutorial rebuild
  passed all eight checks without changing input code.
- A bounded isolated 3440×1440 run using the existing input-state test interface
  reached first-mission gameplay, pause Options and Audio/Video Options. Reviewed
  complete, separate left/right gauges with clear panel spacing in
  `health-pause-3440x1440.png` and `health-audio-video-3440x1440.png` under
  `build/issue-27-display`. Recorded mesh bounds are 25.6–217.0 on the left and
  423.0–614.4 on the right in the 640-unit guest canvas. Return to gameplay
  retained the centred crosshair and right-edge ammo. Earlier timed attempts
  stopped at the briefing/profile editor and are not gameplay evidence.
  Human confirmation of the revised gauges and native Mac validation remain.
- Windows game Release build and its eight native/build checks pass after the
  final picker correction. One intermediate run hit the existing foreground-focus
  prompt-test assertion; subsequent full runs passed without input-code changes.
- Windows PowerShell 5.1 launcher build: 71 launcher regressions and 22 settings
  checks passed. WPF coverage includes aspect filtering, current-ratio reporting,
  custom modes, selection/save/reload and the existing FOV slider. Reviewed the
  graphics preview in `build/issue-27-display/launcher.png`.
- Eight targeted cases passed for display discovery, FOV/menu behavior, cinematic
  framing and framebuffer sampling. Three UI cases pass: production widget scopes,
  verified hook sites, and extracted production viewport/scissor code across seven
  sizes and all anchors. Radar world markers remain outside the corner scope.
- Fourteen complete patch upgrade/transition cases pass, including historical
  controls and texture-branch round trips. The historical fixture excludes these
  later UI patches until their display dependencies are installed.
- Initial 1280×720 actual menu capture confirms a full-width animated background,
  proportional text/panel/FOV slider and a functioning aspect selector. An
  86-second recorded-input guest run then filtered 16:9 modes, confirmed 960×540,
  previewed 1280×720, timed out back to 960×540 and returned to the original menu.
- A fresh isolated 135-second first-mission run at 2560×1080 reached gameplay.
  Reviewed the right-edge ammo counter, centred crosshair and pause Options panel;
  recorder tags confirm the ammo group uses RIGHT and the next menu returns AUTO.
  The first attempt used an existing test profile and stopped at its name editor;
  only the fresh-save run is gameplay evidence. Its tutorial key-label overlap
  prompted the separate measured-column correction described above.
  Radar and sniper scope placement still need a representative manual playtest.
- Reviewed final 1280×1024 (5:4) and 2560×720 (32:9) menu captures: full-width
  animated backgrounds, proportional text/buttons/FOV slider, and unclipped panels.
  Executable provenance and selected evidence are recorded in
  `build/issue-27-display/validation.json`. Actual 8K GPU allocation and native Mac
  execution were not tested. An isolated 1920×1080 build is prepared for user
  radar/sniper and resolution-switching playtests; no personal settings were changed.
## Launcher v1.2.5 release preparation (2026-10-16)

The user requested stable publication of the game-version cleanup in PR #69.
Version and assembly metadata are bumped to 1.2.5. The cleanup implementation is
unchanged from validated `6e49e0a`; the release requires CI on the new revision,
a clean build from its main merge commit, public checksum verification and the
previous v1.2.4 client compatibility check. This is a launcher-only release; the
game remains v0.2.0-alpha.4. Native Mac launcher execution is unverified.

## Game version cleanup (2026-10-16)

Implementation commit `d1f4e42` on `fix/game-update-cleanup`, based on main
`2b3ada8`.
The previous game snapshot is kept until a session has shown a game window and
exited with code zero. Installed runtime files must still match BUILD.json before
cleanup; personal controls and independently updated launcher files are excluded
from game checksums. Confirmation is bound to the manifest hash, so a successful
old version does not authorize deletion of a newer version's recovery copy.
Incomplete installations, failed starts and unfinished transactions keep recovery.
The existing UI window-observation/exit path supplies the observation; this does
not claim verified gameplay or full rendering correctness.

Confirmed cleanup removes the previous game snapshot and obsolete/incomplete
staging only. A retired snapshot is renamed before recursive deletion, so a
partially locked copy is not offered as a usable rollback; startup retries handle
it for the same confirmed build. Cleanup is serialized with other launcher work
and runs off the UI thread. Runtime installation/rollback still refuse while a
game runs. ISOs in managed folders, linked paths and personal backup directories
block deletion. Root saves, settings, ISO, Data and Textures remain untouched.
Newer/unknown staged releases are retained. Unrelated/manual game folders are not
cleanup candidates.

During an update, unchanged old manifest-owned files absent from the new manifest
are removed through the existing transaction journal; rollback restores them.
Modified and untracked files, all personal settings and independent launcher files
remain. Missing/invalid old manifests skip optional pruning. The restore button
tooltip explains the retention boundary.

Windows x64 / .NET Framework 4.8 / Python 3.12.14: local synthetic validation passed
98 updater/bootstrap/cleanup checks, 23 settings checks and five startup/packaging
checks. Ten focused game cleanup cases cover confirmation, changed/corrupt builds,
transactions, staged versions, ISO/personal backup protection, locked-file retry,
junction refusal and safe staging replacement. Two full updater cases cover
obsolete file removal/rollback, modified/untracked file preservation, and ISO
protection in the old snapshot. One settings run encountered a transient Windows
file-replacement IOException; its isolated rerun passed all 23 checks. The final
build passed all 98 updater and 23 settings checks.
Windows CI on `6e49e0a` passed all 98 updater checks and 23 settings checks;
the Mac shared-check job also passed. Tests use tiny synthetic packages and fake
ISO markers only; no real game data, personal saves, or installed runtime were
changed and no actual game session was launched. MAC-021 adaptation is queued;
native Mac execution and other-device synchronization remain unverified.

## Launcher v1.2.4 release preparation (2026-10-16)

Version and assembly metadata are bumped to 1.2.4 for stable launcher-only
publication from main through PR #68. The cleanup implementation is unchanged
from validated `a7246b5`; publication requires the new revision to pass CI and
checks of the public assets and previous v1.2.3 update path. The published game
remains alpha.4.

## Launcher update cleanup (2026-10-16)

Implementation commit `dc019bf` on `fix/launcher-update-cleanup`, based on
main `b1860c6`.
The release launcher previously retained one old launcher snapshot and its helper
copies; snapshots were replaced at each update, rather than accumulating an
unlimited history. Successful normal main-window startup now verifies the current
launcher and removes that prior snapshot, obsolete/incomplete launcher staging,
and helpers no longer needed by an update. Failed preparation cleans its staging
without replacing the original failure. Cleanup errors are logged and retried
on subsequent startups. No cleanup runs during developer render previews.

Recovery transactions and unverified installations prevent cleanup. Newer staged
launcher releases and unknown staging markers remain available; pending game work
keeps its helper. The game rollback, installed game, settings, saves, data,
preferences, logs and unrelated downloads remain untouched. Cleanup rejects
linked paths through the existing checked launcher-owned deletion API.

Windows x64 / .NET Framework 4.8 / Python 3.12.14: both launcher binaries compiled,
nine isolated cleanup checks and all 23 settings checks passed. Cleanup checks
include three consecutive upgrades, protected files, damaged installation,
unfinished transactions, semantic version comparisons, newer/unknown staging,
locked files with retry, pending game work, and a real directory junction.
Five startup/packaging checks also passed. Windows CI on `a7246b5` passed all
86 updater/bootstrap/cleanup checks and 23 settings checks, including checksum,
invalid-archive and cancellation preparation failures. The Mac CI job also
passed its shared checks. Full local updater validation was blocked by an active game;
that session and the installed launcher were preserved. No game build/playtest
is required for this Windows launcher change. MAC-021 adaptation remains queued;
native Mac execution and other-device synchronization are unverified.

## Bootstrap argument diagnostics (2026-10-16)

Launcher v1.2.3 release preparation on `fix/launcher-bootstrap-arguments`, based on
main `de23272`. The installed launcher log records `Unknown bootstrap arguments`
at 2026-10-16 21:45:56 UTC (17:45:56 local). The resolution-development task ran
`TimeSplitters Launcher.exe --render-settings-graphics ...`, although that flag
was only accepted by `Bin/TimeSplitters.App.exe`. Its subsequent command used
the correct EXE. This establishes a concrete trigger for the recorded failure;
the old log did not retain actual arguments for distinguishing other reports.
Both installed launcher files matched their local v1.2.2 manifest. They were
local development builds, not byte-identical to the published v1.2.2 assets.

`Bootstrap.cs` now delegates only the three known application preview modes
before taking the startup mutex, preserves relative/spaced output paths and
child exit codes, and bounds preview execution to 30 seconds. Normal startup,
startup-splash rendering and updater routes retain their behavior. Other
arguments remain rejected, with bounded JSON-escaped argument details in both
the log and error. Startup logging now includes version/count/arguments.

Windows x64 / .NET Framework 4.8 / Python 3.12.14: both binaries compiled. Three
isolated bootstrap checks passed using real synthetic child processes, covering
all forwarded modes, quoting, relative paths, exit status, preserved startup and
helper command routes, and bounded/escaped diagnostics. All 23 settings checks
passed. Real bootstrap execution rendered both the delegated graphics preview
and its own splash successfully into ignored `build/bootstrap-validation`.
The full updater suite stopped at the existing game-running safety guard; the
active game session and installed launcher/user settings were left untouched.
No game runtime source changed, and no new game build/playtest was performed.
Publication is pending the release PR and hosted Windows updater checks.
Compatibility review found that published alpha.4 reads only V1 display settings.
The newer main-branch FOV controls are disabled for installed releases through
alpha.4, and model validation rejects custom FOV before writing any settings.
Two additional checks cover compatible resolution saves, atomic rejection, the
newer-version path, and disabled controls with explanatory text. Native Mac adaptation remains queued under
MAC-021; other-device synchronization is unverified.

## Launcher settings CI startup budget — 2026-10-16

- PR #65's Windows run `38969601167` passed patch transitions and all 71 launcher
  updater checks, then stopped in the first FOV WPF UI check at the ten-second
  STA-thread deadline. Local runs passed, including the CI fallback without
  private banner/logo resources. This is a test-harness change, not game behavior.
- `tests/launcher_settings.cs` now allows up to 60 seconds for a UI worker,
  reports a timeout distinctly, preserves the original worker exception stack,
  and prints each check's name and duration. Cold initialization remains bounded;
  UI assertions and save/cancel checks are retained.
- Windows x64 local `tools/build-launcher.ps1 -Test` passed 71 updater and 21
  settings checks. Hosted Windows validation of the updated commit is pending.
  The previous PR revision's macOS patch-transition job passed; this WPF-only
  harness has no Mac adapter impact. No game data or personal settings were used.

## Configurable gameplay FOV (issue #29) — 2026-10-16

Implemented on `29-fov-support-in-video-settings`, starting from
main `ef17e2a`. Windows x64, MSVC 14.44.35207, the pinned runtime and
complete verified patch stack; supported US XBE SHA-256
`3cfd001a84fc3e08175d6c4b2e42ebf49577a089b5d0bd87ac724f61a41816bc`.
The shared display model now retains a vertical FOV preference (40–100°, default
55°), reading legacy display files and the explicit V2 format. The guest Display
settings menu and Windows launcher graphics tab expose sliders with numeric
values, reset and existing save/cancel behavior. Combined resolution previews
revert both fields. Gameplay projection and CPU culling use the same tangent
scale without changing the original weapon zoom state. Scripted cinematic and
terminal cameras keep their authored lens.

Validation on Windows:

- Six targeted pytest cases passed: shared cinematic/camera production wrapper
  tests (including mutation checks and the original US cinematic routines),
  display mode discovery, and production FOV settings/menu callbacks. The latter
  covers old/new files, range limits, slider drawing/input, unsaved drafts,
  reset, read-only save failure, keep and resolution timeout rollback.
- Windows launcher built with Windows PowerShell 5.1/.NET Framework; all 71
  launcher regressions and 21 settings checks passed. The added checks exercise
  the WPF slider, numeric label, reset, cancel/save, legacy defaults and V2
  persistence. Reviewed `build/issue-29-fov/launcher.png` from its preview renderer.
- MSVC Release game and native test targets compiled and linked successfully.
  Network, texture format, display, save paths, PC controls, settings and debug
  checks passed. The build runner stopped at the existing controller prompt
  fixture's foreground-window assertion (`tests/pc_prompts.c:55`); the same
  assertion failed when rerun directly. No input/prompt implementation changed.
  The remaining checks were run individually, and executable identity was sealed.
- A bounded 65-second isolated game run navigated the actual guest menu with
  recorded input, moved the slider 55 → 58, and saved
  `TSFP_DISPLAY_V2 1280 720 58`. Reviewed the menu capture and found both the
  number and original-style slider inside the panel. Evidence is in
  `build/issue-29-fov/menu-{47,55,62}.png` and `menu.err`.

- First-mission gameplay with isolated 1280×720 / 75° settings produced a
  vertical CPU frustum of `0.76732695` (tan(75°/2)) and projection Y coefficient
  `1.30322552` (its reciprocal), while the original game zoom field stayed 55°.
  This confirms the requested lens reaches the real projection and culling
  paths. The diagnostic run collected 117.25 seconds before its screenshot
  watcher hit a rotating-file race and stopped its owned game in `finally`;
  the game had not crashed. Retained evidence: `gameplay-camera.json`,
  `gameplay-result.json`, `gameplay-final.png` under `build/issue-29-fov`.

The user confirmed that the FOV slider works in the rebuilt game during a
separate isolated playtest (155 seconds, normal exit).

Personal saves and settings were not used. Shared/native Mac integration is
source-reviewed only; Mac build, physical controller navigation, all weapons
and split-screen gameplay are unverified. See MAC-022 in `MAC_PORT_BACKLOG.md`.

## Terminal mouse routing (issue #7) — 2026-10-16

Implemented on `7-breaking-and-entering-camera-turret-not-working`, based on
main `696421f` merged at `093e30f`. On Windows x64, a bounded isolated run
of the supported US XBE reproduced the Breaking and Entering terminal failure:
player mode was `0x2000`; physical mouse movement changed hidden player angles
while the active console pointer stayed fixed. The user confirmed the terminal
menu pointer, rather than a selected camera view, was blocked. Disassembly of
US `sub_003700D0` identifies local-index console ownership, pointer coordinates,
4–251 bounds, inactivity reset, scripted input and pause gates.

The shared `src/pc_terminal.h` now routes counts to that pointer and consumes
terminal input without changing player angles, including invalid transitions.
Coordinates stay in the game's logical space, independent of screen resolution,
FOV or aiming inversion. Windows and SDL adapters use the same helper.
The guarded-memory regression and full MSVC x64 Release build passed on Windows,
including patch-stack validation and the real Windows adapter regression in
`tests/pc_controls.c`. The user confirmed physical pointer movement, but left-click initially still
sent Fire instead of the terminal's Confirm action. The follow-up shared input
context and `zzzzzzzzzzzzzzzzz-terminal-mouse-select.patch` now suppress the
normal left-button binding while selecting and emit A/Confirm. A held button
must be released across either transition to prevent accidental selection or
firing. Windows regression covers the native adapter-to-binding path with bounded synthetic input; the shared
fixture covers click state, and the native SDL fixture has equivalent checks.
The updated MSVC x64 Release build and native checks passed, including Confirm
without Fire and release-before-fire on terminal exit. All 27 patch tracking/
transition cases passed (the historical-upgrade fixture was updated to exclude
this new dependent overlay, then rerun). The shared terminal regression passed.
The user confirmed the final physical playtest: pointer movement, left-click
camera selection, and ordinary aiming/firing after leaving the terminal work.
The read-only capture also records return to mode `0x2` with no active terminal.
Native Mac compilation/execution remain
unverified; see MAC-001.

## Terminal mouse routing (issue #7) — 2026-10-16

Implemented on `7-breaking-and-entering-camera-turret-not-working`, based on
main `696421f` merged at `093e30f`. On Windows x64, a bounded isolated run
of the supported US XBE reproduced the Breaking and Entering terminal failure:
player mode was `0x2000`; physical mouse movement changed hidden player angles
while the active console pointer stayed fixed. The user confirmed the terminal
menu pointer, rather than a selected camera view, was blocked. Disassembly of
US `sub_003700D0` identifies local-index console ownership, pointer coordinates,
4–251 bounds, inactivity reset, scripted input and pause gates.

The shared `src/pc_terminal.h` now routes counts to that pointer and consumes
terminal input without changing player angles, including invalid transitions.
Coordinates stay in the game's logical space, independent of screen resolution,
FOV or aiming inversion. Windows and SDL adapters use the same helper.
The guarded-memory regression and full MSVC x64 Release build passed on Windows,
including patch-stack validation and the real Windows adapter regression in
`tests/pc_controls.c`. The user confirmed physical pointer movement, but left-click initially still
sent Fire instead of the terminal's Confirm action. The follow-up shared input
context and `zzzzzzzzzzzzzzzzz-terminal-mouse-select.patch` now suppress the
normal left-button binding while selecting and emit A/Confirm. A held button
must be released across either transition to prevent accidental selection or
firing. Windows regression covers the native adapter-to-binding path with bounded synthetic input; the shared
fixture covers click state, and the native SDL fixture has equivalent checks.
The updated MSVC x64 Release build and native checks passed, including Confirm
without Fire and release-before-fire on terminal exit. All 27 patch tracking/
transition cases passed (the historical-upgrade fixture was updated to exclude
this new dependent overlay, then rerun). The shared terminal regression passed.
The user confirmed the final physical playtest: pointer movement, left-click
camera selection, and ordinary aiming/firing after leaving the terminal work.
The read-only capture also records return to mode `0x2` with no active terminal.
Native Mac compilation/execution remain
unverified; see MAC-001.

## Issue 19 decal projection (2026-10-16)

- Source: decal fix on `19-floating-decals` (this revision), based on
  `696421fd080a5bd57cb894b76302080f59f52485`; pinned runtime `d90c929` and
  checksum-verified US XBE. Windows x64 build 26300, MSVC 19.42.34435.0,
  Python 3.14.2, CMake 4.4.4, D3D11.
  Release executable SHA-256:
  `ada6ab457f3369c64c89d3a46f4d6ed29a698e5a3861364d876b6076e0e6048d`;
  multiplayer identity `85180d72`.
- Cause: the two decal render paths independently reconstruct a depth-biased
  perspective matrix, bypassing the adjusted camera. A baseline 1920x1080
  Arcade capture used horizontal scale `1.440737` for biased effects versus
  `1.080552` for the widened scene. The fixed capture uses `1.080552` for
  both, retaining the distinct depth terms `-0.10050558` and `-0.10000556`.
  This corrects the guest projection rather than changing HUD classification.
- Validation: the complete patch stack applies; supported `tsfp.ps1 -Action
  Build` regeneration/build and all native build tests passed. Executable seal
  and game-content verification passed. The decal, cinematic and patch-transition
  suites passed (35 cases; one historical fixture was updated and its failing
  case rerun). Wrapper checks cover both decal callers, default/wide/ultrawide
  and cinematic apertures, unchanged depth and other callers, missing camera,
  guest return state, and both Windows/native display build flags.
- Live check: a bounded Hotel Arcade match at 1920x1080 with no bots reached
  gameplay; fired wall impacts stayed aligned while strafing and rotating the
  camera. No crash or unresolved-call report occurred. All save, input and both
  display configuration routes were isolated under `private/decal-validation`.
  Selected screenshots and captures remain ignored there; disposable partition
  images were recycled after the runs. Personal settings and saves were preserved.
- Remaining checks: other Arcade maps, explosion impacts, split screen, and live
  default/4:3/ultrawide rendering were not exercised. Mac source uses the same
  wrapper, but native build/rendering and other-device synchronization remain
  unverified. See [MAC-010](MAC_PORT_BACKLOG.md#mac-010--in-game-resolution-preview-and-rollback).

## Resolution release preparation — October 16, 2026

Game v0.2.0-alpha.4 and launcher v1.2.0 are prepared together on
`release/resolution-v0.2.0-alpha.4`, based on main `92384df`. This integrates
launcher settings commits `caa6f5e`/`3b93941` and issue #28 fix `e3e99c1`
without replacing newer main runtime/Online/audio/rendering changes. The release
package includes both launcher executables and their self-update manifest,
alongside the current portable Online companion, licenses and source bundle.

Earlier issue-branch validation passed 71 updater and 14 settings checks, native
display tests and an 86-second real D3D11 guest-menu replay covering left/right
selection, 720×480 apply/keep/save, Original preview, timed rollback and return.
Both native value-column and WPF graphics-tab captures were reviewed. The US XBE
hash was verified; those earlier tests used an isolated relink. The main-based
full MSVC x64 Release build and native checks subsequently passed, along with
138 Python tests (5 platform/prerequisite skips), 2 launcher packaging tests,
71 updater checks, 14 settings checks and 3 launcher lifecycle checks.
The full executable passed the same isolated 86-second D3D11 resolution replay:
720×480 apply/keep/save, Original preview, 15-second rollback and menu return.
Fresh disposable saves and explicit input/display/runtime paths were used.
Both GitHub patch-transition jobs (Windows and macOS) passed on `647a42a`;
these source-level Mac checks do not establish native Mac game execution.
Publication follows `Publish-Release.ps1` on clean main, including a fresh
package manifest/checksum and portable Online validation. Final artifact/source
hashes and publication records stay under ignored `build/releases`.
The first preparation stopped on pytest's retained `*current` aliases.
Publisher cleanup now validates and unlinks only those direct sibling aliases
inside its own test tree; targets and arbitrary/external/nested links remain
protected. Passed per-test fixtures are discarded by pytest so intentional
hostile-link test cases do not remain in the cleanup tree. Regression fixtures
cover accepted/rejected aliases and a real nested pytest session with retained
session-cache targets. The earlier diagnostic fixtures are preserved separately.
Native Mac execution and paused-Options gameplay traversal remain unverified for
this change; adaptation is tracked in MAC-010 and MAC-021.

## Release regression compiler discovery — 2026-10-16

- Source base `cc35200`, changes on `fix-release-msvc-tests`.
  Host: Windows x64, CPython 3.14.2, Visual Studio 2022 Build Tools / MSVC
  19.42.34435.0. No game or renderer behavior changed.
- The old `vswhere` queries excluded standalone Build Tools. Audio discovery
  also preferred a stale mixed-case `Path` entry after reading uppercase `PATH`.
  `tests/native_compiler.py` now includes all Visual Studio products, loads
  `vcvars64.bat`, normalizes Windows environment names, and shares the resulting
  compiler/headers/libraries across audio, cutscene, framebuffer and lifted-float
  regressions. A ready x64 developer shell is preserved; POSIX retains `CC`/`cc`.
- The 40 affected native regressions and four discovery checks passed from an
  ordinary PowerShell environment. Native checks include the original supported
  US cutscene/pickup/controller routines; the tests verify the executable hash.
  Complete patch-stack validation passed with zero runtime files changed.
- The complete release Python suite subsequently passed from ordinary
  PowerShell: **129 passed, 9 skipped in 116.38 seconds**. Test output is local
  at `logs/release-msvc-validation.log`; disposable fixtures were isolated under
  `build/release-validation-tests`. No existing settings or saves were used.
- Native/full-suite fixture trees were recycled. Recycling the small
  `build/compiler-discovery-tests` directory was denied/unsupported, so it was
  retained; no alternative deletion or permission change was attempted.
- No package, tag, release, main-branch merge, or publication was performed.
  Mac execution and synchronization with the other device remain unverified;
  see [MAC-012](MAC_PORT_BACKLOG.md#mac-012--standalone-mac-runtime-distribution).

## Adaptive cutscene letterboxing (issue #33) — 2026-10-16

- Source base `5c9effd`, branch `33-remove-the-letter-boxes-in-cut-scenes`.
  The requested behavior is adaptive cinematic bars, rather than removal.
  Story routine `0x0006F150` draws its top/bottom matte at return PCs
  `0x0006F322`/`0x0006F34B`; scripted scene routine `0x001396B0` uses
  `0x0013970E`/`0x0013972E`. All four call solid rectangle routine `0x0001EFB0`.
- Added a reproducible manual wrapper and shared `src/pc_cutscene.h` geometry.
  Preserve the aperture of the original 4:3 presentation (approximately 1.88:1,
  with the game's original integer rounding). Adjust bar height for the selected
  output aspect and retain the original animation fraction. 4:3/legacy output
  keeps its original layout; wider-than-cinematic output uses side bars.
  This changes only identified bar rectangles and restores their guest stack
  arguments after drawing. Subtitles, full-screen fades, menu rectangles,
  scopes and prerecorded startup videos retain their existing paths.
- The first live test exposed orange scenery around the opening space backdrop.
  Capture frame 9140 reproduces this: draw 37 is the authored 3D starfield plane;
  the widened camera looks past its edges. Replaying with camera projection
  constants scaled to fit the cinematic aperture eliminates the exposed scenery.
  `pc_display_camera` now adjusts both projection axes and CPU frustum extents
  along with the matte, retaining normal gameplay widening outside cutscenes.
  Source-only checks cover story, no-matte special mode, animated scenes,
  4:3 and ultrawide framing, and detect removing the camera correction.
- The Windows Release build and all build-script native checks passed.
  All 27 patch tracking/upgrade checks and four adaptive-letterbox checks passed.
  The focused suite includes mutations that remove either bar adaptation or
  camera correction, and detects both regressions.
  The new regression compiles the actual wrapper, checks bar anchoring and
  unchanged unrelated rectangle arguments/ABI, and detects bypassing the filter.
  A private-XBE test executes both original routines across legacy, 4:3, 16:9,
  16:10 and ultrawide modes plus three animation phases, comparing original and
  adaptive draws, fade rectangles, subtitle invocations and render-state calls.
  It uses disposable guest RAM and skips in public CI without game data.
- Host: Windows x64, MSVC 14.44.35207, SDK 10.0.26100.0; pinned runtime
  `d90c929`, supported US XBE SHA-256 verified. Executable SHA-256:
  `b42b8511c2abd4df6314fe7059fcb574993ee78e856e659ac8c5a0787b6eaca4`.
  Evidence: ignored `logs/issue-33-build.log`, `logs/issue-33-tests.log` and
  `build/issue-33-letterbox`. In an isolated 1280x720 Windows run, the user
  confirmed the first mission space scene now looks correct. That run lasted
  48.73 seconds and exited successfully. The original diagnostic capture and
  corrected replay are retained; no second capture of the corrected live space
  shot was received. Live ultrawide and other scripted scenes remain unverified.
  Mac execution remains unverified (MAC-020). Personal settings and saves are
  preserved. Source changes are on `33-remove-the-letter-boxes-in-cut-scenes`.

- PR #37 CI follow-up: macOS Clang rejected the test fixture's unsigned
  `g_fp_top`, which conflicts with the runtime header's signed `int` declaration.
  Corrected the fixture type and made MSVC C4142 fatal in the focused test
  compiler. The stricter Windows check reproduced the original failure, then
  all four cutscene tests passed after correction (4.88 s). Production game
  code and the validated executable are unchanged. Hosted CI is pending rerun.

## Khallos Express see-through scenery (issue #9) — 2026-10-16

- Reproduced the transparent train walls and weapon in an isolated Windows
  Khallos Express session. Captured frame 3376 samples a framebuffer texture
  for background blur at draws 142–143, then samples that same texture again
  for additive glow at draws 462–463, after drawing the train and weapon.
  The HLE refreshed this texture only once per swap, so the final effect
  painted the earlier background over otherwise solid geometry.
- Added `patches/zzzzzzz-framebuffer-sampling-passes.patch`. Switching away
  from a framebuffer texture invalidates its cached copy. Rebinding it captures
  the scene including intervening geometry; consecutive taps retain a stable
  snapshot. Frame-zero initialization, changed texture identity and failed-copy
  retry are also handled. This is shared HLE logic, with no generated game C edits.
- `tests/framebuffer_sampling.c` executes the actual texture adapter with a
  fake copy device, through its texture-binding path. Tests cover the two-pass
  sequence, stable adjacent taps, a new frame, unbinding on another sampler,
  changed backing data and copy failure/retry. Deliberately removing the binding
  invalidation makes the regression fail. Added the test to `tsfp.ps1 Test` and
  CI on supported renderer hosts; Linux skips this Windows/macOS HLE test.
- Full Windows `tsfp.ps1 Build` and its native checks passed. All 29 patch
  tracking/upgrade and framebuffer regression checks passed in 93.29 seconds;
  repeated complete-stack application changed zero files. The historical upgrade
  fixture excludes this later patch until the complete-stack upgrade step.
- macOS CI subsequently found a test-only signature mismatch: the mock
  `host_CubeLockRect` had six parameters instead of the declared four. Corrected
  the mock and promoted MSVC parameter-mismatch warnings C4028/C4029 to errors.
  Both Windows framebuffer checks pass with the stricter compiler flags;
  restoring the old signature now fails compilation with C4029 on Windows too.
  The next macOS CI run exposed a missing `GetTickCount` link dependency in the
  standalone mock; its diagnostic clock now uses a deterministic test function
  on both hosts, without linking the operating-system compatibility layer.
  This changes the test harness only, not the runtime fix or game binary.
- The user confirmed **walls now stay solid** during movement/turning in the
  rebuilt game. Reviewed the resulting train-interior screenshot: opaque walls
  and weapon, with the window still visible. The isolated fixed run lasted
  88.11 seconds and exited with code 0. No full-level completion or other-level
  visual regression sweep was performed; Mac execution remains unverified.
- Source base `800687d`; fix delivered on branch
  `9-khallos-express-level-is-transparent`. Windows x64 build 26300,
  MSVC 14.44.35207, SDK 10.0.26100.0,
  SDL 3.4.16, Direct3D 11, NVIDIA RTX 4070 SUPER driver 32.0.16.1742. Upstream
  `d90c929`; supported US XBE SHA-256 verified. Build executable SHA-256:
  `ad897ef0e868de013a28eaf5515e86ece7c182470959a9a027babba48ac061df`.
- Ignored evidence: `build/issue-9-render/{before,after}.bmp`, two adjacent
  `sequence_0337*.d3dcap` reference frames, build/test logs and `validation.json`.
  Replay alone did not reproduce the live ghosting because the capture format
  omits GPU screen-copy contents; live screenshots and playtesting establish the
  visual result. Test saves/configs were isolated under
  `private/test-saves/pickup-range`; personal settings and saves were untouched.
  Redundant captures and disposable test partitions were removed. See MAC-019.

## Temporary diagnostic-output comparison — 2026-10-16

- User confirms the angle improvement but persistent remote strafe lag, with
  FPS dips about every five seconds even while stationary. The cause remains
  unresolved. Added an opt-in Windows quiet experiment on the existing issue
  branch; no personal debug preferences or gameplay clocks are changed.
- `Play-Quiet.cmd` sets `TSFP_DEBUG_QUIET=1` for its child launch only. The
  Windows entry point routes stdout/stderr to `NUL`, the kernel logger skips
  its separate `xbox_kernel.log`, and Debug skips log polling/performance-log
  writes. FPS sampling and overlay options remain active. `tools/play.py` also
  avoids creating play logs in this mode. A normal launch restores reporting.
  Internal periodic report formatting still runs; this isolates reporting I/O
  and polling rather than eliminating all diagnostic computation.
- Sources are `src/pc_debug_output.h`, `src/pc_debug.c`, `tests/pc_debug.c`,
  `tools/play.py`, `Play-Quiet.cmd` and the separate reproducible
  `patches/zzzzzzzzzzzzzzzzz-pc-debug-quiet.patch`. The entry-point define is
  source-specific so generated game C does not need unrelated recompilation.
  MAC-013 records the deferred native startup adaptation.
- Supported Windows Release build completed all native build-script checks
  after earlier attempts hit the existing foreground-window prompt assertion.
  The affected Debug regression additionally verifies suppressed log reads,
  continuing FPS updates, unchanged persisted options and restored polling.
  Windows x64 build 26300, Direct3D 11, pinned upstream and verified US XBE;
  build identity `e73e10e8`, executable SHA-256
  `df66f4ef4a3da023926263310e739c616dd60daf24e4e7e374fe18d108545c72`.
  Evidence: `logs/issue-20-quiet-final-build.log` and
  `logs/issue-20-quiet-debug-test.log`. Final patch-upgrade/transition suite:
  14 passed in 76.18 s (`logs/issue-20-quiet-patch-tests.log`). The preceding
  full suite had 37 passes and one temporary-file PermissionError in a patch
  transition case; the final patch suite covers that case successfully.
- An isolated 42-second quiet startup/menu run produced zero captured stdout
  bytes and zero stderr lines; the existing kernel log was not changed. A fresh
  22-second normal process restored diagnostic output without changing options.
  Both runs used disposable controls and explicit save/runtime-display/PC-display
  paths. These are startup/menu checks, not evidence that live multiplayer FPS
  dips are fixed. Evidence: ignored
  `private/network-validation/issue20-quiet-validation.json`,
  `issue20-quiet-profile-pipe.json` and `issue20-restored-profile-pipe.json`.
- Personal controls/display and the pre-existing tracked input-fixture edit
  and its backup retain their pre-task hashes. They were not repaired or used
  as mutable automated-test settings. Changes remain local and uncommitted;
  native Mac execution and other-device synchronization are unverified.

## Multiplayer strafe updates (issue #20) — 2026-10-16

- **Further candidate, local and uncommitted:** The full-state sampler
  `0x003588D0` compares six controller bytes and otherwise refreshes only after
  60 match ticks. Direct host mouse look bypasses those bytes; independent
  stalls also change prediction without changing a held key. A separate patch
  now wraps the sampler and requests the original full-state packet after
  three match ticks (20 Hz). The original input-change trigger remains active.
  The wrapper preserves the original encoder and ABI, handles up to four
  local players per peer, and retains disabled/lobby/native fallback behavior.
  This bounds state age; it does not diagnose or eliminate host FPS dips.
- Isolated menu pipe/file logging tests did not reproduce periodic clock gaps
  after startup. Two subsequent System Link tests, including a disposable 4K
  client configuration, also had no >30 ms clock gaps in their 15-second
  timing-only windows (largest about 22 ms). Players were at different spawn
  locations rather than mutually visible. These tests do not rule out a
  renderer/input/terminal stall in the user's specific situation. Process-local
  sampling profiles are retained under the isolated host/client directories.
- A further controlled baseline used held strafe keys, repeated mouse turns
  and three deliberate 120 ms suspensions of each test-owned sender. Full-state
  receive gaps were typically 61 ticks and reached 68. Maximum near-simultaneous
  position error was 0.772 client-to-host and 1.466 host-to-client; maximum
  yaw mismatch was about 9.6 degrees. This baseline includes the first faster
  send gate. Input, save and both display paths were isolated; suspensions
  affected only child processes launched for this experiment. Evidence:
  `private/network-validation/issue20-baseline-stall.jsonl` and
  `issue20-stall-comparison.json`.
- The candidate repeated the same input/pause schedule in a separate match
  start with different spawn positions. Full-state receive gaps had median
  three ticks and maximum ten across injected pauses; maximum sampled sender
  full-state age was three ticks. Maximum yaw mismatch fell to 1.60 degrees.
  Maximum position error was 0.642 client-to-host and 0.817 host-to-client;
  maximum remote sampled steps were 0.734 and 0.933 respectively. These are
  improvements, not elimination of the pause-associated snaps. Both tests use
  a default-resolution host and an isolated 4K client, with the same original
  simulation clocks. Evidence: `issue20-refresh-stall.jsonl`,
  `issue20-stall-comparison.json`, `issue20-refresh-validation.json` in ignored
  `private/network-validation`. This does not establish performance with both
  players mutually visible, physical input, a full room or WAN latency/loss.
  The cause of the user's approximately five-second live stalls remains
  unresolved; hidden-console menu testing did not reproduce that periodicity.
- The further candidate regenerated and compiled successfully through the
  supported Windows build; all build-script checks and completed executable/
  content verification passed. Identity `969f9c4a`, executable SHA-256
  `54bd194371473bd6afd051a3eb668869e8d5ca96c1914ac559157d6821cf983e`.
  The redirected outer PowerShell invocation returned 1 while its log completed
  all checks; executable sealing and explicit verification succeeded. All 38
  affected Python/native-wrapper regressions passed in 88.62 s after routing
  pytest to an existing workspace temporary parent. Initial attempts hit an
  inaccessible system temp root, then a missing workspace temp parent; a relay
  timing test also failed during the initial concurrent compile. Evidence:
  `logs/issue-20-refresh-build.log`, `logs/issue-20-refresh-tests-final.log`.
- **Still unresolved in live play:** A subsequent user-driven, 12-second
  observation of both running Windows instances reproduced substantial remote
  corrections with players facing each other on build identity `a6bb2366`.
  Read-only position sampling caught a remote actor step of 1.314 world units
  and maximum near-simultaneous replication error of 1.314; the moving player's
  largest local sampled step was 0.346. The other player remained stationary
  during this capture, so it establishes only one direction of this live case.
  Large corrections coincided with advances of the last full-state receive
  timestamp, despite frequent movement updates. This points to remaining
  prediction/state-reconciliation behavior; it does not establish its cause.
  Eighteen window-only screenshots per instance and samples are retained in
  ignored `private/network-validation/issue20-live-*` and `issue20-live.jsonl`.
  Observation sent no inputs, wrote no game memory, and changed no settings or
  saves. The earlier isolated measurements below do not establish a complete
  fix for issue #20.
- **Frame-drop follow-up:** Two further 15-second, read-only observations used
  memory sampling only, without window captures or injected inputs. The first
  recorded a 0.651-unit correction about 5 ms after a 131 ms pause in sampled
  game-clock progress. The user-cued repeat recorded a 0.939-unit correction
  about 11 ms after the other process resumed from a 55 ms pause. Other jumps
  had no nearby pause, so causality and the complete trigger remain unresolved.
  The repeat's largest game-clock gaps were 116/114 ms, compared with median
  intervals of 16.5 ms. One process had approximately 105/105/116 ms gaps at
  elapsed 1.05/6.05/11.05 seconds, suggesting periodic work worth profiling.
  These are sampled clock-progress gaps, not directly measured GPU presents.
  The network frame counter can be reset by incoming synchronization packets;
  it must not be treated as an independent monotonic render-frame counter.
  Evidence: ignored `private/network-validation/issue20-live-timing.jsonl`,
  `issue20-live-timing.summary.json` and the earlier retained
  `issue20-live-timing-initial.summary.json`. No further runtime change was made
  from this correlation alone.
- Local, uncommitted on `20-player-desync-when-strafing-multiplayer`, based on
  fetched main `c8f8b31`. User clarified that this is the other player's
  movement in multiplayer, not a Story-mode problem; only keyboard/mouse had
  been tested. No Story movement, console clock or rendering changes are made.
- Reproduced with two isolated Windows System Link processes in Vietnam
  Deathmatch. Forty alternating A/D holds of 150 ms produced remote position
  jumps up to 1.338 world units at approximately 20 ms sample intervals; the
  sender's largest sampled step was 0.218. Maximum near-simultaneous replicated
  position error was 1.493. The guest receive timestamps advanced seven ticks
  at a time even over loopback: about 117 ms between movement opportunities.
- The US pump `0x003559A0` computes an eight-Hz network epoch by dividing the
  game clock by `60 / 8`, truncated to seven. The active-match state (15) in
  `0x003544B0` waits for that epoch before sampling movement. The new wrapper
  opens only that send gate once per guest tick; it keeps the original pump,
  packet encoding, network epoch, heartbeat/timeout calculations and idle
  full-state refresh. Both PC transports share this path. Duplicate calls in
  one guest tick, inactive states and disabled/native fallback are covered.
- After rebuilding, the same rapid-reversal input on the same map reduced
  the remote maximum sampled step to 0.153 (sender 0.153) and maximum measured
  replication error to 0.0844. Receive timestamp gaps had median one tick,
  maximum two. Reversing roles and moving the host produced maximum replicated
  error 0.1154 on the client. These are separate match starts with different spawn locations,
  not an identical-position replay. The comparison establishes substantially
  smaller corrections under this loopback workload, not zero jitter on WANs.
- Windows x64 build 26300, MSVC 14.42.34433, SDK 10.0.22621.0, Python 3.14.2,
  SDL 3.4.16 and Direct3D 11. Supported US XBE checksum and pinned upstream
  `d90c929` verified. Complete patch stack applied; generated code refreshed
  through `tsfp.ps1 -Action Build`. Release build and all build-script checks
  passed. The completed executable/content compatibility check passed before
  the fixed run (identity prefix `a6bb2366`, baseline `1d13b6bc`).
- All 38 targeted guest-boundary, identity, Online relay and patch upgrade/
  tracking regressions passed. The historical pre-network Mac fixture now
  excludes the new networking-dependent patch until the upgrade step.
  Native-fallback checks preserve original calls without host transport symbols.
  Evidence: `logs/issue-20-tests-final.log`.
- Tests use `private/network-validation/host` and `client` saves with explicit
  disposable controls and both display configuration paths. Personal saves,
  personal settings and the tracked input fixture were preserved. Evidence:
  `logs/issue-20-build.log`, `private/network-validation/issue20-before.jsonl`,
  `issue20-before-network.jsonl`, `issue20-net-samples.jsonl` and
  `issue20-host-net-samples.jsonl`, `issue20-comparison.json` and
  `issue20-validation.json` in that validation directory. Both processes stopped
  and their disposable virtual disks were recycled. Input is synthetic;
  live Online gameplay, physical controllers, other PCs, WAN conditions and
  sustained/full-capacity matches remain pending. The faster send opportunities
  increase network traffic; full-room bandwidth was not measured. Native Mac
  build/execution and synchronization remain unverified; see
  [MAC-013](MAC_PORT_BACKLOG.md#mac-013--system-link-and-online-multiplayer).

## Controller sensitivity slider (issue #24) — 2026-10-16

- Source base `c17336b` (fetched `origin/main`); this revision adds regression
  coverage/documentation on `24-controller-sensitivity-slider-not-working-correctly`.
  The runtime correction is already present in `837692d`, through the shared
  `patches/recomp-float-joins.patch`. No additional runtime change is required.
- Located the original controller sensitivity callback `0x002EAB30` and its
  shared clamp at `0x002EABEB`. Older generation discarded the comparison when
  decrement and increment paths met, causing the clamp to overwrite intermediate
  values with an endpoint. Restoring that older join behavior in an isolated
  translation reproduces the failure; the current translator preserves it.
- Added `tests/controller_sensitivity.c`, invoked by
  `tests/test_recomp_float_joins.py`: compile the actual checksum-verified US
  routine, retaining its original 0.05 step and clamp constants. All 188 checks
  passed: 0.5 to 0.45/0.55, full ascending/descending ranges on four profiles,
  both endpoints, near-boundary clamping, no input and guest register/stack
  preservation. UI and audio callees are stubs; this is callback execution,
  not live menu or physical-controller validation. No saves/settings are used.
- All 44 float-comparison and patch upgrade/tracking tests passed under MSVC;
  the final float-comparison suite also passed independently (17 tests).
- Host: Windows x64 build 26300, MSVC 14.42.34433, SDK 10.0.22621.0,
  Python 3.14.2, pinned upstream `d90c929`, SDL 3.4.16. The supported US XBE
  SHA-256 and full patch stack were verified. `tsfp.ps1 -Action Build` compiled
  Release successfully; network, DDS and display checks passed before its
  prompts test failed at `pc_controls_accept_pad()` because its test window
  did not obtain foreground input. The targeted Windows controls test passed
  separately with disposable save/display paths. Do not count the full build
  script as passing. Evidence: `logs/issue-24-build.log` and
  `logs/issue-24-tests.log`.
- No game process or renderer was exercised; live menu/controller checks and
  native Mac compilation/execution remain pending. Personal settings, saves and
  the tracked input fixture were preserved. See [MAC-016](MAC_PORT_BACKLOG.md#mac-016--shared-pickup-range-comparison-fix-issue-6).

## Nonzero-player mouse look (issue #18) — 2026-10-16

- Implemented in `8dee97f` on `18-mouse-input-only-works-for-player-1`, based on
  `4189924` (matching fetched `origin/main`). The shared camera helper rejected
  every player whose first guest word was nonzero. Removed that restriction
  from `src/pc_camera.h`; current-player selection remains in the existing
  guest wrapper, and memory/FOV validation remains intact.
- Reproduced the failure in the Windows controls regression by setting the
  guest player ID to 1 while using keyboard port 1. Before the fix the camera
  stayed still; afterward 100/20 mouse counts at sensitivity 0.1 produced
  -10/-2 degrees yaw/pitch. The same regression checks camera field selection
  for IDs 0–15, null player/FOV pointers, invalid FOV and single consumption.
- Windows x64, OS build 26300, MSVC 14.42.34433, Windows SDK 10.0.22621.0,
  SDL 3.4.16, Python 3.14.2; pinned upstream `d90c929` and supported US XBE
  SHA-256 verified by `tsfp.ps1 -Action Build`. Full Release build and all
  build-script regressions passed, including controls, settings, prompts,
  display, save routing, texture formats, debug and network checks. Complete
  runtime patch stack validation passed; all 27 patch upgrade/tracking Python
  regressions also passed. Evidence: `logs/issue-18-build.log`.
- These are adapter/model and build checks; no game was launched and no
  rendering backend or live Online/System Link mouse behavior was validated.
  Confirm mouse look on both host and non-host clients in an actual match,
  including controller input and pause/focus transitions. Personal saves and
  settings were preserved; disposable build test data was recycled by the
  supported build script. Native Mac build/execution and other-device
  synchronization remain unverified; see [MAC-001](MAC_PORT_BACKLOG.md#mac-001--direct-camera-mouse-look).

Tested on Windows and Apple Silicon macOS on October 14-15, 2026. This is an experimental port bring-up;
full gameplay and campaign completion have not been verified.

## Mac branch update before publishing — October 15

- Incorporated the newer Mac camera/prompt/multistage-renderer commit `6e48241`
  into the prepared main compatibility merge, preserving both lines of work.
  Shared prompt wrappers use the new guarded guest-memory helpers while still
  allowing the Online text rewrite in controller mode. Network backlog moved to
  `MAC-013` so existing audited `MAC-001` through `MAC-012` keep their identities.
- Windows/shared validation passed: 47 Python/compiler checks, with five
  Mac-specific checks skipped. Recompiled the shared controls, prompt model,
  guest prompt regression and display-menu sources with MSVC x64; linked the
  prompt regression against the existing input/SDL libraries and passed its
  controller/keyboard switching, guest ABI and Online-label checks.
- A full Windows game build and native Apple Silicon execution of this combined
  revision remain pending; prior evidence is retained below. No game or personal
  settings/saves were used for this merge.

## Mac/main merge compatibility — October 15

- Reconciled `mac-port` at `93dc422` with main at `16fcea5` for PR #10.
  The shared display menu retains POSIX memory-query compatibility and the
  Windows Online/System Link entries without linking the Windows host adapter
  into native Mac. Native network wrappers delegate to their original guest
  functions until a Mac transport is implemented; see backlog `MAC-013`.
- Kept existing runtime patch files intact. The updater applies the existing
  network patches before native boot, retaining its adjacent context around the
  SDK block and reversing the same order. Regression tests cover fresh installs,
  repeated builds, older Windows/Mac stacks, CRLF context and local-edit refusal.
- On Windows, 47 selected shared/native-tool/network tests passed; five Mac
  compiler/configuration/Trash checks were skipped. Compiled the affected shared
  display-menu source with MSVC x64, and compiled/executed guest-ABI tests for
  both the host-network route and native fallback without host-network symbols.
- Native Apple Silicon build, GPU/menu and multiplayer execution are not verified
  by this merge check. No game launches or personal settings/save changes were
  required; the existing Windows texture branch and overhaul files were preserved.

## Shared Windows and Mac feature policy/input model — October 15

- Added tracked shared-development rules to `AGENTS.md`: common behavior has one
  implementation wired into both builds; native API work stays in adapters, and
  unimplemented or untested platform support must be recorded explicitly.
  Git synchronization and rebuilding are still required between devices/branches.
- Found and corrected a concrete gap: the Windows diagonal-movement speed fix
  was behind `TSFP_PC_CONTROLS`, so the Mac binding path skipped it. The new
  `zzzzzzz-shared-input-model.patch` compiles the same `src/pc_input.c` on Mac
  and invokes its normalization from the shared binding parser. Windows's
  existing implementation/build path is unchanged. The Mac SDL regression now
  checks W+D speed normalization and release back to full straight-line speed.
- Native manifests now include shared source hashes. README documents the shared
  paths and remaining differences: Windows settings UI, direct-camera mouse path,
  display/texture-pack hooks and separate graphics APIs do not become portable
  merely by compiling the input model. No complete feature-parity claim is made.
- Rebuilt on macOS 27.0.1 ARM64 / Apple M5 Pro, AppleClang 21, pinned SDL 3.4.16
  and OpenGL 4.1 Metal -91.7. Supported US XBE and upstream `d90c929` verified.
  Source base `ea1979a` plus the prior interpolation repair and this change.
  Three native CTests, real-GPU/SDL checks (including diagonal input) and 25
  tool/patch tests pass; two Windows-only tests skipped. No mission run was
  required for this shared binding change. Windows execution and the other
  computer's synchronization remain unverified.
- Evidence: `logs/shared-pc-mac-build.log`, `logs/shared-pc-mac-tests.log`,
  `logs/shared-pc-mac-validation.json` and `build/macos/BUILD.json`.
  Changes remain local and uncommitted on `mac-port`; personal saves/settings
  were untouched, and disposable native regression disks were recycled.

## Native interpolation mesh-layout repair — October 15

- Investigated the user's missing first-mission terrain/character screenshots.
  Found a shared replay-state bug: vertex declarations changed during a guest
  frame, but interpolation retained neither those changes nor the initial
  declarations. Replaying with the final layout could misread vertices or skip
  a draw entirely. Immediate-mode effects temporarily use a sixteen-float4
  layout, then return to their normal stream layout.
- Added `zzzzzz-interpolation-layouts.patch` after the existing patch stack.
  It snapshots active declarations at frame start and copies successful layout
  changes into the replay in order, preserving the final guest state. The native
  executable was rebuilt; temporary culling/depth/CPU diagnostic edits were
  removed. No generated game C was edited.
- Before the fix, a moving first-level guest/midpoint pair showed fire/smoke
  disappearing in the midpoint; logs also reported a skipped vertex declaration.
  After the fix, both frames retain those effects, terrain and the nearby soldier.
  Reviewed further movement/turn captures retain terrain. **The exact complete
  terrain disappearance in the user's photograph was not reproduced, so its
  full resolution remains unverified.** No full-level playthrough was attempted.
- The final 145-second isolated 120-FPS run ended normally at its time limit,
  with no native fault, sampled GL error or skipped-declaration warning.
  Movement/turning used synthetic SDL keys. In seconds 115–135 after the first
  host present, the stationary sample measured **120.00 host FPS**: 1,200 guest
  plus 1,200 interpolated presents (60.00 FPS each); p95 11.47 ms, worst 13.34 ms.
  The moving/capture sample at seconds 95–110 measured 116.81 host FPS
  (59.67 guest + 57.14 interpolated). These are presents, not display scanout.
- Validation: 25 portable/native-tool tests passed, 2 Windows-only tests skipped;
  all three native CTests and real-GPU/SDL regressions passed. The new regression
  compiles the actual shared recorder with a mock renderer and checks layouts at
  draw boundaries, copied payloads, rejected changes and final restoration. It
  fails when declaration recording is deliberately removed. Complete patch-stack
  checks and the read-only native launcher prerequisite check pass. Native Windows
  execution remains untested for this shared recorder change.
- Build base `ea1979a` plus the new patch, upstream `d90c929`, supported US XBE
  verified. Host: macOS 27.0.1 ARM64, Apple M5 Pro, AppleClang 21, SDL 3.4.16,
  OpenGL 4.1 Metal -91.7. Evidence and full hashes are in
  `logs/render-layout-validation.json`, `logs/render-layout-fixed.*`,
  `logs/render-layout-build.log` and `build/macos/BUILD.json`. Selected before/after
  PNGs are `private/captures/native-mac/render-layout-*.png`; redundant diagnostic
  PNGs were removed. Isolated controls were restored and disposable test disks
  recycled. Personal saves/settings were untouched. Changes are local and
  uncommitted on `mac-port`; the other computer's synchronization is unverified.

## CrossOver integration removed — October 15

- Removed the compatibility launcher, Python implementation, dedicated tests
  and setup instructions. The native launcher owns its game-data/settings
  validation directly; it has no dependency on the removed launcher. Shared
  `AGENTS.md` now documents native Mac and Windows workflows.
- Moved the dedicated TimeSplitters compatibility-test bottle and obsolete
  Python caches to Trash. The bottle contained 309.5 MiB of physical allocation
  (305.2 MiB logical); recycling preserves recovery and does not free that space
  until Trash is emptied. The shared application, Windows build, native binary,
  original game data, personal settings and saves were preserved.
- On macOS 27.0.1 ARM64 with Python 3.12, 24 portable tests pass and 2
  Windows-only checks are skipped. Native launcher `--check`, shell syntax and
  diff whitespace checks pass. No game rebuild or gameplay run was needed:
  runtime source/patches are unchanged from native build `024d9be` (AppleClang
  21, SDL 3.4.16, OpenGL 4.1 Metal -91.7). Source base is `024d9be` plus this
  launcher/documentation removal. Evidence: `logs/remove-crossover-tests.log`,
  `logs/remove-crossover-cleanup.json` and `logs/remove-crossover-validation.json`.
- Changes remain local on `mac-port`; synchronization on the other computer
  is unverified. Historical validation logs remain as recovery records.

## Native Mac mouse-look — October 15

- Clicking inside the focused native game window now enables relative
  mouse-look. The capture click is consumed; subsequent clicks use the shared
  fire binding. F1 remains an alternative toggle. Escape and focus loss release
  capture, clear pending look samples and release held mouse buttons. Disabling
  mouse-look also releases capture on the main thread.
- Corrected the vertical sign after a first-level check showed upward mouse
  movement looking down with the default profile. Native input still feeds the
  original right-stick look path, so profile look speed/inversion also applies;
  it does not yet use the Windows direct-camera mouse path.
- The first 150-second isolated run reviewed horizontal/vertical turning,
  capture without an ammunition decrease, and pause/resume/recapture, and exposed
  that vertical-direction bug. The final 135-second run reviewed horizontal
  turning and upward motion looking up after the correction. Additional live
  input and focus changes interrupted its later scripted sequence: the final
  down/pause/resume/fire sequence is **not** counted as a controlled pass.
  Both runs reached their time limit without a native fault or sampled GL error.
  Mouse actions used macOS event delivery; navigation used synthetic SDL keys.
  This does not establish a completed human hardware playtest.
- Real SDL-window regression checks pass for click capture, swallowed capture
  click, subsequent fire, immediate neutralization on release, F1, focus loss,
  disabled mouse-look, sensitivity and inversion. Three native CTests and GPU
  regressions pass; portable checks report 19 passed, 2 Windows-only skipped,
  and 7 subtests. The read-only native launcher check passes. Windows was not run.
- Build base `9b00846` plus these native source changes; pinned runtime
  `d90c929` and supported US XBE verified. Apple M5 Pro, macOS 27.0.1 ARM64,
  AppleClang 21, SDL 3.4.16, OpenGL 4.1 Metal -91.7. Full source/binary hashes,
  test routing and limitations are in `logs/native-mouse-validation.json` and
  `build/macos/BUILD.json`; evidence includes `logs/mac-mouse-*` and selected
  `private/captures/native-mac/mouse-*.png`. Personal saves/configuration were
  absent and remain absent; disposable test disks were recycled. Changes are
  local on `mac-port`; the other computer's synchronization is unverified.

## Native Mac up-to-120 FPS mode — October 15

- Native launches now default to **2x render interpolation**, retaining the
  60 Hz console clock. `--fps 60` / `--fps 120` override only that launch;
  existing `frame_interp = 1` preferences remain respected. Windows behavior
  and personal saves/settings are unchanged.
- Compiled the decoded NV2A vertex programs into cached OpenGL vertex shaders,
  retaining the portable CPU interpreter as fallback/reference. Native game
  C now uses `-O2` with debug symbols and vectorization disabled for the scalar
  MMIO fault handler. Changes to pinned upstream sources are reproduced by
  `zzzzz-mac-high-refresh.patch`; generated game C was not edited.
- Connected the shared draw replay/matrix interpolation with preservation of
  the completed guest image/depth. Cleared intermediate depth separately after
  visual comparison exposed missing fire/smoke, and added a real-GPU regression
  for depth isolation and restoration. Late cinematic frames consume an already
  delivered vblank rather than waiting an extra tick for interpolation.
- The final **165-second** isolated run's stationary first-level canyon window
  (120–145 seconds after the first present) measured **120.00 host FPS**:
  **1,500 guest + 1,500 interpolated presents**, each **60.00 FPS**. Median host
  interval **10.02 ms**, p95 **13.47 ms**, p99 **15.61 ms**, worst **16.82 ms**;
  none exceeded 33.33 ms. Captures were enabled, with explicit paired readback
  outside the measurement window. These are completed host-present intervals,
  not monitor scanout measurements; spacing remains uneven rather than locked
  to 8.33 ms. Background focus during this window is recorded in the evidence;
  a separate foreground input check is also retained.
- A separate 110-second foreground run measured **119.90 host FPS** (60.00
  guest + 59.90 interpolated) in seconds 80–90: median 9.81 ms, p95 11.76 ms,
  worst 16.00 ms. Reviewed fresh captures of movement, the pause menu, resumed
  gameplay and ammunition decreasing after firing. Keyboard events were synthetic
  SDL input; firing used macOS mouse-event delivery, not a human playtest. The
  paired moving frames retain fire/smoke and show different geometry pixels.
- Host: Apple M5 Pro (16 GPU cores), macOS 27.0.1/26A434 ARM64, built-in display
  reporting 120 Hz; scene 640x480, AppleClang 21, pinned SDL 3.4.16,
  OpenGL 4.1 Metal -91.7 / GLSL 4.10. Build base `83491ef` plus this change,
  pinned runtime `d90c929`, and the supported US XBE; full hashes/tool versions
  are in `build/macos/BUILD.json` and `logs/native-120-fps-validation.json`.
- GPU/CPU transform-feedback comparisons cover MAC/ILU operations, paired
  reads, output/temporary aliasing and relative constants. All GPU/input tests
  and three native CTests pass. Portable checks: **19 passed, 2 Windows-only
  checks skipped, 7 subtests**; read-only native launcher check passes. No native
  fault or sampled GL error in the final timed benchmark.
- This is an up-to-120 target, not a campaign-wide guarantee. Cinematics,
  transitions, unmatched draws and insufficient rendering time can hold frames;
  some particles remain at the guest rate. Full-level completion, save/load,
  physical-controller play and complete rendering fidelity remain unverified.
  Changes are local on `mac-port`; the other computer has not been verified.
  Evidence: `logs/mac-120-verified.*`, `logs/mac-120-input-*`,
  `logs/mac-120-build.log`, `logs/mac-120-portable-tests.log`, and selected
  `private/captures/native-mac/native-120-*.png` / `interpolation-*.png`.

## Earlier 120/144 FPS feasibility check (before implementation) — October 15

- Tested `frame_cap = 0` only in the isolated runtime settings, keeping the
  console vblank clock at 60 Hz. The log confirmed the flip gate was off.
  In the same stationary canyon window (120–145 seconds), 1,500 host-present
  intervals still measured **60.00 FPS**, median **16.57 ms**, p95 **18.99 ms**,
  and worst **31.63 ms**. Removing that gate is not a high-refresh solution for
  this build. The 150-second run had no native fault or sampled GL error.
- The current Apple M5 Pro's only reported display is its built-in Color LCD,
  configured at **120.00 Hz**. Displaying 144 distinct frames per second would
  need an appropriate higher-refresh display; 120 is the relevant built-in target.
- Source inspection confirms numeric caps are divisions of the console vblank
  clock, and at that revision the native Mac interpolation entry points were unimplemented.
  Increasing the console clock may change game speed and was not used as a
  substitute for rendering support. The follow-up CPU sample placed 1,193 of
  2,633 main-thread samples in the CPU vertex interpreter. GPU vertex shaders
  and render-only high-refresh presentation were still development work at that
  point. The later 120 FPS implementation is recorded above; 144 is not a target.
- Restored the isolated runtime settings exactly and recycled disposable test
  images. Personal saves/settings were not used. No runtime source or binary
  changed; only findings/documentation were updated. Evidence:
  `logs/native-higher-fps-validation.json`, `logs/mac-perf-uncapped.csv`,
  `logs/mac-perf-uncapped-boot.json`, and `logs/mac-perf-uncapped-sample.txt`.

## Native frame-time investigation and upload fix — October 15

- Confirmed the reported slowdown on `mac-port`. Measured host-present intervals
  in the stationary first-level canyon, 120–145 seconds after the first present,
  excluding menus, loading, the opening animation and CPU sampling. Three bounded
  150-second runs used the same isolated profile and menu input sequence.

  | Run | Average FPS | Median frame | 95th percentile | Worst frame |
  | --- | ---: | ---: | ---: | ---: |
  | Before, rotating captures enabled | 14.63 | 68.34 ms | 71.61 ms | 80.61 ms |
  | Before, captures disabled | 14.85 | 67.28 ms | 70.48 ms | 74.60 ms |
  | After, rotating captures enabled | 60.00 | 16.44 ms | 19.09 ms | 33.80 ms |

- Captures were not the main cause. CPU sampling placed 1,442 of 1,942 main-thread
  samples in `glBufferSubData`, including driver waits for earlier GPU work.
  The renderer reused offset zero in one transient vertex/index buffer on every
  draw. Replacing each buffer's storage with `glBufferData` lets the driver retain
  earlier uploads while the GPU finishes them. The follow-up sample has no
  `glBufferSubData` calls. CPU vertex interpretation remains a performance limit.
- Compiler settings stayed unchanged to isolate this fix: Debug game code and
  the runtime libraries' existing `-O2`. `BUILD.json` now records the build type
  and complete configure command. The result is specific to this scene and Mac;
  it does not establish 60 FPS throughout the campaign or monitor scanout pacing.
- Added opt-in `--profile-frames` (20,000 CSV rows maximum, no GPU readback) and
  a frame-statistics tool that selects a known gameplay interval. The corrected
  1,500-frame measurement had one interval over 33.33 ms and none over 50 ms.
- Added real-GPU coverage for 128 queued, changing vertex/index uploads before
  readback, including nonindexed draws and both 16/32-bit indices. All graphics,
  input and three native CTests pass; portable tests: **17 passed, 2 Windows-only
  checks skipped, 7 subtests**. The read-only native launcher check passes.
  The final timed run reached at least 7,800 guest frames and host presents,
  with no native fault or sampled GL error. Reviewed the world/weapons/HUD capture.
- Test partition images were recycled; personal saves/settings were not used.
  Evidence: `logs/native-frame-performance-validation.json`,
  `logs/mac-perf-*.csv`, `logs/mac-perf-*-sample.txt`,
  `logs/mac-frame-fixed-build.log`, `logs/mac-frame-portable-tests.log`, and
  `private/captures/native-mac/native-frame-performance.png`.

## Focused native combat smoke test — October 15

- Continued on `mac-port` with the same native binary and isolated test profile.
  Traversed the initial canyon beyond the spawn area, reached the first enemy
  encounter, fired both handguns, observed ammunition consumption and enemies
  falling, and saw the accompanying marines advance. The world remained visible.
  This was an automated smoke test, not a human playthrough.
- Used temporary I/J/K/L aim and O fire bindings in the isolated test configuration
  for reproducible SDL input. Restored the fixture afterward. Personal saves and
  settings were not used or created. During inspection, waited for a fresh
  rotating frame; stale captures had made some turns appear inconsistent.
- The user narrowed the scope before full-level completion. The test was stopped
  intentionally with native exit 0; the runner reports an early `process_exit`
  rather than a passed timed probe. No new gameplay/rendering blocker was confirmed
  in this coverage. The 477-second session reached at least 8,400 guest frames
  and host presents, with no native fault or sampled GL error.
  Full-level completion, checkpoint reload, campaign save/load,
  audio fidelity and physical-controller play remain unverified.
- Extended the runner's explicit duration ceiling to 30 minutes, retaining its
  15-second default, isolated paths, two rotating captures and 8 MiB log limit.
  Portable regressions: **16 passed, 2 Windows-only checks skipped, 7 subtests**.
  No renderer/runtime source changed, so the native binary was not rebuilt.
- Evidence: `logs/native-combat-validation.json` and
  `private/captures/native-mac/first-combat-smoke.png`. Disposable partition images
  were recycled after the run.

## Visible native first-level gameplay — October 15

- Fixed the black world on `mac-port`. A per-draw GPU trace showed the world
  rendered correctly, then a later clear erased it. The game switches to a
  **160×120 offscreen texture** for that pass; the Mac backend previously returned
  no surface from `GetSurfaceLevel` and ignored `SetRenderTarget`, so the clear
  landed on the visible back buffer.
- Added real texture surfaces, OpenGL framebuffers and depth/stencil attachments,
  reference-counted target ownership, a persistent device depth surface, target
  viewport reset, rectangle/viewport clear clipping, back-buffer readback while
  an offscreen target is bound, and correct texture orientation on sampling.
  Failed/incomplete target changes preserve the prior target. These changes are
  confined to the native Mac backend; shared upstream patches and generated
  game C are unchanged.
- Reviewed actual first-level captures after the opening animation: canyon,
  enemies, dual pistols, crosshair and ammunition HUD stay visible. Forward
  input changed the viewpoint; firing reduced both pistols from **16 to 15**.
  Captured OS mouse deltas turned the view from the canyon toward a rock wall.
  Pause/resume returned to the visible world. These are bounded checks using
  SDL test input and macOS CoreGraphics events, not a human full-level playthrough.
- Added GPU regressions for offscreen clear isolation, top/bottom sampling,
  rectangle clears and preserving the old target after an incomplete attachment.
  All GPU/input checks and three native CTests pass. Portable checks:
  **16 passed, 2 Windows-only checks skipped, 7 subtests passed**. Windows
  build/runtime checks remain unrun on this host.
- Added `Play-Native-Mac.command` for interactive play with the native binary.
  It verifies ARM64/XBE/build hashes, preserves personal settings, strips stale
  test overrides and bounds logs to 8 MiB total. Its read-only readiness check
  passes. The interactive launcher was not used for automated game testing;
  personal save/settings paths remain untouched.
- The 120-second exact-binary gameplay probe completed at least **2,700**
  guest frames and host presents, with no native fault or sampled GL error.
  Its last render-target summary recorded **16,206 switches, zero failures**.
  Disposable test images were recycled; selected captures stay under `private/`.
- Evidence: `logs/mac-world-build-and-tests.log`, `logs/mac-world-portable-tests.log`,
  `logs/native-world-validation.json`, `logs/native-fixed-world-draw.txt`, and
  `private/captures/native-mac/world-playtest.png` / `world-resumed.png`.
- Remaining limits: full-level completion, campaign save/load, audio fidelity,
  physical controllers and complete renderer fidelity are unverified. The
  vertex programs still execute on the CPU; general multitexturing, cube/depth
  textures and screen-copy effects need further work. This is a first-level
  gameplay milestone, not a finished Mac release.

## Native Mac controls and first story-level load — October 15

- Added a Mac-only SDL event bridge to the shared JSON binding reader through
  `zzzz-mac-input.patch`. Keyboard/mouse state is published atomically for guest
  worker polls. Main-thread Cocoa events handle focus loss, mouse capture and
  window close. A new launch raises the game window once; it does not steal
  focus back during play. Windows control code and generated game C are unchanged.
- Confirmed the real menu flow: player selection, isolated profile creation,
  storage selection, main menu, Story, **2401: Time to Split**, difficulty,
  briefing and loading. Up/down changed selection. The isolated profile survived
  a new process and was selected again. This is profile persistence only, not
  campaign save/load validation.
- SDL event injection exercised the binding-to-XInput path. Separately, macOS
  CoreGraphics keyboard events delivered through the OS opened the in-level
  pause menu with Escape, moved its selection with Down and resumed with Escape.
  This verifies OS event delivery, not a human keyboard/controller playtest.
- The first level rendered its opening 3D scene and reached an ammunition HUD,
  but the gameplay world became black. No full playability, movement/collision,
  combat or rendering-fidelity claim is made. Renderer gaps remain the immediate
  blocker after menu navigation and level startup.
- F1 toggles relative mouse capture; Escape/focus loss releases it. The shared
  mouse enabled/sensitivity/invert settings are read at startup. The first free
  SDL controller shares keyboard player 1 without taking pads assigned to other
  players. Physical mouse look and hardware-controller play remain unverified.
- Native regression checks pass for key press/release, arrow/WASD bindings,
  focus neutralization, mouse buttons/deltas/settings and SDL virtual-controller
  hotplug/sharing. All three CTests and the real-GPU regression pass. Portable
  checks: **13 passed, 2 Windows-only checks skipped, 7 subtests passed**.
  Windows build/runtime tests were not run on this Mac.
- Repeated bounded 120-second menu/level probes completed without a native fault
  or sampled GL error. The final focused 100-second run logged at least 1,800
  guest frames and host presents and again reached the level. Window-close
  handling now exits cleanly at the probe's
  time limit (exit 0), without the former forced kill. Each probe uses
  `private/test-saves/native-mac`; known disposable partition images are recycled.
  Personal saves and settings remain unused and unchanged.
- Evidence: `logs/mac-input-build-and-tests.log`, `logs/mac-input-patch-tests.log`,
  `logs/native-input-validation.json`, `logs/native-level-summary.json`,
  `logs/native-boot-summary.json`,
  `private/captures/native-mac/menu-navigation.png` and
  `private/captures/native-mac/native-pause-navigation.png`,
  `private/captures/native-mac/first-native-level.png` and
  `private/captures/native-mac/native-gameplay-hud.png`.

## First visible native Mac menu — October 15

- On `mac-port`, connected the existing shared HLE graphics hooks to an Apple
  Silicon SDL3/Cocoa OpenGL 4.1 backend. The driver reports **GL 4.1 Metal - 91.7,
  GLSL 4.10**. The executable is native ARM64 Mach-O, with no Wine/Rosetta process.
- Visually reviewed actual GPU back-buffer captures: the TimeSplitters logo,
  animated menu background, **1 PLAYER / 2 PLAYER / 3 PLAYER / 4 PLAYER**, and
  **A Select** are readable. Repeated bounded runs reached this menu without a
  native fault. The final 45-second probe logged at least 2,100 guest frames
  and host presents, with no sampled GL errors; termination at the limit needed
  SIGKILL after the three-second grace period. This establishes the first visible
  menu; no input was injected,
  and menu navigation or gameplay is not claimed.
- Reused the shared vertex declarations, guest state mapping and texture cache.
  Added CPU execution of NV2A vertex programs with screen-space conversion,
  GLSL register-combiner evaluation, quad triangulation, Xbox texture unswizzling,
  DXT and A8 font texture uploads, and GPU readback. The loading worker exposed
  a missing current-context fault; a recursive lock now hands the context between
  guest threads. Cocoa window creation/event pumping remain on the main thread.
- Added a separate reproducible `zzz-mac-renderer.patch`; prior patches and
  generated game code remain unchanged. The Mac backend is `src/native/renderer_gl.c`;
  the existing Windows and Linux backend source selection remains intact.
  Shared portable format helpers compile from the pinned upstream source.
- Real GPU regression checks pass for quad coverage, A8 unswizzling and alpha
  blending, a synthetic NV2A vertex program, per-stage combiner constants,
  readback, and worker-thread context handoff. The three native CTest suites
  pass. Portable patch/launcher/isolation checks: 13 passed, 2 Windows-only
  PowerShell checks skipped, 7 subtests passed. The Windows executable was not rebuilt or tested.
- The probe supports `--capture`, with two rotating BMPs, isolated save/settings
  paths, bounded run/log size and validated partition-image recycling. Its JSON
  separates guest frames, sampled successful host presents, sampled GL errors,
  and capture hashes; visual menu recognition is recorded separately. Personal
  saves/settings remain unused. Project physical allocation remains about 8.5 GiB,
  with a 258 MiB native build and about 2 MiB of selected/rotating captures.
- Evidence: `private/captures/native-mac/first-native-menu.png`,
  `logs/native-boot-summary.json`, `logs/native-renderer-validation.json`,
  `logs/mac-renderer-build-and-tests.log`, and `logs/mac-renderer-patch-tests.log`.
- Remaining renderer gaps include general multi-texturing, offscreen render
  targets, screen-copy effects, movie playback and full 3D fidelity. Native
  controls/navigation, audible quality/latency, PC display/texture features,
  gameplay, fullscreen and save/load are not verified. Vertex programs currently
  execute on the CPU; a menu result is not a performance claim for gameplay.

## First native Apple Silicon execution — October 15

- Built `build/macos/futureperfect_recomp` as a native ARM64 Mach-O executable
  on macOS 27.0.1 with Apple Clang 21, Python 3.12 and the shared pinned runtime,
  Python dependencies and SDL submodule. No Wine, Rosetta or Windows process
  participates in the native runs. The verified US XBE was regenerated locally.
- A bounded 30-second run loaded all 23 XBE sections, resolved all 151 kernel
  imports, initialized the game's graphics device successfully, loaded game
  assets, ran worker threads and completed at least 1,800 guest frame calls.
  No native fault was recorded. The runner stopped it at the time limit;
  SIGTERM did not exit within the three-second grace period, so it used SIGKILL.
  This is **game-code execution without a connected host renderer**: no menu
  image, game window, gameplay, physical controls or save/load was verified.
- The first attempt exposed missing related address mappings. The Mac now
  reserves one 4 GiB virtual range and maps only its own unused pages; all 28
  low-RAM aliases, contiguous memory, device apertures and tiled alias mapped.
  It does not weaken macOS protections or overwrite unrelated host mappings.
- Implemented native relative/absolute and periodic waitable timers, and an
  ARM64 signal handler for supported scalar MMIO loads/stores. Protected NV2A
  interrupt writes and AC97 reset writes have real-signal regression coverage;
  unsupported instructions stop with diagnostics instead of being skipped.
  Read/write aliases let handlers update device backing without opening the
  protected guest page to concurrent writes. APU accesses use the existing
  device model, whose DSP implementation remains incomplete upstream.
- Implemented POSIX sparse partition images with the existing retail geometry,
  partition-device path routing, volume-directory opens and relative file opens.
  Existing image contents are preserved. These fixed the guest's early request
  to return to the Xbox dashboard. Isolated title metadata was created under
  `private/test-saves/native-mac`; personal saves were not used.
- Added an SDL3 PCM output backend and reused the portable controller-binding
  path. SDL audio output opened and PCM submissions occurred. Audible quality,
  latency and physical input remain unverified; audio uses the existing
  wall-clock fallback rather than claiming a hardware playback cursor. Movies
  are skipped because the native movie/renderer path is not connected.
- The initial graphics probe exposed an additional upstream settings fallback
  at `~/.config/xboxrecomp/titles/45410066.conf`. Its newly created default was
  moved into the isolated test directory. The probe now explicitly routes
  `RECOMP_DISPLAY_CONFIG`, `TSFP_DISPLAY_CONFIG`, input, saves and XDG roots;
  regression tests cover stale override removal and protection of personal paths.
- Three native CTest suites pass: mapping/threads/events/timers, ARM64 MMIO,
  and save/partition routing. Portable checks: 12 passed, 2 Windows-only
  PowerShell tests skipped, 7 subtests passed. Fresh application, upgrades,
  CRLF inputs, conflicts and repeat application of the complete patch stack
  pass. The actual Windows executable was not rebuilt or tested on this Mac.
- The shared rules now distinguish the retained Windows compatibility build
  from one active native build in `build/macos`. Native build provenance is
  recorded in its ignored `BUILD.json`; runtime changes are reproducible from
  `patches/zz-mac-native-boot.patch`, applied after the existing feature stack.
- Native tests recycled their known disposable partition images. The game
  probe's six images had 5,243,406,848 logical bytes but only 135,168 allocated
  bytes; the images are no longer in the project. Metadata/configuration and
  diagnostic logs remain. Physical project allocation is approximately 8.5 GiB,
  including the original ISO, the single game extract, dependencies and builds;
  `build/macos` is approximately 256 MiB.
- Evidence: `logs/native-boot.out`, `logs/native-boot.err`,
  `logs/native-boot-summary.json`, `logs/mac-native-build-and-tests.log`,
  and `logs/mac-native-patch-tests.log`. Earlier feasibility notes below are
  historical and are superseded by this native execution result.

### Next native milestone

The first-menu renderer connection is complete (see the newer result above).
Validate physical input and menu navigation, then extend rendering support for
an in-game scene. Movie presentation, PC settings/display/texture features and
save/load remain separate milestones. The earlier execution-only observations
in this section describe the state before the renderer connection.

## Native Mac feasibility and ISO preparation — October 15

- Work is on `mac-port`; the user requires changes from this chat to stay on
  that branch unless explicitly directed to `main`.
- Validated the supplied 4,047,372,288-byte ISO's embedded executable against
  the supported US SHA-256. Moved the original into ignored `private/discs`
  and extracted one copy: 412 files, 4,046,110,016 bytes, in `private/game`.
  Extraction paths/sizes were checked and the extracted executable rehashed.
  `private/disc.json` and `logs/iso-verification.json` record the checks.
- Game data is now available locally; no Mac gameplay validation was performed
  in this ISO preparation pass.
- Checked out the exact pinned runtime revision under `external/ts3FPrecomp`.
  Found a reproducible patch-installation failure on its CRLF CMake source.
  Made patch context whitespace handling explicit, avoiding reliance on the
  Windows machine's Git settings. The complete patch stack now applies and a
  second pass changes zero files. Conflict protection and CRLF tests pass.
- Added `mac-platform.patch`: the upstream Apple anonymous-mapping function
  returned descriptor 0 instead of creating a backing file. The native test
  failed before this fix and passes afterward. The patch uses an owned file
  descriptor, leaves no named backing file, and is restricted to `__APPLE__`.
- Compiled and ran an actual ARM64 Mach-O test with Apple Clang 21 on macOS
  27.0.1. It verifies 64 MiB aliased mappings, bidirectional data coherence,
  non-destructive collision rejection, a worker thread and event signaling.
  This runs natively. The initial low-address-linker
  experiment was terminated before useful output; the passing test uses the
  normal executable layout and OS-selected addresses. It does not establish
  that the full Xbox address-space mapping is ready.
- The native path-routing source passes a Clang syntax check. The memory-layout
  and kernel-bridge sources still fail on Windows process-affinity/timer APIs.
  Further source review found Windows-only title startup/graphics hooks,
  missing POSIX HLE audio/input implementations, an incomplete OpenGL renderer,
  a Windows-wired Vulkan backend, and Linux `.so` names in the non-Windows
  FFmpeg loader. Native game boot, rendering and gameplay are **not built or
  verified**. No platform support claim is inferred from the upstream README.
- Portable checks: 8 passed, 2 Windows-only PowerShell tests skipped, 7 subtests
  passed, using Python 3.12 and pinned pytest 9.1.1. Native evidence is in
  `logs/native-platform-before.*`, `logs/native-platform-after.*`, and
  `logs/native-kernel-compile-summary.json`. Personal saves and settings were
  not used. No disposable virtual disk images were created.

## Display settings and texture preview — October 15

- The working build now adds **Display settings** through the US game's own
  button builder in main Settings and paused Options. Resolution selection,
  Apply, Keep and Revert all use guest menu widgets and navigation.
- 1080p, 1440p, 4K and monitor-reported modes are offered. Rendering targets and
  depth buffers resize live; a 15-second unconfirmed preview returns to the old
  resolution. Confirmed settings persist beside the PC input configuration.
- The US camera wrapper updates the perspective matrix and horizontal CPU
  frustum together. The HUD retains its proportions in a centered 4:3 region.
  Front-end menus retain the original layout. Fullscreen remains Alt+Enter.
- The actual-menu replay verified 1080p apply/save, 1440p preview, timeout
  rollback, and return to the original menu. Native tests cover resolution
  bounds, monitor-mode deduplication, failed allocation handling at the API
  boundary, aspect calculations and persistence. Original game files, saves
  and keyboard/mouse configuration passed 424 protected-file hash comparisons.
- First-mission runs rendered 3840×2160 and 3440×1440 scene buffers and reached
  paused Options without a recorded crash or unresolved indirect call. The
  resolution checks use GPU render targets on the available display; a physical
  ultrawide monitor, split screen, and a full campaign are not validated.
- The expanded texture preview contains 11 replacements, approximately 44 MiB,
  covering selected terrain, masonry, timber, concrete, floor tiles, metal panels
  and one weapon atlas. Four replacements loaded in a 174-second opening-mission
  run at 1920×1080; gameplay and paused Options were reached without a recorded
  crash or unresolved call. The remaining seven need scene-specific runtime checks.
- DDS files have complete mip chains: two 4096×4096, eight 2048×2048, and one
  2048×1024. Square source art is 1254×1254; stone masonry is 1774×887. These are
  resampled outputs, not native 4K generated art. A character draft shifted facial
  and ear UV details and was kept out of the active pack.
- The coverage audit tracks 5,547 supported texture keys: 11 installed, one draft
  requiring correction/review, and 5,535 awaiting art. Another 753 cataloged
  payloads use unsupported formats. Embedded textures may remain undiscovered.
  This is not a completed game remaster. Eight offline tool tests pass; installed
  DDS hashes, dimensions and mip chains pass the audit. All 427 protected game,
  save and settings files matched their pre-test hashes; temporary disks and
  frame captures were recycled. Exact new prompts and real source sizes are saved.
- Pack compilation enforces the user's 6 GiB cap including metadata. Runtime
  replacement residency remains capped at 512 MiB. The supported-asset size plan
  is approximately 2.5 GiB before source art and unsupported-format additions;
  the plan is not completed artwork or a guarantee of final full-game size.
- Runtime changes are reproducible from the pinned upstream revision using
  `pc-controls.patch`, `save-routing.patch`, and `texture-pack.patch`; the last
  patch includes the renderer wiring shared by display and texture features.
  Fresh application in setup order and repeat-build checks pass.

## Completed

- Online source-build integration: a new network-menu entry, HTTPS browser with
  certificate fingerprint checking, public/private rooms, compatible-only
  listing/joining, peer credentials and leases, and a virtual-address bridge
  using the game host's forwarded UDP port. Guests establish outbound paths;
  the host relays guest-to-guest traffic. Two local instances used Online and
  an HTTPS directory to join a 2/16-player lobby and enter the same Shrink match.
  Cross-internet routing and full-match stability remain unverified. Campaign
  co-op remains a separate future effort.
  Automatic hosting now detects public IPv4 over HTTPS, starts/stops a local
  directory with the bridge, reuses its certificate and offers a copyable
  invitation containing the directory address, fingerprint and join code.

- Experimental PC System Link: native framed UDP, strict build/content identity
  checks, default-protocol discovery sockets, and XNADDR/key mapping. Two isolated
  game instances discovered a match, joined a 2/16-player lobby, and loaded into
  the same Thief match. This is a same-PC loopback test, not a cross-network test
  or a completed-match validation. See [multiplayer status](docs/multiplayer.md)
  for the later Online integration and remaining cross-network validation.

- Read the supplied desktop Xbox ISO without modifying it; extracted 412 files.
- Confirmed the US executable, title ID `45410066`, entry point `0038024D`,
  XDK 5849 and the expected SHA-256 documented in `README.md`.
- Identified 379 Xbox library symbols, including six upstream manual symbols.
- Generated 15,838 C function bodies with zero reported translation failures,
  totaling 2,507,843 lines across 16 chunks plus support files.
- Built `build/windows/Release/futureperfect_recomp.exe` with Visual Studio
  2022 / MSVC 19.44.35224 for Windows x64.
- Compiled and loaded FFmpeg 8.1 movie support.
- Fixed native profile device paths that bypassed `RECOMP_SAVE_DIR`, and
  preserved the fix in `patches/save-routing.patch`. Ten native path regression
  checks pass, including asset routing and switching between save directories.
- Added player-1 keyboard/mouse controls: normalized WASD movement, Windows raw
  relative mouse motion, direct camera-angle updates, mouse buttons and wheel,
  editable sensitivity/invert-Y settings, focus-loss clearing and cursor
  capture/release. The launcher loads `config/pc-controls.json`.
- Preserved the host integration and camera wrapper in `patches/pc-controls.patch`;
  the generated original `sub_000D2B80` body remains available as `_gen`.

## Validation

- Upstream Python suite: 547 passed, 41 skipped, 87 subtests passed.
- Upstream unittest discovery: 289 tests passed, including 26 skipped.
  This overlaps the pytest suite and is not an additional independent total.
- CPU differential conformance: 5,741 instruction vectors and 211 function
  vectors, zero mismatches.
- First 35-second startup test: decoded the EA and Free Radical intro movies
  (88 and 81 pictures), submitted PCM movie sound, reached the rendered
  player-count menu, and remained running until the test stopped it.
  No crash or unresolved indirect call was logged. Frame captures were
  inspected, including the player-count menu.
- Second 35-second input test: scripted A presses reached profile name entry;
  the captured name-entry screen was inspected.
- A subsequent 70-second test reached Story difficulty selection. It also
  exposed profile contamination through the original runtime's save-path bug.
  Earlier test profiles are preserved in `private/pre-fix-test-saves`.
- After fixing save routing, a fresh 100-second run created its profile under
  the requested test save directory, reached the first mission briefing, loaded
  `2401: Time to Split`, and rendered its opening cutscene and the landing scene.
  No crash or unresolved indirect call was logged; 3,578 function entries were
  observed. These scenes were inspected in captured frames.
- A 260-second fresh-save run reached first-person gameplay in the first mission
  and remained running until the test's time limit, with no logged crash or
  unresolved indirect call. It reached 3,855 instrumented functions.
  Scripted movement/look input changed the viewpoint, and trigger input reduced
  the displayed dual-weapon ammunition from 16 rounds per weapon to 14 and 13.
  Level geometry, textures, weapons, crosshair and ammunition display were
  inspected in the captured frames. This is a short gameplay check, not a
  completed mission or campaign playthrough.
- The final incremental build and native save-path tests also passed through
  Windows PowerShell 5.1, the same shell used by `Play.cmd`.
- Native PC input tests cover input clearing on focus/capture changes, consuming
  a recapture click, wheel press/release sequencing, equal mouse counts across
  different polling intervals, FOV scaling, vertical limits, inverted Y,
  normalized diagonal movement, and the actual bindings/guest-camera adapter.
- A 155-second run driven by the PC input model navigated menus with Enter,
  reached first-person gameplay, moved with W, applied raw-count camera changes,
  fired/reloaded, and switched weapons with the wheel. It ended at the test
  limit without a logged crash or unresolved indirect call. This used the
  adapter's deterministic event replay, not physical desktop input injection.
- A fresh-checkout fixture verifies that both local patches apply in Setup's
  order and reproduce all seven modified upstream files.
- A second 150-second run used the corrected aim/crouch bindings and reached
  3,874 instrumented functions with no logged crash or unresolved indirect call.
  Right-mouse aiming reduced the FOV from 55 to 30.25; a 20-count mouse event
  then changed yaw by 1.1 degrees, confirming the expected 0.55 zoom multiplier.
- The final build, both native test suites, and a six-second launcher startup
  test passed through Windows PowerShell 5.1 after the cursor activation fix.

Test output is in `logs/toolchain-tests.log`, `logs/pytest.log`,
`logs/conformance.log`, `logs/build.log`, and the timestamped smoke-test logs.
`logs/gameplay-smoke-summary.log` records the 260-second run;
`logs/final-build-check.log` records the final build and path checks.
PC input work is recorded in `logs/pc-final-bindings-build.log`,
`logs/pc-gameplay-summary.log`, and the newer `logs/pc-final-gameplay-summary.log`.
The final delivered build checks are `logs/pc-release-validation.log` and
`logs/pc-launcher-smoke.log`.
Original graphics captured by tests remain local in `private/captures`.

## Remaining work

- Verify mission progression, combat against enemies, control fidelity,
  profile reloads/checkpoints and sustained play beyond the short first-mission
  test. File creation and save isolation have been tested; a complete save/load
  gameplay round trip has not.
- Investigate translation coverage where execution requires it: the generator
  reports 50 unresolved direct-call targets emitted as diagnostic stubs, plus
  730 unsupported instruction occurrences emitted as comments/no-ops across
  30 mnemonics. Zero translation failures does not imply equivalent behavior.
- Check rendering and audio fidelity against the original game. Captured menu
  frames and host audio submissions are narrower evidence than a full playtest.
- Keyboard/mouse support currently targets player 1's first-person gameplay.
  Vehicle aiming, split-screen, MapMaker mouse navigation and extended hardware
  playtests remain unverified. Focus/capture behavior has automated state and
  activation-policy checks; no physical mouse/Alt-Tab playtest was performed.
- Aim/crouch toggle behavior follows the original game's profile. Mouse
  sensitivity/inversion is controlled by the PC configuration, independently
  of controller turn speed. The supplied PC profile selects keyboard input;
  controller input requires a controller binding configuration.
- Multiplayer and online behavior have not been tested here.

The generated C is a machine-code translation, not recovered original source.
Runtime code is pinned upstream; no claim of a completed decompilation or a
finished PC release is made.

## Interactive launcher correction (2026-10-15)

Normal play now launches through `tools/play.py`, which waits for the game and
writes its output directly to `logs/play-*.out` and `logs/play-*.err` using native
file handles. Interactive runs disable the generated code's default 400,000-line
function trace; diagnostic profiling remains available through SmokeTest.
The previous PowerShell GUI launch could return before the game, and inherited
or redirected output could stall startup. The corrected normal launch produced
captured startup logos and advancing swaps, with keyboard/mouse configuration
loaded. The user also confirmed that the logos appeared. Main-menu and gameplay
interaction for this hardware playtest is still to be tried.

## PC controls menu (2026-10-15)

Added a PC Settings → Keyboard & Mouse entry to the native game window's menu
bar, with F2 access in windowed and fullscreen modes. The owned settings dialog
lists all 24 Xbox actions with four keyboard/mouse binding slots each, mouse
sensitivity, inversion, mouse enable/disable, defaults, and Cancel. Capture
supports keys, five mouse buttons, and wheel directions; conflicting assignments
can be moved after confirmation. F1/F2/F9–F11 and Windows keys are reserved.
Gameplay input is blocked while editing, but simulation is not paused: pause
the mission first.

Saving applies bindings and mouse options immediately. Polls copy an immutable
binding snapshot under an SRW lock; editor input cannot become gameplay input.
Configuration changes preserve other ports and unrelated JSON members. Writes
use a temporary sibling file and atomic replacement, retaining the previous
configuration as `.bak`; failed writes retain the active bindings. Build checks
use a separate default fixture so user-customized bindings do not affect tests.

Validation: native input and save-path suites passed, as did the new settings
suite for live sampling, persistence and fresh-process reload, backup, unrelated
JSON retention, read-only failure, invalid values/reserved keys, defaults, native
dialog Cancel, key/wheel capture, and conflict reassignment. The UI was rendered
and visually inspected. A bounded real-game integration check verified both
entry points, F2 in fullscreen, input ownership/return, rendering after closing,
and that Cancel left the user's file unchanged. Both patches reproduced all
seven modified upstream files from pristine pinned sources. Evidence is in
`logs/pc-settings-release-validation.log` and `logs/pc-settings-game-tests.log`.

## In-game menu integration reverted (2026-10-15)

At the user's request, removed both the original-menu entry and the subsequent
game-rendered editing pages. Restored the original Xbox Settings/Options menus.
Keyboard/mouse gameplay controls, the separate F2 / PC Settings editor, and
user bindings/saves remain. Integration source is archived under
`private/rollback-in-game-menu`; generated code and the executable are rebuilt
without the guest menu wrapper or callback hooks.

Rollback validation: regeneration and Release build succeeded; save-path, PC
input/camera, and settings suites passed. A real-game UI check verified F2,
the PC Settings menu, and F2 in fullscreen; closing the editor returned input
and rendering to the game. The user configuration hash is unchanged. Logs:
`logs/revert-menu-build.log`, `logs/revert-menu-settings-ui.log`.

## Native Mac settings and graphics adapters (2026-10-15)

Local, uncommitted work on `mac-port` over `ea1979a4b6279045d0dc9c7c19472cb75e8a2216`.
Main and the other device were not changed; Windows build and cross-device
synchronization remain unverified. The native executable and every source hash
match `build/macos/BUILD.json` (executable SHA-256
`b6344c2b09bd415905d8675c05947a99f2063a069f5a361764d7954dce47b8ac`).
Host: Apple Silicon macOS 27.0.1, Apple Clang 21.0.0, SDL 3.4.16,
OpenGL 4.1 Metal 91.7 / GLSL 4.10. Shared upstream and supported XBE pins unchanged.

- F2 and the Mac Game Settings menu open a Cocoa Keyboard & Mouse / Graphics
  editor. Bindings, validation, defaults, unknown JSON preservation and display
  state use the PC implementation; POSIX atomic writes retain a controls backup.
  Sensitivity/inversion and bindings apply live. Conflicting keys are rejected
  with the two action names; Mac does not yet offer Windows' move-binding dialog.
  Closing the editor releases its input and raises the SDL game window.
- Native render targets support the shared resolution list, high-resolution
  scenes, widened projection/CPU frustum, and screen-position HUD classification.
  F11 toggles fullscreen; fullscreen is a session preference. The original
  Settings/Options menu's existing Display Settings integration is compiled on
  Mac too. Its 15-second preview logic is shared; the page's timeout interaction
  has not yet been exercised end-to-end on Mac.
- OpenGL replacements use the same texture keys and DDS loader/acceptance rules
  as Windows: full mip chains, BC1/2/3, aspect/no-downscale checks, 512 MiB resident
  replacement budget, and original-texture fallback. Launch with `--texture-pack`.
  Synthetic replacements were sampled on the real GPU and released correctly;
  this is not verification of a complete remaster pack.
- Integration found and fixed nested native settings initialization blocking
  input startup, and Cocoa presenting black while an offscreen FBO was bound.
  Both guest and interpolated swaps now bind the window drawable during swap.
  The GPU regression checks the drawable as well as guest readback.

Validation: complete patch-stack fresh/repeated/historical-upgrade tests;
28 Python tests passed, 2 Windows-only tests skipped; three native CTests passed;
real-GPU renderer/vertex/texture/resolution/offscreen/interpolation tests,
SDL input and shared settings tests passed. Settings checks cover live binding
sampling, exact backup, unrelated members/other controllers, invalid/reserved
values, symlink/read-only refusal and preserving active state on failed writes.
Standalone shared DDS-format tests and the interactive launcher's read-only
prerequisite check passed. Native MSVC/Direct3D tests cannot run on this host.

A bounded 130.15-second isolated game run at 1920x1080 with the 120-present target
reached the first mission and performed short movement checks. Reviewed both
rotating guest captures and an actual macOS window screenshot showing the world,
weapons, NPC, fire and HUD; full campaign completion was not attempted. At least
6,900 completed guest frames and 10,011 host presents were logged, with no sampled
GL errors or native fault. The 90–110-second movement interval measured 86.84
host presents/s: 56.17 guest + 30.66 interpolated, p95 19.32 ms, p99 31.48 ms.
Captures were enabled; these are host intervals, not display scanout or a broad
performance benchmark. The 60 Hz guest clock and personal FPS preferences remain
unchanged. The earlier pre-fix 1080p run established offscreen rendering only;
use the final run for on-screen evidence.

Separate bounded UI checks visually verified windowed/fullscreen menus,
resolution save/reload at 1080p, F2, controls save/reopen, Cancel, and keyboard
navigation after closing Settings. All tests used isolated saves/settings;
personal controls retain SHA-256
`cf44293910a77fb8f397ced933a1cfd5098e1949c93db9cc0571ed438b39f64a` and the absent
personal display file remains absent. Disposable partition images were recycled.
Evidence: `logs/mac-features-build.log`, `logs/mac-features-pytest.log`,
`logs/mac-features-final.json`, `logs/mac-features-ui.json`,
`logs/mac-features-final-frame-times.csv` and selected captures under
`private/captures/native-mac/mac-features-*` / `mac-settings-*`.

Remaining: general multi-texturing, movies/screen-copy effects and exhaustive
HUD placement/rendering fidelity; native direct-camera mouse behavior and PC
prompt/debug features are not claimed as equivalent. This work does not establish
that the previously reported world-disappearance case is completely resolved.


## Mac backlog renderer, camera and prompts (2026-10-15)

Validation performed before commit on `mac-port`, based on
`93dc422139bae4acb64d1e24c528cf3869a4ce20` plus the changes accompanying this
record. The other computer was not accessible for verification. Pinned upstream remains `d90c92988a9f99d0580d2ca3f7efd6ec3a7b7845`.
Host: macOS 27.0.1 arm64, Apple M5 Pro, Apple Clang 21.0.0, SDL 3.4.16,
OpenGL 4.1 Metal 91.7 / GLSL 4.10. The supported US XBE hash was checked:
`3cfd001a84fc3e08175d6c4b2e42ebf49577a089b5d0bd87ac724f61a41816bc`.
The complete patch stack was applied and wrappers regenerated through
`tools/build-native-mac.py --generate`; no generated C was hand-edited.
The final build manifest's recorded source hashes match the current sources.
Executable SHA-256:
`8f50deaa32247b94266b725fc6a00911b81236858743932be8e14edbfdf918e3`.

Implemented and checked in this batch:

- MAC-001: SDL relative counts now use the shared direct-camera model and verified
  US guest fields, preserving sensitivity, inversion, zoom scaling and pitch
  limits without adding mouse input to the controller stick. Native fixtures
  checked sensitivity/inversion/FOV, no stick injection, capture-click suppression,
  release and focus behavior. A live first-mission wrapper logged synthetic
  8/-2 count events producing -1.6/+0.4 degree yaw/pitch changes at sensitivity
  0.2 and FOV 55. Equal-count/polling-rate checks, complete live transition checks
  and physical-device testing remain pending on Mac.
- MAC-002: Mac compiles the shared prompt formatter and guest wrappers, and SDL
  activity selects the active prompt device. Real Mac VM region/protection checks
  replace the compatibility VirtualQuery stub for guest text/camera access;
  Windows retains its VirtualQuery implementation. Native fixtures passed
  custom/unbound/mouse labels, held/release/drift suppression, virtual-controller
  switching, live saved binding changes, guarded memory and guest stack/ABI
  restoration. Reviewed menus show [E] Select and [F] Back. Literal tutorial
  text still says START; artwork and non-token strings are not rewritten.
  Representative gameplay hints and physical-device switching remain unverified.
- MAC-005: all four 2D texture inputs and coordinates now reach fixed vertex,
  GPU vertex-program and CPU fallback paths. Tests cover distinct stages/UVs,
  projected coordinates, linear texture scaling and GPU/CPU agreement. FVF
  coordinate-size flags no longer accidentally select the programmable path.
  Reviewed first-mission captures show improved terrain, weapons, NPCs and
  debris. Dependent/dot texture modes, cube/volume resources and complete
  material fidelity remain unfinished.
- MAC-007: current-scene full/rectangular copies and image surfaces support
  32-bit ARGB/XRGB and linear variants. Real-GPU fixtures passed full/partial
  pixels, clipped offsets, orientation, preserved surrounding pixels/FBO state,
  and image-surface full/subrect read/write, double-lock and invalid-rect checks.
  Original and 1920x1080 scene sizes passed. Other formats explicitly return
  E_NOTIMPL; an actual title effect using the copy path remains to be checked.
- MAC-008: the first inclusive scissor rectangle now clips draws with target
  scaling, GL origin conversion and the existing HUD transform. GPU fixtures
  verified inside/outside pixels, disabling and clear behavior at original/1080p
  targets. Multiple/exclusive rectangles remain unsupported, matching the
  audited Windows limitation. This limited contract is marked Verified.
- MAC-010: the actual paused Options menu previewed 2560x1440, displayed the
  15-second countdown and automatically restored 1920x1080. The isolated saved
  mode stayed 1920x1080 and the game returned to visible first-mission gameplay.
  Keep/save/reopen, explicit Revert and broader aspect/HUD checks remain pending.

Validation commands/results:

- `python3 tools/build-native-mac.py --test`: all three native CTests passed,
  plus real-GPU renderer/multitexture/scissor/copy/image-surface/vertex/texture-pack/
  resolution/offscreen/interpolation/drawable tests and SDL input, prompt and
  settings regressions. Evidence: `logs/mac-backlog-build.log`.
- `.venv/bin/python -m pytest -q tests/test_patch_updates.py tests/test_native_interp_declarations.py tests/test_native_config_files.py tests/test_native_mac_tools.py tests/test_native_mac_launcher.py`:
  26 passed, two Windows-only checks skipped. Fresh, repeated and historical
  patch-upgrade coverage passed. Evidence: `logs/mac-backlog-python.log`.
- Final isolated runner: `--seconds 270 --capture --test-keys --profile-frames
  --fps 120`; 270.12 seconds, normal bounded exit, at least 13,800 completed guest
  frames and 15,530 successful host presents, no sampled GL errors/native fault.
  Captures were reviewed separately; frame counters alone do not establish
  gameplay. The final capture shows terrain, weapons, HUD and the opening
  tutorial. The run included menus, cinematics, resolution preview and brief
  synthetic input; it was not a full-level playthrough or physical-device test.
  Evidence: `logs/mac-backlog-final-summary.json`, `.err`, `-frames.csv`.

The final whole-run timing sample includes loading/cinematics and the higher
resolution preview: 58.33 host presents/s, comprising 51.69 guest and 6.64
interpolated presents/s, p95 25.56 ms and p99 37.18 ms. These are host CPU
intervals, not display scanout or a matched performance comparison. The 120
presentation setting and 60 Hz guest clock remain intact; sustained 120 FPS is
not established. Future renderer work needs scene-specific profiling as well as
fidelity checks.

Selected reviewed images in `private/captures/native-mac`:
`backlog-render-opening.png` (earlier renderer),
`backlog-multitexture-landing.png`, `backlog-multitexture-gameplay.png`,
`backlog-keyboard-prompts.png`, `backlog-paused-display.png`,
`backlog-resolution-countdown.png`, `backlog-final-landing.png`, and
`backlog-final-gameplay.png`. These are guest readbacks, not new macOS window
screenshots. Earlier cinematic border/outside-viewport artifacts and the user's
reported world-disappearance route are not claimed resolved by this batch.

One intermediate probe lost window focus and was stopped; it is not completion
evidence. The isolated SDL input file now accepts `motion`, `button` and `focus`
commands as well as key presses/releases. Synthetic focus allowed reliable menu
checks without changing normal interactive focus handling. This does not test
physical macOS event delivery. Runtime/input/display/save paths stayed isolated;
personal controls retain SHA-256
`cf44293910a77fb8f397ced933a1cfd5098e1949c93db9cc0571ed438b39f64a`, and the personal
display file remains absent. Known disposable partition images were recycled;
none remain in either native test directory.

MAC-003/004/006/009/012 remain queued. MAC-011 needs the real texture pack: its
local directory is absent, and the user confirmed it has not yet been transferred.
Synthetic pack tests passed again, but they do not verify actual artwork coverage.
Windows MSVC/Direct3D runtime checks are unavailable on this Mac; changes to the
shared headers/wrappers must still be checked on Windows. See
[MAC_PORT_BACKLOG.md](MAC_PORT_BACKLOG.md) for per-entry remaining acceptance work.


## Mac binding capture and camera checks (2026-10-15)

Validation performed before commit on `mac-port` based on `30653e3` (the checkout
advanced to the shared merge revision before edits began). Shared root rules are
unchanged. No work was moved to main; the other computer was not verified.
Host remains macOS 27.0.1 arm64 / Apple M5 Pro, Clang 21.0.0, SDL 3.4.16,
OpenGL 4.1 Metal 91.7. Upstream remains pinned to `d90c929`; the complete patch
stack was applied and generated wrappers rebuilt after the merge. The supported
US XBE SHA-256 was verified before generation:
`3cfd001a84fc3e08175d6c4b2e42ebf49577a089b5d0bd87ac724f61a41816bc`.

MAC-004 now has a Record button for each Cocoa binding slot. The capture box
accepts keys, modifiers, five mouse buttons and both wheel directions. Escape
cancels; Use Escape and Unbind provide explicit alternatives. Reserved and
unsupported inputs show feedback. A conflicting capture offers Cancel or Move
binding, with Cancel as the default. All edits remain drafts until Save; a
binding change does not discard pending sensitivity/inversion edits. Existing
unsupported saved keys remain selected and labeled unavailable on Mac.

Cocoa hardware keys and SDL gameplay input use the same portable virtual-key
mapping. Added punctuation/OEM keys, keypad digits/operators, lock keys and
system-key recognition. Windows and Mac share `pc_settings_key_allowed` and
`pc_settings_assign`; the latter removes the chosen key from old slots only
after explicit conflict approval. The Windows editor retains its existing
confirmation UI and uses the shared assignment function. Its native runtime
checks remain unavailable on this host.

Validation:

- `python3 tools/build-native-mac.py --generate --test`, followed by the final
  incremental `--test`: three CTests and all native GPU, input, prompt, settings
  and Cocoa capture checks passed. Final executable SHA-256:
  `c9dee18525ec447939bd1ecec5a6bcd0de431489fbb82d1db266bcb432d050ea`.
  `build/macos/BUILD.json` source hashes match the final sources.
- Cocoa responder fixtures cover key/modifier press/release, repeat suppression,
  five mouse buttons, both wheel directions, unsupported/reserved rejection and
  Escape cancellation. Actual modal capture dialogs test initial keyboard focus,
  Return through NSWindow event dispatch, Use Escape, Unbind and Cancel. These
  are application-generated tests, not physical macOS input validation.
- Shared assignment tests cover conflict refusal without mutation, explicit move,
  removing old occurrences, reserved/invalid rejection and unbinding. Disposable
  settings save/reload tests verify punctuation/keypad input reaches the correct
  guest sticks, live prompts update, unrelated JSON and other controllers remain
  intact, backups match, and failed read-only writes preserve active settings.
- MAC-001 native guest-camera fixtures now compare one batch of counts with 100
  individual updates, both pitch limits and stale motion suppression. Existing
  zoom, inversion, focus, capture-click and no-stick-injection checks still pass.
  This extends synthetic adapter coverage; physical mouse and complete live
  pause/settings transition checks remain pending.
- Relevant Python suites: 28 passed, two Windows-only checks skipped, including
  patch fresh/repeated/historical-upgrade, interpolation declarations, config
  preservation, isolated runner and launcher tests. `git diff --check` passed
  with the project's CRLF whitespace convention.

Evidence: `logs/mac-capture-build.log`, `logs/mac-capture-python.log`. A 90-second
isolated probe reached a reviewed rendered menu and dispatched F2 into Settings.
The automation surface did not expose the unbundled game's Cocoa window, so no
visual claim is made for the new editor layout. The runner force-stopped the
open modal after its timeout (93.17 s including shutdown grace, exit -9); no
native fault or sampled GL error was recorded. This probe used the build before
the final unsupported-key label adjustment and is not normal-shutdown evidence.
Its summary/error log are `logs/mac-capture-game-summary.json` and
`logs/mac-capture-game.err`; reviewed guest menu: `private/captures/native-mac/mac-binding-menu.png`.

The pre-build disk check showed 224 GiB available and 158 MiB physically allocated
for `build/macos`. All checks used the existing isolated native paths. Personal
controls retain SHA-256
`cf44293910a77fb8f397ced933a1cfd5098e1949c93db9cc0571ed438b39f64a`; the personal
display file remains absent. Disposable partition images were recycled through
the shared cleanup helper. MAC-004 is Awaiting validation for visual/editor and
physical-device checks (including non-US keyboards). The remaining rendering,
debug, movies, multiplayer and packaging entries stay in the backlog. Actual
texture-pack validation still awaits the user's asset transfer.

A final 35.16-second isolated menu probe used the final executable above and
exited normally at its bound (exit 0), with at least 1800 completed guest frames
and 1993 successful host presents, no sampled GL errors or native fault.
Evidence: `logs/mac-capture-final-summary.json` and `logs/mac-capture-final.err`.
No gameplay or physical-input claim is made from this short startup check.


## Mac debug and performance tools (2026-10-15)

Published the preceding binding/camera work as `e83e074` to `origin/mac-port`.
The debug adaptation below is local and uncommitted over that revision. No main
merge was performed; the Windows computer's checkout remains unverified.

MAC-003 now uses shared `pc_debug.c` statistics, options, formatting and log
classification. Native adapters provide F9, an F12/Cocoa panel, CoreText/OpenGL
text rendering, POSIX log reading and Finder log-folder access. Save & Close
uses the existing atomic controls-JSON writer and backup behavior. Cancel
preserves active values; Clear counters skips old log text. The log reader
handles incomplete lines, truncation and inode replacement, rejects non-regular
files and consumes at most 64 KiB per main-thread poll. Performance samples are
optional and emitted no more than twice per second. Both launch paths override
inherited log paths to their own bounded stderr files.

The overlay is drawn after the scene blit and stays out of guest captures,
screen copies and interpolation history. The Game count is guest presentations;
Shown includes successful interpolated host presents. The panel also displays
their difference. Average guest interval is derived from these counts; neither
counter measures monitor scanout or changes the 60 Hz guest clock.

Validation on macOS 27.0.1 arm64 / Apple M5 Pro, AppleClang 21.0.0, SDL 3.4.16,
OpenGL 4.1 Metal 91.7 / GLSL 4.10, upstream `d90c929` and the complete patch stack:

- Supported US XBE SHA-256 verified before building and the bounded run:
  `3cfd001a84fc3e08175d6c4b2e42ebf49577a089b5d0bd87ac724f61a41816bc`.
- Final `python3 tools/build-native-mac.py --test` passed all three CTests plus
  real-GPU, input, prompts, binding/settings and debug suites. Debug checks cover
  log rotation/truncation/partial lines, classifier exclusion of its own samples,
  counter reset, invalid samples, Cocoa modal Save/Cancel, read-only failure,
  unrelated JSON/other-controller preservation and exact backups. SDL F9 tests
  verify key repeat and release do not toggle a second time.
- Real-GPU overlay checks verify visible text pixels, disabled overlay, restored
  framebuffer/scissor state and unchanged guest scene. Reviewing the rendered
  fixture caught upside-down CoreGraphics text; the blit now corrects row
  orientation. Final fixture `build/macos/path-tests/debug-overlay.png` is upright
  and readable. Its 60/120 numbers are controlled test values, not measured FPS.
- Relevant Python checks: 28 passed, two Windows-only tests skipped. Launcher
  `--check` passed, source hashes match `build/macos/BUILD.json`, and
  `git diff --check` passed. Final executable SHA-256:
  `40f27d687a9a8bfe5ea1538c74a64171e5e161c55929aa122b1f39ba3f82e290`.
- A 45.14-second isolated run completed at its time limit with exit 0, no native
  fault and no sampled GL errors. It dispatched synthetic F9 and F11 toggles,
  including F9 between fullscreen requests. Its profiler recorded 2,237 guest
  and 852 interpolated presents, 68.78 total presents/s over loading/title
  rendering and transitions (49.81 guest + 18.97 interpolated/s), with p95
  interval 25.38 ms. The debug logger produced 88 bounded samples and kept guest
  and shown rates distinct. This is a shortcut/integration smoke test, not a
  steady gameplay or 120 FPS performance certification.
- That integration run used executable `c699b3e482218f050a2749511bd4055c860c3b9faceeb232612a44d6d751223d`,
  before the orientation correction and added F9 regression assertion; final
  GPU/SDL suites above validate those subsequent changes. No additional
  full-level or campaign run was performed.

Evidence: `logs/mac-debug-build.log`, `logs/mac-debug-python.log`,
`logs/mac-debug-native-boot-summary.json`, `logs/mac-debug-native-boot.err`,
`logs/mac-debug-native-frame-times.csv`, and the small rendered fixture above.
The test controls were restored; personal controls retained SHA-256
`cf44293910a77fb8f397ced933a1cfd5098e1949c93db9cc0571ed438b39f64a`;
personal display settings remained absent, and personal saves were unused.
Both isolated test roots have no remaining disposable partition images.

MAC-003 remains Awaiting validation: physical F9/F12 input, visual Cocoa layout,
Finder action and live fullscreen panel/focus behavior need review. Synthetic
SDL events and application-driven modal tests do not establish physical macOS
event delivery. Windows MSVC/runtime checks remain unavailable on this host.
MAC-011 still awaits the user's real texture pack; no artwork was substituted.

## 2026-10-15 — Windows release patch preparation

Local, uncommitted change on `mac-port` base `e83e074`; release patch inputs
were read from local `main` revision `5cf4866`. Host: Windows 11 x64
10.0.26300, Python 3.14.2, Git 2.47.0.windows.2. The native boot overlay guard
invalidated the Windows watermark patch's original surrounding context.
The loader now requires one adjacent context line for that specific patch
when native boot is present; added/removed code must still match.

`python -m pytest tests/test_patch_updates.py -q`: 11 passed. Added regression
cases cover fresh application, an existing watermark, repeated application,
and rejection of conflicting watermark code without writes. Separate temporary
source-only fixtures using all patches from main reproduced the original error
before the fix and passed afterward for fresh upstream sources and a copy of
the existing Windows runtime (34 and 28 files updated respectively); a second
application changed nothing. The actual runtime was not modified. No game
build, packaging, publishing, or gameplay test was needed or performed for
this Python loader fix. No game data or personal settings were used.

MAC-014 tracks pending macOS validation. Changes have not been committed,
pushed, or merged to main; synchronization with the other device is unverified.

## 2026-10-15 — Windows texture-dump compilation fix

Reported during release preparation on main `38f1a2f`. Fix is local and
uncommitted on `mac-port` base `724b1a1`: restore the missing `FILE *f`
declaration in `src/texture_pack.c::dump_original`. No runtime patch or generated
game source changes are required. MAC-011 records the platform impact.

On Windows x64, the installed VS 2022 BuildTools MSVC compiler accepted the
corrected translation unit with `/Zs /TC` and the Windows renderer include paths
and feature defines. The existing `tsfp_texture_format_test.exe` passed DDS
validation checks; that executable was not rebuilt and does not exercise the
dump function. Texture-tool Python tests could not collect because Pillow is
not installed in this virtual environment. No dependency pins were changed.
No full game build, packaging, game launch or macOS validation was performed;
personal settings and saves were unused. Other-device synchronization remains
unverified. This fix must reach main before release preparation can use it.

Patch validation: `python -m pytest tests/test_patch_updates.py -q` passed all
11 tests on Python 3.14.2. `git diff --check` passed.

## 2026-10-15 — Release cleanup and Windows test privileges

Uncommitted on `fix/release-cleanup`, based on main `2ef7536`. The existing
changes were carried intact from `mac-port` onto this focused branch following
the user's updated branching preference. Host: Windows 11 x64, Python 3.14.2,
Windows PowerShell 5.1.
Symlink test fixtures now skip only Windows error 1314; real symlink assertions
still run on capable hosts and other creation errors propagate. Release work
cleanup uses direct deletion after the existing exact-path and reparse checks,
so disposable test/staging files do not require Recycle Bin support. Release
archives and metadata are retained. A cleanup failure during failed tests is
reported without replacing the original test failure.

Targeted checks: `pytest tests/test_publish_release.py
tests/test_native_mac_tools.py -q -p no:cacheprovider --basetemp
build/cleanup-validation`: 22 passed, three skipped. Two skips are Windows
symlink privileges and one is the existing macOS allocation check. Actual
PowerShell cleanup tests passed for disposable test and staging directories,
rejected a personal-save-shaped path, and preserved an adjacent archive.
The original-error/cleanup-error regression passed. The first sandbox attempt
could not execute scripts; validation succeeded outside the sandbox using an
isolated build directory, without changing execution policy.

No game build, release preparation/upload, game launch, or Mac checks were
performed. Personal saves/settings and existing release outputs were untouched.
MAC-015 tracks Mac validation. Changes are local, uncommitted and unpushed;
other-device synchronization remains unverified.

## Pickup range comparison fix — October 16, 2026

Issue [#6](https://github.com/TheDoffman/timesplitters-future-perfect-pc/issues/6):
the US pickup tick at `0x000C15B0` compares the original radius squared with
horizontal distance on two paths (`COMISS xmm3,xmm0` and `COMISS xmm0,xmm1`).
At their shared branch, `0x000C17D9`, the translator discarded the differing
operand states and emitted `if (_flags)` with `_flags` initialized to zero.
The horizontal rejection therefore never ran. Eligibility and vertical checks
still ran, explaining early pickups rather than indiscriminate item grants.

`patches/recomp-float-joins.patch` merges SSE comparison snapshots across those
paths, retaining their original ordered/unordered conditions. Pickup radii,
weapon grants and input bindings are unchanged. No generated game C was edited.
`tools/recomp-generation.py` records generator source hashes; Windows Build
regenerates stale C, while native Mac requests `--generate`. The normal Windows
Test command includes the new regression. Existing generated checkouts migrate
on their next build.

Local validation baseline `f847832ef3588000b6d4251de657ea34f308fac1`, in
the existing `codex/integrate-display-textures` checkout; branch, HEAD and user
edits were preserved. The fix is isolated on `fix-pickup-range` over newer main for the user-requested push. No binary release is included.
Runtime pin `d90c92988a9f99d0580d2ca3f7efd6ec3a7b7845`; the supported US XBE SHA-256
was verified before generation and execution. Host: Windows x64, Python 3.12.14,
MSVC 19.44.35224.0, Release, Direct3D 11.

Validation:

- The pinned original translator reproduced a real out-of-range grant in the
  compiled pickup tick: radius 100, position `(101,0,0)`, grant 1 instead of 0.
  Before the patch, 13 new structural/integration checks failed; after it, all
  15 new tests pass. The actual translated pickup function passes 208 isolated
  cases covering both shape paths, 32 directions, near/far and exact-radius
  boundaries, vertical separation, NaN/infinite coordinates and guest register,
  stack and x87-stack preservation. Only eligibility/grant callbacks are stubbed.
- 25 patch-stack and upstream flag/float checks passed. Mac build-tool/launcher
  tests: 16 passed, one Windows symlink-privilege skip. Native Mac compilation
  and gameplay were unavailable and remain unverified.
- All Windows build targets compiled. The full Build command stopped at the
  unchanged controller-prompt test's foreground-window check (line 78), also
  reproduced on retry. This is **not an all-tests-passed build**. Network, DDS,
  texture switching and display regressions passed; save routing, PC controls,
  settings and debug tests then ran separately and passed. The executable's
  network identity was sealed after those checks; the focus-test failure remains
  recorded rather than hidden.
- A bounded 178-second isolated run loaded the first mission opening sequence at
  1280x720. Synthetic left/right strafe keys were injected and 12 raw mouse yaw
  updates were logged. Reviewed captures show the opening animation; this does
  not establish a normal gameplay route, on-scene pickup collection, every story
  level or the separately reported body-collision behavior. No crash or unresolved
  call was logged. Personal controls/display hashes remained unchanged; saves
  and both runtime/PC display configuration paths were isolated.
- Recycled 18 disposable test partition images after their test passes, retaining
  profiles, settings and diagnostics. Their apparent size was 15,730,220,544 bytes;
  physical allocation was only 138,752 bytes. No personal saves were used.

Evidence: `build/collision-validation/VALIDATION.json`, `baseline.json`,
`build.log`, the native test logs, `game.json`, `game-150.png`, `game-172.png`
and `disk-cleanup.json`. Fixed executable:
`build/windows/Release/futureperfect_recomp.exe`, SHA-256
`4e6c801ed83e123fcfe43e32d3ace1332ac62daeaa5f1367d3c30e330bebf929`.
The launcher-installed running game was left alone; it retains its previous
executable until updated. Mac regeneration/validation and cross-device Git
synchronization remain pending under MAC-016.

Publication source note: this focused change is based on main `8f5e1bd2b500ad1d87379a81254ad936df10df3f`. The executable and gameplay evidence above were produced in the
earlier local checkout, not from this publication commit. Its source-only
patch stack and regressions passed 51 checks before the push, including the
208-case compiled pickup regression; one Windows symlink-privilege check skipped.


## Runtime patch branch-switch repair — October 16, 2026

Branch-switch fix on `fix/runtime-patch-branch-switch`, based on main
`837692d3ffbd81fbb6252ff819ff219f8e3ce097`. The same runtime directory had been
built on `codex/integrate-display-textures`. Its texture-toggle patch remained
applied after switching to main, preventing the installer from unwinding the
controls dependency. A disposable source-only copy reproduced the reported
`pc-controls.patch conflicts with the runtime checkout` error before the fix.

The loader now recognizes exact patches in `patches/legacy/` only for unwinding,
then applies the current branch's active stack. An active copy takes precedence
when the same patch is present in both locations. The archived texture-toggle
patch matches the texture branch exactly. No upstream reset or manual generated
C edit was used; unrelated runtime edits remain intact. Conflicting edits still
fail before runtime writes.

Validation: Windows 11 x64 10.0.26300, Python 3.12.14, MSVC x64 14.44.35207,
Release / Direct3D 11; pinned upstream `d90c929`. The supported US XBE SHA-256
was verified as `3cfd001a84fc3e08175d6c4b2e42ebf49577a089b5d0bd87ac724f61a41816bc`.

- Patch-loader regression suite: 14 passed in 75.84 seconds. New checks cover
  texture-to-main-to-texture transitions with LF and CRLF, repeated no-op
  application, preservation of an unrelated edit and rejection of a modified
  legacy patch without writes.
- Full `tsfp.ps1 Build` passed after the user closed the running development
  executable. Seven runtime source files were updated by the validated stack.
  Native checks passed for UDP/guest network routing and packet compatibility,
  texture formats, display persistence, virtual-controller prompts, save paths,
  controls, settings and debug. Disposable build/path-tests data was recycled
  by the normal build command. Existing compiler warnings remain.
- Rebuilt executable SHA-256: `625769a2fdfa5497a5bdf735b960f5c3100882d3d3b6b68389f515877e04176a`.
  Its network-build stamp was sealed by the successful build. Controls/display
  hashes and the pre-existing package-playable.py edit match the pre-build
  record. Personal saves were not used by the isolated build checks.

Evidence: `logs/runtime-patch-switch-build.log` and
`build/patch-switch-validation/VALIDATION.json`; a small runtime recovery patch
was retained before the transition. No game launch, end-to-end multiplayer
session, packaging or release publication was performed. MAC-014 tracks pending
macOS validation; other-device synchronization is unverified.


## Automatic runtime patch tracking — October 16, 2026

Implemented on `fix/automatic-patch-tracking`, based on main `979a16c`, for
commit and publication at the user's request. Each successful loader invocation
records exact patch bytes, SHA-256 hashes, order and context options in the
runtime's per-worktree Git metadata (`tsfp-patches.json`). The next invocation
unwinds that recorded stack before applying the current branch's stack, so
removed, renamed or edited patch files no longer require a manual archive.
Existing runtimes bootstrap through the active/legacy recognition path once.
Branches need the updated loader to maintain this record.

The entire transition is validated on disposable source copies. Invalid records
and conflicting edits fail before runtime writes. Concurrent source/record edits
observed during validation are preserved. Failed source writes or metadata
publication roll back source changes and retain the old record. Successful
records are replaced atomically; repeated no-op runs do not rewrite metadata.
No game data, generated C or build tree is duplicated.

Validation on Windows 11 x64 10.0.26300, Python 3.12.14, MSVC 14.44.35207,
Release / Direct3D 11, pinned runtime `d90c929` and the verified supported US XBE:

- Existing patch-upgrade and new tracking tests: 27 passed. Real Git branch
  switches cover unknown patch removal, edited patch content, both directions,
  LF/CRLF and unrelated edits. Additional cases cover rename/empty stacks,
  exact-byte/hash retention, corrupt/unsupported records, partial write and
  record-publication rollback, and concurrent edit detection.
- Native Mac tools/launcher Python suites on Windows: 16 passed, one skip for
  Windows symlink privilege. These are shared-tool checks, not Mac game tests.
- The actual runtime enrolled 27 patches in a 152,426-byte record without source
  changes. A subsequent Build used the record successfully. Compilation and
  network, texture and display checks passed; Build then stopped at the existing
  tests/pc_prompts.c:55 foreground-window assertion. A direct retry failed the
  same assertion. Save-path, controls, settings and debug checks passed when run
  separately. The executable SHA-256 remains
  `625769a2fdfa5497a5bdf735b960f5c3100882d3d3b6b68389f515877e04176a`, identical to
  the preceding successful build; multiplayer content verification passed.
- Personal controls/display and the existing package-playable.py edit retain
  their baseline hashes. Disposable native test data was recycled after checks.

The Runtime patch transitions GitHub Actions workflow uses pinned actions,
reads the upstream/pytest pins from shared configuration, and runs both patch
suites on Windows, Linux and macOS without game data or submodules. Remote
matrix results are available in GitHub Actions. `tsfp.ps1 Test` includes the new tracking suite.
Evidence: logs/patch-tracking-tests.log, logs/patch-tracking-unit.log,
logs/patch-tracking-shared-tests.log, logs/automatic-patch-tracking-build.log,
logs/patch-tracking-prompt-retry.log and build/patch-tracking-validation/VALIDATION.json.
No gameplay session, binary packaging or release publication was performed.
MAC-014 retains pending native Mac validation and other-device synchronization.


## Issue #8 audio timing and buffering — October 16, 2026

Local, uncommitted work on `fix/issue-8-audio-timing`, based on main
`4189924038729030d890faa93edacdeccc74a1e1`. The user clarified that delayed
cutscene audio concerns story cutscenes. The separate XMV video path was not
changed. `patches/dsound-audio-timing.patch` fixes four reproduced runtime bugs:
stream completion advancing through unheard audio after queue starvation;
flush retaining the previous voice's timeline origin; refused buffer submissions
discarding PCM; and ADPCM resume copying from the wrong offset inside a block.
Process, pause and format changes now synchronize to the available device clock.
The existing fallback remains for backends without a playback-position report.

Validation host: Windows 11 x64 10.0.26300, Python 3.12.14, MSVC 14.44.35207
(compiler 19.44), Release / Direct3D 11. Runtime remains pinned to
`d90c92988a9f99d0580d2ca3f7efd6ec3a7b7845`; supported US XBE SHA-256 verified as
`3cfd001a84fc3e08175d6c4b2e42ebf49577a089b5d0bd87ac724f61a41816bc`.

- Compiled tests execute the actual stream/buffer adapters and Xbox ADPCM
  decoder with deterministic clocks, guest memory and a fake output device.
  Original pinned sources fail eight of thirteen cases; patched sources pass
  all thirteen, including starvation, submission, pause, flush, exact completion,
  fallback/device loss, diagnostic drift, partial ADPCM blocks and refused
  looping/nonlooping buffer submissions. Together with patch upgrade/tracking
  tests, 40 checks passed in 93.17 seconds. The final clean patch's audio suite
  also passed all thirteen checks. CI hooks were added for Windows/Linux/macOS;
  those new remote matrix checks have not run.
- Final Windows compilation and linking succeeded. Network, texture and display
  checks passed, but the full Build command stopped at the existing
  `tests/pc_prompts.c:55` foreground-window assertion on two attempts. Save-path,
  controls, settings and debug checks passed separately. The network executable
  stamp was sealed separately after these checks and content verification
  passed. This is not a passing final full Build. Final executable SHA-256:
  `44f92c618c768b98db0d0600bd2c491ea3f883d5c3fc87b032f0f00d3a18c143`.
- A bounded 165.12-second run used isolated saves and both isolated display
  configurations, with the real audio device active and muted output. It rendered
  profile/menu screens without a crash or unresolved call. Inspection of the
  final capture confirms profile-name entry; the probe did not reach story or
  gameplay. It cannot establish audible quality or story A/V synchronization.
  Probe executable SHA-256 was
  `da1ed1873074bffaa97b532d96b1015f6433b96b6294e0e156bef93872aa0b7b`, before a
  comment-encoding cleanup and rebuild, with identical audio implementation.
- Personal controls/display and the pre-existing unrelated package-playable.py
  edit retain their baseline hashes. No personal saves were used. Disposable
  build path-test data and six known isolated test partition images were
  recycled, preserving profiles and metadata. Those six images represented
  5,243,406,848 logical bytes but only 133,632 physically allocated bytes.

Evidence: `build/issue-8-audio/VALIDATION.json`, `baseline-full.json`,
`partition-cleanup.json`, `game.json`, `game-160.png` in the same directory;
`logs/issue-8-final-regressions.log`, `logs/issue-8-build-verified.log`, and
`logs/issue-8-game-smoke.log`. Story listening, repeated level starts and A/V
synchronization remain pending. MAC-018 records SDL device-position adaptation
and native Mac validation. Other-device synchronization is unverified. No
commit, push, binary packaging or release publication was performed for #8.

### Follow-up: first-mission distortion and missing stream headroom

The user reports harsh static/distortion at the first mission's start with the
initial build above. That playtest establishes that the initial timing changes
did not resolve the reported issue. No audible-fix claim is supported by those
earlier passing regression tests.

Read-only inspection of the isolated game's stream settings found a further
volume mismatch: the XDK stereo voice records 600 hundredths of a dB of headroom
and subtracts it from SetVolume, but the host stream used the raw SetVolume
argument. Stereo audio was therefore about twice the intended amplitude. The
shared stream adjustment now includes headroom alongside mix-bin/3D attenuation;
it still falls back unchanged outside the checked XDK layout. A new regression
fails before this correction and passes afterward, including a headroom change,
mix-bin attenuation, and unsupported-layout fallback.

- Audio suite: 14 passed in 2.42 seconds. The full Windows `tsfp.ps1 Build`
  succeeded, including the formerly blocked controller-prompt check and all
  remaining native tests. Its normal cleanup and executable stamp completed.
  Network-content verification and repeated patch application (zero updates)
  passed. Executable SHA-256:
  `c645c07abd7076dd78122498990868a09b3adb730d991704d1619f3c0f5cc4f7`.
- A bounded isolated replay of the preceding executable reached the first
  mission's opening (a capture was visually reviewed, then overwritten by a
  later menu probe). Read-only process
  inspection retained twelve compressed packets. All their ADPCM block headers
  were valid; none of their independently decoded samples clipped individually.
  These observations do not rule out mixed-output overload or prove audible
  correctness. Subsequent profile-navigation attempts remained at name entry,
  so the headroom build has menu/device-level evidence, not another completed
  story-cutscene probe. Its menu music diagnostic reflects the extra 6 dB
  attenuation (`-1737` versus the preceding `-1137`).
- The probes used the existing isolated pickup-range settings/saves, muted real
  device output, fixed time/capture/log bounds and read-only PDB/memory access.
  They did not change personal saves/configuration or the executable in memory.
  Local diagnostic source/data remain ignored in `build/issue-8-audio`.

Evidence: `logs/issue-8-headroom-baseline.log`,
`logs/issue-8-audio-headroom-tests.log`, `logs/issue-8-headroom-build.log`, and
`build/issue-8-audio/HEADROOM-VALIDATION.json`. The headroom mismatch is fixed;
its effect on the user's reported distortion still needs an audible retest.
Changes remain local and uncommitted; no release was published. MAC-018 still
requires native Mac validation.

### Follow-up: stale music packets at level transitions (2026-10-16)

The user's headroom-build retest still had harsh distortion. A bounded,
read-only capture attached to their human-controlled Windows playtest (PID 7616,
headroom executable `c645c07a...`) isolated a separate fault. Music stream slot
266 retained three old 73,728-byte packet addresses across the menu-to-story
transition. Their memory now contained unrelated game data. Respectively
1,642, 1,605 and 1,770 of 2,048 stereo ADPCM headers were invalid; independent
decoding clipped 16.04%, 13.48% and 18.30% of samples. Captured host PCM matched
that corrupted input. The next five music packets had valid headers and no
clipped decoded samples. Captured cutscene/dialogue PCM matched its source.
Earlier probes only saved the 22,042 Hz story streams, missing this 32 kHz
music stream. Reducing headroom could not repair its corrupted input.

The shared adapter intercepted ordinary Flush but neither FlushEx nor Release.
The pinned XBE's stream stop routine calls FlushEx with zero timestamp and ASYNC;
its Release method can destroy the stream while the host still retains packets.
The new `dsound-stream-lifecycle.patch` retires the host voice and packet model
only on final Release, preserving non-final references and allowing the allocator
to reuse the same interface address with fresh format/queue state. It handles
immediate FlushEx and zero-time ASYNC (packet callbacks completed in DoWork),
blocking submission/status completion while the asynchronous flush is pending.
Other scheduled/envelope FlushEx modes retain the existing XDK fallback and
remain outside this fix's validated scope.

Three lifecycle tests reproduced stale host queues before the hooks; 17 audio
cases passed after the initial implementation. The async regression also checks
that intervening GetStatus/Process calls cannot complete cancelled packets from
the wall clock. Build discovery initially missed the new HLE wrappers, producing
unresolved original-body symbols. `tools/recomp-generation.py` now fingerprints
HLE export/original/import declarations, with a regression for addition/removal
and body-only edits. Wrappers are regenerated from source, never edited by hand.

Read-only inspection did not modify the running process, inputs, personal
settings or saves; the user navigated and closed the game. Captures are capped
at 12 MiB and 240 seconds in ignored `build/issue-8-audio/live` with source/PCM
metadata and analysis. Current local rebuild and post-fix listening results are
recorded below when available. Changes remain uncommitted on
`fix/issue-8-audio-timing`; native Mac listening/build and other-device
synchronization are unverified (MAC-018).

Lifecycle validation: 45 checks passed in 187.94 seconds (17 actual audio-adapter
cases, one HLE-generation invalidation case, and 27 patch/branch-transition
checks). Full `tsfp.ps1 Build` passed all native checks and recycled its disposable
path-test data. Generated wrappers now intercept Release at `0x00407286` and
FlushEx at `0x004074EC`, with their original bodies retained. Network game-content
verification passed. Windows Release executable SHA-256:
`1482f8ca68663f6980474efb801193fa0726424e6b92678f71be0a5919e33852`.
Evidence: `logs/issue-8-lifecycle-baseline.log`,
`logs/issue-8-lifecycle-regressions.log`, `logs/issue-8-lifecycle-build.log`,
`build/issue-8-audio/live/music-corruption.json`.

The user retested the rebuilt first-mission opening and confirmed: "The opening
sounds clean." Read-only capture of that playtest retained eight 32 kHz music
packets, all with valid ADPCM headers and zero clipped decoded samples, versus
three corrupted stale packets before the fix. The new run logged FlushEx calls
(43 by the sampled mission opening, versus zero previously), confirming the
previously missed stop path now reaches the host adapter. The user then closed
the game normally. This verifies the reported first-mission distortion; other
levels, long sessions and full A/V synchronization remain broader playtest gaps.
Post-fix evidence is in `build/issue-8-audio/live-fixed/packet-analysis.json` and
`logs/play-20261016-030301.err`. The audio changes are being committed to the
user-requested `8-audio-issues` branch; no main merge or release publication is
part of this update.

Branch integration: the audio commit was applied cleanly to `8-audio-issues`
on top of `c8f8b31`, preserving the newer mouse-player and controller-sensitivity
changes. The full branch Build passed on retry after the known foreground-window
controller assertion failed once; no source changes were needed for that retry.
Its native checks, path-test cleanup and network-content verification passed.
Logs: `logs/issue-8-branch-build.log` and `logs/issue-8-branch-build-retry.log`.
Branch executable SHA-256: `7807b189c890a31fcb265dc4395ec67ad453837ea035efd30f287c9b8cc95710`.
The audio implementation is unchanged from the clean human playtest above.

## Launcher startup skipping (2026-10-16)

Launcher v1.2.1 implementation `bad108c` on `feat/launcher-skip-startup`, based on published main
`d83ca5e57d36da163680f7c21e0b5f52dd785faf`. Game settings → Graphics & resolution
now has a Startup checkbox, **Skip startup splash screens**, default off.
Saving persists `launcher.skip_startup_splashes` in portable controls JSON with
the existing backup, external-edit protection and rollback rules. Both the
direct launcher and newly packaged `Launch.ps1` explicitly set the existing
`RECOMP_XMV_PLAY` switch after sanitizing inherited overrides. This skips all
file-based XMV playback (opening EA/Free Radical logos and the idle attract
movie); story scenes use the game's separate rendered-cutscene system. The UI
explains the attract-video effect. No runtime patch or game rebuild is needed.

Windows x64, .NET Framework 4.8 C# compiler file version 14.8.10001.0,
Python 3.12.14: launcher
build passed 71 updater checks and 16 settings checks, including default-off,
save/reopen/disable, unknown-field preservation, malformed-value rejection,
inherited-override sanitization and actual child-process consumption. Five
pytest checks passed (three real PowerShell/synthetic-child launches covering
missing/false/true preferences plus two packaging checks). The WPF graphics
preview was rendered and visually reviewed; the checkbox/help text fit the
existing blue/silver layout. Evidence: ignored `build/launcher/startup-settings.png`.

`tests/launcher_startup_game.py` ran two bounded 20-second launches against the
verified published alpha.4 executable, SHA-256
`bc7cbb26ab90fc0af47eb8f8bff90d91c4074893cf443352d4e0c5bf41076345`, using its
existing D3D11 backend and pinned FFmpeg libraries. The supported US XBE hash
was reverified before testing. Normal mode played both opening movies; skip
mode completed both XMV objects with end-of-file status without decoding movies.
Both runs remained alive and rendered the player-selection screen, confirmed in
reviewed captures. No crash or unresolved indirect call appeared in their logs.
All input, saves and both display-config paths were isolated; this invocation's
disposable saves/frame dumps were removed. Evidence: ignored
`private/startup-validation/{normal,skip}/startup.png`, `game.err`, `game.out`
and `runtime/provenance.json`. This is compatibility validation of the published
game, not a rebuild of concurrent runtime work. Story gameplay/cutscene playback
was not retested. Mac UI/playback and other-device synchronization remain
unverified (MAC-021). Release preparation targets stable `launcher-v1.2.1` on
main, using the documented launcher-only packaging channel. `Publish-Release.ps1`
publishes the game `VERSION`; no new game version is needed for this option.
The Windows CI workflow now runs the launcher and portable-script regressions.

## Embedded launcher settings (2026-10-16)

Windows launcher v1.2.2 implementation `5eea0ca` on `feat/launcher-inline-settings`,
based on main `696421fd080a5bd57cb894b76302080f59f52485`. `SettingsPage.cs`
replaces the separate settings window: controls, graphics, startup options,
binding capture and confirmations stay in the main content area with the sidebar
visible. `Launcher.xaml`/`Launcher.cs` switch between Command center and settings
with active navigation styling. Home keeps the draft/tab and dismisses capture;
Cancel discards the draft; Save validates and keeps the page open. Atomic file
save, backup, stale-edit detection and rollback remain in the existing model.
The preview commands now render the whole launcher rather than a separate window.

Windows x64 / .NET Framework 4.8 / Python 3.12.14: both launcher binaries compiled.
The full launcher build passed 71 updater checks and 19 isolated settings
checks, including actual WPF sidebar/page navigation, 1000 × 720 minimum and
1152 × 772 layout sizes, draft retention/cancel, inline binding-conflict cancellation and
reassignment, successful/invalid save, and existing settings/environment checks.
Five portable-script and packaging checks passed. Both full-shell 1152 × 772
controls/graphics previews were visually reviewed; captures are in ignored
`build/launcher/inline-controls.png` and `inline-graphics.png`.

Evidence for the complete build/test pass: ignored
`build/launcher/inline-build-test.log`. No game runtime source changed or game
playtest was needed. Release preparation targets stable `launcher-v1.2.2` on
main through the documented launcher-only channel; no game update is required.
Native Mac settings/navigation adaptation remains
queued under MAC-021; other-device synchronization is unverified.

## Story Objectives health and shield (issue #11) — 2026-10-16

- Implemented on `11-objectives-tab-not-showing-health-and-shield`,
  based on main `d83ca5e`. Windows x64, CPython 3.14.2, MSVC x64, D3D11.
  Supported US XBE SHA-256 verified before generation and isolated testing.
- The before capture shows the Story Status/Objectives panel without indicators.
  A read-only sample of the user's open Objectives screen found menu flags
  `0x191`, a valid health/shield data pointer, and the display enable flag set.
  The original quick-menu opener sets `0x100`; the original menu renderer tests
  this bit and skips `sub_0013AB60`, its health/shield renderer.
- The shared `story-objectives-health` patch temporarily masks `0x100` only while
  drawing active Story quick menus, then restores it and transient `0x80`.
  The original indicator data, animation and remaining visibility guards stay
  in use. Input/pause state and other game modes retain their behavior.
- The first corrected live capture confirmed both indicators and calls to the
  original renderer, but the user reported clipped sides. Replay identified
  indicator vertices at guest x=-48.64 in a 640-wide viewport. The new
  `src/pc_objectives.h` and health-draw wrapper shrink the menu indicators to
  the current viewport with a 4% inset on each side, retaining a common scale
  for both axes and smaller animation phases. Guest viewport coordinates map
  through the existing renderer to any selected output resolution. The scale
  is recomputed each draw, supports split-screen, and leaves normal gameplay
  HUD draws alone. Caller arguments are restored afterward.
- The user confirmed the viewport fit, then reported the crescents covering
  the Objectives panel edges. `build/windows/Release/frame001.d3dcap` reproduced
  this with a nonempty first-mission objective. The panel still used its original
  80% width while the gauges had shrunk. The shared Objectives builder now uses
  the same fit ratio for both container and text-measurement widths, restoring
  the shared US constants immediately afterward. Font size remains unchanged;
  long objectives use the original wrapping/scrolling layout. Live verification
  of this additional spacing correction initially failed: the next capture was
  unchanged. The builder wrapper incorrectly treated the camera base as its
  viewport, so invalid dimensions bypassed sizing. The US menu renderer uses
  the viewport embedded at camera+0x224. The wrapper now uses that same offset,
  and the fixture separates the camera base from its embedded viewport.
  Seven focused tests pass, including a mutant reproducing the missing offset;
  the user confirmed the rebuilt correction is working on 2026-10-16.
- Seven focused regressions passed: production visibility and sizing wrappers,
  deliberately broken variants, and the original checksum-verified US menu
  routine. Tests cover full pause/inactive menus, other modes, invalid indices,
  hidden widgets, display-disabled state, guest stack/register preservation,
  changing 4:3/16:9/16:10/ultrawide/portrait dimensions, local viewports 1–4,
  off-center animation, matching panel/text-measurement width and restoration
  of shared constants. A deliberately unscaled panel reproduces the regression.
  Portable tests run in the Windows/macOS transition
  workflow; the original-XBE test skips in CI without private game data.
- Final camera-offset correction: the complete Windows Release build and all
  native build checks passed, including the prompts/focus test. Seven focused
  Objectives tests passed. Before this last offset correction, all 34 focused
  and patch-transition regressions passed together in 207.45 seconds. The
  historical upgrade fixture excludes the new patch when constructing its
  pre-letterbox state, then upgrades to the complete stack. `git diff --check`
  passed. Earlier sizing builds intermittently failed the unchanged prompts
  test's foreground-window assertion; both later full builds passed it.
- The user confirmed the indicators fit inside the screen and, after correcting
  the camera+0x224 viewport lookup, confirmed the panel-spacing fix is working.
  This is user-reported Windows visual validation. Automated UI verification
  was stopped by the user's Escape key. Live coverage of all resolutions,
  split-screen and long/multiple objectives remains unverified; numerical
  layout cases cover the viewport sizes listed above. Mac compilation/gameplay
  remain unverified; see MAC-023. Local captures and test provenance are under
  `build/issue-11-objectives`; the final full-build log is
  `logs/issue-11-build.log`, and the earlier combined 34-test result is in
  `logs/issue-11-regressions.log`.
- PR #55 conflict resolution integrates main `de23272` while retaining the
  Objectives implementation and fixtures byte-for-byte. CI keeps the Objectives,
  cinematic/decal, terminal, FOV and launcher checks; the historical-upgrade
  fixture retains all dependent-patch exclusions. Objectives is now MAC-023,
  preserving main's MAC-022 FOV entry and all existing validation notes.
  On Windows x64 / Python 3.12.14 / MSVC x64, all 44 merged Objectives, cinematic,
  decal, terminal, FOV and patch-transition checks passed in 129.91 seconds.
  The supported US XBE hash was reverified. The complete patch stack was applied
  to an isolated 3,986,182-byte source fixture from pinned upstream `d90c929`;
  no game assets were copied, and the other task's runtime/build was untouched.
  No full game rebuild or new live playtest was performed for this conflict-only
  update. Evidence: ignored `build/pr55-validation/tests.log` and
  `fixture-provenance.json`. The PR diff passes `git diff --check origin/main`.
  The follow-up main merge adds release documentation and the bounded cold-WPF
  startup allowance; native sources/patches are unchanged from the 44-test pass.
  The current launcher build passes 71 updater and 21 settings checks
  (`build/pr55-validation/launcher.log`). Objectives notes are appended here so
  new status entries at the top do not repeatedly conflict with this PR.

## Live Classic / 4K texture toggle — October 15

- F10 switches the cached original/replacement texture views without restarting
  or reloading a level. PC Settings > 4K Textures also switches the mode and
  shows its checkmark. Keyboard repeats are ignored; fullscreen and hidden-menu
  shortcuts work. F10 no longer also cycles the runtime's diagnostic frame cap.
- Play.cmd starts Classic with the installed local pack available; Play-Textures
  starts 4K. Packaged launchers discover an optional Textures folder beside Play.
  This session choice is independent of render resolution. Missing, invalid and
  memory-budget-limited replacements retain Classic; original assets stay intact.
- Windows Release build and all build-time native checks passed. Counted RHI
  loader tests cover cached switches, unchanged original data, missing/invalid
  DDS files, screen copies/render targets, content changes, allocation failures
  and release. Seven fresh/upgrade/idempotence patch-stack tests passed.
- One brief actual-runtime startup smoke check passed F10, held repeats, hidden
  menu, fullscreen, menu selection/checkmarks, one replacement load, clean exit,
  and personal controls/display settings preservation. Evidence is in
  logs/texture-toggle-validation.json and logs/texture-toggle-game.err.
- Full gameplay scene/artwork validation remains pending under the existing
  overhaul cadence. The current partial pack and 512 MiB resident GPU budget
  are unchanged. Disposable validation disk images were recycled after the run.

## Imported texture-overhaul history for issue #74

### Texture equipment batch 57 - October 16, 2026

- Installed four character/equipment atlases: dark cloth/brass, partial face with yellow-striped clothing and controls, blue/white/gold uniform, and padded blue-gray armor. All original catalog payload hashes/opaque alpha checked; enlarged source comparisons, compiled DDS and repeated-boundary sheets reviewed. Final model UV, identity, lighting and functional-marking interpretation remain pending.
- Two corrections before acceptance: dark-cloth V1 was +50.51% mean brightness; V2 restores low-key cloth/brass response, with final compiled mean -5.93% versus original. Blue-uniform V1 was too saturated; representative cloth saturation changed from original 82.57 to V1 124.91, corrected V2 91.49 (0-255 scale). Final compiled uniform brightness -0.33%. Face/equipment and padded armor brightness -3.48% and -6.34% respectively.
- Preserved dark eye opening and separate narrow eye/glint detail, paired yellow stripes, small indicator colors, three dark uniform details, ear/skin islands and mechanical hole groupings. Small control marks are preserved as abstract shapes; no exact readable-text claim. Atlas repeat sheets do not establish seamless tiling or model alignment.
- Six built-in image-generation calls, four selected 1254x1254 source images downsampled to 1024x1024 DXT1 with full mip chains. Not native 4K art. Superseded local V1 images and redundant V1 comparison sheets recycled; generator references, exact prompt history and focused revision sheet retained.
- Windows x64, source f847832 with existing uncommitted texture/launcher work and concurrent float-join/runtime edits: all 230 native DDS checks, worklist hash/dimension/mip/aspect/budget checks and independent inventory pass. This batch did not rebuild or launch the game and preserves concurrent changes.
- Coverage: 230/6,812 supported keys; 6,575 need artwork and seven drafts held. Visual classification remains 387 keys; full category, embedded/procedural and scene coverage unfinished. Pack is 384,784,477 bytes (about 367 MiB), up 2,798,496 bytes including metadata; texture workspace about 1.103 GiB. Temporary exports/reviews remain below 256 MiB. Caps remain 6 GiB on disk and 512 MiB residency.
- Evidence: private review/equipment-57-final original/DDS and repeat sheets, selected detail/revision sheets, color metrics, original hash checks and cleanup record, plus per-key provenance and current audits. Full overhaul and runtime/display/residency validation remain pending.

### Texture mechanical batch 56 - October 16, 2026

- Installed four reviewed replacements: olive medical-case atlas with clipped red cross, red-striped silver panel, copper/steel machinery atlas, and circular four-quadrant steel cap. Verified every catalog payload hash and opaque alpha before compilation; original/generated/compiled/repeat sheets inspected.
- Machinery V1 made the small oval cutout and gear-center opening polygonal. V2 restores their smooth oval/circular shapes. Selected V2 and focused correction evidence retained; superseded local V1 and redundant full draft comparison recycled. Five built-in image-generation calls, four selected sources. Ten narrow fins retained (the initial prompt said eight incorrectly; original-image review governs acceptance). Original and V2 center-fitted gear-outline spectra share the strongest frequency 67; this supports similarity but does not establish exact tooth count or UV correspondence.
- Medical cross and latch/fastener layout preserved. Steel cap retains 16 outer holes, eight center holes and cross ribs. Silver panel keeps stripe/slot/fastener layout but is 18.58% darker on average; verify brushed-metal lighting/readability in final scene testing. Other luminance changes: +4.21%, -1.66%, +5.04%. Repeat checks are diagnostics, not a claim that these atlases tile seamlessly.
- Three selected sources are 1254x1254 and one is 1774x887; downsampled to 1024x1024 or 1024x512 DXT1 full mip chains. Not native 4K artwork. Added 2,448,978 pack bytes including metadata; one active pack and no game-data staging copies.
- Windows x64, source f847832 with existing uncommitted tooling: all 226 native DDS checks, worklist hash/dimension/mip/aspect/budget checks and independent inventory pass. Coverage: 226/6,812 supported keys, 6,579 need artwork and seven drafts held. Visual classification remains 387 keys; full category/embedded/procedural coverage incomplete.
- Pack is 381,985,981 bytes (364.3 MiB) including metadata; texture workspace about 1.089 GiB. Temporary exports/reviews remain below 256 MiB. Pack cap 6 GiB and residency limit 512 MiB unchanged. No game launch or runtime build; model UV, adjacency, lighting, display and residency tests remain pending.
- Evidence: private review/mechanical-56-final original/DDS and repeat sheets, detail/aperture comparisons, original hash checks, gear diagnostic and cleanup record; canonical per-key prompts include source resolution, generator file, revision history and resampling. This remains a partial overhaul.

### Texture materials batch 55 - October 16, 2026

- Generated and installed four material replacements: riveted brushed steel, ornate wood/silver cabinet atlas, steel cylinder with red lenses, and vent/control/hatch atlas. All original payload hashes and opaque channels checked; compared original, generated source, compiled DDS and repeated-boundary sheets. Four generation calls, no discarded variants or duplicate source artwork.
- Preserved cabinet inset panels, floral trim and diamond openings; the cylinder's 15-hole staggered grid, two red lenses and lower dark aperture; and the vent's eleven upper ribs/five lower ribs and circular fittings. One-dimensional profile diagnostics support inter-rib alignment but noisy peaks are not feature counts or exact UV proof. Final model/scene sampling remains pending.
- Luminance changes: +2.93%, -6.63%, +19.34%, +0.82%. Brighter brushed steel on the cylinder requires final lighting/glare validation; original red cues and dark openings remain distinguishable offline. Repeat checks do not claim that the atlases are seamless.
- All four selected sources are 1254x1254. Steel surface resampled to 2048x2048, the three prop atlases to 1024x1024, DXT1 with full mip chains. Not native 4K source art. Added 4,895,649 pack bytes including metadata.
- Classified 32 more originals, now 387 visually inspected keys. Separate alpha inspection found curved arrow segments, a partial-opacity hexagonal grid and effect masks; these remain unfinished work rather than treating their uniform RGB channels as completed textures. Archive ownership is separate from visual category.
- Windows x64 / source f847832 with existing uncommitted texture tooling: worklist hash/size/mip/aspect/budget checks, independent inventory and all 222 native DDS checks pass. Coverage: 222/6,812 supported keys; 6,583 need artwork and seven drafts remain held. Pack is 379,537,003 bytes (about 362 MiB), below 6 GiB. Texture workspace about 1.075 GiB; temporary exports/reviews remain below 256 MiB. The 512 MiB residency setting is unchanged.
- Evidence: private review/inventory-55.json and four owner sheets plus alpha sheet; materials-55-final original/DDS and repeat sheets, four detail sheets, metrics, original hash checks and landmark diagnostics; per-key prompts, generator references and actual source dimensions. No game launch, runtime build or duplicate game/pack staging. Scene/display/UV/animation/residency validation and broad coverage are still pending.

### Texture variants batch 54 - October 16, 2026

- Reviewed twelve resolution/compression variants against ten existing generated sources, then installed all twelve after original, enlarged feature, compiled DDS and repeated-boundary review. Added three amber glow variants, a flame, hand/nails, weapon equipment, clothing/gear, grated trim, partial face/uniform, corrected eye/mouth, creature torso and wall panel. No new source image copies or image-generation calls.
- Every catalog payload for the twelve keys passed original SHA-256 verification. Multiple records sharing a key decode identically; all original and compiled alpha channels are opaque. Eye glint/iris, tooth rows, nail islands, facial openings and equipment cues checked enlarged. The weapon variant is 14.58% darker on average; darker recesses retain readable orange cues offline, with final scene lighting validation pending.
- Source art remains 1254x1254 or 1774x887; output sizes are 256 through 2048 pixels with full mips and recorded resampling. These are not native 4K sources. The 64px glow uses 256px, and the smaller clothing atlas uses 1024px to limit storage.
- Windows x64, source f847832 with existing uncommitted tooling: all 218 installed DDS files pass the native validator; worklist hash/dimension/mip/aspect/budget checks and independent inventory pass. No runtime rebuild, game launch or duplicate game/pack staging.
- Coverage is 218 of 6,812 supported keys; 6,587 need artwork and seven drafts remain held. Three generation gaps remain included in unfinished work. 355 keys have visual classification; category-wide review and embedded/procedural coverage remain incomplete. Pack is 374,641,354 bytes (357.3 MiB) including metadata, below 6 GiB; texture workspace about 1.05 GiB. Runtime/model/animation/adjacency/display/residency checks remain pending; 512 MiB residency setting unchanged.
- Evidence: private review/variants-54.json, variants-54-final metrics and three final/repeat sheets, two enlarged detail sheets, per-key prompt reuse provenance, progress.json, inventory.json, worklist.json and current native DDS audit log. This is a partial pack, not a completed remaster.

### Texture overhaul: equipment batch 50 (offline)

- Added skin/blue-gold hardware, soft ivory/gold material, pink hand/nail and dark tapered metal-channel atlases. The private pack now has 194 replacements (~341 MiB), within 6144 MiB. Five source payload records and all four compiled outputs are opaque.
- Enlarged review retains hardware ring/gold notch, blue islands and three oval marks; hand retains two pale nails and skin islands; metal retains tapered/stepped channels and right cylindrical shading. Bounded blue-mark minima remain rows 13,33 and shift 54 to53. An initial wider lower window included adjacent bottom shadow; corrected diagnostics are not exact geometric centers.
- Pale pattern remains ambiguous; soft light/dark shapes and the gold patch were preserved without inventing a material identity. Actual model assignment and sampling remain unverified. Luminance changes -6.76%, -1.16%, -7.62%, -15.00%; sharper skin folds, bright hardware and darker metal shadows need final scene/lighting review.
- Four built-in 1774x887 sources downsampled to 1024x512 DXT1 full mip chains, adding about 1.34 MiB. These are not native 4K artwork. Recycled redundant enlarged originals; retained chosen art, exact prompts, compact atlas/repeat/detail sheets and diagnostic JSON. No gameplay launch or duplicate game/pack tree.
- All 194 DDS files passed native format validation; worklist and independent inventory passed hashes, dimensions, full mips and storage budgets. Texture workspace ~1025 MiB; temporary originals/reviews/previews below 256 MiB. Revision f847832ef3588000b6d4251de657ea34f308fac1 with existing uncommitted tooling and concurrent launcher changes; no runtime rebuild.
- Still incomplete: 6611 keys need artwork, seven drafts need correction and three generation rejections remain explicit gaps. Final scene/model/display/residency checks deferred. Evidence: private/texture-remaster/review/equipment-50-* and progress.json.

### Texture overhaul: metal panels batch 49 (offline)

- Added brown weathered trim, a gray four-rib strip, green lit panel and gold highlighted panel. The active private pack now has 190 replacements (~340 MiB), within 6144 MiB. Six source payload records and all four outputs are opaque.
- Brown middle seam remains source row37; lower lip minimum shifts row57 to58. Gray rib highlights remain rows25,37,50,63; top padding remains approximately original RGB24 and two recessed marks are retained. Green lower highlight remains row51. Gold highlight peak shifts x66 to64; dark lower trim remains row60. Profile measurements are appearance diagnostics, not exact UV/model proof.
- Green panel originally has a black right edge; compiled mean RGB at source scale is 5.70/7.17/3.92. This small edge lift remains a final scene-check item. Average luminance changes -10.28%, -6.88%, -9.30%, -5.84%; stronger metal grain, oxide and edge contrast need final lighting checks. Repeat sheets are diagnostic rather than evidence of actual scene adjacency.
- Four built-in 1774x887 sources downsampled to 1024x512 DXT1 with full mip chains, adding about 1.34 MiB. These are not native 4K artwork. One active pack and source art only; no duplicate game/pack trees or gameplay launches.
- All 190 DDS files passed the existing native format validator; worklist and independent inventory passed hashes, dimensions, full mips and storage budgets. Texture workspace ~1012 MiB; temporary originals/reviews/previews remain below 256 MiB. Source revision f847832ef3588000b6d4251de657ea34f308fac1 with existing uncommitted tooling; no runtime rebuild. Concurrent launcher/release changes preserved.
- Still incomplete: 6615 keys need artwork, seven drafts need correction and three generation rejections remain explicit unfinished gaps. Final scene/model/display/residency checks deferred. Evidence: private/texture-remaster/review/metal-49-* and progress.json.

### Texture overhaul: warning panels batch 48 (offline)

- Added red, amber and cyan warning strips plus a green disc/bar-bank panel. The private pack now has 186 replacements (~339 MiB), within 6144 MiB. Five source payload records and all four installed outputs are opaque.
- Classified 32 additional original keys (280 to 312), retaining archive-owner versus visual-category distinctions. New alpha-only corner/radial/rectangular masks, flame/smoke frames and translucent/cracked panels remain unfinished pending channel/blend/animation analysis. No scope exclusions added. Inventory evidence: private/texture-remaster/review/inventory-48-*.jpg and inventory-48.json.
- All warning strips retain eight diagonal runs at source row35 and dominant horizontal frequency eight. Run starts shift <=1,2,1 source pixels; yellow mask IoU 0.8924, 0.8180, 0.9403. Green bar-bank row20 retains twelve threshold runs; one endpoint shifts one pixel. Enlarged pattern, disc, color variants and light hotspots reviewed; metrics are diagnostics, not model/UV proof.
- Mean luminance changes -4.50%, -8.73%, +2.87%, -7.91%. Stronger lamp hotspots, finer ribbing and possible color-variant switching need final scene/animation checks. Repetition sheets do not prove actual model adjacency.
- Four built-in 1774x887 sources downsampled to 1024x512 DXT1 with full mip chains. These smaller targets add about 1.34 MiB, not native 4K artwork. Recycled the redundant enlarged original sheet, retained compact inventory/alpha and review evidence.
- All 186 DDS files passed native format validation; worklist and independent inventory passed hashes, dimensions, full mips and storage budgets. Texture workspace ~998 MiB; temporary originals/reviews/previews remain below 256 MiB. No game launch or runtime rebuild; revision f847832ef3588000b6d4251de657ea34f308fac1 with existing uncommitted tooling.
- Still incomplete: 6619 keys need artwork, seven drafts need correction and three generation rejections remain explicit unfinished gaps. Final scene/model/display/residency checks deferred. Evidence: private/texture-remaster/review/warnings-48-*, visual-classification.json and progress.json.

### Texture overhaul: surfaces batch 46 (offline)

- Added separate green and blue-gray riveted panels, a dense mechanical armor atlas and a recessed industrial panel. The private pack now has 179 replacements (~331 MiB), within 6144 MiB. Six source payload records and all four outputs are opaque.
- Retained thirty rivets per panel; local dark-point shifts <=2px horizontal/3px vertical are diagnostic rather than exact centers. Enlarged armor comparisons retain the major three-hole housing, slot groups, four-hole white plate and lower vent banks. Recessed panel retains nine side slots per side, paired bottom controls and hazy center; tiny original lettering is unreadable and no new wording is claimed.
- Corrected the recessed panel's new dark outer-edge seam in revision 2. Left/right edge MAE: original 5.85, v1 32.47, v2 11.64. The revised pale border is brighter than source and awaits scene verification. Final luminance changes -7.26%, -12.04%, +13.10%, -0.24%; darker paint and brighter armor metal require final lighting checks.
- Five built-in image generations including one correction. Accepted source art is two 1774x887 and two 1254-square images, resampled to two 2048x1024 and two 2048-square DXT1 full mip chains; not native 4K artwork. Recycled superseded local panel art, replaced compile files and four redundant review images; retained prompts, generator originals and focused correction evidence.
- All 179 DDS files passed native format validation; worklist and independent inventory passed hashes, dimensions, full mips and storage budgets. Texture workspace ~968 MiB; temporary originals/reviews/previews ~82 MiB, below 256 MiB. Revision f847832ef3588000b6d4251de657ea34f308fac1 with existing uncommitted tooling; no runtime rebuild or gameplay launch.
- Still incomplete: 6626 keys need artwork, seven drafts need correction, and two generation rejections remain explicit unfinished gaps. Consolidated scene/model/display/residency checks remain deferred. Evidence: private/texture-remaster/review/surfaces-46-final-*, surfaces-46-details.jpg, surfaces-46-correction.jpg, surfaces-46-landmarks.json and progress.json.

### Texture overhaul: MapMaker batch 45 (offline)

- Added four modernized MapMaker surfaces: separate red-circuit and charcoal technical walls, a silver doorway and an ON/OFF warning panel. The active private pack now contains 175 replacements (~323 MiB), within its 6144 MiB cap. No game launch or duplicate game/pack tree.
- Eight source payload records and all four outputs are opaque. Four 1254-square built-in generated sources were resampled to 2048-square DXT1 with full mip chains; these are not native 4K artwork.
- Offline comparisons preserve nine circular features including cropped edges, tapered insets, four large panel slots, the lower double slit and yellow doorway bands. Enlarged original labels read ON / OFF after a vertical diagnostic flip; exported UV orientation and warning icon remain as supplied.
- Doorway black-opening component matches at original scale: bbox [90,20,166,208], 14553 pixels, centroid [128,114], IoU 1.0. Mean luminance changes -2.81%, -5.23%, +1.98%, -5.46%. Refined circuit traces, stronger paint, brighter fine edges/fasteners and darker charcoal right edge need final scene review; repeat diagnostics show no new large boundary band.
- All 175 DDS files passed the existing native format validator. Worklist and independent inventory passed hashes, dimensions, full mips and storage budgets. Texture workspace ~947 MiB; temporary originals/reviews/previews ~81 MiB, below 256 MiB. Revision f847832ef3588000b6d4251de657ea34f308fac1 with existing uncommitted tooling; no runtime rebuild.
- Coverage remains incomplete: 6630 keys need artwork and seven drafts need correction; two generation rejections remain explicit unfinished gaps. Final scene/model/alpha/display/residency tests remain deferred under the agreed near-completion gate. Evidence: private/texture-remaster/review/mapmaker-45-* and progress.json.

- Texture overhaul batch 44: four machinery/weapon textures accepted offline, bringing the local pack to 171 replacements (about 312 MiB of the 6144 MiB cap). Texture workspace about 921 MiB; original exports/reviews about 79 MiB, below the 256 MiB temporary limit. One active pack, no duplicate game copies or staging trees.
  - Oblong plate/recesses, hub/pipe/grille machinery panel, angular weapon atlas and dark tubular atlas modernized. Enlarged comparisons checked vent counts, grille, pipes, cutouts, seam layout and red/orange markings. Main plate seam remains at source column 295; orange indicator centroid shifts under one source pixel/axis.
  - Circular-opening threshold mixed the opening with adjacent cylinder shadow, so that measurement is explicitly inconclusive for geometry. Enlarged comparison retained; brighter cylinder separates more strongly from the opening. Overall luminance increases +5.69%, +9.51%, +6.11%, +8.57% require final scene lighting review.
  - Six source payload records opaque; each machinery plate's duplicate payload records decode identically. All four compiled DDS files opaque. Four 1254-square built-in sources resampled to three 2048-square and one 1024-square DXT1 full mip chains. Not native 4K artwork.
  - All 171 DDS files passed native format validation; worklist and independent inventory passed. Evidence: private/texture-remaster/review/machinery-44-1.jpg, machinery-44-repeat-1.jpg, machinery-44-details.jpg, machinery-44-opening.jpg and machinery-44-landmarks.json. Windows offline tools at f847832ef3588000b6d4251de657ea34f308fac1 with existing uncommitted texture tooling. No runtime rebuild or game launch.
  - Coverage remains incomplete: 6,634 supported keys need artwork (including two generation gaps), seven draft keys remain held, 280 keys visually classified, and non-PAK/procedural coverage remains open. Scene/model, seams, animation, lighting, display and residency validation remain deferred until near completion.

- Texture overhaul batch 43: three equipment/material atlases accepted offline, bringing the local pack to 167 replacements (about 303 MiB of the 6144 MiB cap). Texture workspace about 898 MiB; original exports/reviews about 77 MiB, below the 256 MiB temporary limit. Superseded face DDS files/metadata, obsolete draft PNG and eight redundant full comparison/repeat sheets recycled; close-up rejection evidence and prompt/compile provenance retained.
  - Light and dark leather/cloth/metal equipment versions authored independently after source comparison showed RGB MAE 27.29. Their brightness distinction is retained. Separate gray/brown/gold equipment atlas reviewed for strap rectangles, trim, fittings and folds. Accepted luminance changes +3.36%, +6.76%, -5.17%; final scene lighting/UV checks pending.
  - Partial-face pair source RGB MAE 2.148 and matching alpha/layout supports shared artwork, but both keys remain held. Initial generation changed the upper iris blue; first correction removed the blue but broadened the pupil/lost its highlight; second correction still missed the original eye appearance. Both originals remain in use. No separate PNG copy for the lower-resolution candidate.
  - Six 1254-square built-in outputs including two corrections. Three accepted sources resampled to two 2048-square and one 1024-square DXT1 full mip chains. Not native 4K artwork. Five source records opaque and three accepted DDS files opaque.
  - All 167 installed DDS files passed native format validation; worklist and independent inventory passed. Evidence: private/texture-remaster/review/equipment-43-accepted-1.jpg, equipment-43-accepted-repeat-1.jpg, equipment-43-details.jpg, equipment-43-variants.jpg and equipment-43-eye-revisions.jpg. Windows offline tools at f847832ef3588000b6d4251de657ea34f308fac1 with existing uncommitted texture tooling. No runtime rebuild or game launch.
  - Coverage remains incomplete: 6,638 supported keys need artwork (including two generation gaps), seven draft keys remain held, 280 keys visually classified, and non-PAK/procedural coverage remains open. Scene/model identity, seams, animation, lighting, display and residency validation remain deferred until near completion.

- Texture overhaul batch 42: four panel/prop textures accepted offline, bringing the local pack to 164 replacements (about 297 MiB of the 6144 MiB cap). Texture workspace about 878 MiB; original exports/reviews about 74 MiB, below the 256 MiB temporary limit. One active pack and no duplicate game/staging trees.
  - Yellow riveted panel, diagonal tread plate, red/silver recessed-slot plate and green medical-case atlas modernized. Thirty rivets retained visually; slot silhouette IoU 0.9939 and red-cross silhouette IoU 0.9760 at original scale. Tread dominant frequencies retained with best local alignment at zero offset. These diagnostics supplement visual comparison and do not prove runtime UV/tiling behavior.
  - Five original payload records and all four DDS files opaque. Tread/slot/case luminance changes -14.95%, -13.49%, -9.49%; darker materials and stronger edge highlights require final lighting review. Yellow panel edge variation increased modestly and remains flagged for scene review.
  - Three 1774x887 and one 1254-square built-in source images resampled to three 2048x1024 and one 2048-square DXT1 full mip chains. Not native 4K artwork. Another 32 originals visually classified across four archive owners, bringing inspected classification to 280 keys; category-wide review remains unfinished.
  - All 164 DDS files passed native format validation; worklist and independent inventory passed. Evidence: private/texture-remaster/review/panels-42-1.jpg, panels-42-repeat-1.jpg, panels-42-landmarks.json and inventory-42.json. Windows offline tools at f847832ef3588000b6d4251de657ea34f308fac1 with existing uncommitted texture tooling. No runtime rebuild or game launch.
  - Coverage remains incomplete: 6,643 supported keys need artwork (including two explicit generation gaps), five drafts remain held, and non-PAK/procedural coverage remains open. Scene/model, animation, lighting, display and residency validation remain deferred until near completion.

- Texture overhaul batch 41: three character atlases accepted offline, bringing the local pack to 160 replacements (about 291 MiB of the 6144 MiB cap). Texture workspace about 856 MiB; original exports/reviews about 73 MiB, below the 256 MiB temporary limit. Redundant initial contact sheet recycled; final comparisons and source provenance retained.
  - Red/white uniform, partial-face/armor and pale damaged-uniform atlases modernized. Enlarged rotated face comparisons, clothing boundaries, separate eye/lip regions, hair, badges and control colors reviewed. Local eye-centroid shifts below one original pixel per axis; contrast-dependent red-mark mask area increased 16.5% within the original general footprint. These diagnostics do not prove identity on animated models or exact UV seam behavior.
  - Four source payload records for installed keys and all compiled DDS files opaque; partial-face payload variants decode identically. Two 1254-square and one 1774x887 source images resampled to two 2048-square and one 2048x1024 DXT1 full mip chains. Not native 4K artwork.
  - One separate stained-clothing atlas, b22a38a91d990c75, was rejected by the built-in image generator at output moderation (violence; request 6aa0e4fb-3651-4d5f-bc57-862504253599). No retry or output; original remains in use. Exact prompt/rejection recorded privately. This is the second explicit generation gap and remains in needs_art, separate from held drafts.
  - All 160 DDS files passed native format validation; worklist and independent inventory passed. Evidence: private/texture-remaster/review/characters-41-1.jpg, characters-41-repeat-1.jpg, characters-41-details.jpg and characters-41-landmarks.json. Windows offline tools at f847832ef3588000b6d4251de657ea34f308fac1 with existing uncommitted texture tooling. No runtime rebuild or game launch.
  - Coverage remains incomplete: 6,647 supported keys need artwork, five drafts remain held, classification and non-PAK/procedural coverage remain open. Model identity, seams, animation, lighting, display and residency validation remain deferred until near completion.

- Texture overhaul batch 40: three equipment/architecture atlases accepted offline, bringing the local pack to 157 replacements (about 284 MiB of the 6144 MiB cap). Texture workspace about 840 MiB; original exports/reviews about 72 MiB, below the 256 MiB temporary limit. One active pack; no duplicate game copies or staging trees.
  - Distinct yellow/silver equipment, cropped circular metal fixture, and cockpit atlas modernized. Enlarged source/compiled comparisons reviewed gauge/readout patterns, radio controls, nine-hole plate, colored controls, ports, concentric rims and UV islands. Tiny radio marks remain pixel-limited, with no exact transcription asserted. Warmer fixture corrosion and brighter perimeter require final scene lighting checks.
  - Five original payload records opaque; the fixture's three records decode to identical top-mip pixels. Three generated sources (two 1254-square, one 1774x887) resampled to 1024-square, 2048-square and 2048x1024 DXT1 full mip chains. Not native 4K artwork.
  - All 157 DDS files passed native format validation; worklist and independent inventory passed. Evidence: private/texture-remaster/review/equipment-40-1.jpg, equipment-40-repeat-1.jpg, equipment-40-details.jpg, equipment-40-radio.jpg and equipment-40-landmarks.json. Windows offline tools at f847832ef3588000b6d4251de657ea34f308fac1 with existing uncommitted texture tooling. Supported default.xbe SHA-256 reconfirmed. No runtime rebuild or game launch.
  - Coverage remains incomplete: 6,650 supported keys need artwork, five drafts remain held, classification and non-PAK/procedural coverage remain open. Model seams, animation, lighting, display and residency validation remain deferred until near completion.

- Texture overhaul batch 39: four armor/equipment atlases accepted offline, bringing the local pack to 154 replacements (about 279 MiB of the 6144 MiB cap). Texture workspace about 824 MiB; original exports/reviews about 70 MiB, below the 256 MiB temporary limit. Two superseded local draft PNGs and a redundant repeat sheet recycled; revision prompts and rejected comparison evidence retained.
  - Two corrections restored three equipment recesses, six green screen indicators, and the padded atlas gray-right/green-left fittings. Six 1254-square generator outputs including two revisions; four selected sources resampled to three 1024-square and one 2048-square DXT1 full mip chains. Not native 4K artwork.
  - Enlarged original/compiled comparisons, atlas boundaries, repeat sheets, alpha and indicator colors reviewed. Green-run threshold did not reliably count the low-resolution original; visual review was used instead. Robot highlights are brighter and model/lighting validation remains pending.
  - All 154 DDS files passed native format validation; worklist and independent inventory passed. Evidence: private/texture-remaster/review/armor-39-final-1.jpg, armor-39-final-repeat-1.jpg, armor-39-corrections.jpg and armor-39-final-metrics.json. Windows offline tooling at f847832ef3588000b6d4251de657ea34f308fac1 with existing uncommitted texture tooling; no runtime rebuild or game launch.
  - Coverage remains incomplete: 6,653 supported keys need artwork, five drafts remain held, classification and non-PAK/procedural coverage remain open. Consolidated scene testing stays deferred until the agreed near-completion gate.

- Texture mechanical batch 38: four equipment/robot atlases accepted offline; 150 installed / about 275 MiB. Twenty copper holes align within one source pixel; eight-hole circular port, copper slats and robot fittings reviewed enlarged. Isolated red disc mask IoU 0.947 and robot pale indicator 0.891. Brighter bronze/red coating and sharper edge wear require final model and lighting checks.
- All 150 DDS files passed native validation and worklist hash, mip, aspect and budget audits. Two 1254-square and two 1774x887 built-in sources resampled to 2048-square, 1024-square, 2048x1024 and 1024x512 DXT1 with full mips. No game launches. Texture workspace about 806 MiB; 6657 supported keys need art plus five held drafts. Full overhaul and category/scene validation remain unfinished.

- Texture panels/hatches batch 37: five materials accepted offline; 146 installed / about 270 MiB. Brown hatch outer/small ring fits match; silver hatch fitted center/radius differ by 0.5 source pixel and small fitting centroid by 0.34 pixel. Paired wall center seam matches. Sharper grain, rim highlights and modular edge contrast require final scene lighting/adjacency checks.
- All 146 DDS files passed native validation and worklist hash, mip, aspect and budget audits. Four 1254-square and one 1774x887 built-in sources resampled to three 1024-square, one 2048-square and one 2048x1024 DXT1 with full mips. No game launches. Texture workspace about 789 MiB; 6661 supported keys need art plus five held drafts. Full overhaul and category/scene validation remain unfinished.

- Texture metal trim batch 36: four wall/trim materials accepted offline; 141 installed / about 264 MiB. Ribbed inset keeps 60 profile peaks within one source pixel. Curved/straight yellow stripe masks have IoU 0.957/0.971; green rivet positions match within one pixel and steel trim within two. Original curved-band gap preserved; stronger edge highlights and darker steel remain final lighting/adjacency checks.
- All 141 DDS files passed native validation and worklist hash, mip, aspect and budget audits. Four 1774x887 sources resampled to 2048x1024 DXT1 with 12 mips. No game launches. Texture workspace about 768 MiB; 6666 supported keys need art plus five held drafts. Full overhaul and category/scene validation remain unfinished.

- Texture equipment batch 35: two armor/character atlases accepted offline, for 137 installed / about 258 MiB. Armor rib peaks match within one source pixel; face-eye dark masks have IoU 0.875/0.913 with centroids within 0.25 source pixel per axis. Full model identity and lighting checks remain pending. Two blue-uniform drafts held for reinterpreted small equipment labels, with one unsuccessful local correction; their DDS files retired.
- All 137 DDS files passed native validation and worklist hash, mip, aspect and budget audits after concurrent repository merge restored texture tools. Two 1254-square sources resampled to 2048- and 1024-square DXT1. No game launches. Visual classification expanded by 32 to 248 keys; 6670 supported keys need art plus five held drafts. Texture workspace about 750 MiB; full overhaul remains unfinished.

- Online source-build integration: a new network-menu entry, HTTPS browser with
  certificate fingerprint checking, public/private rooms, compatible-only
  listing/joining, peer credentials and leases, and a virtual-address bridge
  using the game host's forwarded UDP port. Guests establish outbound paths;
  the host relays guest-to-guest traffic. Two local instances used Online and
  an HTTPS directory to join a 2/16-player lobby and enter the same Shrink match.
  Cross-internet routing and full-match stability remain unverified. Campaign
  co-op remains a separate future effort.
  Automatic hosting now detects public IPv4 over HTTPS, starts/stops a local
  directory with the bridge, reuses its certificate and offers a copyable
  invitation containing the directory address, fingerprint and join code.

- Experimental PC System Link: native framed UDP, strict build/content identity
  checks, default-protocol discovery sockets, and XNADDR/key mapping. Two isolated
  game instances discovered a match, joined a 2/16-player lobby, and loaded into
  the same Thief match. This is a same-PC loopback test, not a cross-network test
  or a completed-match validation. See [multiplayer status](docs/multiplayer.md)
  for the later Online integration and remaining cross-network validation.
- Texture architecture batch 34: illuminated wall, horizontal wall strip, hazard paint and carved trim accepted offline; 135 installed / about 255 MiB. Stripe pitch and phase retained, wall bright seams match at source rows 31/98, lamp bright centroid within 0.16 source pixel per axis. Carved motifs and atlas bands reviewed at enlarged scale. Stronger wear/contrast and brightness remain final scene checks.
- All 135 DDS files passed native validation and worklist hash, mip, aspect and budget audits. Three 1254-square and one 1774x887 generated sources resampled to two 2048-square, one 2048x1024 and one 1024-square DXT1. No game launches. Texture workspace about 733 MiB; 6674 supported keys need art plus three held drafts. Full overhaul and category/scene validation remain unfinished.

- Texture walls batch 33: three wall replacements accepted offline (green-gray panel and two pale marked walls), for 131 installed / about 248 MiB. The split green panel remains held after its edge correction still darkened the repeat boundary; failed DDS retired. Distinct dark marks and seam positions reviewed. All 131 DDS files passed native validation and worklist hash, mip, aspect and budget audits.
- Three 1254-square sources resampled to two 2048-square and one 1024-square DXT1; not native 4K. No game launches. Texture workspace about 711 MiB; pack remains below 6 GiB. 6678 supported keys need art plus three held drafts, with category and final scene validation unfinished.

- Texture walls/variants batch 32: four green-gray wall materials and two visually verified armor-resolution variants installed, for 128 replacements / about 242 MiB. Reused selected source artwork for armor variants without duplicate PNGs. Source comparisons RGB MAE 2.88/2.86, alpha differences zero; layouts reviewed. All 128 DDS files passed native validation and worklist audits.
- Wall repetition and geometry checked: 32 yellow bars retain exact source-scale centers; conduit rod edge positions match. New wall sources are 1254-square, resampled to 2048-square; armor outputs 1024-square from existing sources. Final scene/lighting checks pending, no game launches. Visual inventory expanded by 64 to 216 keys. 6682 supported keys need art plus two held drafts; full overhaul remains unfinished.

- Texture warning/effects batch 31: two mirrored warning panels and two fire sprites accepted offline, for 122 installed / about 230 MiB. NEVER LET FALL / INTO ENEMY HANDS text verified in diagnostic reading views; original vertical UV flip retained. Flame/explosion brightness changes -0.45%/+4.29%, luminous centroids within one source pixel per axis; animation/blend/scene checks remain pending.
- All 122 DDS files passed native validation and worklist hash, dimension, mip, aspect and budget audits. Two 1774x887 sources compiled to 1024x512 DXT1; two 1254-square sources to 1024- and 512-square RGBA8 for smooth effect falloff, with original opaque black backgrounds. Resampled art, not native 4K. No game launches. 6688 supported keys need art plus two held drafts; full overhaul unfinished.

- Texture armor/panels batch 30: four offline-reviewed replacements added (two armor atlases and two mechanical panels), for 118 installed / about 222 MiB. Native DDS and worklist hash, dimension, mip, aspect and budget audits passed. Full installed alpha audit checked 134 catalog payload records for 118 keys and found no lost/introduced transparency categories; exact shader/scene behavior still awaits final validation.
- Built-in sources are two 1254-square and two 1774x887 images, resampled to two 1024-square and two 1024x512 DXT1 outputs. Cyan cue, black opening and small recess details reviewed; changed wear/contrast remains a final lighting check. No game launches. 6692 supported keys need art plus two held drafts; full overhaul remains unfinished.

- Texture machinery batch 29: three offline-reviewed mechanical atlases added, for 114 installed replacements / about 220 MiB. All 114 DDS files passed native validation and worklist hash, mip, aspect and budget audits. Sources are 1254-square, resampled to one 2048-square and two 1024-square outputs. No gameplay launches.
- Visual classification expanded to 152 keys with 32 additional shared, character, weapon and MapMaker-owner candidates. Wheel artwork remains held after offline checks caught a missing transparent bottom row; its opaque DDS was recycled. Radial geometry is also still held. 6696 supported keys need art plus two drafts; full scope and runtime validation remain incomplete.

- Texture remainder batch 28: two more offline-reviewed replacements (mottled prop strip and creature atlas), bringing the active pack to 111 files / about 216 MiB. All 111 DDS files passed native validation; worklist hash, mip, aspect and budget audits passed. Generated 1774x887 and 1254-square art resampled to 1024x512 and 1024-square; not native 4K. No game launches.
- Expanded appearance inventory from 96 to 120 keys: shared UI/controller symbols, alpha masks and a particle need semantic/alpha-sensitive treatment. Radial-panel revision 3 still omits intermediate geometry and remains uninstalled. 6700 keys need art plus one held draft; other encoding/coverage gaps and final scene tests remain open. Pack and review evidence stay private.

### Warning markings and ring correction batch 27 (offline)

- Added two numbered warning atlases and accepted revision 3 of held circular fitting 8c1c795671ee976a. 109 installed replacements. Two 1254-square and one 1774x887 sources resampled to two 1024-square and one 1024x512 DXT1, full 11 mips.
- Read vertically flipped comparisons: 04 and Caution / weapon may / explode when hot retained. Texture outputs keep original vertically mirrored UV orientation. Small-atlas wording inferred from matching clearer label; its original fine text is not independently legible.
- Ring revision used original alone; 256-scale inner-edge fit matches original center [101,187], radius 48. Rejected v1 was [99,178], radius 46. Visual review supports correction; fit is a heuristic, not full model proof. Failed local source recycled, generator source/prompts and prior comparison retained.
- Mean luminance changes +2.38%, -10.90%, -10.54%; all alpha opaque. Flatter warning border shading, stronger edge wear and dark red remain final scene/model checks. No gameplay launches.
- All 109 DDS passed native format/hash/dimension/aspect/mip audits. Pack about 215 MiB of 6144 MiB; texture workspace about 603 MiB. Temporary files below 256 MiB; redundant reading/source previews recycled.
- Remaining: 6702 supported keys need art and one radial draft needs correction. Category-wide review, discovery and runtime validation unfinished. Evidence: private/texture-remaster/review/markings-27-1.jpg, markings-27-repeat-1.jpg, markings-27-labels-reading.png, markings-27-ring-fit.json and markings-27-metrics.json.

### Weapon and organic materials batch 26 (offline)

- Added two weapon atlases and two organic wall materials. Sources three 1254-square plus one 1774x887; outputs one 1024-square, one 1024x512 and two 2048-square DXT1 with full mips. Resampled, not native 4K generated art.
- Compiled/repeated comparisons inspected: bottom-clipped fitting, six-chamber cylinder, wood-grip outline and organic macro folds/ridges retained. Fine metal relief and organic gloss need final model/scene review.
- Mean luminance changes +2.40%, -3.18%, -17.05%, -14.55%; all alpha opaque. Darker wall materials and brighter fine highlights are explicit scene-lighting checks.
- Organic repeats retain visible boundaries; no seamless claim. Edge MAE first material LR 9.79 to 8.40/TB 10.92 to 8.10; second LR 9.24 to 12.07/TB 5.91 to 9.09. Actual wall adjacency unverified.
- All 106 installed DDS passed native format/hash/dimension/aspect/mip audits. Pack about 214 MiB of 6144 MiB; texture workspace about 596 MiB. Temporary originals/reviews remain below 256 MiB. Redundant source contact sheet recycled; no gameplay launches.
- 6704 supported keys need art plus two held drafts; discovery, category review and final runtime validation unfinished. Evidence: private/texture-remaster/review/mixed-26-1.jpg, mixed-26-repeat-1.jpg, mixed-26-metrics.json. Exact prompts under private/texture-remaster/authored.

### Dark metal and frame atlases batch 25 (offline)

- Added four dark-metal/weapon-frame atlases. Two 1774x887 and two 1254-square sources resampled to two 1024x512 and two 1024-square DXT1 with 11 mips. Not native 4K generated art.
- Compiled atlas/repetition and enlarged grip detail reviewed. Lower grip inset exists in original; generated border is sharper. Frame prompt miscounted three narrow grooves; original and result have two narrow recessed grooves beside longer framing lines, correction recorded with exact prompt preserved.
- Main panel shapes, five-hole clusters and red cues retained visually. Slot mask overlap 0.9850; red-cue overlaps 0.7931 and 0.5556. d255 red threshold area is 1.561 times original: stronger tiny cues need final readability/lighting checks. Threshold masks are brightness-sensitive and do not prove UV registration.
- Mean luminance changes -10.83%, -10.22%, +10.06%, +6.40%; all alpha opaque. Model adjacency, specular appearance and scene lighting remain pending. No gameplay launches.
- All 102 installed DDS files passed native format/hash/dimension/aspect/mip checks. Pack about 207 MiB of 6144 MiB; texture workspace about 579 MiB. Temporary files below 256 MiB; redundant source contact sheet recycled.
- 6708 supported keys need art plus two held drafts; category-wide review and discovery remain unfinished. Evidence: private/texture-remaster/review/weapons-25-1.jpg, weapons-25-repeat-1.jpg, weapons-25-grip-detail.png and weapons-25-metrics.json. Exact prompts retained under private/texture-remaster/authored.

### Tan and silver atlases batch 24 (offline)

- Added four tan/silver mechanical atlases with preserved panel boundaries, openings, fastener/slot positions and amber visual cue. Generated sources 1254-square, resampled to 1024-square DXT1 with 11 mips; not native 4K generation.
- Compiled atlas/repetition sheets and enlarged display/vent comparisons inspected. Mean luminance changes -0.36%, +5.49%, +11.97%, +8.70%; all alpha opaque. Brighter silver, sharper bevels and material wear require final weapon-model/scene checks.
- Amber source marks are not reliably readable numerals; original abstract segment arrangement retained without inventing digits. This is not OCR verification. Silver vent has nine main ridges in both versions; measured maxima within three original pixels, profile correlation 0.9467. Enlarged review resolved initial spacing concern.
- All 98 installed DDS files passed native format and hash/dimension/aspect/full-mip audits. Pack about 205 MiB of 6144 MiB; texture workspace about 566 MiB. Temporary files below 256 MiB; redundant original previews recycled. No gameplay launches.
- 6712 supported keys still need art and two held drafts need correction. Discovery/category review and final scene validation remain unfinished.
- Evidence: private/texture-remaster/review/weapons-24-1.jpg, weapons-24-repeat-1.jpg, weapons-24-counter-comparison.png, weapons-24-vent-comparison.png and weapons-24-metrics.json. Exact prompts retained under private/texture-remaster/authored.

### Weapon atlases batch 23 (offline; one held)

- Added three mechanically detailed grey/blue-green atlases with preserved main openings, chip patterns and indicator positions. Two 1254-square and one 1774x887 sources resampled to two 1024-square and one 1024x512 DXT1 with 11 mips. Not native 4K generated art.
- Actual compiled atlas/repetition sheets reviewed; all 94 installed files passed native DDS and hash/dimension/aspect/mip audits. Mean luminance differences -14.45%, -14.30%, -2.96%; sharper relief and darker metal require final model/lighting validation. All alpha opaque.
- First atlas retained copper hole groups 6,4,4,6. Red cue threshold bounds original [112,107,125,117], replacement [112,106,125,117], overlap 0.832 and areas 110/119; heuristic only, blending still untested.
- Held 8c1c795671ee976a: ring moved upward and became smaller. Revision also failed layout registration. Provisional DDS/metadata recycled; original stays active. Selected failed source and exact prompts retained without a duplicate revision image in the project.
- Pack about 203 MiB of 6144 MiB; texture workspace about 552 MiB. Temporary files below 256 MiB, redundant review previews recycled. No gameplay launches.
- 6716 keys need art plus two held drafts; full coverage, category review and discovery remain unfinished. Evidence: private/texture-remaster/review/weapons-23-accepted-1.jpg, weapons-23-accepted-repeat-1.jpg, weapons-23-accepted-metrics.json; fourth row of weapons-23-1.jpg records the held ring.

### Armor and props batch 22 (offline)

- Added segmented grey armor, damaged mechanical armor, dark clothing/skin atlas and circular fitting: four replacements, bringing the pack to 91. All passed native DDS and hash/dimension/aspect/mip audits.
- Enlarged originals corrected the clothing candidate classification: brown sections are skin islands, not padding. Skin-color mask overlap 0.9912 and cyan-cue overlap 0.9417 support visual placement review; these are heuristic diagnostics, not model/identity proof.
- Actual compiled atlas and repetition sheets reviewed. Mean luminance changes -2.58%, -18.21%, +2.92%, -6.25%; all alpha opaque. Darker clothing, sharper armor relief and skin shading require final model/scene checks. Atlas repetition does not prove seamless model adjacency.
- Three 1254-square sources and one 1774x887 source resampled to two 1024-square, one 1024x512 and one 512-square DDS. Full mip chains; not native 4K generated art.
- Pack about 201 MiB of 6144 MiB; texture workspace about 537 MiB. Temporary files remain below 256 MiB; redundant enlarged/contact previews recycled. No gameplay launches.
- 6720 supported keys still need art, plus one held radial draft. Yellow panel d7bc1f2e36f004fe remains unmodified pending tiny-marking inspection. Category-wide review and discovery remain unfinished.
- Evidence and exact prompts: private/texture-remaster/review/armor-props-22-1.jpg, armor-props-22-repeat-1.jpg, armor-props-22-metrics.json and private/texture-remaster/authored.

### Machinery and indicators batch 21 (offline)

- Added four distinct 1024-square DXT1 replacements: green recessed panel, paired mechanism atlases with separate pale/orange inset states, and blue-indicator machinery atlas. Generated sources 1254-square; full 11-mip outputs are resampled.
- Compared actual compiled DDS to originals and repeated atlases; retained major island silhouettes, grille placements and indicator cues. Stronger relief and darker materials need final model/scene checks. Green pair mean luminance -17.30%/-19.50%; other panels -11.52%/-7.96%.
- Cyan indicator threshold-mask overlap 0.9036; orange polygon 0.8057 and edge slot 0.8947. Indicator areas within about 1.3% of original; these are brightness-sensitive heuristics, not UV-mapping proof.
- All 87 installed DDS files passed native validation and worklist audits. Pack about 199 MiB of 6144 MiB; texture workspace about 522 MiB. Temporary originals/reviews remain below 256 MiB. Recycled redundant original contact sheet.
- Coverage remains partial: 6724 keys need art plus one held radial draft; non-PAK discovery and category review remain open. No game launches; final scene, UV, glow, brightness, display and residency testing pending.
- Evidence: private/texture-remaster/review/machinery-21-1.jpg, machinery-21-repeat-1.jpg and machinery-21-metrics.json. Exact prompts/source provenance retained under private/texture-remaster/authored.

### 2026-10-15: grey machinery batch 20 (offline)

- Added two damaged structural atlases, a soot-marked panel and circular fitting.
  Main UV boundaries, damage openings and dark features retained in compiled
  review. Brightness changes +10.25%, +2.49%, -2.95%, -4.15%; fully opaque.
- Enlarged source disproved an initial small-preview concern: the circular
  fitting's hard horizontal seam already exists in the original. Center-hole
  bounds differ by at most one original pixel in a fixed threshold ROI; final
  model sampling and scene lighting still require validation.
- Sources: three 1254-square, one 1774x887. DXT1 outputs: two 2048-square,
  one 1024x512, one 512-square, full mips. Exact prompts/resampling retained.
  Redundant preliminary fitting comparison recycled after actual DDS review.
- All 83 DDS files passed native parsing and worklist checks. Active pack:
  206,092,669 bytes (about 197 MiB); texture workspace about 504 MiB. Temporary
  originals/reviews below 256 MiB. No gameplay launches.
- Remaining: 6,728 needing art and one held radial draft. Full overhaul unfinished.
  Private evidence: review/grey-machinery-20-1.jpg and
  grey-machinery-20-fitting-1.jpg, corresponding repeats and metrics JSON.

### 2026-10-15: container/control atlas batch 19 (offline)

- Added medical container, dark green fittings and tan control-panel atlases.
  Main islands and pickup symbols retained. Medical-cross mask IoU 0.9894,
  area ratio 1.0107; final model mapping still requires runtime validation.
- Mean brightness changes -8.51%, +2.71%, +6.47%. Fully opaque. Stronger wear
  and baked shadows remain final scene-check items. Enlarged source confirms
  three vent bars and four control-panel marks, despite erroneous five-count
  prompt wording; output followed the actual source, recorded in provenance.
- A radial correction attempt still omitted intermediate geometry. It remains
  excluded, with revision prompt and generator path recorded; no extra rejected
  PNG copied into the project. One redundant control draft comparison recycled.
- Three 1254x1254 sources resampled to 1024-square DXT1, 11 mips. All 79 active
  DDS files passed native parsing and worklist checks. Pack 199,973,659 bytes
  (about 191 MiB); texture workspace about 485 MiB, temporary evidence below
  256 MiB. No gameplay launches.
- Remaining: 6,732 needing art and one held radial draft. Full overhaul unfinished.
  Private evidence: review/containers-19-1.jpg, containers-19-control-1.jpg,
  corresponding repeat sheets and metrics JSON under private/texture-remaster.

### 2026-10-15: architecture batch 18 (offline; one held draft)

- Added ornamental trim, a shuttered masonry window, metal slats and a hazard-
  marked stepped panel. Main divisions and markings retained in compiled review.
  Brightness changes +3.09%, +8.82%, +1.59%, -12.25%; all opaque. Stronger side
  joins on slats/trim and darker hazard metal remain final scene-check items.
- A fifth generated radial-panel draft omitted intermediate segments and changed
  geometry. It is NOT compiled or installed; original remains active. Retained
  exact prompt/source and comparison for correction; not counted as complete.
- Two rectangular 1774x887 and two square 1254x1254 accepted sources resampled
  to 2048x1024 / 2048 square DXT1 with full mips. One redundant draft comparison
  recycled after actual DDS review; held-draft evidence retained.
- All 76 DDS files passed native parsing and worklist integrity checks. Pack:
  197,874,787 bytes (about 189 MiB), below 6 GiB. Texture workspace about 474 MiB;
  temporary originals/reviews below 256 MiB. No game launches.
- Coverage: 76 installed, 6,735 needs art, one held draft requiring correction.
  Full overhaul and final scene/UV validation unfinished. Private evidence:
  review/architecture-18-1.jpg, architecture-18-repeat-1.jpg,
  architecture-18-metrics.json and architecture-18-draft-2.jpg.

### 2026-10-15: rock and plaster batch 17 (offline)

- Added two layered rock surfaces, one scorched rock-like impact surface and
  cracked plaster. Main strata, crack shapes and scorch location preserved in
  compiled review. Refined the prior provisional soil classification after
  enlarged inspection; actual scene orientation/use still unverified.
- Mean brightness changes +2.43%, +4.30%, -3.68%, +8.14%; all opaque. Repeated
  boundaries reviewed. The cliff has a visible horizontal ledge boundary in both
  original and replacement; not claimed seamless. Warmer rock hue and brighter
  plaster remain final scene-check items.
- Four built-in generated sources are 1254x1254, resampled to three 2048 square
  and one 1024 square DXT1 outputs, full mips. Exact prompts retained privately.
- All 72 DDS files passed native parsing and worklist integrity checks. Pack:
  189,483,829 bytes (about 181 MiB), within 6 GiB. Texture workspace about 448 MiB;
  temporary originals/reviews remain below 256 MiB. No gameplay launches.
- 6,740 known supported keys still need art. Other encodings and final runtime
  validation remain unfinished. Private evidence: review/mineral-surfaces-17-1.jpg,
  mineral-surfaces-17-repeat-1.jpg and mineral-surfaces-17-metrics.json.
- Global git whitespace check found unrelated control/settings/script edits;
  those files were not modified by this artwork batch.

### 2026-10-15: intact panels and trim batch 16 (offline)

- Added four intact concrete panels and one green industrial trim atlas. Retained
  the distinct bright right-edge variant rather than sharing artwork across two
  nearly identical source textures. Main seams, notches and slots reviewed.
- Concrete brightness changes +2.36% to +4.41%; green trim -15.73%. Its grille
  is coarser/darker, with stronger metal highlights; final model/lighting checks
  remain pending. All textures opaque. Repeated boundaries visually reviewed.
- Three 1254x1254 sources resampled to 2048 square; two 1774x887 sources to
  2048x1024 and 1024x512. Actual source sizes and exact prompts retained.
- Added tools/texture-review.py for consistent, aspect-preserving compiled DDS
  comparisons, repeated-boundary sheets and brightness/alpha metrics. Maximum
  16 inputs and four rows per page, refuses overwrite; does not auto-approve art.
  Five real assets reviewed and overwrite/invalid-key rejection checked.
- All 68 DDS files passed native parsing and worklist integrity checks. Pack:
  180,393,850 bytes (about 172 MiB), within 6 GiB. Texture workspace about 424 MiB;
  temporary originals/reviews remain below 256 MiB. No gameplay launches.
- 6,744 known supported keys still need art. Full overhaul and final runtime
  validation unfinished. Private evidence: review/wall-trim-16-1.jpg,
  wall-trim-16-2.jpg, wall-trim-16-repeat-1.jpg, wall-trim-16-repeat-2.jpg and
  wall-trim-16-metrics.json under private/texture-remaster.

### 2026-10-15: damaged concrete artwork batch 15 (offline)

- Added five related concrete wall textures: broken reinforcement panel, two
  blue-striped damaged walls and two impact-damaged strips. Compiled comparison
  retains main panel seams, stripe placement and damage silhouettes; material
  grain and fracture relief are stronger. All remain fully opaque.
- Mean brightness changes range from -6.68% to +2.94%. Repeated boundary review
  shows stronger bottom trim and visible joins; final assembled wall continuity
  remains pending. These modular panels are not claimed to tile seamlessly.
- Three 1254x1254 and two 1774x887 built-in generated sources resampled to 2048
  square / 2048x1024 DXT1, with 12 mips. Exact prompts and provenance retained.
- All 63 installed DDS files passed native parsing and worklist checks. Pack:
  170,254,737 bytes (about 162 MiB), below 6 GiB. Texture workspace about 397 MiB;
  temporary originals/review evidence stays below the 256 MiB allowance.
- No game launches. 6,749 known supported keys still need artwork; full overhaul
  and scene validation unfinished. Evidence under private/texture-remaster/review:
  damaged-concrete-15-1.jpg, damaged-concrete-15-2.jpg,
  damaged-concrete-repeat-15.jpg and damaged-concrete-15-metrics.json.

### 2026-10-15: localized error-screen artwork batch 14 (offline)

- Added all five verified XBE disc-error images: English, French, German,
  Italian and Spanish. Actual compiled DDS comparisons preserve exact wording,
  punctuation, diacritics and line breaks; glyph strokes are slightly heavier.
  Threshold line bounds stay within two original pixels. All remain opaque.
- Built-in generated sources are 1254x1254, resampled to 1024x1024 BGRA8 with
  11 mip levels. Minor background residuals (1/255 at the border) and non-red
  antialias values are recorded; these are not exact two-color reproductions.
- All 58 installed DDS files passed native parsing and worklist integrity checks.
  Active pack: 159,067,011 bytes (about 152 MiB), below the 6 GiB cap.
  Texture workspace is about 364 MiB; temporary originals/reviews about 38 MiB.
- No game launches. Runtime display verification remains pending. Coverage is
  58 installed of 6,812 known supported keys, with 6,754 still awaiting art.
  Other encodings and procedural assets remain open; full overhaul unfinished.
- Private evidence: review/error-screens-14-1.png, review/error-screens-14-2.png
  and review/error-screens-14-metrics.json under private/texture-remaster,
  with exact prompts beside authored sources.

### Non-PAK scope discovery and XBE integration � October 15

- Scanned all 11 non-PAK files (121,733,539 bytes) read-only: three Xbox
  executables, six movie files and two analysis JSON sidecars. No payload copies
  were extracted. Found 19 texture-header-shaped matches and 164 image-container
  signatures; signatures alone do not establish a texture or its runtime use.
- Verified five complete 512�512 DXT1 sections in the pinned game executable:
  `xbreadfail_en/fr/ge/it/sp`, containing localized disc-read error text. The
  language dispatch references their names, copies the section, calls wrapper
  0x18a30 at 0x236dc, and reaches texture loader 0x18390. Section bounds, header,
  payload hashes and runtime identities are now part of catalog validation.
- Catalog schema 3 includes five additional keys: 6,812 unique keys, 7,147
  header/payload records and 31,186 source spans. All five need artwork; the pack
  stays at 53 replacements and about 125 MiB. Remaining known artwork: 6,759.
- Five tiny executable matches and nine unknown-format movie matches were not
  promoted. XPR/JPEG signature hits, other encodings and procedural assets remain
  open investigations. No claim of complete discovery or scene validation.
- 37 offline tests passed before the final rescan regression was added; the
  updated non-PAK suite passes all 8 tests (38 total tests across the four suites).
  Live catalog regeneration, worklist and inventory checks passed. No game
  launches, runtime changes or duplicate extraction/staging trees occurred.
- Private evidence: `nonpak-scan.json`, `nonpak-research.json` and
  `review/nonpak-xbe-candidates.jpg`. Catalog regeneration now retains the five
  verified executable sections; archive rescans do not mistake them for PAK data.

### Particle and glow batch 13 � October 15

- Added amber glow, flame, white streak and soft white rectangular glow textures.
  Original and replacement alpha are fully opaque; black-surround color behavior
  is retained. Actual particle blending and animation still need runtime review.
- Used lossless BGRA8 to avoid block-compression gradient bands: two 512�512,
  one 512�256 and one 256�256 outputs, with full mip chains. Sources are
  1254�1254 or 1774�887 and resampled. Actual DDS output was visually compared.
- Mean luminance changes: +5.2%, -6.2%, -9.2%, +0.6%; brightness centers shift
  less than 1.4% of each canvas axis. Amber edges have small 1�2/255 residuals
  versus original zero; other edge peaks generally decrease. Final overdraw,
  animated shape and blend validation remain pending; no exact-black claim.
- All 53 DDS files passed native parsing and worklist checks. Active pack:
  131,102,186 bytes (about 125 MiB), within the 6 GiB cap. Texture workspace:
  about 334 MiB; temporary originals/review images remain below 38 MiB.
  No game launches, duplicate extracts or staging copies were created.
- Coverage: 53 installed, 6,754 supported keys awaiting art. Private evidence:
  `review/particles-13.jpg` and metrics JSON; built-in generation source art,
  exact prompts and real source dimensions remain under `authored/`.

### Character material batch 12 � October 15

- Added two character atlases with refined white/blue uniform fabric, leather
  and hair detail. Main facial identities and island layouts were compared to
  enlarged originals. Outputs are 1024�1024 opaque DXT1 with 11 mips, resampled
  from 1254�1254 built-in generated sources.
- Mean luminance changes are +3.9% and -9.9%. Dark-region mask IoU is 0.810 and
  0.916; thin island-edge differences remain. These metrics include dark material
  as well as gaps, so final model/face mapping and edge continuity are still
  required. Offline visual review does not prove exact in-game registration.
- The dark-uniform atlas `12360a460f281067` remains unfinished and uses original
  art: built-in generation rejected its output with a sexual moderation category.
  The rejection and exact prompt are recorded privately; it was not retried or
  installed. Other asset work can continue.
- All 49 DDS files passed native parsing and worklist checks. Active pack:
  127,255,179 bytes (about 121 MiB). Texture workspace is about 326 MiB, with
  temporary originals/reviews below 38 MiB. Four disposable enlarged/cropped
  review images were recycled; no game launches or duplicate extracts occurred.
- Coverage: 49 installed, 6,758 supported keys awaiting artwork, including the
  rejected asset. Private evidence: `review/character-materials-12.jpg` and
  metrics JSON. Source artwork and exact prompts remain under `authored/`.

### Teal machinery batch 11 � October 15

- Added five related green/grey machinery panels, preserving main curves, vents,
  hatches and orange indicator locations. All are 1024�1024 opaque DXT1 with
  11 mip levels, resampled from 1254�1254 generated sources.
- Reviewed the actual compiled DDS alongside originals. Fine relief and vent
  details are more pronounced; mean luminance is 7.6�14.0% below the source.
  Final model alignment, lighting and edge continuity remain pending. These
  shaped UV panels are not intended as arbitrary seamless environment tiles.
- Orange threshold masks retain 0.930�1.017 of original area with 0.698�0.926
  intersection-over-union. This is a glow-sensitive review aid, not proof of
  exact geometry alignment. Visual comparison retains the original indicator
  locations; final scene validation remains required.
- All 47 installed DDS files passed native parsing and worklist checks. Pack:
  125,855,924 bytes (about 120 MiB), within the 6 GiB cap. Texture workspace:
  about 318 MiB. Temporary originals/reviews remain below 37 MiB. No game
  launches, duplicate extracts or staging copies were created.
- Coverage: 47 installed, 6,760 supported keys awaiting art. Private comparison:
  `review/teal-machinery-11.jpg`, with corresponding metrics JSON. Built-in
  generation artwork, exact prompts and source sizes remain under `authored/`.

### Industrial wall batch 10 � October 15

- Recorded appearance-based categories and authoring constraints for the existing
  96-key candidate set in private `visual-classification.json`: 42 environment,
  19 props, 20 weapons, 5 characters, 3 character/prop ambiguities, 4 effects,
  2 spherical reflection maps and 1 uncertain material. These labels guide
  authoring; scene usage and category-wide review remain unfinished. Reflection
  maps need mapping-aware work rather than ordinary flat-surface edits.
- Added three grey/blue industrial wall panels with modern galvanized finish,
  worn paint and fine surface detail. Band positions, vent placement, main seams
  and stepped panel notches remain aligned with the originals. Fine vent bars
  and relief are more pronounced and remain a final scene-review item.
- Compiled two 2048�2048 and one 2048�1024 DXT1 textures with full mip chains.
  Actual generated sources are 1254�1254 and 1774�887, resampled for output.
  Compared the actual DDS against originals and 2�2 repeats; alpha stays opaque.
  Mean luminance changes are +2.5%, -9.3% and -0.9%. Modular seams and repeated
  motifs remain visible; these are not claimed seamless.
- All 42 DDS files passed native parsing and worklist validation. The active
  pack is 122,357,794 bytes (about 117 MiB), within the 6 GiB cap. The texture
  workspace is about 298 MiB; original exports and review output remain below
  37 MiB. No game launches, duplicate extracts or release staging occurred.
- Coverage: 42 installed, 6,765 supported keys awaiting art. Private evidence:
  `review/industrial-panels-10.jpg`, `review/industrial-repeat-10.jpg` and metrics
  JSON. Built-in generation artwork and exact prompts remain under `authored/`.
  Final scene, lighting and modular-wall continuity validation remain pending.

### Canyon material batch 09 � October 15

- Added a sandstone tile and two cliff/ledge surfaces, retaining major fractures
  and formations with finer stone grain. Two 2048�2048 and one 2048�1024 DXT1
  outputs have full mip chains. Source art is 1254�1254 or 1774�887 and resampled.
- Actual compiled DDS was compared with originals and in 2�2 repeats. Mean
  luminance changed -3.5%, +7.8% and +9.7%. Alpha stays fully opaque. Repeated
  motifs and joins remain visible, particularly the ledge's vertical repeat;
  edge mismatch metrics increased with finer detail. These are not claimed
  seamless. Final scene mapping, lighting and repeat visibility remain pending.
- All 39 DDS files passed native parsing and worklist validation. Pack size is
  115,365,549 bytes (about 110 MiB), within the 6 GiB cap. Texture workspace is
  about 280 MiB; temporary originals/review output remains below 36 MiB.
  No game launches, duplicate extracts or release staging copies were created.
- Coverage: 39 installed, 6,768 supported keys awaiting artwork. Private evidence:
  `review/canyon-surfaces-09.jpg`, `review/canyon-repeat-09.jpg` and the metrics
  JSON. Exact built-in generation prompts and source dimensions are in `authored/`.

### Weapon material batch 08 � October 15

- Added three opaque weapon textures with modern metal finish while retaining
  original atlas islands, fittings and color cues. Compiled output sizes are
  1024�512 and two 1024�1024, DXT1 with complete mip chains. Generated sources
  are 1774�887 and 1254�1254; outputs are resampled.
- Reviewed the actual compiled DDS against originals. All 36 installed textures
  passed native parsing, hashes, dimensions, aspect and mip-chain checks.
  Mean luminance falls 6.2%, 12.6% and 15.2% for these three assets; model lighting
  balance remains a final scene-validation item. No game launches occurred.
- Active pack: 108,373,323 bytes (about 103 MiB), within the 6 GiB cap.
  Texture workspace: about 262 MiB; original exports/review images: about 34 MiB,
  within the 256 MiB temporary-output limit. No duplicate game or staging copies.
- Coverage: 36 installed, 6,771 supported keys still awaiting art. The full
  overhaul remains unfinished. Private comparison and metrics are in
  `review/weapon-materials-08.jpg` and `review/weapon-materials-08-metrics.json`;
  exact built-in generation prompts and source sizes remain under `authored/`.

### Interface and panel batch 07 — October 15

- Added a 256×256 loading-disc icon and two 512×512 panel textures. All three use
  fully mipmapped lossless BGRA8 to preserve smooth gradients and icon alpha.
  Generated sources are 1254×1254 and resampled; no native 4K claim applies.
- The disc's alpha mask has 0.98194 intersection-over-union with the original at
  its original size. Its center and corners remain fully transparent. Compiled
  output was inspected on black, white, grey and colored backgrounds. Panel
  borders and opaque alpha remain intact; mean luminance changes are within 4%.
- Visually classified the 68 interface-owner keys: 65 scene previews, one editor
  preview, one promotional illustration and one loading icon. The 66 previews
  remain unfinished and are planned for matching-scene capture after world art
  production. The illustration still needs artwork; UI inside shared archives
  remains to classify. See private `interface-art-plan.json`.
- All 33 installed DDS files passed native parsing and worklist checks. Pack size
  is 106,623,969 bytes, about 102 MiB. The full texture workspace is about 252 MiB;
  original exports and review/preview images total about 33.4 MiB, below the
  256 MiB temporary-output limit. No game launches or duplicate extracts occurred.
  Final loading-animation, panel usage and scene validation remain pending.
- Private evidence: `review/interface-panels-07.jpg`, its metrics JSON and
  `review/disc-alpha-07.jpg`. Artwork and exact built-in generation prompts remain
  under `authored/`. Coverage is 33 installed and 6,774 keys awaiting artwork.

### Shared surfaces batch 06 — October 15

- Added a tire tread, weathered five-board wood surface and riveted steel sheet
  found in both shared level archives and MapMaker tilesets. Original component
  layouts, opaque alpha, compiled DDS output and repeated-tile previews were
  inspected offline. Tire output is 1024×1024; wood and steel are 2048×2048, all
  fully mipmapped DXT1 resampled from 1254×1254 generated sources.
- All 30 installed DDS files passed native parsing and worklist checks. The pack
  is 103,476,570 bytes, about 99 MiB; the texture workspace is about 243 MiB.
  Average luminance changes are -9.4% for the tire, +12.9% for wood and -5.3% for
  steel. Their model fit, scene lighting and tiling remain final-test items.
  No gameplay launches occurred. Coverage is 30 installed and 6,777 needing art.
- Added a conservative variant-review tool to avoid redundant source artwork.
  It compared 4,203 eligible original textures against 27 existing references,
  preserving aspect/format/size compatibility, and found no further proposals.
  Three tests cover small compression differences, palette/alpha mismatches and
  shifted texture islands. Similarity never automatically approves or installs art.
- Private evidence: `review/shared-surfaces-06.jpg`, its metrics and repeated-tile
  preview, plus `variant-candidates.json`. Sources and exact prompts are retained
  under `authored/`; no duplicate game data or temporary build trees were created.

### Embedded artwork batch 05 — October 15

- Added three new generated materials for embedded cutscene textures: a blue-steel
  light panel, matching vent/mechanical panel and red rocky ground. The atlas
  regions and original light cues remain aligned in compiled DDS comparisons.
  All three use 2048×2048 DXT1 with full mip chains, resampled from 1254×1254 art.
- A 512×512 embedded character atlas shares the layout of the previously reworked
  256×256 atlas. Its face/ear, clothing and hardware landmarks match visually;
  normalized original RGB mean absolute differences are 3.4–4.1 on a 0–255 scale.
  Reused the existing generated artwork for its 1024×1024 replacement, retaining
  one source image and recording both runtime identities and reuse evidence.
- Reviewed actual DDS pixels, opaque alpha and repeated ground tiles. The ground
  retains a visible large-scale repeated motif, also present in the original.
  Mean luminance changes range from -9.0% to +7.6%; scene material balance and
  character-model fit remain pending final validation. No gameplay was launched.
- All 27 installed DDS files passed native parsing and worklist validation.
  The pack occupies 97,183,396 bytes, about 93 MiB, and the full texture workspace
  is about 226 MiB. Coverage remains 27 of 6,807 keys, with 6,780 awaiting art.
  Private comparisons: `review/embedded-art-05.jpg`, `review/red-ground-repeat.jpg`
  and `review/character-resolution-variants.jpg`; prompts remain under `authored/`.

### Embedded catalog integration — October 15

- Integrated all 1,672 verified embedded source references. The catalog now holds
  7,142 header/payload records and the worklist deduplicates them to 6,807 runtime
  keys: 23 installed and 6,784 needing artwork. The original top-level sources
  still represent 6,297 keys; embedded containers add 510. Multiple header/payload
  variants can share one base-level identity and require only one replacement.
- Import rechecks every archive SHA-256, parent archive-index entry, texture
  header, payload hash, container reference and calculated runtime key. Inventory
  checks embedded byte spans and table references; it continues to mark the rest
  of each container as unclassified. Missing scan records cannot silently remove
  previously integrated sources on catalog regeneration.
- Twenty-seven offline tests pass, including truncated payloads, changed pixels,
  mismatched dimensions/identity, broken table references and rescanning a catalog
  that already includes embedded sources. No runtime code or gameplay changed.
- Planned DDS payload is 3,208,165,560 bytes, about 2.99 GiB. This is a production
  estimate rather than completed art; category classification, other asset
  encodings, non-PAK data and procedural textures remain open work. The active pack
  stays at about 84 MiB. Catalog and research reports stay private.

### Embedded texture discovery — October 15

- A read-only scan covered all 78,081 entries in 403 PAK archives, scanning
  3,841,760,562 entry bytes at every byte offset for the verified header pattern.
  It rediscovered all 29,509 cataloged top-level occurrences and found 1,672 nested
  occurrences. There were no compressed entries to skip. Archive SHA-256 hashes
  and candidate header/base-payload hashes are retained in the private report.
- All 1,672 candidates have references in their containers' 16-byte texture
  records. The pinned executable's relocation routine at `0x8e3e0` uses flag
  `0x10000000`, adds the container base to the texture offset, and walks records
  to a `0xffffffff` terminator. The loader call at `0x89e5f` reaches wrapper
  `0x18a30` and texture loader `0x18390`; instruction bytes and targets are checked.
- All 691 unique base images decoded and have recorded RGBA hashes. A bounded
  contact sheet shows coherent atlases, effects, masks and a diagnostic image.
  Of these identities, 181 already exist in the catalog and 510 are additional.
  Catalog/worklist integration remains next, with potential known scope of 6,807
  keys. Container references do not establish that every asset appears in play.
- Twenty-four offline tests pass. Only compact JSON evidence and one contact
  sheet were written; there were no bulk exports, artwork additions or game
  launches. The active pack remains 23 textures, about 84 MiB. Other header
  formats, non-PAK assets and procedural textures remain discovery limitations.
  Evidence: private `embedded-scan.json`, `embedded-decode-research.json`, and
  `review/embedded-candidates.jpg`.

### Material and atlas batch 04 — October 15

- Reworked the held machinery and character drafts against the original UV layouts.
  Revision 2 retains the principal cap/vent positions, ear and cropped face regions,
  clothing boundaries and fittings in offline side-by-side comparisons. Installed
  both at 1024×1024 and a new four-panel steel surface at 2048×2048. All three use
  fully mipmapped DXT1 and retain the originals' opaque alpha.
- Reviewed actual compiled DDS pixels and a repeated metal-panel preview. The
  character is approximately 6.6% darker on average; the panel surface 11.4% darker.
  Scene material balance and character-model fit remain explicit final-test items.
  Offline comparisons cannot establish how UV seams look on animated models.
- All 23 installed DDS files passed native parsing and the worklist's hash,
  dimension, aspect, mip and storage checks. Pack size is 88,093,427 bytes, about
  84 MiB; the texture workspace is about 201 MiB after recycling superseded local
  drafts and redundant comparison images. Selected sources and prompts remain.
- There were no gameplay launches or runtime code changes. Coverage is 23 of
  6,297 supported keys, with 6,274 awaiting artwork. The full overhaul and embedded
  asset discovery remain unfinished. Private evidence: `review/material-atlas-04.jpg`,
  its metrics JSON and `review/recessed-panels-repeat.jpg`.

### Sky and effects batch 03 — October 15

- Added two storm skies and two luminance effects: a glow sprite and square grid
  mask. Corrected effect drafts against the original layouts before installation.
  Final comparisons use the compiled DDS pixels, not just generated source art.
- The two small effects use fully mipmapped lossless BGRA8 DDS because BC1
  introduced visible gradient banding. The compiler and native parser now support
  this format; channel normalization preserves luminance and alpha-only semantics.
  The runtime's existing 24 MiB per-file and 512 MiB residency limits remain.
- The active pack contains 20 replacements and occupies 83,897,396 bytes, about
  80 MiB of the 6 GiB budget. The texture workspace is approximately 192 MiB.
  Rejected local drafts and superseded compressed effect files were recycled.
- Seventeen Python tests, the Windows Release build and native regression suites
  passed. All 20 actual DDS files passed native parsing and the offline worklist
  audit. Comparisons and metrics are retained under private `review/sky-effects-03`.
- No gameplay launches occurred. Sky projection/wrap and effect blend brightness
  remain pending scene checks. Coverage remains partial: 6,275 keys need art and
  two drafts require review, out of 6,297 supported keys. Embedded asset discovery
  is still incomplete. Sources are 1254×1254 or 1774×887, resampled at compilation.

### Uncompressed replacement loader support — October 15

- The host texture loader now keys swizzled A8R8G8B8, A8, L8 and A8L8 level-zero
  bytes in addition to DXT1/3/5. Render targets, screen copies, unsupported formats
  and invalid dimensions remain excluded. Guest texture data is not rewritten.
- Replacement DDS files remain compressed and fully mipmapped. Alpha-bearing
  originals reject DXT1 replacements; opaque L8 can use DXT1. Alpha-only masks
  use white RGB and preserve their alpha channel, matching the existing shader
  alpha-only handling. Optional raw dumps use tagged `.tsft` files rather than
  incorrectly identifying swizzled pixels as compressed DDS.
- The Windows Release build and native regression suites passed. Fifteen Python
  offline tests passed, including alpha-only BC3 encoding. Native identity checks
  on one original payload of each new format matched the offline catalog keys.
  Those four temporary fixtures totaled 537,664 bytes and were recycled.
- The refreshed catalog/worklist now includes 6,297 supported keys: 16 installed,
  two held drafts, and 6,279 awaiting art. All known cataloged formats have a
  supported decoding/key path, but embedded asset discovery remains incomplete.
  The 750 new uncompressed keys retain a pending scene-validation status.
- No gameplay launch was performed. This is compiled loader support and offline
  validation, not proof that every effect, UI or material renders correctly.
  Build evidence: `logs/texture-formats-build.log`; key evidence is recorded in
  private `format-runtime-validation.json`. Pack remains about 64 MiB.

### Uncompressed texture decoding — October 15

- Located the original texture loader at guest address `0x18390`. Its switch
  reads archive format at header offset 20; the table at `0x185b0` maps archive
  formats 0 through 7 to Xbox formats `[12,14,6,5,15,25,0,26]`. The new offline
  `tools/texture-formats.py` verifies the pinned executable SHA-256, dispatch
  instruction bytes and all eight case immediates before using this mapping.
- The 753 previously unknown-format payloads decode as 38 A8R8G8B8, 434 A8,
  155 L8 and 126 A8L8 images. All payload source hashes were checked and decoded
  RGBA hashes recorded. They produce 750 candidate base-level identities; three
  additional payloads share an identity. These candidates are not promoted into
  the supported runtime catalog yet.
- Sixteen small samples were visually inspected, showing coherent sky images,
  transparency masks, glow/noise textures and UI icons. One contact sheet and
  compact JSON research records are retained; no bulk PNG export was made.
- Thirteen offline tests pass, including rectangular Morton ordering, channel
  order, alpha/luminance semantics, truncated input and wrong-executable rejection.
  Runtime integration and identity/alpha verification remain next steps. The
  active pack is unchanged at 16 replacements; no gameplay launches occurred.

### Environment texture batch 02 — October 15

- Added weathered steel panels, wind-shaped sand, horizontal weathered trim and
  plaster over metal panels. Source comparisons and repeated-tile previews were
  inspected offline; all four await scene-specific runtime checks. No game
  launch, new game-data copy or packaged ZIP was needed.
- One major sand surface uses 4096-square DDS; two materials use 2048-square and
  the panel strip uses 2048x1024. These resample three 1254-square generated sources
  and one 1774x887 source. Exact prompts and review notes accompany each source.
- All 16 installed replacements pass the format, dimensions, aspect, full mip
  chain and hash audit. Active pack: 67,117,865 bytes (about 64 MiB), below 6 GiB.
  The texture workspace, including source artwork, is about 168 MiB. There are
  5,529 supported keys awaiting art, two held drafts and 753 unsupported payloads.
- Evidence: private `review/environment-batch-02.jpg`, per-asset prompt records,
  `worklist.json`, and `progress.json`. The existing early gameplay evidence
  applies only to its recorded keys, not this newly added batch.

### Texture production audit and offline batch — October 15

- `tools/texture-inventory.py` checked catalog references against all 403 P5CK
  archives: 78,081 entries, including 29,509 cataloged texture occurrences.
  The 48,572 other entries are unclassified, not presumed texture-free.
  Embedded texture discovery and four unsupported format families remain open.
- Private `inventory.json` assigns shared keys to provisional production groups
  without duplicating outputs. The 6144 MiB budget is allocated as 3584 MiB for
  levels/shared assets, 768 characters, 512 weapons, 384 MapMaker, 256 interface,
  and 640 reserve. These are ceilings, not completed content; archive ownership
  does not prove visual category (effects and props may be embedded/shared).
- All 753 unsupported payloads match plausible 8-, 16- or 32-bit base/mip byte
  counts. This is only a decoding lead: channel order, memory layout and runtime
  identity are not established and support has not been enabled.
- Added one sand material after source and repeated-tile visual review. The
  1254-square generated source was resampled to 2048-square DXT1 with complete
  mips. Pack total: 12 replacements, 48,940,325 bytes (about 47 MiB). The new
  machinery draft changed vent details and remains excluded pending correction.
- Ten offline tool tests passed; all installed DDS files passed their format,
  hash, aspect and mip audit. No gameplay launches or game-data copies were
  made for this batch. The new sand texture awaits scene-specific verification.
- Coverage remains 5,533 supported keys awaiting art, two held drafts, twelve
  installed replacements and 753 unsupported payloads. The goal remains active.

- Read the supplied desktop Xbox ISO without modifying it; extracted 412 files.
- Confirmed the US executable, title ID `45410066`, entry point `0038024D`,
  XDK 5849 and the expected SHA-256 documented in `README.md`.
- Identified 379 Xbox library symbols, including six upstream manual symbols.
- Generated 15,838 C function bodies with zero reported translation failures,
  totaling 2,507,843 lines across 16 chunks plus support files.
- Built `build/windows/Release/futureperfect_recomp.exe` with Visual Studio
  2022 / MSVC 19.44.35224 for Windows x64.
- Compiled and loaded FFmpeg 8.1 movie support.
- Fixed native profile device paths that bypassed `RECOMP_SAVE_DIR`, and
  preserved the fix in `patches/save-routing.patch`. Ten native path regression
  checks pass, including asset routing and switching between save directories.
- Added player-1 keyboard/mouse controls: normalized WASD movement, Windows raw
  relative mouse motion, direct camera-angle updates, mouse buttons and wheel,
  editable sensitivity/invert-Y settings, focus-loss clearing and cursor
  capture/release. The launcher loads `config/pc-controls.json`.
- Preserved the host integration and camera wrapper in `patches/pc-controls.patch`;
  the generated original `sub_000D2B80` body remains available as `_gen`.

## Texture overhaul moved to issue #74 - October 16, 2026

- Active branch: `74-enhancement-gfx-overhaul-to-modernize-textures`, based on
  current main `d430e8b`. Transferred the texture portions of `fcee85c` and
  `8f789ad` from `codex/integrate-display-textures`; preserved current launcher,
  controls, networking, release and documentation changes on main.
- Includes live Classic/4K selection, uncompressed identity/alpha handling,
  embedded/non-PAK catalog tools, inventory/category budgets, DDS compilation,
  comparison sheets, variant discovery, tests and historical texture evidence.
  Artwork, originals, the active pack, settings and saves remain private in the
  same checkout. The earlier branch is retained as a recovery reference.
- Latest private artwork batch 58 accepts the vent and banded steel surface:
  both source images are 1254x1254, resampled to 1024x1024 and 2048x2048 DXT1.
  Compiled comparisons and repetition sheets were inspected offline; stone and
  rusted-panel drafts remain held for edge correction. No native 4K-source claim.
- Current catalog: 7,147 payload records, 6,812 supported keys, 232 installed,
  6,571 needing art and nine held drafts. Embedded discovery/category review
  remain incomplete. The 232 assets are a partial preview, not a full remaster.
- Pack including metadata: 388,280,874 bytes (370.29 MiB), under the 6 GiB cap.
  Source art stays in the existing approximately 1.125 GiB texture workspace;
  original exports/review evidence remain about 106 MiB, below the 256 MiB limit.
- Fresh MSVC native DDS validation passed all 232 installed files. Native
  counted-resource loader tests passed classic/4k/implicit/missing modes,
  cached switching, original preservation, invalid files, allocation failure,
  screen-copy/render-target exclusion and resource cleanup. Worklist hashes,
  dimensions, mip chains, aspect ratios and disk budget passed.
- No game launch or full runtime rebuild was performed for this transfer.
  Existing runtime evidence remains historical; final scene/UV/alpha, display,
  residency and Mac checks are still pending under the 90% validation gate.
- Transfer checks: 38 texture-tool tests passed, then all 14 patch-stack tests
  passed after updating branch-switch fixtures to model main without the toggle.
  This covers fresh/upgrade/idempotent stacks, LF/CRLF round trips and preservation
  of unrelated edits. The complete main patch stack remains compatible.

## Texture props batch 59 - October 16, 2026

- Continued on issue #74 branch from `21926c8`. Four new offline-reviewed
  replacements: brass cylinder/caps atlas, slotted bronze lid, two-pin steel
  equipment strip and recessed industrial wall panel. Ring retains eight holes;
  panel retains four outer fasteners per side and corner brackets. All four
  original and compiled alpha channels are fully opaque.
- Three square sources are 1254x1254 and the rectangular source is 1774x887.
  Compiled DXT1 outputs are two 1024x1024, one 1024x512 and one 2048x2048 with
  full mip chains. All are resampled; these are not native 4K generated sources.
- Two additional built-in generation calls attempted held stone/rust repeat
  corrections. Both failed offline review and remain uninstalled. Retained
  original V1 sources, prompts, generator references and comparison evidence;
  recycled 7.04 MiB of unsuccessful local V2 copies.
- All 236 installed DDS files pass native format validation and worklist
  hash/dimension/mip/aspect/budget checks. Inventory refreshed: 7,147 payloads,
  6,812 supported keys, 6,567 needing art and nine held drafts. Existing gaps,
  unclassified categories and discovery scope remain open.
- Pack: 392,826,981 bytes (374.63 MiB),
  adding 4.34 MiB this batch. Texture workspace
  allocation is approximately 1.141 GiB;
  original exports and review outputs total 107.34 MiB,
  below the 256 MiB temporary limit. One active pack, no duplicate game or build.
- Private evidence: `review/props-59-final-1.jpg`, repeat sheet and metrics,
  `review/props-59-native-audit.txt`, `review/props-59-acceptance.json`,
  `review/props-59-cleanup.json` and `review/surfaces-59-source-metrics.json`
  under `private/texture-remaster`. Exact prompts accompany each source.
- No runtime rebuild or game launch. Model UV/lighting, scene adjacency,
  1080p/4K/ultrawide rendering, residency and full-pack stability remain pending
  the existing near-completion gate. This is a partial preview, not a full remaster.


### Texture overhaul batch 60 - machinery (issue #74)

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures`; no game launches.
- Added three offline-reviewed machinery replacements: steel mechanism/vent atlas,
  circular disc, and industrial atlas with corrected vertical ULTRANET lettering.
  Actual compressed DDS comparisons, repeat sheets and lettering crop were reviewed.
  The cable-panel draft remains excluded because its small ochre marking changed;
  its eight center louvers match the original (the initial prompt miscounted ten).
- All 239 installed DDS files passed the native format validator and the worklist's
  hash, mip-chain, dimensions, aspect-ratio and 6 GiB budget checks on Windows x64.
  Model UV sampling, scene appearance and runtime residency remain unverified for
  this batch. This is asset-only work; no runtime code or binaries changed.
- Selected 1254-square generated sources were resampled into two 2048-square and
  one 1024-square DXT1 textures; these are not native 4K source images.
- Catalog remains 6,812 supported keys: 239 installed, 6,563 awaiting art and ten
  held drafts. Reviewed 32 further candidates with native alpha checks, bringing
  provisional visual classifications to 451; category-wide review is incomplete.
- Pack: 399,120,159 bytes (380.63 MiB). Texture workspace at audit: 1,245,492,034
  allocated bytes (1.16 GiB). Superseded V1 art was recycled with provenance and
  comparison evidence retained; no game copies or packaging trees were created.
- Private evidence: `review/machinery-60-final-1.jpg`,
  `review/machinery-60-final-repeat-1.jpg`, `review/machinery-60-lettering-final.jpg`,
  `review/machinery-60-final-metrics.json`, `review/machinery-60-native-audit.txt`,
  and `review/inventory-60-alpha.jpg` under `private/texture-remaster`.
  Artwork and pack remain ignored/local. Consolidated playtesting stays deferred.


### Hybrid texture production adopted - issue #74 (2026-10-16)

- User approved individual redesigns for prominent assets plus faithful batch
  enhancement for simpler materials. Added `tools/texture-hybrid-plan.py` and
  documented the policy in AGENTS and the texture guide. Eight-asset queues group
  classified environments by material/occurrence; unclassified assets, sensitive
  categories and held drafts remain separate. Routing never approves artwork.
- Five planner tests pass on Windows x64: sensitive/unknown routing, installed
  and held precedence, bounded deterministic batches, fixed-budget overflow and
  duplicate-key rejection, and batch-size bounds. No runtime code changed or
  game rebuild was required. MAC-011 records portable tooling and unverified Mac
  execution/scene coverage.
- Two built-in image-generation batches produced 14 sources; nine passed offline
  source and actual compressed-DDS review. Five failed drafts remain excluded:
  a riveted panel's bright repeat frame, changed tread arrangement and shifted
  bottom fittings in three similar metal-panel variants. Three compiled rejected
  files and metadata were retired after recording hashes and keeping evidence.
  Do not treat hybrid-62-final-2.jpg as installed coverage: it records rejections.
- Added four square 1024x1024 and five landscape 1024x512 DXT1 replacements.
  Source art is 1254x1254 or 1774x887, resampled for encoding; not native 4K art.
  Stronger highlights and the dark circuit material's -16.56% mean luminance
  remain explicit final-scene appearance checks. No new asset is runtime-verified.
- All 248 retained DDS files passed the native format validator and worklist
  hashes, dimensions/aspect ratios, full mip chains and pack-budget checks.
  Catalog: 6,812 supported keys, 248 installed, 6,549 awaiting art, 15 held.
  Reviewed 32 additional originals/alpha channels: 483 visual classifications.
  Category-wide review is incomplete; 6,315 keys remain in visual triage.
- Active pack: 403,669,138 bytes (384.97 MiB), up only 4,548,979 bytes this turn.
  Texture workspace at audit: 1,306,285,021 allocated bytes (1.22 GiB), including
  source art; originals/review/preview files total 115.38 MiB. One active pack,
  no extra game extraction, release staging tree or gameplay capture was created.
- Private evidence: `review/hybrid-61-final-*`, `review/hybrid-62-final-*`,
  `review/hybrid-61-62-native-audit.txt`, both batch prompt/generated records,
  `review/hybrid-62-retired.json`, and `review/inventory-61-*` under
  `private/texture-remaster`. Private artwork remains local and ignored.
- Branch: `74-enhancement-gfx-overhaul-to-modernize-textures`, based on `fdfac3f`.
  Supported Xbox executable SHA-256 reverified. Zero game launches; consolidated
  playtesting remains deferred until 90% audited scope and category review.


### Hybrid texture batch 63 - metal and surface materials (issue #74)

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` at `70d19b5`.
  Added eight replacements after original/source and actual-DDS comparisons,
  including 2x2 repetition and opaque-channel checks. No game launches or rebuild.
- Materials include divided and yellow-striped panels, worn steel, muted metal,
  polished trim, two small band atlases and a gray gridded surface. Small bottom
  fittings retained their original variant-specific locations. Mean luminance
  changes of -14.69% for the striped wall and -20.91% for worn steel remain explicit
  final-scene brightness checks. Stronger grid detail also needs scene review.
- Six 1774x887 and two 1254x1254 generated sources were resampled into five
  1024x512, one 512x256, one 512x512 and one 2048x2048 full-mip DXT1 texture.
  These are generated enhancements, not native 4K sources or verified game scenes.
- All 256 retained DDS files passed the native format validator and the worklist
  hash, mip-chain, size/aspect and fixed 6 GiB budget checks on Windows x64.
  Supported catalog: 6,812 keys; 256 installed, 6,541 awaiting art, 15 held drafts.
- Reviewed 64 additional originals and all 34 nonopaque alpha channels. The 547
  provisional visual classifications now distinguish more reticles, warning and
  grenade symbols, particle masks and robot atlases from environmental materials.
  All-category review is unfinished; 6,251 keys remain in visual triage.
- Pack: 408,479,691 bytes (389.56 MiB), including metadata.
  Texture workspace audit: 1,340,936,887 allocated bytes
  (1.25 GiB). Originals/reviews/previews: 118.76 MiB, below 256 MiB.
  No duplicated game data, pack, ZIP or release staging. Private art stays local.
- Private evidence under `private/texture-remaster`: `review/hybrid-63-final-*`,
  `review/hybrid-63-native-audit.txt`, source/prompt/generated records and
  `review/inventory-63-*` RGB/alpha sheets. Consolidated scene validation remains
  deferred to 90% audited coverage and completed category review.


### Hybrid texture batch 64 - terrain and industrial materials (issue #74)

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` at `2bb5564`.
  Added 16 replacements after original/source/actual compressed-DDS comparisons,
  opaque-channel checks and 2x2 repeat review. No game launches or runtime rebuild.
- Materials cover concrete, scratched metal, dirt, rock, sandstone, brickwork,
  gray stone wall, two tile grids and a five-stone atlas. Broad original layout
  retained; fine detail and local contrast enhanced. Rusted brown panel mean
  luminance changed -12.65%; other changes range -8.32% to +0.77%. Final scene
  brightness, adjacency and material appearance remain unverified.
- Sixteen 1254x1254 sources were resampled to ten 1024x1024 and six 2048x2048
  full-mip DXT1 files. These are generated enhancements, not native 4K sources.
- All 272 retained DDS passed native format validation and worklist hashes,
  mip chains, dimensions/aspect and fixed 6 GiB budget checks on Windows x64.
  A missing source field in new private provenance records was corrected before
  the successful worklist audit. No tooling/runtime source changed.
- Catalog: 6,812 supported keys; 272 installed, 6,525 awaiting art, 15 held drafts.
  Reviewed 80 additional originals and 14 nonopaque alpha channels. There are now
  627 provisional visual classifications; category-wide review remains unfinished.
  Queue: seven material candidates, 140 individual and 207 specialist reviews,
  15 corrections and 6,171 keys requiring visual triage.
- Pack including metadata: 432,256,545 bytes (412.23 MiB),
  increased by 23,776,854 bytes. Texture workspace at audit:
  1,442,399,162 allocated bytes (1.34 GiB).
  Originals/reviews/previews: 131.57 MiB, below 256 MiB. Private art
  remains local; no duplicate game, pack, ZIP or release staging was created.
- Private evidence: `review/hybrid-64-final-*`, `review/hybrid-64-native-audit.txt`,
  `review/hybrid-64-acceptance.json`, batch source/prompt records, and inventory-64
  RGB/alpha sheets under `private/texture-remaster`. Consolidated scene testing
  remains deferred until 90% audited coverage and completed category review.


### Hybrid texture batch 65 - materials and equipment (issue #74)

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` at `d3f1f01`.
  Generated 16 sources; accepted seven material and seven individually prompted
  equipment/weapon textures after actual compressed-DDS and 2x2 repeat review.
  Original payload hashes and supported US XBE SHA-256 reverified on Windows x64.
- New art includes a framed grille, riveted panels, peeling wall, railway ballast,
  star carpet, polished metal, leather/metal equipment, weapon atlas, cylindrical
  highlights, machinery panels and a circular vent. Preserve model/scene checks
  for the final phase; finer grille detail and highlights remain runtime-unverified.
- Two drafts held and excluded: purple reflection band peak shifted from row 38
  to row 32 at original 64-pixel scale; rusted armor panel brightness increased
  54.02%. Kept source art, prompts and evidence; removed four rejected pack files
  (DDS and metadata) after recording hashes and validating contained paths.
  Review sheets 2 and 4 include rejected drafts, not exclusively installed art.
- Accepted carpet mean luminance changed -21.47%; brighter metal changes reach
  +14.41%. Final scene validation must check dark carpet visibility and highlights.
  Twelve 1254-square and four 1774x887 sources were generated. Accepted outputs:
  eight 1024-square, two 512-square, three 1024x512 and one 2048x1024 full-mip DXT1.
  All were resampled; none is a native 4K generated source.
- All 286 retained DDS passed native format and worklist hashes, complete mips,
  dimensions/aspect and fixed-budget checks. Catalog: 6,812 supported keys,
  286 installed, 6,509 awaiting art and 17 held drafts. Classification remains
  627 provisional keys; 131 individual, 207 specialist and 6,171 visual-triage
  items remain, with no currently classified material batch candidates.
- Pack: 440,653,149 bytes (420.24 MiB), including metadata,
  up 8,396,604 bytes. Texture workspace at audit:
  1,509,938,424 allocated bytes (1.41 GiB).
  Original/review/preview files: 134.98 MiB, below 256 MiB. No duplicated
  game extract, pack, ZIP or staging directory. Private art stays local.
- Evidence under `private/texture-remaster`: `review/hybrid-65-final-*`, native
  audit, acceptance and retirement records, highlight profile, source/prompt files.
  Zero gameplay launches or rebuilds; no runtime/tooling code changed. Consolidated
  playtesting remains deferred to 90% audited coverage and completed category review.


### Hybrid texture batch 66 - characters and shared armor (issue #74)

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` at `73919dd`.
  Added 15 character/equipment replacements after original/source/actual-DDS
  comparison, opaque-channel checks and repeated-edge inspection. Supported XBE
  SHA-256 and original catalog payload hashes verified on Windows x64.
- Art covers white fur and gold accessories, cloth/skin trim, coat and trousers,
  gray/orange uniform, primate fur, dark leather equipment, shared gray armor and
  four small robot atlases. Character identity, model UV seams and all new scene
  appearance remain runtime-unverified; no gameplay or game rebuild occurred.
- Fifteen sources generated; fourteen selected. Held eye/accessory draft
  `d5b07a49859a271d` because the iris became excessively saturated cyan-blue.
  Source/prompt/evidence retained; rejected DDS and metadata removed after hash
  recording and contained-path validation. Review sheet 1 includes that rejection.
- One selected armor source serves both `47ec02a8b3aaebee` and `7b988340e93f849a`.
  Original resolution-variant comparison: RGB MAE 2.625, p95 7, alpha error zero;
  original layouts and both actual DDS versions visually reviewed. One source PNG
  retained. This is offline-approved reuse, not proof of model rendering.
- Generated sizes: nine 1774x887 and six 1254x1254. Accepted full-mip DXT1 outputs:
  four 1024x512, six 1024x1024, one 2048x2048 and four 512x256, all resampled.
  Not native 4K source artwork. Shared armor luminance increases 19.31%/19.68%;
  two leather atlases increase 14.65%/14.18%. Final checks must cover highlights,
  eyes, emblems and exact UV alignment in the character models.
- All 301 retained DDS passed native format and worklist hash, mip-chain,
  dimensions/aspect and fixed-budget checks. Catalog: 6,812 supported keys;
  301 installed, 6,493 awaiting art, 18 held. Visual classification remains 627
  provisional keys; 115 individual, 207 specialist and 6,171 visual-triage items.
- Pack including metadata: 449,399,857 bytes (428.58 MiB),
  up 8,746,708 bytes. Texture workspace at audit:
  1,567,022,125 allocated bytes (1.46 GiB).
  Original/review/preview files: 137.84 MiB, below 256 MiB. No duplicate
  game extract, pack, ZIP or staging tree. Artwork remains private/local.
- Private evidence: `review/hybrid-66-final-*`, native audit, acceptance/retirement
  and variant-check records, original-selection metadata and source/prompt files.
  Consolidated playtesting remains deferred to 90% audited scope and completed
  category review. No runtime or tooling source changed this batch.


## Texture overhaul hybrid batch 67 — offline equipment review

- Continued issue #74 on `74-enhancement-gfx-overhaul-to-modernize-textures`, starting from `cfffe0a`. Verified supported US XBE and original payload hashes; no runtime/source changes or gameplay launches.
- Generated eight equipment drafts; accepted five after original/compiled-DDS and 2x2 repeat review: worn equipment, olive machinery and yellow/red/blue circuit panels. Held three for grip-pattern pitch, added panel fittings/bevels and maker-plate markings. Original game textures remain active for held keys; rejected DDS/metadata removed with recorded hashes, source art/provenance retained privately.
- Four generated sources are 1254 square and four are 1774x887. Accepted full-mip DXT1 outputs: one 2048 square, one 1024 square and three 1024x512. All are resampled; no native 4K claim. Accepted luminance changes range from -8.68% to +15.82%; final model/scene lighting and seams remain unverified.
- All 306 retained DDS pass native format validation and worklist hash, dimensions/aspect, mip and cap checks. Coverage: 306 installed / 6,812 supported keys, 6,485 needing art and 21 held drafts. The 627 provisional classifications and unresolved embedded/procedural scope remain open. Offline installation is not full-game visual validation.
- Active pack: 453,946,549 bytes (432.92 MiB) / 6 GiB, including metadata; batch adds 4,546,692 bytes. Texture workspace at audit: 1,600,297,394 allocated bytes (1.49 GiB); original exports/review/preview: 146,229,407 bytes / 256 MiB. Free disk: 137,601,146,880 bytes. No duplicate game, staging or archive created; saves/settings unchanged.
- Private evidence: `review/hybrid-67-final-*.jpg`, metrics, native audit, retirement hashes and acceptance record under `private/texture-remaster`. Artwork remains local/ignored. Existing shared DDS path is unchanged; Mac artwork/scene validation remains pending. Consolidated playtesting stays deferred to the established near-completion gate.


## Texture overhaul hybrid batch 68 — metal surfaces and inventory

- Continued issue #74 on `74-enhancement-gfx-overhaul-to-modernize-textures`, starting at `04ae123`. Verified supported US XBE and original payload hashes. No runtime/source changes or game launches.
- Visually classified 64 more original textures with decoded alpha ranges, bringing provisional classification to 691 keys. Masks, interface, effects and reflections stay in specialist review; classification is not completed artwork or proven scene usage.
- Accepted eight compressed-DDS replacements: pale silver panel, three-light panel, recessed grid, louver, blue slats, vertical vent, rusty tapered apertures and brown circular fitting. Reviewed original/DDS comparisons and 2x2 repetition. Louver slot concern resolved by measured profiles: 12 slots on both, center deviations at most one original pixel. No failed drafts added this batch.
- Seven sources are 1774x887 and one is 1254 square. Outputs are seven 1024x512 and one 512 square, full-mip DXT1; all resampled, not native 4K. Brown fitting luminance rises 18.51%; other changes range -8.65% to +1.17%. Scene lighting, UV adjacency and mapped tiling remain pending.
- All 314 retained DDS pass native format validation and worklist hash, dimensions/aspect, mip and cap checks. Coverage: 314 installed / 6,812 supported keys, 6,477 needing art and 21 held drafts. Unresolved embedded/procedural scope remains open; no full-remaster claim.
- Active pack: 456,572,573 bytes (435.42 MiB) / 6 GiB including metadata; batch adds 2,626,024 bytes. Texture workspace at audit: 1,634,636,828 allocated bytes (1.52 GiB). Original exports/review/preview: 149,403,778 bytes / 256 MiB; free disk: 137,182,937,088 bytes. No game copies, archives or staging trees created; personal saves/settings unchanged.
- Evidence remains ignored/local under `private/texture-remaster/review/hybrid-68-*`, including original triage, compressed DDS/repeat sheets, metrics, slot profiles, native audit and acceptance. Shared DDS path unchanged; Mac artwork/scene validation pending. Consolidated playtesting remains deferred to the existing near-completion gate.


## Texture overhaul hybrid batch 69 — broader material production

- Continued issue #74 from `acadffc`; verified supported US XBE and original payload hashes. Classified 64 additional originals/alpha ranges: 755 provisional classifications total, with all unresolved coverage retained.
- Accepted 15 material replacements across metal, concrete, stone, wood and leather after actual-DDS and 2x2 repeat review. Held gold panel `220cc5080517b5db` for added corner screws/frame changes; original stays active. Retired only its compiled DDS/metadata with recorded hashes; source art remains private.
- Generated six 1254-square and ten 1774x887 sources. Accepted full-mip DXT1 outputs: two 512-square, two 1024-square, one 2048-square and ten 1024x512. All resampled, not native 4K. Accepted luminance changes -13.98% to +16.59%; final mapped lighting, adjacency and repetition remain pending. Pattern profiles supplement the visual checks for repeated ribs and rivets.
- The main checkout switched externally to issue-17 during generation. Created the minimal `build/texture-74-source` worktree on the existing #74 branch, sharing the single artwork/game folder by explicit paths. Re-encoded all 16 drafts in memory with #74's encoder and confirmed byte-identical DDS outputs; color channel metadata recorded. Validation and this commit use #74; concurrent branch and edits preserved. Keep subsequent texture source work in this worktree while that concurrent work continues.
- All 329 retained DDS pass native format and #74 worklist hash, dimensions/aspect, mip and cap checks. Coverage: 329 installed / 6,812 supported keys; 6,461 need art and 22 drafts need correction. Installed/offline-reviewed does not establish scene validation or full remaster completion.
- Pack: 464,620,511 bytes (443.10 MiB) / 6 GiB including metadata, +8,047,938 this batch. Texture workspace at audit: 1,705,656,196 allocated bytes (1.59 GiB); temporary original/review/preview: 155,260,377 bytes / 256 MiB. Separate minimal source checkout: 2,684,911 bytes; no duplicated private art/game/build. Free disk: 137,629,822,976 bytes.
- Zero game launches; saves/settings untouched. Evidence is ignored/local under `private/texture-remaster/review/hybrid-69-*`. Shared runtime unchanged; Mac scene/art validation remains pending. Consolidated playtesting remains deferred to the near-completion gate.


## Texture overhaul hybrid batch 70 — equipment material production

- Continued issue #74 from `b8a4327` in the minimal `build/texture-74-source` worktree, sharing the authoritative private artwork/game paths. Other checkout preserved. Supported US XBE and original payloads verified; no gameplay launches or runtime/tooling changes.
- Generated 16 drafts and accepted 13 after original/actual-DDS and 2x2 repeat review. Held three: changed bracket lettering, changed white weapon marking, and added rotor fasteners. Originals remain active for held keys; retired only their compiled DDS/metadata with recorded hashes, preserving source/provenance.
- Eleven generated sources are 1254-square and five 1774x887. Accepted full-mip DXT1 outputs: six 1024-square, four 1024x512, one 512-square, one 2048-square and one 2048x1024. All resampled, not native 4K. Accepted luminance changes -5.72% to +21.26%; mapped brightness and UV adjacency remain pending. Slot/coil profile diagnostics supplement the offline visual review.
- All 342 retained DDS pass native format and worklist hash, dimensions/aspect, mip-chain and cap checks. Coverage: 342 installed / 6,812 supported keys; 6,445 need art and 25 drafts need correction. Visual classification remains 755 provisional keys; full category and unresolved embedded/procedural coverage remain open.
- Active pack including metadata: 474,589,415 bytes (452.60 MiB) / 6 GiB, +9,968,904 this batch. Texture workspace at audit: 1,767,689,868 allocated bytes (1.65 GiB); original/review/preview files: 158,403,381 bytes / 256 MiB. Free disk: 137,342,664,704 bytes. No duplicate game, pack, ZIP or staging tree; personal saves/settings untouched.
- Private evidence: `review/hybrid-70-*`, including compressed comparisons, repetition, metrics, pattern profiles, native audit, retirement hashes and acceptance record. Artwork stays ignored/local. Shared DDS path unchanged; Mac scene/art verification pending. Consolidated playtesting stays deferred to the near-completion gate; this remains a partial offline-reviewed pack.


## Texture overhaul hybrid batch 71 — machinery and effects triage

- Continued issue #74 from `db8a296` in `build/texture-74-source`, sharing the single authoritative private artwork/game folder. Verified supported US XBE and original payload hashes; no runtime/tooling changes or gameplay launches.
- Generated 14 material/equipment drafts; accepted 13 after actual-DDS and 2x2 repeat review. Held brass casing `7ee7018cb8e3a39a` for changed reflection bands. Original remains active; retired only its DDS/metadata with recorded hashes and retained source/provenance. Similar receiver `470d6c6497546120` failed color-similarity reuse thresholds; stamped-text atlas `dc35c80ac987baa2` remains for specialist treatment.
- Eight generated sources are 1254-square, six 1774x887. Accepted full-mip DXT1 outputs: three 512-square, two 1024-square, two 2048-square, five 1024x512 and one 512x256. All resampled, not native 4K. Accepted luminance changes -8.92% to +21.51%; vehicle atlas +21.51%. Final scene brightness, mapped seams and detail remain pending. Repetitive profile frequencies agree for sampled rivets, vents, louvers and treads; this supplements visual review, not scene validation.
- Classified 32 additional originals using visible color and decoded alpha: 787 provisional classifications total. These effects, reticles, masks and interface shapes require specialist review. All 355 retained DDS pass native format and worklist hash, dimensions/aspect, mip and cap checks. Coverage: 355 installed / 6,812 supported keys; 6,431 need art and 26 drafts need correction. Full-category and unresolved embedded/procedural coverage remain open.
- Pack including metadata: 483,946,646 bytes (461.53 MiB) / 6 GiB; +9,357,231 this batch. Texture workspace at audit: 1,822,735,910 allocated bytes (1.70 GiB); original/review/preview files 162,128,360 bytes / 256 MiB. Free disk 137,074,503,680 bytes. No duplicate game, pack, archive or staging tree; saves/settings untouched.
- Evidence stays ignored/local under `private/texture-remaster/review/hybrid-71-*`: originals/alpha triage, generation prompts, compressed comparisons, repeat sheets, metrics, pattern profiles, variant decision, native audit, retirement hashes and acceptance. Shared DDS path unchanged; Mac scene/art validation pending. Consolidated playtesting remains deferred to the established near-completion gate.


## Texture overhaul hybrid batch 72 — specialist interface alpha review

- Continued issue #74 from `6cfff18` in the isolated source worktree with explicit shared artwork/game paths. Verified supported US XBE and original payload hashes. No runtime/tooling changes or gameplay launches.
- Generated eight interface-mask drafts; accepted zero. Actual DXT5 alpha review exposed changed glyph scale, stroke/opacity, negative gaps, ruler ticks, extra marks, panel placement and bracket baseline. The closest glowing corner still shifted the right-edge profile relative to the top, risking modular seams. All eight originals remain active. Retired only candidate DDS/metadata with recorded hashes; preserved source/provenance and specific correction targets.
- Original format is verified A8 (Xbox format 25). Compiled using alpha semantics so white RGB does not hide alpha errors. Measured alpha mass ratios 0.22-1.68 and alpha64 silhouette overlap 0.20-0.82; format compatibility alone did not qualify any draft. Six sources are 1254-square, one 1774x887 and one 887x1774; candidate dimensions 128-256 per axis, all resampled. No native-4K claim.
- Inspected 32 additional opaque originals: 819 provisional classifications total, with ten environment-material candidates now available. Continue material production and defer exact UI correction to specialist batches. Coverage remains 355 installed / 6,812 supported keys; 6,423 need art, 34 held drafts. Full category/scene review and unresolved embedded/procedural scope remain open.
- All 355 retained DDS pass native format and worklist hash, dimensions/aspect, mip-chain and cap checks. Pack unchanged at 483,946,646 bytes (461.53 MiB) / 6 GiB. Texture workspace at audit: 1,825,513,207 allocated bytes (1.70 GiB); original/review/preview files 163,250,400 bytes / 256 MiB; free disk 134,966,263,808 bytes. No duplicate game/pack, archive or staging tree; saves/settings untouched.
- Evidence is private/local under `review/hybrid-72-*`: verified selection, transparent generation prompts, compiled-alpha comparisons/metrics, held reasons, retirement hashes, candidate and retained native audits, and triage sheets. Shared DDS path unchanged; Mac scene validation remains pending. Playtesting stays deferred to the near-completion gate.


## Texture overhaul hybrid batch 73 — environment material production

- Continued issue #74 from `d734d78` in `build/texture-74-source`, sharing the single private artwork/game folder. Verified supported US XBE and all ten original payload hashes and opaque channels. No runtime/tooling changes or game launches.
- Accepted ten materials after original/actual-DDS and 2x2 repeat review: rusted panels, pale panel, scrap metal, brown molding, wall mounting lip, dark double panel, pale concrete, banded concrete and stone tiles. No new held drafts. Existing UI corrections stay incomplete and originals remain active for those keys.
- Seven generated sources are 1254-square, three 1774x887. Accepted full-mip DXT1 outputs: six 1024-square, one 2048-square and three 1024x512. All resampled, not native 4K. Luminance changes -4.83% to +9.26%; scrap shifts toward warmer rust (mean RGB 91.33/93.64/99.99 to 95.03/88.09/83.60). Final scene color, lighting and UV adjacency remain pending.
- Stone grout minima align at source x84/x172 and y87/y173; top trim keeps dominant repetition frequency 54. Pattern measurements supplement reviewed sheets and do not establish runtime seam behavior. Concrete holes and upper marks visually checked against source.
- All 365 retained DDS pass native format and worklist hash, dimensions/aspect, mip and cap checks. Coverage: 365 installed / 6,812 supported keys; 6,413 need art and 34 drafts need correction. Classification remains 819 provisional keys. Full-category and unresolved embedded/procedural coverage remain open.
- Pack including metadata: 491,991,443 bytes (469.20 MiB) / 6 GiB, +8,044,797 this batch. Texture workspace at audit: 1,873,489,335 allocated bytes (1.74 GiB); original/review/preview files 166,702,470 bytes / 256 MiB. Free disk 134,719,901,696 bytes. No duplicate game/pack, archive or staging tree; saves/settings untouched.
- Evidence stays private/local under `review/hybrid-73-*`: verified selection, prompts, compiled comparisons, repetition, metrics, feature/pattern checks, native audit and acceptance. Shared DDS path unchanged; Mac scene validation remains pending. Consolidated playtesting remains deferred to the near-completion gate.

## Texture hybrid batch 74: character and equipment atlas production

- Source tools: issue #74 branch `74-enhancement-gfx-overhaul-to-modernize-textures`, base `108a7af`, Windows x64, shared source checkout `build/texture-74-source`; authoritative private art remains in the main project. Concurrent issue-17 work preserved.
- Pinned supported US XBE and selected catalog payload hashes checked before original export; original alpha is opaque for all twelve selected keys.
- Installed ten offline-approved replacements: `5fee9f04eab08992`, `6aa486cb72b96bfd`, `4a771e9180d9e045`, `2ff59899ffc1ad54`, `989ba43c180bb790`, `0fde6ed26dbdb7d7`, `141676ea5835f51d`, `d35e27e9d2839bab`, `f85c4e7d87a81055`, `5916532bea0f65e0`.
- Accepted ten character/equipment atlases after original-versus-compiled DDS, repeat sheets and targeted detail review. Existing UV island orientation/coverage and opaque alpha retained; final mapped scene appearance remains unverified.
- Two drafts held: 90181f75b2978276 badge gains an incorrect raised inner border; 28346cd221a64a88 face gains stronger wrinkles and altered lip shading. Removed only their four compiled DDS/metadata files after exact path/hash verification. Originals remain active; source drafts retained for correction.
- Twelve generated sources: six 1254-square, four 1774x887 and two 887x1774. Ten accepted DXT1 full-mip outputs: two each at 1024-square, 512-square, 1024x512, 512x1024 and 512x256. All resampled, not native 4K.
- Accepted mean luminance changes range from -9.81% to +11.83%; stronger metallic highlights and fabric microdetail require eventual scene review. Rib counts/spacing, mechanical apertures, green indicator and face atlas placement inspected offline. Repeat views of atlases are diagnostic, not evidence of seamless mapped adjacency.
- All 375 retained DDS pass native format and worklist hash/dimension/aspect/full-mip/cap checks. 6401 keys need artwork; 36 drafts held. Visual classifications remain 819 provisional keys, with 5979 keys still in visual triage. Full category, embedded/procedural coverage and scene validation remain open.
- No game launches or runtime/tooling changes. Shared DDS path unchanged; Mac scene validation remains pending. One active private pack; no duplicate private data, archive or staging tree. Artwork remains local/ignored and saves/settings untouched.
- Storage at audit: active pack 495,317,649 bytes (472.37 MiB), including metadata, under 6 GiB; this batch adds 3,326,206 bytes. Texture workspace 1,916,778,887 logical / 1,916,778,887 allocated bytes (1.79 GiB). Temporary originals/review/previews 169,504,086 bytes (161.65 MiB), under 256 MiB. Free disk 134,477,381,632 bytes. These are texture-workspace measurements, not whole-project totals.
- Private evidence: `review/hybrid-74-selection.json`, `hybrid-74-final-*`, `hybrid-74-details.jpg`, `hybrid-74-retired.json`, `hybrid-74-native-audit.txt`, `hybrid-74-acceptance.json`; refreshed worklist, hybrid plan, inventory and progress. Only this progress record is tracked.

## Texture hybrid batch 75: weapon and equipment atlas production

- Source tools: issue #74 branch `74-enhancement-gfx-overhaul-to-modernize-textures`, base `094da0c`, Windows x64, source checkout `build/texture-74-source`; authoritative private art remains in the main project. Concurrent checkout work preserved.
- Supported US XBE SHA-256 and all sixteen selected catalog payload hashes verified before original export. All originals and generated sources have opaque alpha.
- Added eleven offline-approved keys: `51c5764a9f508075`, `6248b2fbe26534b9`, `470d6c6497546120`, `b44ba3f45d32abcc`, `0cc1bb991450f4e6`, `0cde9bc92ec13a8f`, `14273563419860f2`, `149894461d25dd1b`, `3fff1f1267d08d2b`, `5257e5c4375e4186`, `7209c6fb7d243979`.
- Accepted eleven weapon/equipment atlases after source-versus-compiled DDS, four repeat sheets and targeted detail review: dark cylinder and receiver, brass double panel and rim, three wood grip/stock surfaces, cracked wood/olive hardware, yellow component, gold/blue equipment and yellow equipment variant.
- Five held drafts remain incomplete: circular assembly 5a5398ad59c0c66f is +128.08% brighter; bronze panel 6b155a1de1bc3634 is +45.39%; mechanism 4f9d301a17dc6866 shifts/reconstructs lower-right fitting; 6970d358c57872cb loses emblem notches and alters display; e466e4017a3c9949 is +48.08% brighter with changed port/panel details. Exact hashes/paths recorded before removing only their ten compiled DDS/metadata files. Original game textures remain active.
- Sixteen generated sources: eleven 1254-square, four 1774x887 and one 887x1774. Accepted DXT1 full-mip outputs: six 1024-square, three 1024x512, one 512x1024 and one 512-square. All resampled, not native 4K.
- Accepted mean luminance changes -10.70% to +23.68%; material sheen differs and requires final scene validation. Cylinder reflection profile remains centered at original column 175. Dominant cropped pattern frequencies match for checkered grip and both carved wood stocks (sign-equivalent FFT pair accepted); these are diagnostic checks, not mapped seam proof.
- Yellow component original has four groups of three bars, twelve total. Prompt incorrectly requested eight, but actual generated/compiled output retains original twelve; enlarged source/output review governs acceptance. Gold/blue grille and yellow variant emblem reviewed separately. Repeat atlas sheets do not establish seamless model adjacency.
- All 386 retained DDS pass native format and worklist hashes/dimensions/aspect/full-mips/cap checks. 6385 supported keys still need art and 41 drafts are held; 819 provisional classifications, 5979 visual-triage keys, 114 individual and 292 specialist queue entries. Unclassified, embedded/procedural coverage and full category/scene validation remain open.
- No game launches or runtime/tooling changes. Shared DDS path unchanged; Mac scene validation pending. Single local private pack, no duplicate game/extract/archive/staging. Source art/provenance retained privately; saves/settings untouched.
- Storage at audit: active pack 501,091,102 bytes (477.88 MiB), including metadata, under 6 GiB; batch adds 5,773,453 bytes. Texture workspace 1,973,530,313 logical / 1,973,530,313 allocated bytes (1.84 GiB). Temporary originals/review/previews 174,337,625 bytes (166.26 MiB), under 256 MiB. Free disk 133,734,461,440 bytes. Texture-workspace totals are not whole-project totals.
- Private evidence: `review/hybrid-75-selection.json`, `hybrid-75-final-*`, `hybrid-75-details.jpg`, `hybrid-75-detail-counts.jpg`, `hybrid-75-pattern-diagnostics.json`, `hybrid-75-grille-diagnostics.json`, `hybrid-75-retired.json`, `hybrid-75-native-audit.txt`, `hybrid-75-acceptance.json`. Worklist, hybrid plan, inventory and progress refreshed and reconciled against retained pack. Only progress documentation is tracked.


## Texture hybrid batch 76: grouped environment material production

- Issue #74 branch `74-enhancement-gfx-overhaul-to-modernize-textures`, base `bfe2c57`, Windows x64 source checkout `build/texture-74-source`; single authoritative private artwork folder shared with main project. Concurrent checkout work preserved. Supported US XBE and selected original payload hashes verified before generation.
- Classified 32 additional opaque originals; 851 provisional visual classifications. Produced and accepted fourteen environment materials after source/compiled DDS comparison, four repeat sheets and enlarged structural detail review. No new held drafts.
- Accepted concrete panels, molded walls, cracked/layered surfaces, blue-gray door atlas, olive vent panel, rusted riveted rails, tan wallpaper, wavy steel braces and bamboo. Existing UV layouts, baked lighting cues and opaque alpha retained offline; final scene appearance remains unverified.
- Fourteen generated sources: nine 1254-square and five 1774x887. Full-mip DXT1 outputs: eight 1024-square, one 2048-square, four 1024x512 and one 2048x1024. All resampled; the two 2048-wide outputs are upscaled from smaller generated sources, not native 4K.
- Mean luminance changes range -5.92% to +22.56%; olive vent panel has the largest increase and requires final scene brightness review. No scene-level color/lighting validation claimed.
- 72867f9cad388e77 prompt incorrectly called for three rounded flutes. Enlarged original/output review confirms two rounded flutes and adjacent stepped edge; classification corrected and original prompt preserved with review note. Architectural rail groups also inspected at larger scale.
- Bamboo and wallpaper cropped dominant frequencies match and all five sampled profiles have zero best cyclic shift. Rivet and wavy-steel dominant harmonics differ; enlarged comparisons resolve twelve upper rivets and seven wavy braces. Banded FFT is supplementary, not geometric proof. Repeat sheets do not establish actual mapped adjacency.
- All 400 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/cap checks. 6371 supported keys need art; 41 drafts remain held. Queue: 5947 visual triage, 128 individual and 296 specialist. Full categories, embedded/procedural gaps and scene coverage remain unfinished.
- No game launches or runtime/tool changes. Shared DDS path unchanged; Mac scene validation remains pending. One active private pack, no duplicate game/pack/archive/staging; source artwork remains local and ignored. Saves/settings untouched.
- Storage at audit: pack including metadata 512,283,917 bytes (488.55 MiB) / 6 GiB; batch adds 11,192,815 bytes. Texture workspace 2,041,176,085 logical / 2,041,176,085 allocated bytes (1.90 GiB). Temporary originals/review/previews 180,989,044 bytes (172.60 MiB) / 256 MiB; free disk 131,927,351,296 bytes. These are texture-workspace measurements, not whole-project totals.
- Private evidence under `review/hybrid-76-*`: triage/selection, prompts, compiled and repeat comparisons, structural details, pattern diagnostics, native audit and acceptance. Worklist, hybrid plan, inventory and progress refreshed and reconciled. Only progress documentation is tracked.


## Texture hybrid batch 77: individual equipment and garment atlases

- Issue #74 branch `74-enhancement-gfx-overhaul-to-modernize-textures`, base `e60b17d`, Windows x64 source checkout `build/texture-74-source`. Supported US XBE SHA-256 and eight original payload hashes verified; all selected originals and generated sources have opaque alpha. Concurrent checkout work preserved.
- Generated eight individually inspected equipment/weapon/garment drafts and accepted seven after original-versus-compiled DDS, two repeat sheets and enlarged detail review. Wood/metal atlas, blue-striped housing, blue garment, dark mechanism, gray angular housing, gray/red controls and red housing accepted offline; scene appearance pending.
- Held 700acb4d380ae3c3: grip changes to substantially coarser diagonal diamonds and tiny colored receiver/edge markings are lost or changed. Exact candidate DDS/metadata paths and hashes recorded before removing only those two files. Original remains active; source and prompt retained for focused correction.
- Eight generated sources: six 1254-square, one 1774x887 and one 887x1774. Seven accepted full-mip DXT1 outputs: four 1024-square, two 2048-square and one 512x1024. All resampled; the 2048-square outputs are upscaled from 1254-square, not native 4K artwork.
- Accepted mean luminance changes -16.24% to +16.69%; blue garment is darker and gray angular housing brighter/warmer. Clothing fasteners/buckle, brass-strip counts, mechanical circles and red housing ring openings reviewed at enlarged scale. Final material sheen, color, lighting and mapped seams require scene verification; repeat views of atlases are diagnostic only.
- All 407 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/cap checks. 6363 supported keys need artwork; 42 held drafts. Provisional classifications remain 851; queue has 5947 visual-triage, 120 individual and 296 specialist entries. Full-category, embedded/procedural and scene coverage remain open.
- No game launches or runtime/tool changes. Shared DDS path unchanged; Mac scene validation pending. Single active local private pack, no duplicate game/pack/archive/staging; source artwork stays ignored and local. Personal saves/settings untouched.
- Drive free space fell from 131916271616 bytes before the batch to about 48725544960 bytes at audit, while texture workspace grew only about 34 MiB. A read-only project directory scan is saved in review/hybrid-77-project-storage.json; build allocates about 15.73 GB and private about 6.66 GB (decimal), with sparse logical sizes larger. The drive-wide change is unexplained and is not attributed to this batch. No unrelated files removed.
- Storage at audit: active pack including metadata 521,026,048 bytes (496.89 MiB) / 6 GiB; adds 8,742,131 bytes. Texture workspace 2,075,505,930 logical / 2,075,505,930 allocated bytes (1.93 GiB). Temporary originals/review/previews 182,820,262 bytes (174.35 MiB) / 256 MiB. Texture-workspace totals are not whole-project totals.
- Private source art saved in `private/texture-remaster/authored/*-hybrid-v1.png`; built-in imagegen prompts in `review/hybrid-77-prompts.json` and per-asset provenance. Other private evidence under `review/hybrid-77-*`: selection, comparisons, repeat sheets, details, retirement, native audit, storage scan and acceptance. Worklist, hybrid plan, inventory and progress refreshed/reconciled. Only progress documentation is tracked.


## Texture hybrid batch 78: material classification and production

- Issue #74 branch `74-enhancement-gfx-overhaul-to-modernize-textures`, base `14acadd`, Windows x64 source checkout `build/texture-74-source`. Supported US XBE SHA-256 and selected source payload hashes verified before generation. All selected original/generated alpha is opaque; concurrent checkout work preserved.
- Inspected 32 additional opaque originals and recorded provisional visual classifications, bringing reviewed keys to 883. Reflection/effect maps, control-panel labels and character atlases remain in specialist or individual queues. Classification is not completion.
- Generated and accepted sixteen environment materials after original/compiled DDS, four repeat sheets and selected structural/profile review: green light panel, red/white arc and bands, tread steel, olive vent, stained concrete, angular concrete frame, blue-gray stripe panel, two riveted plates, two green-gray panels, curved dark metal, brushed silver metal, brickwork and large cream vent. No new held drafts.
- Sixteen generated sources: ten 1254-square and six 1774x887. Full-mip DXT1 outputs: five 512-square, three 1024-square, two 2048-square, five 1024x512 and one 2048x1024. All resampled; the three 2048-wide outputs are upscaled from smaller generated sources, not native 4K artwork.
- Mean luminance changes range -19.36% to +10.99%; olive vent and tread are darker, angular concrete brighter. Structure and color cues accepted offline; final scene lighting, material sheen and UV adjacency remain unverified.
- Pattern diagnostics support visual review: bottom grille frequency 32 retained, tread shares diagonal spectral peaks (+/-11,+/-14), dense rivets share 32/16 harmonics, brick vertical profile retains dominant frequency 23. Plain rivet plate retains visible count/placement although rust shifts dominant spectral strength. Numeric frequency is not an automatic acceptance test or scene proof.
- Curved metal highlight peak remains at source row 91 with zero best profile shift. Brushed silver brightest column changes 205 to 199, best normalized shift -2; broad band placement accepted with small profile difference recorded. Stripe and painted-band profiles have zero best shift; their brightest pixels differ due to grain. Repeat atlas views do not establish actual mapped adjacency.
- All 423 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/cap checks. 6347 supported keys need art, 42 held drafts; queue has 5915 visual triage, 127 individual, 303 specialist and two remaining material candidates. Full-category, embedded/procedural and scene coverage remain unfinished.
- No gameplay launches or runtime/tool changes. Shared DDS path unchanged; Mac scene validation pending. Single active private pack, no duplicate game/pack/archive/staging; source art/provenance remain ignored and local. Personal saves/settings untouched.
- Storage at audit: active pack including metadata 532,744,286 bytes (508.06 MiB) / 6 GiB; batch adds 11,718,238 bytes. Texture workspace 2,149,950,790 logical / 2,149,950,790 allocated bytes (2.00 GiB). Temporary originals/review/previews 188,426,312 bytes (179.70 MiB) / 256 MiB; disk free 48,161,325,056 bytes. Texture-workspace totals are not whole-project totals.
- Private source art in `private/texture-remaster/authored/*-hybrid-v1.png`; built-in imagegen prompts in `review/hybrid-78-prompts.json` and per-key provenance. Evidence under `review/hybrid-78-*`: triage, selection, compiled comparisons, repeat sheets, pattern/detail/profile checks, native audit and acceptance. Worklist, hybrid plan, inventory and progress refreshed/reconciled. Only progress documentation is tracked.


## Texture hybrid batch 79: materials, equipment and character atlases

- Issue #74 branch `74-enhancement-gfx-overhaul-to-modernize-textures`, base `36c3715`, Windows x64 source checkout `build/texture-74-source`. Supported US XBE and twelve source payload hashes verified; originals/generated sources opaque. Concurrent checkout work preserved.
- Generated twelve hybrid material/equipment/character drafts; accepted eleven after original/compiled DDS, three repeat sheets, enlarged face/equipment details and functional-cue checks. Accepted six-strip light panel, carved stone molding, dark machined atlas, medical box, two tan garments, hair/skin/cloth atlas, gray/blue uniform/face atlas, stone disc, red/silver mechanism and gray mechanism atlas.
- Held control panel 9946e3336d16bb64: red indicator is dimmer and gains a grid pattern. With identical source-scale thresholds, red core falls 206 to 121 pixels and green core 186 to 153; this supports the visual readability concern, not a claim of exact physical light area. Original stays active. Only its DDS/metadata removed after exact path/hash verification; source art and provenance retained.
- Twelve generated sources: eight 1254-square, three 1774x887 and one 887x1774. Eleven accepted DXT1 full-mip outputs: two 512-square, four 1024-square, one 2048-square, three 1024x512 and one 512x1024. All resampled; 2048-square mechanism is upscaled from 1254-square, not native 4K artwork.
- Accepted mean luminance changes -13.35% to +20.94%; dark machined atlas is darker overall despite sharper blue highlights, while large gray mechanism is brighter. Clothing folds, identity/UV boundaries and equipment layouts accepted offline. Final mapped identity, color, lighting and material sheen remain unverified.
- Medical cross source-scale threshold bounds exactly match [100,26,162,92], with centroid difference below 0.01 pixel. Light panel has six bright row runs at the original positions; two runs narrow by one source pixel at the chosen threshold. These are diagnostic checks alongside visual review, not runtime proof.
- Control vent original/output have seven bars; prompt incorrectly requested five. Preserved original prompt and recorded correction in provenance. Matching vent geometry does not resolve the held indicator treatment. Repeat views of atlases are diagnostic, not mapped-adjacency evidence.
- All 434 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/cap checks. 6335 supported keys need artwork; 43 held drafts. Classification remains 883 provisional keys; queue 5915 visual triage, 117 individual and 303 specialist. Replenish material candidates through original inspection. Full-category, embedded/procedural and scene coverage remain open.
- No game launches or runtime/tool changes. Shared DDS path unchanged; Mac scene validation pending. One active private pack, no duplicate game/pack/archive/staging; source artwork stays ignored/local. Personal saves/settings untouched. Drive free space returned to about 130.41 GB (decimal) at audit; unrelated drive activity remains unattributed.
- Storage at audit: active pack including metadata 540,090,602 bytes (515.07 MiB) / 6 GiB; batch adds 7,346,316 bytes. Texture workspace 2,197,892,324 logical / 2,197,892,324 allocated bytes (2.05 GiB). Temporary originals/review/previews 191,031,117 bytes (182.18 MiB) / 256 MiB; disk free 130,408,751,104 bytes. Texture-workspace totals are not whole-project totals.
- Private source art in `private/texture-remaster/authored/*-hybrid-v1.png`; built-in imagegen prompts in `review/hybrid-79-prompts.json` and per-key provenance. Evidence under `review/hybrid-79-*`: selection, compiled/repeated comparisons, details, functional diagnostics, retirement hashes, native audit and acceptance. Worklist, hybrid plan, inventory and progress refreshed/reconciled. Only progress documentation is tracked.

### Texture hybrid batch 80: reviewed resolution variants (issue #74)

- On `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `494ebb4`, Windows x64. Compared 4,382 eligible originals against 434 installed references; six proposals accepted after enlarged original/compiled DDS and repeat review. Installed 6 variants, bringing the offline-approved pack to 440; runtime scenes pending.
- Reused existing source artwork for brown fur/skin, metal equipment, dark uniform/equipment/face, and three cutscene garment/hair/skin atlases. No new source images or copies. All six compiled hashes equal their reviewed references; source sizes/resampling remain recorded. Three 1254-square and three 887x1774 sources; outputs two 1024-square, one 2048-square, three 512x1024.
- All 440 DDS pass native format and worklist hash/dimensions/aspect/full-mip/budget checks. Six originals remain opaque; luminance changes -16.14% to +9.18%. UV/identity and cues accepted offline; mapped scene fidelity is unverified. Repeat sheets are diagnostic, not adjacency proof.
- Pack 545,336,915 bytes (520.07 MiB), including metadata, below 6 GiB; added 5,246,313 bytes. Texture workspace 2,207,289,466 allocated/logical bytes; exports/reviews 195,155,119 bytes below 256 MiB. Free drive space 130,501,058,560 bytes at audit. No duplicate game/pack/staging or personal-save/settings changes.
- Remaining: 6,329 need art; 43 held drafts; 889 provisional visual classifications; 5,909 visual-triage, 117 individual and 303 specialist keys. Full category, embedded/procedural and scene coverage remain open. No game launches; near-completion testing gate unchanged. Shared DDS path unchanged; Mac scene validation pending.
- Private evidence: `review/hybrid-80-variant-candidates.json`, `hybrid-80-variants-1.jpg`, two detail sheets, two final/repeat pairs, metrics, native audit and acceptance JSON. Artwork/provenance remain ignored and local.

### Texture hybrid batch 81: twelve material replacements (issue #74)

- Branch `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `f0d2c44`, Windows x64. Inspected 32 opaque originals; generated and accepted 12 material/crate replacements, bringing the offline-approved pack to 452. Runtime scene validation remains pending.
- Inspected 32 previously unclassified opaque originals. Generated and accepted twelve replacements: dark grained panel, ochre plaster/stone, brushed metal, lit rock, brass molding, tread/riveted steel, bronze strip, pale concrete, two stone slabs, olive crate, dark grass and four-screw metal plate. Other candidates retain individual or specialist routing.
- Built-in imagegen produced five 1254-square and seven 1774x887 sources. All resampled downward to five 1024-square and seven 1024x512 DXT1 full-mip outputs. No native 4K claim; source art and exact prompts retained locally.
- All twelve original/compiled comparisons, three repeated views and enlarged hardware details accepted offline. Mean luminance changes -16.82% bronze to +10.81% crate. Broad material palettes and structures retained; mapped lighting, metallic sheen and grass density remain scene-validation items.
- Initial visual concern about moved concrete anchors was disproved at source scale: four dark-region centroid differences <=0.5 pixel; dark threshold core areas grow from 1-2 to 6-7 pixels with sharper recesses. Four corner plate screw dark-region centroid differences <0.3 pixel. These threshold measurements supplement visual review, not geometric proof.
- Rock retains eight light-bar profile peaks at source rows [51,71,93,117,138,161,184,204], versus original [51,71,93,118,138,161,184,204]. Tread/rivet and crate cross-brace/hinge layouts reviewed in enlarged comparisons. Repeat views retain existing edge divisions; they do not establish actual scene adjacency or seamlessness.
- All 452 retained DDS pass native format and worklist hashes, dimensions, aspect, full-mip and budget audits. 6317 need art, 43 held drafts unchanged, 921 provisional classifications. Queue: 5877 visual triage, 131 individual and 309 specialist. All-category, scene and embedded/procedural coverage remain unfinished.
- No game launches or runtime/tool changes. Shared DDS path unchanged; Mac scene validation pending. One private pack; no duplicate game/pack/ZIP/staging; personal saves/settings untouched. Temporary outputs remain below 256 MiB.
- Storage at audit: pack 551,285,713 bytes (525.75 MiB) including metadata / 6 GiB; batch adds 5,948,798 bytes. Texture workspace 2,262,363,472 allocated/logical bytes (2.11 GiB); temporary originals/review/previews 200,882,134 bytes (191.58 MiB) / 256 MiB. Free disk 130,147,004,416 bytes. Workspace figures are not whole-project totals.
- Private sources `authored/*-hybrid-v1.png`, exact prompts `review/hybrid-81-prompts.json` and per-key provenance. Evidence `review/hybrid-81-*`: four triage sheets, three compiled/repeat pairs, hardware details, anchor/light diagnostics, metrics, native audit and acceptance. Worklist, hybrid plan, inventory and progress reconciled; only this progress documentation tracked.

### Texture hybrid batch 82: seven equipment/prop replacements (issue #74)

- Branch `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `562190a`, Windows x64. Seven new offline-approved replacements, one held, 459 installed total.
- Generated eight individually reviewed prop/equipment drafts; accepted seven: horizontal metal trim atlas, three-slot dark steel plate, brass-handled dark wooden door, gray vented equipment atlas, red/white striped paint, inset dark panel with cylindrical fitting, olive diagonal equipment atlas.
- Held b2d963f371d58a1a: stronger orange rust and +34.76% mean luminance change the original dark steel treatment. Original remains active. Removed only its DDS and metadata after resolved-path/hash checks; source, prompts and review retained for correction. No retry this batch.
- Eight built-in imagegen sources: seven 1254-square and one 1774x887. Seven accepted full-mip DXT1 outputs: two 512-square, three 1024-square, one 2048-square and one 1024x512. All resampled; 2048-square atlas is upscaled from 1254-square, not native 4K art.
- Reviewed two compiled comparison sheets, two repeat sheets and enlarged vents/slots/bar details. Accepted mean luminance changes -11.50% to +20.28%; fitting panel has stronger wear highlights. Broad UV islands and functional shapes accepted offline, mapped lighting/material sheen still unverified.
- Red stripe column runs exactly match [0,15], [32,47], [112,127] at original 128px width. Three-slot dark runs match except third begins at 128 versus 129 at 256px width. Equipment horizontal vent diagnostic peaks match exactly; vertical profile differs at one secondary peak by one source pixel. Profiles include secondary highlights and are not rib counts or automatic approval.
- Olive atlas original/output have three upper bars; prompt incorrectly requested four. Exact original prompt preserved and review correction recorded. Threshold aperture measurements for held steel panel also contain nearby dark material and do not establish circle geometry; hold decision follows appearance and luminance review.
- All 459 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/budget checks. Remaining 6309 need art, 44 held drafts; provisional classification 921 unchanged. Queue 5877 visual triage, 123 individual, 309 specialist. Full category, scene and embedded/procedural coverage remain unfinished.
- No gameplay launches or runtime/tool changes. Shared DDS path unchanged; Mac scene validation pending. One active private pack, no duplicate game/pack/archive/staging; artwork stays ignored/local and personal saves/settings untouched. Temporary files remain below 256 MiB.
- Storage at audit: pack 556,882,111 bytes (531.08 MiB) including metadata / 6 GiB; added 5,596,398 bytes. Texture workspace 2,296,270,973 allocated/logical bytes (2.14 GiB); temporary originals/review/previews 202,967,836 bytes (193.57 MiB) / 256 MiB; disk free 132,098,547,712 bytes. Texture totals are not whole-project usage.
- Private evidence `review/hybrid-82-*`: exact prompts, selection, compiled/repeat sheets, details, functional diagnostics, retirement hashes, native audit and acceptance. Sources `authored/*-hybrid-v1.png`; canonical per-key provenance records support correction routing. Worklist, hybrid plan, inventory and progress reconciled. Only progress documentation tracked.

### Texture hybrid batch 83: eleven material/architecture replacements (issue #74)

- Branch `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `c96366e`, Windows x64. Eleven additions, one held, 470 installed/offline-approved total; runtime scenes remain unverified.
- Inspected 32 previously unclassified opaque originals. Generated twelve material/architectural drafts; accepted eleven: two molding strips, blue-green equipment atlas, paired blue-gray panels, concrete seam panel, framed sliding door, building atlas, weathered wall molding, pitted concrete, framed slat grate and orange brick.
- Held eabd63635e9221d0: dense fine scratched grid became strong coarse masonry-like seams and mean luminance fell 27.69%. Original remains active. Only compiled DDS/metadata removed after exact path/hash checks; authored source/prompts/reviews retained for correction.
- Twelve built-in imagegen sources: six 1254-square, six 1774x887. Eleven accepted DXT1 full-mip outputs: one 512-square, three 1024-square, one 2048-square, six 1024x512. All resampled; brick 2048-square is upscaled from 1254-square, not native 4K art.
- Reviewed three original/compiled and three repeat sheets plus enlarged red vent, grate, building-window and held-grid details. Accepted luminance changes -16.19% brick to +8.21% grate. Atlas partitions, openings, color cues and material layouts accepted offline; scene appearance, brick darkness and material sheen remain pending.
- Concrete seam darkest row changes 80 to 79 at original 128px scale. Grate upper-left eight highlight peaks match exactly; lower-row highlight peak shifts include an 8px change around an irregular slat, reviewed visually with the original dark gap and edge locations. Profile peaks are not slat counts or exact geometry.
- Red-vent threshold centroid differs about 0.76 original pixel; threshold bounds expand into adjacent reddish frame pixels ([28,86,55,115] to [22,82,55,116]). The frame/vent visual layout and color cue were reviewed separately. Stronger rib relief remains an in-scene review item; threshold bounds are not physical vent bounds.
- All 470 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/budget checks. 6297 keys need art and 45 drafts held; 953 provisional visual classifications. Queue 5845 visual triage, 139 individual and 313 specialist. Full category, scene and embedded/procedural coverage remains incomplete.
- No game launches or runtime/tool changes. Shared DDS path unchanged; Mac scene validation pending. One active private pack, no duplicate game/pack/archive/staging; artwork local/ignored and personal saves/settings untouched. Temporary outputs below 256 MiB.
- Storage at audit: pack 564,053,673 bytes (537.92 MiB) including metadata / 6 GiB; adds 7,171,562 bytes. Texture workspace 2,354,605,156 allocated/logical bytes (2.19 GiB); temporary originals/review/previews 208,295,553 bytes (198.65 MiB) / 256 MiB; free disk 131,773,374,464 bytes. These are texture totals, not whole-project totals.
- Private evidence `review/hybrid-83-*`: four triage sheets, exact prompts/selection, three compiled/repeat pairs, details, functional diagnostics, retirement hashes, native audit and acceptance. Source art `authored/*-hybrid-v1.png`, canonical per-key provenance retained. Worklist, hybrid plan, inventory and progress reconciled; only progress documentation tracked.

### Texture hybrid batch 84: seven character/equipment replacements (issue #74)

- Branch `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `7b7d130`, Windows x64. Seven additions, one held, 477 installed/offline-approved total; runtime scenes remain unverified.
- Eight individual character/equipment drafts generated; seven accepted: blue shirt/ear/sleeve, pale olive machine, dark two-circle receiver/grille, damaged gray housing, tan marked skin/hair/cream cloth, blue trimmed garment and olive four-fastener fitting.
- Held ce909c4ca5a0a0b4: trigger guard shifted right/down and receiver boundaries drifted. Original remains active; only compiled DDS/metadata removed after exact resolved-path/hash checks. Source, exact prompts and reviews retained for focused correction.
- Eight built-in imagegen sources: one 887x1774, four 1774x887 and three 1254-square. Seven accepted full-mip DXT1 outputs: one 512x1024, three 1024x512, one 512-square, one 1024-square and one 2048-square. All resampled; housing upscaled from 1254-square, not native 4K art.
- Reviewed two original/compiled and two repeat sheets, plus enlarged trigger, shoulder markings and grille comparisons. Accepted luminance changes -19.91% dark receiver to +22.84% damaged housing. Broad atlas partitions, markings, hair direction, trim, openings and color cues accepted offline; material sheen, skin tone and brightness remain scene-validation items.
- Grille intensity profiles contain extra secondary highlights in the new source; they are not rib counts or exact geometry proof. Manual crop comparison reviewed recurring slot spacing and overall opening boundaries. Repeat sheets do not establish seamless mapped adjacency for these atlases.
- All 477 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/budget checks. 6289 keys need art and 46 drafts held; 953 provisional classifications unchanged. Queue 5845 visual triage, 131 individual, 313 specialist. Full category, scene and embedded/procedural coverage remains incomplete.
- No game launches or runtime/tool changes. Shared DDS path unchanged; Mac scene validation pending. Single active private pack, no duplicate game/pack/archive/staging. Artwork local/ignored and personal saves/settings untouched. Temporary outputs below 256 MiB.
- Storage at audit: pack 569,125,795 bytes (542.76 MiB) including metadata / 6 GiB; adds 5,072,122 bytes. Texture workspace 2,385,919,555 allocated/logical bytes (2.22 GiB); temporary originals/review/previews 209,725,641 bytes (200.01 MiB) / 256 MiB; free disk 131,651,829,760 bytes. Texture totals are not whole-project totals.
- Private evidence `review/hybrid-84-*`: exact prompts/selection, two compiled/repeat pairs, detail crops, functional diagnostics, retirement hashes, native audit and acceptance. Sources `authored/*-hybrid-v1.png`; canonical per-key provenance retained. Worklist, hybrid plan, inventory and progress reconciled; only progress documentation tracked.

### Texture hybrid batch 85: original-alpha compilation and seven surfaces (issue #74)

- Branch `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `52f6946`, compiler commit `ab53ce3`, Windows x64. Seven additions, one held, 484 installed/offline-approved total; runtime scenes remain unverified.
- Inspected RGB and alpha for 32 additional originals; classification now 985 keys. Generated eight material drafts; accepted seven: four gray slabs, dark framed metal, seven dark window bays, pale scratched metal, weathered brown strip, alternating blue/gray grid and pale paired panel rows.
- Held 882032900617d989: rock draft has a rounded RGB cutout and missing corner coverage. Original alpha restoration cannot repair missing color or tiling. Original remains active. Removed only compiled DDS/metadata after exact resolved-path/hash checks; authored source/prompts/reviews retained.
- Compiler commit ab53ce3 adds optional verified original-alpha conversion with required SHA-256, aspect/channel/format checks, independent RGB resampling and alpha provenance. Generated alpha is not used in this batch. Original exported pixels were verified against catalog payloads before compilation. Defaults and runtime loaders unchanged.
- 31 offline compiler/planner tests pass on Windows, including lossless/compressed channel routing, stale hashes, missing arguments, wrong aspect, absent alpha and unsupported channel/format rejection with no output writes. BC3 fixture max error 23/255 is documented; it is not a visual approval threshold.
- Eight built-in imagegen sources: four 1254-square and four 1774x887. Seven final outputs: three 1024-square and four 1024x512; all generated RGB downsampled. Original alpha bilinearly enlarged, lower mips Lanczos. Five RGBA8 and two DXT5 full-mip outputs; no native 4K art claim.
- Initial DXT5 lower-mip alpha errors reached 38/255 among retained drafts. Five surfaces converted to RGBA8 after exact-path/hash retirement of candidate encodings; final decoded alpha matches expected resampled original at every mip exactly. Two retained DXT5 surfaces have max all-mip errors 9/255 (slabs) and 15/255 (pale metal); top-mip means 0.54/255 and 0.40/255. Masks and dark/light composites reviewed; alpha usage remains scene-unverified.
- Reviewed original/compiled and repeat sheets before/after format choice plus alpha diagnostics. Tile counts, panel boundaries, seven window bays, stain placement and material cues accepted offline. Final RGB luminance change -8.89% weathered strip to +17.64% window bays; sheen, fine relief, mapped adjacency and scene alpha effects pending.
- All 484 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/budget checks. 6281 keys need art, 47 held drafts; queue 5813 visual triage, 142 individual, 326 specialist. Full category, scene and embedded/procedural coverage remains incomplete.
- No game launches. Shared offline tooling needs no Mac adapter; Mac execution and artwork scene validation pending in MAC-011. Single active private pack, no duplicate game/pack/archive/staging; source art local/ignored and personal saves/settings untouched. Temporary outputs remain below 256 MiB.
- Storage at audit: pack 590,803,529 bytes (563.43 MiB) including metadata / 6 GiB; adds 21,677,734 bytes. Texture workspace 2,437,213,640 allocated/logical bytes (2.27 GiB); temporary originals/review/previews 215,227,589 bytes (205.26 MiB) / 256 MiB; free disk 131,450,015,744 bytes. Texture totals are not whole-project totals.
- Private evidence `review/hybrid-85-*`: RGB/alpha triage sheets, exact prompts/selection, candidate and accepted compiled/repeat comparisons, alpha composites, candidate/final all-mip alpha metrics, retirement hashes, native audits and acceptance. Sources `authored/*-hybrid-v1.png`, canonical provenance retained. Worklist, hybrid plan, inventory and progress reconciled. Tracked changes are compiler/tests/docs/progress only.

### Texture hybrid batch 86: five equipment replacements with mask review (issue #74)

- Branch `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `4dfd911`, Windows x64. Five additions, three held, 489 installed/offline-approved total; runtime scenes remain unverified.
- Generated eight individually reviewed equipment drafts from the classified queue. Accepted five: circular socket/rectangular inset atlas, riveted cross-braced girder, radial toothed fitting, brown rung/rail strip and pale circular drum/fitting.
- Held 007e952dabb45daf for colored RGB edge contamination around the opening and +41.99% original-alpha-weighted brightness; held 5052dd08245abc4a for drifting rib spacing/phase and +31.67% weighted brightness; held 89c3c70c89c865a4 for shifted ribs/lower strips and clipped pale end cap after original alpha. Originals remain active. Only candidate DDS/metadata removed after exact resolved-path/hash checks; sources/prompts/reviews retained.
- Eight built-in imagegen sources: four 1254-square and four 1774x887. Five accepted outputs: two 1024-square, two 512-square and one 1024x512. All generated RGB downsampled. Four full-mip RGBA8 cutout assets use verified original alpha, one full-mip DXT1 asset is opaque. No native 4K art claim.
- Original exported pixels rechecked against catalog payloads. Accepted RGBA8 alpha matches bilinear original at the top mip and recursive Lanczos at every lower mip exactly. Opaque circular asset retains opaque top-mip alpha. Technical alpha retention does not prove mapped color alignment or shader semantics.
- Reviewed two compiled comparison and two repeat sheets, original-alpha sheet, two dark/light original-versus-compiled composite sheets and enlarged rib crops. Full-canvas RGB luminance is misleading for formerly gray transparent unused regions; original-alpha-weighted diagnostic changes for accepted assets range +0.23% rung strip to +26.17% radial fitting. Material sheen and mapped lighting remain scene checks.
- Smoothed rib intensity minima and visual crops expose spacing drift in held strips. These profiles include secondary highlights and are not exact geometry counts. Accepted socket/inset partitions, two girder X braces, radial silhouette, slot/rung structure and drum rim reviewed with original masks; scene adjacency and cutout appearance remain unverified.
- All 489 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/budget checks. 6273 keys need art, 50 drafts held; classification remains 985. Queue 5813 visual triage, 134 individual, 326 specialist. Full category, scene and embedded/procedural coverage remains incomplete.
- No game launches or source-tool/runtime changes. Shared original-alpha compiler ab53ce3 unchanged; prior 31 compiler/planner tests retained as evidence, not rerun for artwork-only additions. Mac real-art scene validation pending. Single active private pack, no duplicate game/pack/archive/staging; artwork local/ignored and personal saves/settings untouched.
- Storage at audit: pack 602,692,008 bytes (574.77 MiB) including metadata / 6 GiB; adds 11,888,479 bytes. Texture workspace 2,469,629,528 allocated/logical bytes (2.30 GiB); temporary originals/review/previews 217,072,587 bytes (207.02 MiB) / 256 MiB; free disk 131,322,777,600 bytes. Texture totals are not whole-project totals.
- Private evidence `review/hybrid-86-*`: original alpha, exact prompts/selection, compiled/repeat and composite sheets, rib detail/profiles, visible-region metrics, final alpha-mip audit, retirement hashes, native audit and acceptance. Sources `authored/*-hybrid-v1.png`; canonical provenance retained. Worklist, hybrid plan, inventory and progress reconciled; only progress documentation tracked.

### Texture hybrid batch 87: six held drafts corrected (issue #74)

- Branch `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `c4cebb0`, Windows x64. Six additions, 495 installed/offline-approved total; runtime scenes remain unverified.
- Seven new built-in imagegen corrections and one reused wheel source reviewed. Six prior holds resolved: olive wheel, brown framed inset, dark circular mechanism, bronze riveted atlas, four-hole gray steel atlas and fine scratched grid. Two revisions remain held.
- Riveted strap panel 5ffb12de4fe0f43e overcorrected brightness (-37.54%); arrow panel 3e671a5cc072ddc1 lost small original dash/label markings above the yellow stripe. Originals remain active. Removed only candidate DDS/metadata after exact path/hash checks; correction sources and prompts retained.
- All eight selected sources are 1254-square. Six accepted full-mip outputs: one 512-square DXT1, three 1024-square DXT1, one 2048-square DXT1 and one 1024-square RGBA8 wheel. Steel atlas upscaled from 1254-square; other accepted RGB downsampled. No native 4K source claim.
- Original exports match verified catalog payloads for all eight keys. Wheel alpha exactly matches bilinear original enlargement and recursive Lanczos at all eleven mip levels, retaining the transparent final row. Generated opaque wheel RGB reused without new generation.
- Reviewed two original/compiled and two repeat sheets, enlarged arrow and four-hole steel comparisons, and all-mip wheel alpha. Four holes, original lower small circle, seven ringed mechanism apertures, bronze four openings/two slots and major panel boundaries reviewed. Accepted RGB luminance changes -9.41% to +26.45%; sheen, fine relief, UV adjacency and mapped lighting remain scene checks.
- All 495 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/budget checks. 6273 keys need art, 44 drafts held; 985 provisional classifications unchanged. Queue 5813 visual triage, 134 individual and 326 specialist. Full categories, scenes and embedded/procedural coverage remain incomplete.
- No game launches, source-tool or runtime changes. Prior compiler/planner tests retained; no artwork-only test rerun. Mac scene validation pending. Single private pack; no duplicate game, pack, ZIP or staging. Personal saves/settings untouched.
- Five superseded failed source copies removed from project after exact SHA-256 matching against retained generator recovery files, reclaiming 16,771,703 bytes. Accepted art, failed current drafts, prompts, reviews and recovery originals preserved.
- Storage at audit: pack 613,356,397 bytes (584.94 MiB) including metadata / 6 GiB; adds 10,664,389 bytes. Texture workspace 2,487,036,465 allocated/logical bytes (2.32 GiB); temporary originals/review/previews 219,389,911 bytes (209.23 MiB) / 256 MiB; free disk 131,168,124,928 bytes. Texture totals are not whole-project totals.
- Private evidence `review/hybrid-87-*`: exact prompts/selection, compiled/repeat and detail comparisons, source verification and wheel alpha-mip audit, candidate/source retirement hashes, native audit and acceptance. Canonical and versioned provenance plus pre-correction prompt backups retained. Worklist, hybrid plan, inventory and progress reconciled; only progress documentation tracked.

### Texture hybrid batch 88: sixteen material replacements (issue #74)

- Branch `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `21d43f1`, Windows x64. Sixteen additions, 511 installed/offline-approved total; runtime scenes remain unverified.
- Inspected 32 further opaque originals from verified catalog payloads; 1,017 keys now provisionally classified. Control displays, precise lettering, characters and weapon atlases remain separate from routine material production.
- Sixteen built-in imagegen sources accepted offline: pale wall strips, green-gray plaster, gray aggregate, bronze riveted strip, stepped-groove concrete, paired metal panels, irregular stone paving, dark striped wall, worn plaster, light chipped aggregate, brown oxidation, pale center-seamed concrete, dark wood, green framed metal and two mottled metal surfaces. No held drafts added in this batch.
- Sources comprise nine 1774x887 landscapes and seven 1254-square images. All generated RGB downsampled into nine 1024x512 and seven 1024-square full-mip DXT1 replacements. Original and compiled top-mip alpha are fully opaque. No native 4K artwork claim.
- Reviewed four original/compiled comparison sheets and four repeat sheets. Paving divisions, bronze alternating rivets, stepped grooves, angular stripe, center seam, wood grain/joins and panel trim remain recognizable at original-relative positions. Relief and fine wear are stronger on several surfaces; mapped appearance and lighting still require scene validation.
- Accepted full-canvas luminance changes range -10.48% brown oxidation to +2.43% trimmed metal. Edge metrics were reviewed alongside repeated images; some originals already contain deliberate borders and directional shading, so numerical edge equality alone is not a seamlessness criterion. No scene adjacency claim from repeat sheets.
- All 511 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/budget checks. 6257 keys need art and 44 prior drafts remain held. Queue: 5781 visual triage, four batch candidates, 143 individual and 329 specialist. Full category, scene and embedded/procedural coverage remains unfinished.
- No game launches, source-tool or runtime changes. Existing compiler tests retained; artwork-only batch checked through native DDS and offline visual audits. Mac artwork scene validation pending. One active private pack, no duplicate game, pack, ZIP or staging; personal saves/settings untouched.
- Storage at audit: pack 621,404,635 bytes (592.62 MiB) including metadata / 6 GiB; adds 8,048,238 bytes. Texture workspace 2,562,315,285 allocated/logical bytes (2.39 GiB); temporary originals/review/previews 224,627,579 bytes (214.22 MiB) / 256 MiB; free disk 130,777,354,240 bytes. Texture totals are not whole-project totals.
- Private evidence `review/hybrid-88-*`: triage and classifications, exact prompts/selection, compiled/repeat comparisons, functional diagnostics, native audit and acceptance. Source art `authored/*-hybrid-v1.png` with canonical per-key provenance. Worklist, hybrid plan, inventory and progress reconciled; only progress documentation tracked.

### Texture hybrid batch 89: fifteen environment replacements (issue #74)

- Branch `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `e3bc569`, Windows x64. Fifteen additions, 526 installed/offline-approved total; runtime scenes remain unverified.
- Inspected 32 additional opaque originals from verified catalog payloads; classification reaches 1,049 keys. Selected four previously queued materials and twelve newly inspected surfaces. Reflection, scene-image, controls, lettering and sensitive equipment remain separately queued.
- Fifteen built-in imagegen drafts accepted offline: corrugated metal, dark center-striped wall, olive trim, paired shaded panels, inset black marble, brushed metal atlas, brickwork, split masonry, dark jagged rock, gold-trimmed marble, pale rock, weathered white plaster, rusty blue-gray metal, fine ground cover and ochre plaster with broken base.
- Held 13e0cd434ca21cf4: narrow original vertical divisions are weakened or omitted, including left-of-center strips. Major dark seams alone do not preserve the original pattern. Original remains active. Candidate DDS and metadata removed after exact resolved-path/hash checks; source, exact prompt and review retained for correction.
- Sixteen generated sources: fourteen 1254-square and two 1774x887. Fifteen accepted full-mip DXT1 outputs: eleven 1024-square, two 1024x512, one 512-square masonry and one 2048-square rock. Rock upscaled from 1254-square; remaining RGB downsampled. All original/compiled top-mip alpha opaque. No native 4K source claim.
- Reviewed four compiled comparison sheets, four repeat sheets and enlarged vertical-strip detail. Column luminance profiles support the held seam decision but also detect grain and corrosion; extrema are not exact geometry counts. Accepted corrugation cadence, baked dark bands, trim positions, marble insets, masonry transitions and broad rock/ground patterns reviewed offline.
- Accepted luminance changes range -9.75% shaded panels to +26.95% blue-gray metal. Inset black marble is +22.21% and has stronger edge relief; blue-gray metal is brighter while retaining wear placement. These material-lighting changes, repeating adjacency, rock relief and ground-cover scale remain explicit scene checks.
- All 526 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/budget checks. 6241 keys need art, 45 held drafts; queue 5749 visual triage, four batch candidates, 155 individual and 333 specialist. Full categories, scenes and embedded/procedural coverage remain unfinished.
- No game launches, source-tool or runtime changes. Existing compiler tests retained; offline native DDS checks and visual audits used for this art-only batch. Mac artwork scene validation pending. One active private pack, no duplicate game, pack, ZIP or staging; personal saves/settings untouched.
- Storage at audit: pack 632,772,769 bytes (603.46 MiB) including metadata / 6 GiB; adds 11,368,134 bytes. Texture workspace 2,639,910,022 allocated/logical bytes (2.46 GiB); temporary originals/review/previews 231,422,431 bytes (220.70 MiB) / 256 MiB; free disk 130,381,619,200 bytes. Texture totals are not whole-project totals.
- Private evidence `review/hybrid-89-*`: triage/classifications, exact prompts/selection, compiled/repeat and strip detail comparisons, seam profiles, functional diagnostics, retirement hashes, native audit and acceptance. Source art `authored/*-hybrid-v1.png`, canonical per-key provenance retained. Worklist, hybrid plan, inventory and progress reconciled; only progress documentation tracked.

### Texture hybrid batch 90: twelve material and machinery replacements (issue #74)

- Branch `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `aacd007`, Windows x64. Twelve additions; 538 installed/offline-approved total. Runtime scenes remain unverified.
- Four environment panels/molding and eight individually reviewed machinery/prop atlases accepted. No additional held drafts. Classification remains 1,049 keys.
- All twelve original/compiled comparison and repeat rows visually reviewed; three enlarged mechanical details inspected.
- Pipe classification/prompt said four incorrectly. Original shows five main horizontal highlight bands plus a lower partial band, retained by the compiled texture. Exact generation prompt retained unchanged; source image controls acceptance.
- Ladder rungs, circular assembly holes/rivets, gray equipment slots/eight circles/triangular windows, rib stacks, lamp inset and plaque relief reviewed at original-relative positions.
- Material detail and local highlights are stronger. Scene lighting, mapped UV appearance and adjacency remain pending. No launch.
- Possible sibling 122caee769fd7eb4 reuse deferred: low RGB difference alone is insufficient to establish matching UV/functional content. No replacement installed for this key.
- Twelve built-in imagegen sources: eight 1254-square and four 1774x887; downsampled RGB into five 1024-square, three 512-square and four 1024x512 full-mip DXT1 replacements. Original/compiled alpha opaque. No native 4K source claim.
- Accepted luminance changes range -10.60% ladder to +9.88% gray equipment atlas. Offline approval does not establish scene appearance.
- All 538 DDS pass native format and worklist hash/dimensions/aspect/full-mip/budget checks; 6229 need art, 45 held corrections. Full category, scene and embedded/procedural coverage remains unfinished.
- Storage at audit: pack 638,197,263 bytes (608.63 MiB) including metadata / 6 GiB; adds 5,424,494 bytes. Texture workspace 2,685,965,594 allocated bytes (2.50 GiB); temporary originals/review/previews 234,132,617 bytes (223.29 MiB) / 256 MiB. These are texture totals, not whole-project totals.
- No launches, runtime/tool changes, duplicate game, pack, ZIP or staging. Saves/settings untouched. Existing compiler tests retained; art-only work validated offline. Private evidence `review/hybrid-90-*`, source art and exact prompts retained. Worklist, inventory, hybrid plan and progress reconciled; only progress documentation tracked.

### Texture hybrid batch 91: sixteen environment replacements (issue #74)

- Branch `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `d7d7873`, Windows x64. Sixteen additions; 554 installed/offline-approved total, scenes pending. No new held drafts.
- Inspected 32 additional opaque catalog-verified originals; classifications reach 1081. Selected sixteen environment materials; mechanical grids, window/door frames, vents, equipment, wooden stock, decorative trim, landscape, uncertain lattice and reflection remain separately queued.
- Sixteen built-in imagegen sources accepted offline: pale marble, chipped plaster, gray molding, plaster with dark base, rusty blue-gray metal, gold-trimmed black marble, split mossy stonework, dark olive metal, horizontally coursed masonry, stained concrete, dark brown weathered wall, blue-banded metal, red bricks, ochre-over-pale plaster, layered stone strip and muted mottled wall.
- Reviewed four original/compiled comparison sheets and four repeat sheets. Major veins, chipped boundaries, molding grooves, gold-strip positions, stone divisions, masonry courses, central concrete pit, blue band and brick layout retain original-relative placement. No proof of mapped scene adjacency from repeated images alone.
- Luminance changes range -16.36% olive metal to +30.54% black marble. Black marble veins/gold are brighter, with stronger relief; dark brown wall +13.30%. These, fine-grain scale, tiling and surface shading remain explicit scene checks.
- Sixteen generated sources: eleven 1254-square and five 1774x887. All RGB downsampled into nine 1024-square, two 512-square and five 1024x512 full-mip DXT1 replacements. Original/compiled top-mip alpha opaque. No native 4K source claim.
- No game launches or runtime/tool changes. One private active pack; no duplicate game, pack, ZIP or staging. Personal saves/settings untouched. Full category, scene and embedded/procedural coverage remains unfinished.
- All 554 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/budget checks. Counts {'needs_art': 6213, 'installed': 554, 'draft_requires_review': 45}; routing {'batch_candidate': 3, 'correction': 45, 'individual_review': 157, 'installed': 554, 'specialist_review': 336, 'visual_triage': 5717}. Existing compiler tests retained for this art-only batch.
- Storage at audit: pack 646,595,009 bytes (616.64 MiB) including metadata / 6 GiB; adds 8,397,746 bytes. Texture workspace 2,761,407,337 allocated/logical bytes (2.57 GiB); temporary originals/review/previews 240,271,846 bytes (229.14 MiB) / 256 MiB. These are texture totals, not whole-project totals.
- Private evidence `review/hybrid-91-*`, authored sources and exact per-key prompts retained. Worklist, hybrid plan, inventory and progress reconciled; only progress documentation tracked.

### Texture hybrid batch 92: eight material and prop replacements (issue #74)

- Branch `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `361e405`, Windows x64. Eight additions; 562 installed/offline-approved total. Runtime scenes pending; no new held drafts.
- Three queued materials and five individually reviewed props accepted: rusty corrugation, pale streaked metal, mossy cracked rock, curved mechanical frame, riveted plate, shuttered window facade, blue door pair and gray equipment atlas. Classification remains 1081 keys.
- Eight built-in imagegen sources: six 1254-square and two 1774x887; all RGB downsampled into five 1024-square, one 512-square and two 1024x512 full-mip DXT1 replacements. Original/compiled top-mip alpha opaque; source sizes/resampling recorded. No native 4K source claim.
- Reviewed two enlarged original sheets, two compiled comparisons, two repeat sheets and targeted window/frame/equipment details. Corrugation cadence, rock cracks, bright bent pipes, frame rivet row, plate perimeter, window cross-mullions/slats, door hinges and equipment vents/slots/perforations retain original-relative placement.
- Equipment stamp 80171 visually matches the original after normalized-coordinate enlargement. First fixed-coordinate equipment crop targeted the wrong region because original is 512-square; superseded by hybrid-92-equipment-panel.jpg and hybrid-92-equipment-stamp.jpg. Full-size catalog verification passed before compilation.
- Accepted luminance changes range -8.74% riveted plate to +7.53% equipment atlas. Materials have stronger fine relief and highlights; mapped appearance, tiling adjacency, lighting and memory remain runtime checks.
- No launches or source-tool/runtime changes. Existing compiler tests retained for this art-only batch. One private active pack; no duplicate game, pack, ZIP or staging. Personal saves/settings untouched. Full categories, scenes and embedded/procedural coverage remain unfinished.
- All 562 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/budget checks. Remaining {'needs_art': 6205, 'installed': 562, 'draft_requires_review': 45}; queue {'correction': 45, 'individual_review': 152, 'installed': 562, 'specialist_review': 336, 'visual_triage': 5717}.
- Storage at audit: pack 650,968,643 bytes (620.81 MiB) including metadata / 6 GiB; adds 4,373,634 bytes. Texture workspace 2,797,037,332 allocated/logical bytes (2.60 GiB); temporary originals/review/previews 243,790,935 bytes (232.50 MiB) / 256 MiB. These are texture totals, not whole-project totals.
- Private evidence `review/hybrid-92-*`, authored sources and exact per-key prompts retained. Worklist, hybrid plan, inventory and progress reconciled; only progress documentation tracked.

### Texture hybrid batch 93: sixteen material and panel replacements (issue #74)

- Branch `74-enhancement-gfx-overhaul-to-modernize-textures`, source base `7b0b2b7`, Windows x64. Sixteen additions; 578 installed/offline-approved total. Runtime scenes pending; no new held drafts.
- Inspected 32 additional opaque originals verified against catalog payloads; classifications reach 1113. Thirteen materials and three individually reviewed panels accepted. Controls, facial atlases, reflection/utility images, roundel, mechanisms and ambiguous effects remain separately queued.
- Accepted dark red material, green-gray concrete, chipped green boards, ochre/blue-gray plaster, green-over-white peeling wall, brushed framed plate, dark recessed panel strip, stone blocks, pale chipped metal, mossy stonework, floor tiles, blue grooved metal, green-brown stained wall, vented pale wall, pale mottled plaster and brown-gray material.
- Sixteen built-in imagegen sources: thirteen 1254-square and three 1774x887. All RGB downsampled into twelve 1024-square, one 512-square and three 1024x512 full-mip DXT1 replacements. Original/compiled top-mip alpha opaque. No native 4K source claim.
- Reviewed four original/compiled comparison sheets, four repeat sheets and enlarged board-joint, vent and grooved-metal details. Original-relative board joints/fasteners, frame hardware, recessed panels, material boundaries, stone/tile grid and vent slats remain recognizable. Repeated images alone do not prove scene adjacency.
- Luminance changes range -16.61% dark red material to +8.15% ochre plaster. Fine relief and local highlights are stronger; brightness, material scale, tiling and mapped appearance remain scene-validation items.
- No launches or source-tool/runtime changes. Existing compiler tests retained for this art-only batch. One private active pack; no duplicate game, pack, ZIP or staging. Personal saves/settings untouched. Full categories, scenes and embedded/procedural coverage remain unfinished.
- All 578 retained DDS pass native format and worklist hash/dimensions/aspect/full-mip/budget checks. Remaining {'needs_art': 6189, 'installed': 578, 'draft_requires_review': 45}; queue {'correction': 45, 'individual_review': 160, 'installed': 578, 'specialist_review': 344, 'visual_triage': 5685}.
- Storage at audit: pack 660,589,723 bytes (629.99 MiB) including metadata / 6 GiB; adds 9,621,080 bytes. Texture workspace 2,873,984,680 allocated/logical bytes (2.68 GiB); temporary originals/review/previews 251,329,066 bytes (239.69 MiB) / 256 MiB. Reclaim verified reproducible export headroom before the next batch. These are texture totals, not whole-project totals.
- Private evidence `review/hybrid-93-*`, authored sources and exact per-key prompts retained. Worklist, hybrid plan, inventory and progress reconciled; only progress documentation tracked.


### Texture issue #74: hybrid batch 94, props and focused corrections

- Six replacements accepted offline: four new individual props and two repaired drafts. Total **584 / 6,812** supported keys; **6,185** need artwork and **43** held drafts remain. Classification remains 1,113 provisional reviews. Full overhaul is unfinished.
- Accepted metallic inset, six-cap mechanism, machine hatch, olive roundel panel, restored narrow-strip surface and repaired indicator atlas. Two control atlases remain held for altered tiny port/display details; original game textures stay active for them.
- Eight built-in imagegen sources (six 1254-square, two 1774x887); six retained resampled DXT1 full-mip outputs (four 1024-square, one 512-square, one 1024x512). No native 4K source claim. Two comparison sheets, two repeat sheets, four correction details and explicit indicator diagnostics reviewed; all 584 DDS passed native/worklist audits. Scene appearance remains unverified.
- Retired 240 reproducible reference PNGs, **32.01 MiB**, with catalog/hash/pixel recovery evidence and exact-path checks. Authored sources/provenance, working game, saves/settings preserved. Pack **663,913,649 bytes (633.16 MiB / 6144 MiB)**; texture workspace measured **2,871,182,423 bytes (2.67 GiB)** logical/allocated; temporary reference/review **210.84 MiB / 256 MiB**. No duplicate game, ZIP or staging.
- Private evidence: `private/texture-remaster/review/hybrid-94-*`, per-source prompts and `progress.json`. All artwork remains ignored/local. No runtime/tool changes, no game launches; previous compiler checks retained. Continue on `74-enhancement-gfx-overhaul-to-modernize-textures`, preserving concurrent root-checkout work. Mac/scene validation remains pending.


### Texture issue #74: hybrid batch 95, original-alpha materials

- **587 / 6,812** supported replacements installed and approved offline. Added leaded glass, blue-gray window grid and chain-link mesh; five other drafts held for brightness or RGB edge defects. **6,177** need art and **48** drafts need correction. Inspected 32 more RGB/alpha originals; provisional classification reaches **1,145**. Full overhaul remains unfinished.
- Eight built-in imagegen sources: seven 1254-square, one 1774x887. Three retained outputs use existing lossless RGBA8 (two 512-square, one 1024-square) after DXT5 showed significant opacity quantization in small mips. Every pixel in all **31 retained mip levels** exactly matches expected RGBA with verified original alpha. These are resampled assets, not native 4K art.
- Comparison, repeat, alpha/composite and edge reviews complete for retained files; original-relative window divisions and fence intersections preserved. Five held drafts have no active DDS. Their artwork/provenance remains available; original game textures remain active. Alpha identity alone does not prove RGB edge alignment. Scene appearance, lighting and transparency remain unverified.
- All **587** retained DDS pass native/worklist audits. Pack **672,305,297 bytes (641.16 MiB / 6144 MiB)**; texture workspace measured **2,911,074,240 bytes (2.71 GiB)** logical/allocated; temporary exports/reviews **218.57 MiB / 256 MiB**. Source art is included in workspace usage. No duplicate game/pack/ZIP/staging; saves/settings untouched.
- Private evidence: `private/texture-remaster/review/hybrid-95-*`, exact per-key prompts and progress. Retired initial DXT5 files by exact parent/hash checks. No runtime/tool change and no launches; existing tests retained. Continue on issue #74 while preserving concurrent source work. Mac/scene validation remains pending.


### Texture issue #74: hybrid batch 96, materials and panels

- **602 / 6,812** supported replacements installed and approved offline; **15** added, one access-panel draft held for four added fasteners. **6,161** need artwork and **49** drafts remain held. Inspected 32 more originals, bringing provisional classification to **1,177**. Full overhaul remains unfinished.
- Accepted stone, brick, plaster, timber door, metal panel/trim, rock, vented panel, concrete and road/verge materials. Sixteen built-in imagegen sources: ten 1254-square and six 1774x887. Retained fifteen full-mip DXT1 outputs: seven 1024-square, one 512-square, six 1024x512 and one 2048-square wall. The 2048 wall is upsampled from 1254-square; no native 4K source claim.
- Four comparisons, four repeat sheets and three enlarged detail sheets reviewed; original alpha stays opaque. All **602** retained DDS pass native/worklist checks. Accepted mean luminance changes **-14.30% to +7.54%**; scene lighting, tiling, material scale and mapped appearance remain pending. Existing bright borders on pale plaster retained and flagged for later scene-use review.
- Pack **682,275,373 bytes (650.67 MiB / 6144 MiB)**. Texture workspace measured **2,990,408,418 bytes (2.79 GiB)** logical/allocated, including authored sources. Temporary exports/reviews **225.85 MiB / 256 MiB**. No duplicate game/pack/ZIP/staging. Only held DDS/metadata retired after exact parent/hash checks; art/provenance and saves/settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-96-*`, prompts and progress. No game launches or runtime/tool changes; previous compiler tests retained. Work stays on #74; concurrent source work preserved. Mac/scene validation pending.


## Texture overhaul issue #74: hybrid batch 97

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `a327327`, using the isolated source checkout and one ignored private pack. Inspected 32 more opaque originals; 1,209 provisionally classified keys.
- Generated and accepted sixteen material/panel replacements with built-in image generation. Nine 1254-square and seven 1774x887 authored sources downsampled to nine 1024-square and seven 1024x512 full-mip DXT1 files; no native 4K claim.
- Reviewed four comparisons, four repeat sheets and enlarged vent detail. Preserved major UV divisions, grid/course patterns, trim and joints. Mean luminance changes -12.45% to +6.37%; local contrast/material scale and scene adjacency remain runtime checks.
- All 618 installed/offline-approved replacements pass native DDS and worklist validation. Still 6,145 need art and 49 drafts need correction; the full overhaul remains unfinished. No new game launches, runtime edits or shared-tool changes. Playtesting stays deferred until the near-completion gate.
- Pack including metadata: 691,022,687 bytes (659.01 MiB), below 6 GiB. Texture workspace: 3,066,077,378 logical and 3,066,077,378 allocated bytes. Temporary originals/reviews: 243,245,392 bytes (231.98 MiB), below 256 MiB. Free disk at audit: 127,800,561,664 bytes. No duplicate game, ZIP or staging copy created.
- Private evidence: `review/hybrid-97-acceptance.json`, `hybrid-97-functional-audit.json`, `hybrid-97-native-audit.txt`, `hybrid-97-selection.json` and exact authored prompt records. Artwork remains local and ignored; this commit records progress only.


## Texture overhaul issue #74: hybrid batch 98

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `1fbe154`. Inspected 32 more catalog-verified opaque originals; 1,241 provisionally classified keys. Faces, text, reflection-like and ambiguous bright textures remain separately queued.
- Accepted sixteen material replacements: plaster/concrete, olive paint, blue barrel, molding, rusty sheet, streaked wall, ground, brick/base wall, mottled surface, tiles, timber, fractured rock and mossy stone wall. Thirteen 1254-square and three 1774x887 built-in imagegen sources downsampled to eleven 1024-square, two 512-square and three 1024x512 full-mip DXT1 files. No native 4K claim.
- Four comparisons and four repeat sheets reviewed for original-relative joints, ribs, fasteners, courses, grid, trim and shading. Mean luminance changes -15.86% to +7.01%; ground scale, local relief, lighting and existing bright wall borders remain scene checks.
- All 634 installed/offline-approved replacements pass native DDS and worklist checks. Still 6,129 need art and 49 held drafts need correction; full overhaul unfinished. No game launches or shared-tool/runtime changes. Near-completion testing gate retained.
- Pack: 700,119,509 bytes (667.69 MiB / 6144 MiB). Texture workspace: 3,131,448,476 logical/allocated bytes (2.92 GiB). Temporary exports/reviews: 236,384,605 bytes (225.43 MiB / 256 MiB). Free disk at audit: 127,412,867,072 bytes.
- Reclaimed 13,965,689 bytes from 140 old decoded opaque reference PNGs after catalog payload, exact pixel, resolved-parent and SHA256 verification. Excluded alpha-source dependencies and recent batches 90-98. Kept recovery recipes, authored art, prompts, reviews, game files, saves and settings. No duplicate game/pack/ZIP/staging.
- Private evidence under `private/texture-remaster/review/hybrid-98-*`, including selection, acceptance, functional/native audits, reference recovery manifest and exact authored prompt records. This commit records progress; artwork stays local and ignored.


## Texture overhaul issue #74: hybrid batch 99

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `66328a8`. Inspected 32 catalog-verified opaque originals; 1,273 provisionally classified keys. Faces, labels, previews, effects and ambiguous materials remain queued.
- Accepted sixteen material/prop replacements including masonry, plaster, rubble, ground, alternating paving, braced crate, double metal panel and marble trim. Eleven 1254-square and five 1774x887 built-in imagegen sources downsampled to eight 1024-square, three 512-square and five 1024x512 full-mip DXT1 files. No native 4K claim.
- Four comparisons, four repeat sheets and three enlarged crate/panel/marble detail sheets reviewed. Original-relative divisions, courses, openings, brace, corner fittings, latch and molding retained. Mean luminance changes -9.65% to +6.73%; local relief, lighting, material scale and scene adjacency remain runtime checks.
- All 650 installed/offline-approved DDS pass native and worklist validation. Still 6,113 need art and 49 drafts need correction; full overhaul unfinished. No game launches, runtime changes or shared-tool edits. Near-completion testing gate retained.
- Pack: 707,992,997 bytes (675.19 MiB / 6144 MiB). Texture workspace: 3,206,699,870 logical/allocated bytes (2.99 GiB). Temporary exports/reviews: 243,571,476 bytes (232.29 MiB / 256 MiB). Free disk at audit: 128,358,879,232 bytes. No duplicate game/pack/ZIP/staging. Saves and settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-99-*`, exact prompts, authored sources and progress. Artwork remains ignored/local; this commit records progress only.


## Texture overhaul issue #74: hybrid batch 100

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `7b6f0b1`. Selected eight previously classified originals: four material candidates and four individual props. Classification remains 1,273 keys.
- Accepted wainscoting, plaster, masonry strip, layered stone wall, shuttered windows, wooden hatch, ornate relief and metal door. Six 1254-square and two 1774x887 built-in imagegen sources downsampled to five 1024-square, one 512-square and two 1024x512 full-mip DXT1 files. No native 4K claim.
- Two comparisons, two repeat sheets and four enlarged details reviewed. Major divisions and functional placements retained; studs and relief are more pronounced. Mean luminance changes -14.14% to +5.89%. Scene mapping, lighting, material scale and adjacency remain unverified.
- All 658 installed/offline-approved replacements pass native DDS and worklist checks. Still 6,105 need art and 49 drafts need correction; full overhaul unfinished. Zero new game launches, runtime changes or shared-tool edits. Playtesting remains deferred to the near-completion gate.
- Pack: 712,366,655 bytes (679.37 MiB / 6144 MiB). Texture workspace at worklist audit: 3,241,918,728 logical/allocated bytes (3.02 GiB). Temporary exports/reviews: 246,454,153 bytes (235.04 MiB / 256 MiB). Free disk at audit: 128,133,189,632 bytes. No duplicate game/pack/ZIP/staging; saves and settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-100-*`, exact prompts, authored art and progress. Artwork remains local and ignored; this commit records progress only.


## Texture overhaul issue #74: hybrid batch 101

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `3b85fcd`. Inspected 32 catalog-verified opaque originals; 1,305 provisionally classified keys. Faces, labels, equipment and ambiguous effects remain separately queued.
- Accepted sixteen material/prop replacements including stained walls, recessed panels, tartan, molding, riveted plates, gravel, plaster, masonry, brick/stone patch and wood end grain. Eleven 1254-square and five 1774x887 built-in imagegen sources produce seven 1024-square, three 512-square, one 2048-square and five 1024x512 full-mip DXT1 files. The 2048-square concrete panel is upsampled from 1254-square; no native 2K/4K claim.
- Four comparisons, four repeat sheets and four enlarged details reviewed. Main divisions, folds, stripe, molding, rivet rows, patch boundary and ring focus retained. Mean luminance changes -10.95% to +13.67%; local relief, exact hardware spacing, scene lighting, scale and adjacency remain scene checks.
- All 674 installed/offline-approved replacements pass native DDS and worklist checks. Still 6,089 need art and 49 drafts need correction; full overhaul unfinished. Zero game launches, runtime changes or shared-tool edits. Near-completion testing gate retained.
- Pack: 722,337,312 bytes (688.87 MiB / 6144 MiB). Texture workspace at audit: 3,317,984,772 logical/allocated bytes (3.09 GiB). Temporary exports/reviews: 253,204,287 bytes (241.47 MiB / 256 MiB). Free disk at audit: 127,513,735,168 bytes. No duplicate game/pack/ZIP/staging; saves and settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-101-*`, exact prompts, authored sources and progress. Corrected one mistyped identifier before successful generation; all 32 triage identifiers verified against the catalog. Artwork remains local and ignored; this commit records progress only.


## Texture overhaul issue #74: hybrid batch 102

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `a40d2e9`. Selected eight previously classified individual props/weapon surfaces; verified original payload hashes, exact reference pixels and opaque alpha. Classification remains 1,305 keys.
- Accepted broken window, gold/silver atlas, olive door, aircraft markings, faceted teal panel, crate/cabinet atlas, mechanical panel and wooden weapon grip. Seven 1254-square and one 1774x887 built-in imagegen sources downsampled to four 1024-square, three 512-square and one 1024x512 full-mip DXT1 files. No native 4K claim.
- Two comparisons, two repeat sheets and eight enlarged individual details reviewed. Main silhouettes, grille rows, hinges/knob, roundel/tricolor, angled facets, atlas islands/braces, insets and grip fittings retained. Mean luminance changes -9.69% to +1.21%; model UV mapping, local relief, checkering scale, markings and scene lighting remain runtime checks.
- All 682 installed/offline-approved replacements pass native DDS and worklist checks. Still 6,081 need art and 49 drafts need correction; full overhaul unfinished. Zero game launches, runtime changes or shared-tool edits. Near-completion testing gate retained.
- Pack: 726,011,912 bytes (692.38 MiB / 6144 MiB). Texture workspace at audit: 3,341,796,029 logical/allocated bytes (3.11 GiB). Temporary exports/reviews: 248,756,093 bytes (237.23 MiB / 256 MiB). Free disk at audit: 127,328,124,928 bytes.
- Reclaimed 7,835,754 bytes from 180 old opaque reference PNGs after payload, decoded-pixel, resolved-parent and SHA256 checks. Excluded alpha dependencies and recent batches 90-101; recovery recipes and review evidence retained. No duplicate game/pack/ZIP/staging; saves, settings, source artwork and original game data preserved.
- Private evidence: `private/texture-remaster/review/hybrid-102-*`, exact prompts, authored sources, recovery manifest and progress. Artwork remains local and ignored; this commit records progress only.


## Texture overhaul issue #74: hybrid batch 103

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `15705eb`. Retried eight held drafts from verified originals; six accepted and two still held. Classification remains 1,305 keys.
- Accepted dark riveted atlas, rusty sheet, four teal panels and three brushed-metal strip variants. Dark atlas brightness now +4.72%; rust-sheet top/bottom edge MAE improves to 7.45 (original 4.33, previous draft 25.99). Teal panel now has the original four corner marks/four hinges; its -18.63% mean brightness remains a scene-review item.
- Two comparisons, two repeat sheets, eight enlarged details and original-fitting sheet reviewed. Three strip-fitting run endpoints differ by at most one original pixel, supported by manual detail review. Reflections/top edges are stronger; scene mapping, lighting, scale and adjacency remain unverified.
- Four 1254-square and four 1774x887 built-in imagegen sources; six retained full-mip DXT1 outputs comprise two 1024-square, one 512-square and three 1024x512. No native 4K claim. Prior artwork and prompts retained.
- Held pale arrow panel still loses tiny marking strokes and changes rails; blue sheet retains bright repeat perimeter (edge MAE 25.27 versus original 1.08). Their DDS/metadata files were retired after resolved-parent and SHA256 checks; art and review evidence retained.
- All 688 installed/offline-approved replacements pass native DDS and worklist checks. Still 6,081 need art and 43 drafts need correction; full overhaul unfinished. Zero game launches, runtime changes or shared-tool edits. Near-completion testing gate retained.
- Pack: 728,636,800 bytes (694.88 MiB / 6144 MiB). Texture workspace at audit: 3,372,430,551 logical/allocated bytes (3.14 GiB). Temporary exports/reviews: 251,880,394 bytes (240.21 MiB / 256 MiB). Free disk at audit: 126,942,121,984 bytes. No duplicate game/pack/ZIP/staging; saves and settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-103-*`, exact prompts, authored corrections, fitting audit and retirement record. Artwork remains local and ignored; this commit records progress only.


## Texture overhaul issue #74: hybrid batch 104

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `55d6f2d`. Inspected 32 catalog-verified opaque originals; 1,337 provisionally classified keys. Screens, faces, effects, reflections, weapons and ambiguous surfaces remain separately queued.
- Accepted sixteen replacements: ten environmental materials and six individual props, including concrete/walls, gridded windows, riveted beam/panel, metal fittings, masonry, tread, brick/perforated strips and industrial vent. Twelve 1254-square and four 1774x887 built-in imagegen sources produce eleven 1024-square, one 512-square and four 1024x512 full-mip DXT1 files. All downsampled; no native 4K claim.
- Four comparisons, four repeat sheets and nine enlarged details reviewed. Main divisions and hardware/pattern counts retained; exact texel or spacing equivalence is not claimed. Concrete perimeter exists in the original. Mean luminance changes -14.00% to +8.15%; windows -13.77% and tread -12.63%. Stronger surface relief, darker tones, scene lighting, scale and adjacency remain scene checks.
- All 704 installed/offline-approved replacements pass native DDS and worklist checks. Still 6,065 need art and 43 drafts need correction; discovery/category coverage and full overhaul remain unfinished. Zero game launches, runtime changes or shared-tool edits. Near-completion playtesting gate retained.
- Pack: 737,908,406 bytes (703.72 MiB / 6144 MiB). Texture workspace at audit: 3,446,795,618 logical/allocated bytes (3.21 GiB). Temporary exports/reviews: 259,808,559 bytes (247.77 MiB / 256 MiB). Free disk at audit: 126,097,854,464 bytes. Inspect/reclaim reproducible temporary references before the next batch approaches its cap. No duplicate game/pack/ZIP/staging; saves and settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-104-*`, exact prompts, authored sources and progress. Artwork remains local and ignored; this commit records progress only. Mac scene validation remains pending alongside Windows scene validation; no platform behavior changed.


## Texture overhaul issue #74: hybrid batch 105

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `ce8d08d`. Eight catalog-verified opaque originals from the individual queue inspected. Classification remains 1,337 keys; clarified radial-mechanism description to include faint vertical teeth visible in the original.
- Accepted seven replacements: ribbed lamp panel, olive fasteners, radial mechanism, pointed blue-silver panel, double metal door, stepped metal strips and faceted cap/cylinder atlas. One mechanical atlas held because small circular-emblem strokes/arc segmentation need focused correction/review; its DDS/metadata retired after exact path/hash checks, artwork retained.
- Five 1254-square and three 1774x887 built-in imagegen sources. Seven retained full-mip DXT1 outputs: three 1024-square, one 512-square and three 1024x512. All downsampled; no native 4K claim. Two comparisons, two repeat sheets, eight enlarged asset details and one complete emblem crop reviewed.
- Main layout, fastener arrangement, radial components, pointed recess/fitting, trim, stepped strips and quadrant divisions retained; exact texel/tooth-spacing equivalence not claimed. Accepted luminance changes -9.26% to +10.90%. Radial ridges/red field, relief and reflections are stronger; material scale, scene lighting, UV use and adjacency remain deferred checks.
- Retired 340 reproducible old opaque original PNGs, reclaiming 6,037,418 bytes after catalog-payload hash, exact decoded RGBA, resolved-parent and file-hash verification. Alpha dependencies and recent selections protected; recovery manifest, source artwork and comparison evidence retained.
- All 711 installed/offline-approved replacements pass native DDS and worklist checks. Still 6,057 need art and 44 drafts need correction; discovery/category coverage and full overhaul unfinished. Zero game launches or shared-tool/runtime changes; near-completion playtesting gate retained.
- Pack: 741,232,918 bytes (706.89 MiB / 6144 MiB). Texture workspace at audit: 3,469,836,277 logical/allocated bytes (3.23 GiB). Temporary exports/reviews: 256,824,828 bytes (244.93 MiB / 256 MiB). Free disk: 125,942,497,280 bytes. No duplicate game/pack/ZIP/staging; saves/settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-105-*`, exact prompts, authored sources, recovery manifest and progress. Artwork remains local/ignored; this commit records progress only. Mac and Windows scene validation remain pending; no platform behavior changed.


## Texture overhaul issue #74: hybrid batch 106

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `0a0c49a`. Inspected 32 catalog-verified opaque originals; provisional visual classification now covers 1,369 keys. Lettering, identity-sensitive atlases, interface imagery and uncertain surfaces remain separately queued.
- Accepted sixteen replacements: twelve environment materials and four individual panels. Includes stained walls/plaster, cobblestone, brushed metal, inset and ornate panels, slotted panels, rust, fastened concrete, wood boards and chipped orange paint.
- Seven 1254-square and nine 1774x887 built-in imagegen sources. Six 1024-square, one 512-square and nine 1024x512 full-mip DXT1 outputs. All downsampled; no native 4K claim. Four comparisons, four repeat sheets and six enlarged details reviewed.
- Main UV divisions, slots, wood joints, original corner fittings and six concrete fasteners retained. Ornamental motifs reinterpreted with stronger relief; exact motif pitch/texel equivalence not claimed. Luminance changes -10.42% to +16.65%; darker material contrast, near-black wall staining and stronger reflections/relief remain deferred scene checks.
- All 727 installed/offline-approved replacements pass native DDS and worklist checks. Still 6,041 need art and 44 drafts need correction; full overhaul and category/scene coverage remain unfinished. Zero game launches or shared-tool/runtime changes; near-completion playtesting gate retained.
- Pack: 748,756,914 bytes (714.07 MiB / 6144 MiB). Texture workspace at audit: 3,538,803,030 logical/allocated bytes (3.30 GiB). Temporary exports/reviews: 263,159,720 bytes (250.97 MiB / 256 MiB). Free disk: 125,261,328,384 bytes. Recycle redundant reproducible triage sheets before the next large batch. No duplicate game/pack/ZIP/staging; saves/settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-106-*`, exact prompts, authored sources and progress. Artwork remains local/ignored; this commit records progress only. Mac and Windows scene validation remain pending; no platform behavior changed.


## Texture overhaul issue #74: hybrid batch 107

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `17e9a2a`. Eight catalog-verified opaque originals inspected from existing queues; classification remains 1,369 keys.
- Seven accepted: silver plates, red fitting atlas, crossed-trim panel, pale rock, stacked metal strips, eroded brown wall and nested riveted straps. Blue-gray mechanical atlas held because two small insets became multi-slat vents; DDS/metadata retired after exact path/hash verification, artwork retained for correction.
- Seven 1254-square and one 1774x887 built-in imagegen sources. Seven retained full-mip DXT1 outputs: four 1024-square, two 512-square, one 1024x512. All downsampled; no native 4K claim. Two comparisons, two repeat sheets and eight enlarged details inspected.
- Main divisions, seams, four plate corners, fitting contours, rock clefts/patches, masonry strip and strap paths retained. Crossed trim retains three original horizontal lines despite imprecise prompt wording. Exact rivet spacing/texel equivalence not claimed. Luminance changes -12.64% rock to +21.97% masonry; warmer wall, darker rock clefts, brighter straps and stronger metal relief remain scene-review items.
- Retired 44 redundant original-only triage JPEGs before batch 90, reclaiming 6,351,651 bytes after verifying catalog/decoded source recovery and exact resolved-parent/file hashes. Recovery manifest, selection records, artwork, final comparisons/repeats and alpha sheets retained.
- All 734 installed/offline-approved replacements pass native DDS and worklist checks. Still 6,033 need art and 45 drafts need correction; full overhaul, discovery/category coverage and scene validation unfinished. Zero game launches or shared runtime/tool changes; near-completion playtesting gate retained.
- Pack: 752,256,180 bytes (717.41 MiB / 6144 MiB). Texture workspace at audit: 3,566,640,305 logical/allocated bytes (3.32 GiB). Temporary exports/reviews: 260,370,955 bytes (248.31 MiB / 256 MiB). Free disk: 125,270,286,336 bytes. No duplicate game/pack/ZIP/staging; saves/settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-107-*`, exact prompts, authored sources, recovery manifest and progress. Artwork remains local/ignored; commit records progress only. Mac/Windows scene validation pending; no platform behavior changed.


## Texture overhaul issue #74: hybrid batch 108

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `67a9d05`. Retried eight held drafts from verified originals; five accepted, three still held. Classification remains 1,369 keys.
- Accepted rotor atlas, fine tooth-grid atlas, gray-green tread, dark olive four-panel sheet and blue-gray panel atlas. Original rotor rim marks retained without prior invented circular fasteners; blue-gray insets retain simple rectangles/internal marks instead of vents. Olive-panel horizontal edge MAE 11.75 versus original 9.92; excessive right-edge darkening resolved offline.
- Two comparisons, two repeat sheets, eight enlarged details and one registered grip crop inspected. Grip dominant frequencies +/-13 horizontal and +/-19 vertical match original; tread peaks (+10,-4),(-10,+4),(+/-20,0) match original. Diagnostics support visual review, not exact texel or scene-mapping equivalence.
- Eight 1254-square built-in imagegen sources. Five retained full-mip DXT1 outputs: three 1024-square, one 512-square, one 2048-square. Grip output resampled upward from 1254-square; no native 2K/4K claim. Prior art and prompt history retained.
- Held cool reflection peak at row 35 versus original 38, brass row 34 versus original 39; both lower dark bands row 47 versus 52 at original 64-pixel scale. Stone edge MAE 32.45/22.51 versus original 15.74/14.21. All three remain incomplete; six DDS/metadata files retired after exact path/hash verification, artwork retained.
- Accepted luminance changes -5.98% to +1.80%. Stronger relief, tread shadows, edges and local contrast remain scene-review items. All 739 installed/offline-approved replacements pass native DDS and worklist checks. Still 6,033 need art and 40 drafts need correction; full overhaul/category/scene coverage unfinished. Zero game launches or shared runtime/tool changes.
- Pack: 757,327,163 bytes (722.24 MiB / 6144 MiB). Texture workspace at audit: 3,600,897,424 logical/allocated bytes (3.35 GiB). Temporary exports/reviews: 264,624,347 bytes (252.37 MiB / 256 MiB). Free disk: 125,084,114,944 bytes. Recycle redundant reproducible review files before next large batch. No duplicate game/pack/ZIP/staging; saves/settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-108-*`, exact prompts, authored corrections, gradient/pattern audits and progress. Artwork remains local/ignored; commit records progress only. Mac/Windows scene validation pending; no platform behavior changed.


## Texture overhaul issue #74: hybrid batch 109

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `a02d811`. Inspected 32 catalog-verified opaque originals; 1,401 keys provisionally classified. Labels, gauges, weapon atlases and ambiguous imagery remain separately queued.
- Fifteen materials/panels accepted offline, covering masonry, plaster, metal bands/covers, concrete trim, panel grid and wood insets. One olive trim held because its broad reflection weakened and mean luminance fell 22.96%; DDS/metadata retired after exact path/hash checks, art retained.
- Initial four generated outputs recovered from disk after interrupted tool response and visually matched to originals/prompts; no duplicate generation. Ten 1254-square and six 1774x887 built-in imagegen sources. Fifteen retained full-mip DXT1 outputs: seven 1024-square, two 512-square, five 1024x512 and one 2048-square. Pale grid resampled upward from 1254-square; no native 2K/4K claim.
- Four comparisons, four repeat sheets and six selected enlarged details reviewed. Main masonry courses, five vent slots, dentil row, wood insets/notches, grid lines/fasteners and bottom fitting retained. Exact texel equivalence not claimed. Accepted brightness changes -14.98% to +13.21%; stronger relief, mortar contrast, orange hue and grid mottling remain deferred scene checks.
- Retired 28 redundant triage JPEGs (5,672,882 bytes) and 96 reproducible opaque original PNGs (10,487,332 bytes), with verified catalog/decoded-RGBA recovery, resolved-parent and file-SHA256 checks. Recovery records, alpha dependencies, recent selections, source art and final comparison evidence retained.
- All 754 installed/offline-approved replacements pass native DDS and worklist checks. Still 6,017 need art and 41 drafts need correction; full overhaul, discovery/category coverage and scene validation unfinished. Zero game launches or shared runtime/tool changes; near-completion playtesting gate retained.
- Pack: 767,122,486 bytes (731.59 MiB / 6144 MiB). Texture workspace at audit: 3,660,276,668 logical/allocated bytes (3.41 GiB). Temporary exports/reviews: 257,315,983 bytes (245.40 MiB / 256 MiB). Free disk: 125,295,374,336 bytes. No duplicate game/pack/ZIP/staging; saves/settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-109-*`, exact prompts, authored art, recovery manifests and progress. Artwork remains local/ignored; commit records progress only. Mac/Windows scene validation pending; no platform behavior changed.


## Texture overhaul issue #74: hybrid batch 110

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `2e9bb1a`. Eight previously classified equipment/architectural panels individually reworked from catalog-verified opaque originals; all eight accepted offline. Classification remains 1,401 keys.
- Accepted teal hinged doors, rusty braced doors, illuminated olive cabinet, vented blue-gray equipment panel, angled olive panel, illuminated architectural frame, red mechanism atlas and gold/orange sleeve. Two comparison sheets, two repeat sheets, eight enlarged details and one registered vent crop inspected.
- Door fittings/bracing, paired green indicators, opaque white opening, four white ring segments and main atlas divisions retained. Vented equipment crop has matching 14/28 dominant frequency peaks, supporting pitch review without claiming exact texel or scene equivalence. Brightness changes -10.70% to +18.46%; stronger reflection/relief, rusty warmth, red saturation and material grain remain final scene-review items.
- Seven 1254-square and one 1774x887 built-in imagegen sources, downsampled to seven 1024-square and one 1024x512 full-mip DXT1 outputs. No native 4K claim. Exact prompts and artwork remain private; no duplicate game/pack/staging created.
- All 762 installed/offline-approved replacements pass native DDS and worklist checks. Corrected a BOM in eight prompt records before successful worklist audit. Still 6,009 need art and 41 drafts need correction; full overhaul/category/scene coverage unfinished. Zero game launches or shared runtime/tool changes; near-completion testing gate retained.
- Pack: 772,369,956 bytes (736.59 MiB / 6144 MiB). Texture workspace at audit: 3,694,663,928 logical/allocated bytes (3.44 GiB). Temporary exports/reviews: 261,379,767 bytes (249.27 MiB / 256 MiB). Free disk: 125,123,239,936 bytes. Recycle redundant reproducible review files before next large batch. Saves/settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-110-*`, authored sources/prompts and progress. Commit records progress only. Mac/Windows scene validation pending; no platform behavior changed.


## Texture overhaul issue #74: hybrid batch 111

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `6cc3758`. Inspected 32 catalog-verified opaque originals; 1,433 keys provisionally classified. Generated eleven environment materials and three individual panels; thirteen accepted offline, one held.
- Accepted tile grid, brick/base, gravel, circular grate, bordered marble, olive mottled surface, plaster, rusty paint, seamed concrete, dry grass, flaked plaster, pale masonry and four-inset silver panel. Brass louver/disc atlas held for +37.43% mean brightness and stronger reflection/relief; DDS/metadata retired after exact path/hash checks, artwork retained.
- Four comparison sheets, four repeat sheets and six selected enlarged details inspected. Main courses, seams, border, grate/grid, crack-patch positions and four purple insets retained; exact texel/scene equivalence not claimed. Accepted brightness changes -6.45% to +17.75%; material contrast, roughness and color changes remain deferred scene checks.
- Fourteen 1254-square built-in imagegen sources; thirteen downsampled full-mip DXT1 outputs, ten 1024-square and three 512-square. No native 4K claim. Exact prompts and artwork remain local/private.
- Retired 98 reproducible old opaque original PNGs (3,629,159 bytes), with catalog/decoded-RGBA recovery and all resolved-parent/file-SHA256 checks. Recovery records, alpha dependencies, recent references, artwork and final review evidence retained. No triage JPEGs matched the retirement range; zero removed.
- All 775 installed/offline-approved replacements pass native DDS and worklist checks. Still 5,995 need art and 42 drafts need correction; full overhaul/category/scene coverage unfinished. Zero game launches or shared runtime/tool changes; near-completion testing gate retained.
- Pack: 779,892,198 bytes (743.76 MiB / 6144 MiB). Texture workspace at audit: 3,762,244,913 logical/allocated bytes (3.50 GiB). Temporary exports/reviews: 266,383,504 bytes (254.04 MiB / 256 MiB). Free disk: 124,556,144,640 bytes. Recycle redundant reproducible review files before next large batch. No duplicate game/pack/ZIP/staging; saves/settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-111-*`, authored sources/prompts, recovery manifest and progress. Commit records progress only. Mac/Windows scene validation pending; no platform behavior changed.


## Texture overhaul issue #74: hybrid batch 112

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `198bb0c`. Eight held drafts retried from catalog-verified opaque originals; three accepted, five still held. Classification remains 1,433 keys.
- Accepted olive trim with restored broad reflection (+2.00% mean brightness), brass louver/disc with restrained shading (-6.87%, previously +37.43%), and green-screen equipment atlas with rectangular ports and rectilinear lower-right island (-3.51%). Main UV divisions retained; fine relief and scene appearance remain pending validation.
- Two comparison sheets, two repeat sheets and eight enlarged details inspected. At original 64-pixel scale, cool reflection peak row 33 versus 38, lower dark row 45 versus 52; brass peak row 31 versus 39, lower dark row 43 versus 52. Both remain held. Radial atlas still omits intermediate boundaries. Stone edge MAE 22.78/21.03 versus original 15.74/14.21; riveted plate 16.04/13.13 versus 0.00/1.08; both remain held for repeat corrections.
- Five 1254-square and three 1774x887 built-in imagegen sources. Three retained downsampled full-mip DXT1 outputs: one 1024-square, one 1024x512, one 512x256. No native 4K claim. Prior art and prompt history retained.
- Retired ten held DDS/metadata files after all exact resolved-parent/SHA256 checks. Retired 117 reproducible older opaque original PNGs (7,852,542 bytes; one selected reference regenerated from verified catalog afterward, net 7,803,824 bytes), with catalog/decoded-RGBA recovery verification and path/hash checks. Recovery records, alpha dependencies, recent references, artwork and final evidence retained.
- All 778 installed/offline-approved replacements pass native DDS and worklist checks. Still 5,995 need art and 39 drafts need correction; full overhaul/category/scene coverage unfinished. Zero game launches or shared runtime/tool changes; near-completion testing gate retained.
- Pack: 781,029,880 bytes (744.85 MiB / 6144 MiB). Texture workspace at audit: 3,779,370,018 logical/allocated bytes (3.52 GiB). Temporary exports/reviews: 261,459,586 bytes (249.35 MiB / 256 MiB). Free disk: 121,601,974,272 bytes. No duplicate game/pack/ZIP/staging; saves/settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-112-*`, authored corrections/prompts, recovery manifest and progress. Commit records progress only. Mac/Windows scene validation pending; no platform behavior changed.


## Texture overhaul issue #74: hybrid batch 113

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `3154cb3`. Individually edited eight catalog-verified opaque character atlases with built-in imagegen; six accepted offline and two held. Provisional classification remains 1,433 keys.
- Accepted two male face atlases, swept brown hair, tan ear/skin, three-button garment and mixed stubble/metal-strip atlas. Main texture islands, eye openings, eyebrows, unused gaps, buttons and strip divisions retained on visual comparison. Two comparison sheets, two repeat sheets and all eight enlarged details inspected, including complete portrait atlases. Exact texel equivalence and scene adjacency not established.
- Held compact hair/grid atlas for missing oval stem and changed grid spacing/rows; held complex facial atlas for ear-opening and eye/mouth drift. Four DDS/metadata files removed after exact resolved-parent/SHA256 checks. Original game textures remain active for held keys; generated art and prompts retained for correction.
- Sources: five 1774x887, two 887x1774 and one 1254-square. Six retained downsampled full-mip DXT1 outputs: four 1024x512 and two 512x1024. No native 4K claim. Accepted mean luminance changes -12.45% to +3.93%; pores, eyebrow/stubble sharpness, hair contrast, warmer skin and garment sheen remain scene-review items.
- All 784 installed/offline-approved replacements pass native DDS and worklist checks. Still 5,987 need art and 41 drafts need correction. Full overhaul/category/scene coverage unfinished; zero game launches. Near-completion testing gate retained. No shared runtime/tool or platform behavior changes; Windows/Mac scene validation pending.
- Pack: 783,130,492 bytes (746.85 MiB / 6144 MiB). Texture workspace at audit: 3,803,910,283 logical/allocated bytes (3.54 GiB). Temporary exports/reviews: 263,848,688 bytes (251.63 MiB / 256 MiB). Free disk: 121,157,996,544 bytes. Recycle reproducible temporary outputs before another large batch; no duplicate game/ZIP/staging, saves/settings preserved.
- Artwork, exact prompts, review evidence, retirement manifest and progress remain local under ignored `private/texture-remaster`; this commit records progress only.


## Texture overhaul issue #74: hybrid batch 114

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `731ad1d`. Inspected 32 catalog-verified opaque originals; 1,465 keys now provisionally classified. Generated twelve environment materials and four individual panels with built-in imagegen; fifteen accepted offline, one held.
- Accepted rubble, red brick, earthy rock, scraped plaster, framed wood, split stone, weathered plaster, horizontal stone trim, olive rusty trim, vertical pale trim, mottled stone, concrete shadow atlas, twin wood panels, four-bolt plate and brick/stone masonry. Grille panel held for changed fine line spacing; its two DDS/metadata files retired after exact parent/hash checks, art retained.
- Four comparison sheets, four repeat sheets and eight enlarged details inspected. Main seams, courses, trim, fasteners and shadow regions preserved on visual comparison. Brick profile retains dominant bin 14 and harmonics 28/29; grille profile diverges. Pattern metrics are supporting diagnostics, not proof of exact texel or scene equivalence. Grain/relief, edge contrast, warmer wood and brightness changes (-8.58% to +7.71%) remain scene-review items.
- Thirteen 1254-square and three 1774x887 source images; fifteen retained downsampled full-mip DXT1 files: ten 1024-square, two 512-square and three 1024x512. No native 4K claim. Artwork and exact prompts remain private.
- Retired 75 reproducible old opaque reference PNGs (5,660,999 bytes), with catalog/decoded-RGBA recovery records and all resolved-parent/file-SHA256 checks. Temporary output briefly reached 268,496,252 bytes, 60,796 above 256 MiB; production stopped and additional verified reference cleanup restored the cap before review continued. Final comparison evidence, source art, alpha dependencies, saves and settings retained.
- All 799 installed/offline-approved replacements pass native DDS/worklist checks. Still 5,971 need art and 42 drafts need correction; full overhaul/category/scene coverage unfinished. Zero game launches or shared runtime/tool changes; near-completion gate retained. Windows/Mac scene validation pending.
- Pack: 791,527,706 bytes (754.86 MiB / 6144 MiB). Texture workspace at audit: 3,873,265,716 logical/allocated bytes (3.61 GiB). Temporary exports/reviews: 266,155,312 bytes (253.83 MiB / 256 MiB). Free disk: 116,300,247,040 bytes. Recycle temporary references before another large batch; no duplicate game/pack/ZIP/staging.
- Private evidence: `private/texture-remaster/review/hybrid-114-*`, authored sources/prompts, recovery manifests and progress. This commit records progress only.


## Texture overhaul issue #74: hybrid batch 115

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `888756c`. Generated eight individual metal/wood assets from the catalog-verified existing queue with built-in imagegen; six accepted offline, two held. Classification remains 1,465 reviewed keys; no new coverage claimed from routing.
- Accepted silver panel trio with hazard stripe, repeating chevron tread, peeling double wood panels, wood wedge atlas, curved wood stock/diamond inset and brass/copper/steel mechanism atlas. Two dark equipment panels held for +53.69% and +64.38% mean-luminance changes; vent/rivet correspondence must be rechecked after correction. Four candidate DDS/metadata files retired after all resolved-parent/hash checks, sources retained.
- Two comparison sheets, two repeat sheets and eight full-aspect details inspected for panel slots, tread bands/bolts, seams, insets and atlas boundaries. Accepted luminance changes -2.95% to +20.78%; increased grain, highlights and silver-panel edge contrast remain scene-review items. Silver-panel left/right edge MAE 6.10 to 39.61 requires adjacency attention. Offline visual review is not exact texel equivalence or scene validation.
- Sources: four 1254-square, three 1774x887 and one 887x1774 images. Six full-mip opaque DXT1 replacements retained: three 1024x512 and one 1024-square downsampled, one 1024x2048 tread and one 2048-square mechanical atlas upsampled. No native 4K claim. Exact prompts, actual source sizes and resampling remain recorded privately.
- Recycled 40 redundant original-only contact sheets (5,931,591 bytes), after catalog/decoded-RGBA recovery verification and all resolved-parent/file-SHA256 checks. Final comparisons/repeats/details, authoritative original payloads, generated art and recovery records retained. Temporary outputs stayed below 256 MiB this batch.
- All 805 installed/offline-approved replacements pass native DDS/worklist checks. Remaining: 5,963 need art and 44 drafts need correction. Full category/discovery/scene coverage unfinished. Zero game launches or shared runtime/tool changes; 512 MiB residency cap, near-completion testing gate and personal saves/settings preserved. Windows/Mac scene validation remains pending.
- Pack: 797,473,093 bytes (760.53 MiB / 6144 MiB). Texture workspace at audit: 3,904,035,579 logical/allocated bytes (3.64 GiB). Temporary exports/reviews: 264,081,304 bytes (251.85 MiB / 256 MiB). Free disk: 116,123,303,936 bytes. Recycle reproducible temporary references before the next large batch; no duplicate game/pack/ZIP/staging.
- Private evidence: `private/texture-remaster/review/hybrid-115-*`, authored sources/prompts, recovery manifest and progress. This records progress only; full overhaul remains active.


## Texture overhaul issue #74: hybrid batch 116

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `981528f`. Inspected 32 catalog-verified opaque originals; 1,497 keys now provisionally classified. Eight were remaining eligible end-of-queue assets with at least three occurrences; 24 came from the two-occurrence queue. Alpha, small assets and specialist content remain in scope. Generated eight environment materials and six individual atlases with built-in imagegen; eleven accepted offline, three held.
- Accepted snowy slab, concrete, narrow brickwork, ornate trim, ocher earth, strapped wood, carved wall, red-striped paint, silver rib/hex component, brown equipment panel and four-opening window surround. Held mechanical atlas for extra tube joints, sci-fi panel for changed small plate markings, and pale panel for altered perforation geometry. Six held DDS/metadata files retired after exact parent/hash checks; art retained.
- Four comparisons, four repeat sheets, fourteen enlarged details and four diagnostic crops inspected. Main seams, slots, courses, ornamental bands and openings retained in accepted drafts. Sci-fi panel has three light bars and matching lower-grille dominant bin 30, but plate-marking drift remains. Pattern diagnostics are not exact UV or scene validation. Contrast, stronger grain/relief, warmer wood, red saturation and luminance changes (-11.01% to +16.95%) remain scene-review items.
- Ten 1254-square, three 1774x887 and one 887x1774 generated sources. Eleven downsampled full-mip opaque DXT1 replacements retained: eight 1024-square, two 1024x512 and one 512x1024. No native 4K claim. Actual source sizes, resampling and exact prompts saved privately.
- Recycled 55 redundant repeat-presentation sheets from 17 older fully accepted batches (23,548,098 bytes), after catalog/decoded-RGBA and DDS/source recovery verification plus all resolved-parent/file-SHA256 checks. All final comparisons, selected detail evidence and source art retained. Historical progress records annotate retired presentation references; recovery manifest records exact source hashes. Temporary outputs remained below 256 MiB.
- All 816 installed/offline-approved replacements pass native DDS/worklist checks. Remaining: 5,949 need art and 47 drafts need correction. Full category/discovery/scene coverage unfinished. Zero game launches or shared runtime/tool changes; near-completion testing gate, 512 MiB residency cap and personal saves/settings preserved. Windows/Mac scene validation pending.
- Pack: 804,120,391 bytes (766.87 MiB / 6144 MiB). Texture workspace at audit: 3,946,047,186 logical/allocated bytes (3.68 GiB). Temporary exports/reviews: 249,582,180 bytes (238.02 MiB / 256 MiB). Free disk: 115,728,343,040 bytes. No duplicate game/pack/ZIP/staging.
- Private evidence: `private/texture-remaster/review/hybrid-116-*`, authored sources/prompts, retirement/recovery manifest and progress. Full overhaul remains active.


## Texture overhaul issue #74: hybrid batch 117

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `7514afb`. Eight held drafts retried; four accepted offline and four still held. Classification remains 1,497 keys.
- Corrected two over-bright dark panels: blue-gray equipment panel now -4.68% mean luminance versus original (previous +53.69%); three-slot riveted panel -6.81% (previous +64.38%). Main vent/fixture/slot layouts retained. Mechanical atlas removes added bottom-tube joints, retaining the principal joint near 64% (-3.54% luminance). Sci-fi panel retains three lights and understated abstract plate marks instead of invented lettering (-7.46%). Stronger microdetail/edge contrast still require scene review.
- Two comparisons, two repeat sheets and eight enlarged details inspected. Four held drafts still fail fidelity: enlarged perforation holes; missing upper grille lines/changed lower cadence; thin oval stem restored but net grid thickness/registration altered; face atlas changes eye/lip/ear details and net grid. These do not count as complete.
- Six 1254-square and two 1774x887 built-in imagegen sources. Four retained downsampled full-mip DXT1 outputs: three 1024-square and one 1024x512. No native 4K claim. Prior artwork and prompt versions retained. Eight held candidate DDS/metadata files safely retired after all resolved-parent/SHA256 checks.
- All 820 installed/offline-approved replacements pass native DDS and worklist checks. Still 5,949 need art and 43 drafts need correction. Full overhaul/category/discovery/scene coverage remains unfinished. Zero game launches; near-completion testing gate retained. No shared runtime or tool changes; Mac and Windows scene validation pending.
- Pack: 806,569,365 bytes (769.20 MiB / 6144 MiB). Texture workspace at audit: 3,972,878,045 logical/allocated bytes (3.70 GiB). Temporary exports/reviews: 252,375,505 bytes (240.68 MiB / 256 MiB). Free disk: 115,334,217,728 bytes. One active pack; no duplicate extracts/ZIPs/staging. Saves/settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-117-*`, authored correction sources/prompts and progress. Commit records progress only; artwork stays local and ignored.


## Texture overhaul issue #74: hybrid batch 118

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `7ccd419`. Classified 32 catalog-verified opaque originals from the two-occurrence/min128 queue; 1,529 keys now classified. Alpha, smaller assets, specialist imagery and unknown assets remain in scope. Generated ten environment surfaces and two individual prop panels with built-in imagegen; ten accepted, two held.
- Accepted riveted quarter-ring/brickwork, six-board crate face, jagged rock, fluted pale wall, four-recess metal panel, four-hole concrete, cracked plaster, gray wood paneling, golden strapped wood and worn brickwork. Held ornate shutters for altered louver cadence/fan ornament, and blue-striped equipment door for turning a subdued painted mark into exposed metallic hardware. Four held DDS/metadata files retired after exact parent/hash checks; artwork retained.
- Three comparison sheets, three repeat sheets, twelve enlarged details and three diagnostic crops inspected. Main UV divisions, seams, bolts, holes and recesses retained on accepted comparisons. Noisy louver profile extrema do not establish exact counts. Fine grain/relief and scene adjacency remain unverified. Luminance shifts -13.85% to +24.45%; brighter brickwork/plaster and stronger rust/rock relief need scene checks. Fluted-wall edge MAE 6.06/8.33 to 17.50/15.51; concrete left/right 9.59 to 21.26: pending adjacency concerns, not seamlessness claims.
- Eleven 1254-square and one 1774x887 source images. Ten retained downsampled full-mip opaque DXT1 replacements: eight 1024-square, one 512-square and one 1024x512. No native 4K claim. Prompts, actual source sizes, source hashes and resampling recorded privately.
- Recycled 41 reproducible older opaque reference PNGs (2,251,840 bytes), verified against catalog payload/decoded RGBA and every resolved-parent/file-SHA256 before removal. Recent references and alpha dependencies protected; originals in game data, authored art and final evidence preserved. Temporary texture outputs remained below 256 MiB.
- All 830 installed/offline-approved replacements pass native DDS/worklist checks. Remaining: 5,937 need art and 45 drafts need correction. Full category/discovery/scene validation unfinished. Zero game launches or shared runtime/tool changes; 512 MiB residency cap, near-completion testing gate and personal saves/settings preserved. Windows/Mac scene validation pending.
- Pack: 812,691,793 bytes (775.04 MiB / 6144 MiB). Texture workspace at audit: 4,029,425,391 logical/allocated bytes (3.75 GiB). Temporary exports/reviews: 258,904,439 bytes (246.91 MiB / 256 MiB). Free disk: 115,049,431,040 bytes. One active pack; no duplicate game/ZIP/staging.
- Private evidence: `private/texture-remaster/review/hybrid-118-*`, authored sources/prompts, reference-recovery manifest and progress. Commit records progress only; full overhaul remains active.


## Texture overhaul issue #74: hybrid batch 119

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `e5baf5a`. Eight already-classified opaque originals verified against catalog payloads and decoded RGBA; seven individual panels and one environment door authored. Classification remains 1,529 keys.
- Seven accepted offline: multipart metal atlas, orange-striped twin-inset door, brass mechanical atlas, dark steel/red housing, double recessed wooden door, angled wood structural atlas and diagonal silver panel. Main UV regions, openings, recesses and hardware groups retained. Gray-blue geometric panel held because lower-left thin horizontal bands and rectangular shading patches were flattened; original-scale crop confirms the loss. Two held DDS/metadata files retired after all parent/hash checks; source and prompt retained.
- Two comparisons, two repeat sheets, eight enlarged details and one band crop inspected. Accepted mean luminance changes -14.18% to +16.56%; brass -0.04%, multipart metal +13.03%, orange door -4.68%, angled wood +5.46%, silver panel -4.82%. Dark exposure, stronger grain/edge contrast, brass highlights and atlas adjacency remain scene-validation items; repeat sheets do not prove actual scene mapping.
- Seven 1254-square and one 1774x887 built-in imagegen sources. Seven retained downsampled full-mip opaque DXT1 replacements: six 1024-square and one 1024x512. No native 4K claim. Exact prompts, source hashes and resampling saved privately. Hybrid-118 prompt-set quoting reconciled with the unchanged exact generation records.
- All 837 installed/offline-approved replacements pass native DDS/worklist checks. Still 5,929 need art and 46 drafts need correction. Full category/discovery/scene coverage unfinished. Zero game launches or shared runtime/tool changes; near-completion testing gate and 512 MiB residency cap retained. Windows/Mac scene validation pending.
- Pack: 817,239,639 bytes (779.38 MiB / 6144 MiB). Texture workspace at audit: 4,062,130,789 logical/allocated bytes (3.78 GiB). Temporary exports/reviews: 261,931,890 bytes (249.80 MiB / 256 MiB). Free disk: 114,943,606,784 bytes. Recycle reproducible review output before the next large batch. One active pack; no duplicate game/ZIP/staging; saves/settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-119-*`, authored sources/prompts and progress. Commit records progress only. Full overhaul remains active.


## Texture overhaul issue #74: hybrid inventory batch 120

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `135fecf`. Inspected 32 smaller textures and 32 additional nonopaque textures through eight paired RGB/alpha-composite sheets. Catalog payloads and decoded RGBA hashes verified. Classified 64 new keys; 1,593 now visually classified. No new artwork or installed replacements in this inventory pass.
- New classifications: 27 effects, 12 effect/utility, 9 props, 6 uncertain materials, 4 UI, 3 interface/utility and 3 environment. 56 have nonopaque alpha; eight are opaque, and one has constant partial alpha 84. White/black RGB alone can hide icons, rings, masks or effects encoded in alpha. Recorded alpha ranges and preservation constraints; opaque fire/flare blend modes, shader masks, readable UI and uncertain assets retain specialist obligations. Routing is not completion.
- Safely retired four redundant original triage sheets (545,264 bytes) and sixteen redundant repeat sheets (6,680,557 bytes) from five fully accepted older batches. Catalog/RGBA recovery and DDS/source hashes verified, followed by all resolved-parent/file-SHA256 checks. Final comparisons, selected detailed evidence, source art and original game data retained. Historical progress references annotate retired presentations.
- Current audited catalog: 7,147 payloads / 6,812 supported keys; the original 753 unsupported payloads have offline decoder support, with runtime scene validation pending. Verified nested textures and five XBE error-screen sections are integrated; broader archive, unknown encoding and procedural discovery remains open. No full coverage claim.
- Pack unchanged: 837 installed/offline-approved textures. Every DDS hash matches the previous progress record; refreshed worklist dimensions/aspect/mip/format/hash checks, planner and inventory pass. Native DDS evidence retained from hybrid-119; no redundant native rerun or gameplay launch. Still 5,929 need art and 46 drafts need correction. Full category/discovery/scene validation unfinished; 512 MiB residency limit and near-completion testing gate retained.
- Pack: 817,239,639 bytes (779.38 MiB / 6144 MiB). Texture workspace at audit: 4,057,974,413 logical/allocated bytes (3.78 GiB). Temporary exports/reviews: 257,688,084 bytes (245.75 MiB / 256 MiB). Free disk: 114,939,904,000 bytes. No duplicate games, packs, ZIPs or staging; saves/settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-120-*`, updated visual classification/planner, recovery manifests and progress. No shared runtime/tool changes; no new Mac adaptation. Windows/Mac scene validation remains pending. Full overhaul stays active.


## Texture overhaul issue #74: hybrid alpha batch 121

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `a474cfc`. Eight individually authored nonopaque originals verified against catalog/RGBA evidence. Two accepted offline; six held. Classification remains 1,593 keys.
- Accepted green leaf and nine-pane window using verified original-alpha PNGs in the existing compiler. Original masks are bilinearly resized and DXT5-compressed: mean alpha error 1.0028/255 and 0.7109/255, maxima 32 and 13. Not bit-exact alpha; glass retains partial opacity. Original-alpha-weighted luminance changes +10.23% and -0.47%. Veins, frame grain, glass blending, edge appearance and distant mips remain scene-validation items.
- Held feather, rope and torn wood for visible colored/dark edge contamination; equipment atlas for altered grille pattern; scrollwork and foliage for RGB drift against original cutout contours. Twelve held DDS/metadata files retired after all exact resolved-parent/SHA256 checks. Generated artwork and prompts retained; no failed draft counts as complete.
- Two RGB comparisons, two repeat sheets and eight original/generated/compiled checkerboard details reviewed. Alpha-preserving compilation does not establish RGB registration. Generated-alpha coverage diagnostics are not automatic pass/fail rules because original opaque black regions can be intentional. Future corrections must cover the original mask footprint and inspect hidden RGB at soft edges before compilation.
- Four 1254-square and four 1774x887 built-in transparent imagegen sources. Two retained downsampled full-mip DXT5 textures: one 256-square and one 1024-square. No native 4K claim. Exact prompts, source hashes, alpha-source hashes, generated alpha ranges and resampling recorded privately. Accepted alpha-source PNGs remain protected compiler dependencies.
- All 839 installed/offline-approved replacements pass native DDS/worklist checks. Still 5,921 need art and 52 drafts need correction. Full category/discovery/scene coverage unfinished. Zero game launches or runtime/tool changes; 512 MiB residency limit and near-completion testing gate retained. Windows/Mac scene validation pending.
- Pack: 818,727,200 bytes (780.80 MiB / 6144 MiB). Texture workspace at audit: 4,081,339,837 logical/allocated bytes (3.80 GiB). Temporary exports/reviews: 260,171,001 bytes (248.12 MiB / 256 MiB). Free disk: 114,395,357,184 bytes. One active pack; no duplicate games, ZIPs or staging; saves/settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-121-*`, authored sources/prompts, alpha masks and progress. Commit records progress only. Full overhaul remains active.


## Texture overhaul issue #74: hybrid batch 122

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `6861099`. Diagnosed colored cutout fringes in low-opacity authored RGB; opacity-aware resizing did not resolve them, so the compiler is unchanged. One transparent edit still failed. A second built-in imagegen opaque color-bleed plate succeeds with the verified original mask reapplied using the existing compiler options.
- Torn wood `90972e1be16763c9` accepted offline after RGB, repetition and full-output alpha-composite review. Principal ridge/splinter layout retained and conspicuous colored halos removed; stronger grain contrast and distant mip behavior await scene review. Threshold counts are diagnostics, not visual acceptance criteria. Other cutout layout failures remain held.
- Two 1774x887 authored sources retained privately, accepted output downsampled to 1024x512 full-mip DXT5. Original-alpha error versus bilinear reference: MAE 0.798189/255, max 29, 0.704575% of pixels above 16. Weighted luminance +4.53%. Original-mask policy does not imply lossless DDS alpha or native 4K artwork.
- All 840 installed/offline-approved replacements pass native DDS and worklist audits; planner and inventory refreshed. 51 held keys and 5,921 needing art remain, classification unchanged at 1,593. Full overhaul, discovery gaps and scene validation remain unfinished. Zero gameplay launches; near-completion gate retained. No runtime/tool change or Mac adapter requirement; actual Windows/Mac appearance remains pending.
- Pack: 819,427,301 bytes (781.47 MiB / 6144 MiB). Texture workspace at audit: 4,090,379,429 logical and allocated bytes (3.81 GiB). Temporary exports/reviews: 261,497,680 bytes (249.38 MiB / 256 MiB). Free disk: 114,328,350,720 bytes. One active pack; original game, saves and settings preserved.
- Private evidence: `private/texture-remaster/review/hybrid-122-*`, authored versioned sources/prompts and progress. Authoring guidance updated in `docs/textures.md`; artwork remains private. No full-pack or source duplication.


## Texture overhaul issue #74: hybrid batch 123

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `4721624`. Inspected/classified 32 more catalog-verified opaque originals, now 1,625 provisional classifications. Text-heavy graphics, transit map, painting and uncertain shader assets remain separate review work; small/alpha textures remain in scope.
- Twelve built-in imagegen drafts produced; eleven accepted offline: slab grid, exposed brick/plaster, interlocking pavement, ochre rock, dark trim, olive boards, worn plaster, wood door, rough dark ground, silver hatch and perforated wall. Held the oval mechanism because it loses the isolated fastener below its housing near original pixel (66,123). Two held DDS/metadata files safely retired; all source art/prompts preserved.
- Three comparison sheets, three repeat sheets and twelve enlarged details reviewed. Main UV divisions, panel/board layouts, handle/lock and perforated strip retained in accepted drafts. Mean luminance changes range from -14.46% door to +23.17% rock. Slab left/right edge MAE 7.49 to 21.24; perforated wall 16.64 to 29.72. Stronger grain, relief, highlights and boundary shading require scene-distance/adjacency validation; repeat sheets do not prove seamlessness on every model.
- Eight 1254-square and four 1774x887 generated sources; eleven accepted downsampled full-mip DXT1 outputs (eight 1024-square, three 1024x512). All original/compiled candidate alpha remains opaque. No native 4K claim.
- All 851 installed/offline-approved replacements pass native DDS and worklist audits; planner and inventory refreshed. 5,909 still need art and 52 drafts need correction. Discovery/category/scene gaps remain; full overhaul unfinished. Zero gameplay launches; no runtime/tool change. Windows/Mac scene checks and near-completion gate remain pending.
- Recycled 16 older original-only contact sheets (2,406,716 bytes) after catalog/RGBA recovery checks and all resolved-parent/SHA256 checks. Selections, classifications, final comparisons and original game retained. Pack: 826,074,599 bytes (787.81 MiB / 6144 MiB); texture workspace 4,143,281,467 logical/allocated bytes (3.86 GiB). Temporary output at audit 266,660,053 bytes (254.31 MiB / 256 MiB); free disk 114,049,458,176 bytes. Saves/settings untouched; no duplicate game/ZIP/staging.
- Evidence stays private under `private/texture-remaster/review/hybrid-123-*`, authored PNG/prompts and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 124

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `6985ca8`. Inspected/classified 32 additional catalog-verified opaque originals, now 1,657 provisional classifications. Weapon, character, interface and effect/utility candidates remain separate review work; small/alpha assets remain in scope.
- Sixteen built-in imagegen drafts produced; fifteen accepted offline: metal relief atlas, oxidized panel, mechanical grille, concrete, damaged wall, brushed-metal panel, masonry trim, ivory tiles, ochre stone, turquoise paint/rust, wood door, riveted panel, slotted wall, stone relief and pale grille. Held metal trim `1ff921cfd492469d`: focused crop shows the far-right small fastener remains, but has moved downward and weakened. Two candidate DDS/metadata files safely retired; all artwork/prompts retained.
- Four comparison sheets, four repeat sheets, sixteen enlarged details and two focused crops reviewed. Mechanical grille diagnostic finds 25 interior peaks in both original/compiled original-scale strip, supporting close spacing; rusty panel retains five large holes per band and ivory tiles retain a 5x5 grid. Accepted mean luminance changes -5.01% to +21.60%; stronger grain, relief/highlights and door brightness need final scene checks. Mechanical grille left/right edge MAE 25.48 to 39.80; slotted wall 57.81 to 69.69; door top/bottom 3.40 to 25.47. Repeat evidence does not establish seamlessness on all models.
- Fourteen 1254-square and two 1774x887 generated sources. Fifteen accepted downsampled full-mip DXT1 outputs: twelve 1024-square, two 512-square and one 1024x512. Candidate original/compiled alpha remains opaque. No native 4K claim.
- All 866 installed/offline-approved replacements pass native DDS and worklist audits; planner/inventory refreshed. 5,893 need art and 53 drafts need correction. Audited supported scope remains 6,812 unique keys with discovery/category/scene gaps; full overhaul unfinished. Zero game launches; no runtime/tool changes. Windows/Mac scene validation and near-completion gate remain pending.
- Safely retired 105 older redundant enlarged comparison JPGs (16,855,915 bytes), preserving final comparison sheets and source/game/selection provenance with recovery hashes. Pack: 835,170,857 bytes (796.48 MiB / 6144 MiB). Texture workspace at audit: 4,199,979,189 logical/allocated bytes (3.91 GiB); temporary output 259,255,682 bytes (247.25 MiB / 256 MiB); free disk 109,204,045,824 bytes. Saves/settings untouched; no duplicate extracts/ZIPs/staging.
- Evidence and prompts stay private under `private/texture-remaster/review/hybrid-124-*`, `authored/` and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 125

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `d52e030`. Inspected/classified 32 more catalog-verified opaque originals, now 1,689 provisional classifications. Characters, text and uncertain effects remain separate review work; classification is not full category/scene coverage.
- Fifteen built-in imagegen sources produced; fourteen accepted offline: decorative wood, terracotta plaster, floral wallpaper, tread plate, dark wall atlas, vent panel, curved metal slot panel, curved stone atlas, layered rock, curved grille panel, pale concrete, dark stone trim, wood door and ornate trim. Held `229b8d9a4a3c8111`: gray lower-left atlas region lost its broad shadow/gradient. Its actual UV usage is unverified; treating it as unused was unsupported. Two candidate DDS/metadata files safely retired, preserving source/prompt/evidence.
- Four comparison sheets, four repeat sheets and fifteen enlarged details reviewed. Main floral layout, door hardware/plank direction, vent slats, curved islands and seams retained in accepted drafts. Mean luminance changes -10.22% to +19.01%; stronger highlights/grain and door brightness await final scene checks. Curved grille edge MAE rises left/right 19.89 to 49.21 and top/bottom 42.94 to 70.48; curved metal left/right 41.71 to 66.84. Repeat sheets do not prove seamlessness on every model.
- Nine 1254-square and six 1774x887 sources retained. Fourteen accepted downsampled full-mip DXT1 outputs: eight 1024-square and six 1024x512. Original/compiled candidate alpha remains opaque; no native 4K claim.
- All 880 installed/offline-approved replacements pass native DDS and worklist audits; planner/inventory refreshed. 5,878 need art and 54 drafts need correction. Scope remains 6,812 supported unique keys plus discovery/category/scene gaps. Zero game launches; no runtime/tool changes. Windows/Mac scene validation and near-completion gate remain pending; full overhaul unfinished.
- Safely retired 24 older redundant enlarged comparison JPGs (4,291,914 bytes), preserving final comparisons, selections, original game, artwork and recovery hashes. Pack: 842,868,461 bytes (803.82 MiB / 6144 MiB). Texture workspace at audit: 4,261,439,547 logical/allocated bytes (3.97 GiB); temporary output 263,461,870 bytes (251.26 MiB / 256 MiB); free disk 108,656,508,928 bytes. Personal saves/settings untouched; no duplicate extracts/ZIPs/staging.
- Artwork/prompts and evidence remain private under `private/texture-remaster/authored`, `review/hybrid-125-*` and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 126 corrections

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `f993841`. Reverified eight held opaque originals against catalog payload/RGBA hashes; rebuilt missing disposable exports from authoritative game data. Classification remains 1,689 provisional keys.
- Eight built-in imagegen corrections produced; five accepted offline: pale arrow/yellow-stripe panel without invented side fasteners, blue-mark panel, panel atlas with restored thin bands/rectangular shading, oval mechanism with restored separate bottom fastener, and rusty atlas with restored gray-field gradient. Previous draft artwork and prompt snapshots retained; resolved records preserve the failure history.
- Three remain held. Silver reflection peaks at row 33 versus original 38; brass at 32 versus 39. Their lower dark minima are both 44 versus original 51/52. Profiles average luminance at original 64x64 scale across x14..57. Green-inset trim still loses the separate far-right small fastener, confirmed in an enlarged original-scale crop. Six failed DDS/metadata files safely retired; no failed correction counted as complete.
- Two comparison sheets, two repeat sheets, eight enlarged details and focused references reviewed. Accepted mean luminance changes -5.49% to +7.98%; stronger grain/highlights require final scene checks. Restored panel atlas edge MAE left/right 83.15 to 92.36 and top/bottom 19.35 to 31.18; blue panel top/bottom 57.95 to 73.70. Repeat evidence does not establish all model adjacency or seamlessness.
- Six 1254-square and two 1774x887 sources generated. Five accepted downsampled full-mip DXT1 outputs: two 1024-square, two 512-square and one 1024x512. All original/compiled candidate alpha remains opaque; no native 4K claim.
- All 885 installed/offline-approved replacements pass native DDS and worklist audits; planner/inventory refreshed. 5,878 need art and 49 held drafts need correction. Scope remains 6,812 supported unique keys plus discovery/category/scene gaps. Zero game launches; no runtime/tool changes. Windows/Mac scene validation and near-completion gate remain pending; full overhaul unfinished.
- Safely retired 25 older redundant enlarged comparison JPGs (4,222,204 bytes), retaining final comparisons and recovery hashes. Pack: 844,968,479 bytes (805.82 MiB / 6144 MiB). Texture workspace at audit: 4,284,755,115 logical/allocated bytes (3.99 GiB); temporary output 261,989,711 bytes (249.85 MiB / 256 MiB); free disk 108,512,960,512 bytes. Artwork, original game, personal saves/settings preserved; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-126-*`, versioned/canonical authored prompt records, source PNGs and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 127

- Continued on the #74 branch from f63e2c1. Inspected/classified 32 more catalog-verified opaque originals, now 1,721 provisional classifications. Character identity, text and uncertain shader uses remain specialist work; category/scene coverage is unfinished.
- Twelve built-in imagegen sources produced; eleven accepted offline: weathered sheet, twin-vent hatch, rusty riveted strip, silver machinery recess, olive arrow/stripe wall, stone blocks, cracked plaster, pale door atlas, blue-striped hatch, shadowed rock and pale riveted sheet. Held 26ff884bf5a373a8 because the pale vertical divider around original x126 is weakened/recolored into rust shading. Two failed candidate DDS/metadata files safely retired; source/prompt/review retained.
- Three comparison sheets, three repeat sheets, twelve enlarged details and two focus crops reviewed. Initial suspicion of an added corner hole on blue-striped hatch was not substantiated: the original already has a dark corner cutout. Main cracks, joints, twelve-rivet layout, arrow/stripe and machinery islands retained in accepted drafts; stronger grain, bevels and highlights require scene checks.
- Accepted luminance changes -8.10% to +11.79%. Silver machinery left/right edge MAE 1.18 to 10.33; olive panel 1.16 to 8.91; rock 20.32 to 31.19; door 91.02 to 110.48. Repeat evidence does not establish all model adjacency or seamlessness.
- Six 1254-square and six 1774x887 sources retained. Eleven accepted downsampled full-mip DXT1 outputs: five 1024-square, one 512-square and five 1024x512. Alpha remains opaque; no native 4K claim.
- All 896 installed/offline-approved replacements pass native DDS and worklist audits; planner/inventory refreshed. 5,866 need art and 50 held drafts need correction. Scope remains 6,812 supported unique keys plus discovery/category/scene gaps. Zero game launches; no runtime/tool changes. Final Windows/Mac scene validation and near-completion gate remain pending; full overhaul unfinished.
- Safely retired 26 older redundant enlarged review JPGs (4,277,409 bytes), retaining final comparisons and recovery hashes. Pack: 850,392,443 bytes (811.00 MiB / 6144 MiB). Texture workspace at audit: 4,335,447,609 logical/allocated bytes (4.04 GiB); temporary output 264,345,234 bytes (252.10 MiB / 256 MiB); free disk 108,199,550,976 bytes. Artwork, original game, personal saves/settings preserved; no duplicate extracts/ZIPs/staging.
- Private artwork/prompts: authored/*-hybrid127-v1.png, canonical prompt records and review/hybrid-127-prompt-set.json. Evidence: review/hybrid-127-* and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 128

- Continued on the #74 branch from fc53ce9. Inspected/classified 32 more catalog-verified opaque originals, now 1,753 provisional classifications. Full category/scene coverage remains unfinished.
- Twelve independent built-in imagegen requests attempted, eleven sources returned, eight accepted offline: bamboo slats, aged plaster/timber, pale metal panel, X-braced hatch, masonry, inset wood, ribbed cylinders and vertical planks. Three held: sci-fi weapon label glyphs changed; grip medallion decorative strokes drifted; red metal field lost broad dark shading. Six failed candidate DDS/metadata files safely retired; artwork/prompts/reviews retained.
- Separate generation gap 30a7da52dde92c62: built-in service rejected output with illicit moderation category (request 67f55563-4077-42be-8f42-f59cd8c73105). No output, retry or rephrasing; original stays in use. It remains unfinished, not an accepted replacement. Four explicit generation gaps now recorded.
- Three comparison sheets, three repeat sheets, eleven enlarged details and four focus crops inspected. Red-field x65..114/y38..95 mean grayscale rises 22.18 to 38.68 (+74.41%), confirming broad-shading loss despite whole-image change +8.57%. Grip full-medallion focus and rotated weapon-label comparison document the other failures.
- Accepted luminance changes -2.20% to +7.97%. Pale panel left/right edge MAE 27.71 to 54.91; bamboo top/bottom 6.79 to 22.35; masonry left/right 14.63 to 23.51. Stronger relief/grain/highlights await scene-distance checks; repeat sheets do not prove all mapped adjacency or seamlessness.
- Seven 1254-square and four 1774x887 sources retained. Eight accepted downsampled full-mip DXT1 outputs: five 1024-square and three 1024x512. Candidate alpha opaque. Failed weapon candidate upsampled to 2048-square then removed; no native 2K/4K claim.
- All 904 installed/offline-approved replacements pass native DDS and worklist audits; planner/inventory refreshed. 5,855 need art and 53 held drafts need correction. Scope remains 6,812 supported unique keys plus discovery/category/scene gaps. Zero game launches; no runtime/tool changes. Final Windows/Mac scene validation and near-completion gate remain pending; full overhaul unfinished.
- Retired 138 verified-reproducible opaque original exports (9,365,291 bytes), protecting held/recent references and original-alpha dependencies; catalog payload/RGBA and all resolved-parent/SHA256 checks passed. Initial 4 MiB detail cleanup had only 1,115,498 eligible bytes and made no changes. Pack: 854,940,869 bytes (815.34 MiB / 6144 MiB). Texture workspace at audit: 4,372,229,116 logical/allocated bytes (4.07 GiB); temporary output 261,747,881 bytes (249.62 MiB / 256 MiB); free disk 108,191,662,080 bytes. Original game, artwork, saves/settings and recovery evidence preserved; no duplicate extracts/ZIPs/staging.
- Private artwork/prompts: authored/*-hybrid128-v1.png, canonical/rejected prompt records and review/hybrid-128-prompt-set.json. Evidence: review/hybrid-128-* and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 129 corrections

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `f748fc3`. Eight held opaque corrections completed and reviewed; catalog payload/RGBA references verified. Classification remains 1,753 provisional keys; no new category coverage claim.
- Three accepted offline: rusty blue hatch with restored pale divider, red panel with restored broad dark shading, and brass mechanism atlas with corrected small/radial fittings. Original-scale detail checks resolve the initial concern about the mechanism loop; lower-right radial silhouette and coverage compared directly. Previous source artwork and failure history retained.
- Red panel problem ROI x65..114/y38..95 grayscale mean: original 22.1755, corrected 23.5121 (+6.03%), previous draft 38.6769 (+74.41%). Whole-image luminance +10.57%; hatch -1.43%, mechanism -0.62%. Fine grain, metallic highlights and stronger local contrast await scene-distance validation.
- Five drafts remain held for clothing grid thickness/intersections, grille line registration/frequency, perforation shapes/distribution, shutter louver pitch/phase, and rivet-plate boundaries/darkening. Grille bottom molded detail already exists in the original. Rivet-plate edge MAE LR 0 to 15.04 and TB 1.08 to 14.64; mean luminance -16.08%. Ten failed candidate DDS/metadata files safely retired after resolved-parent/hash checks; all source art preserved.
- Two comparison sheets, two repeat sheets, eight enlarged details and eight original-scale focus comparisons inspected. Accepted hatch edge MAE LR 53.64 to 60.65, TB 84.64 to 100.95; red panel LR 77.23 to 64.64, TB 98.58 to 100.31; mechanism LR 15.04 to 12.74, TB 19.59 to 22.26. These diagnostics do not prove mapped adjacency or seamlessness.
- Built-in imagegen produced four 1254-square and four 1774x887 sources. Three accepted outputs downsampled to one 1024-square and two 1024x512, DXT1 with full mip chains; original/compiled alpha opaque. No native 4K claim.
- All 907 installed/offline-approved replacements pass native DDS/worklist audits; planner/inventory refreshed. 5,855 need art and 50 held drafts require correction. Audited catalog remains 6,812 supported unique keys / 7,147 payloads, with noncatalog discovery/category/scene gaps. Zero game launches; no runtime/tool changes. Full overhaul and Windows/Mac scene validation remain incomplete; near-completion gate not reached.
- Pack 856,340,697 bytes (816.67 MiB / 6144 MiB). Texture workspace at audit 4,402,439,459 logical/allocated bytes (4.10 GiB); temporary output 265,134,321 bytes (252.85 MiB / 256 MiB); free disk 108,028,887,040 bytes. Personal saves/settings and original game preserved; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-129-*`, canonical/versioned prompt records, retained source PNGs and progress. Commit records progress only; other-device synchronization unverified.


## Texture overhaul issue #74: hybrid batch 130

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `0051df3`.
- Inspected 32 catalog-verified opaque originals: twelve prop, seven environment, seven interface, four reflection, one character and one effect/utility classification. 1785 provisional classifications now recorded. Corrected stale classification summary text (previously 1465 despite 1753 records) to match current records; classification is not full category or scene approval.
- Twelve independent built-in imagegen sources produced. Eleven accepted offline: twin-slot vent, curved riveted plate, fractured rock, gray wall tiles, pipe/cable strip, gray block masonry, mottled gray metal, wood/rivet strip, ribbed panel atlas, silver curved reflection and weathered paneled door.
- Golden carved stone 384fc0ce4033ef03 held: whole-image luminance +28.67%, middle-band original-scale x0..255/y90..173 mean grayscale 39.5404 to 55.0421 (+39.20%). Broad middle shadow depth is lost despite motif positions being retained. Two failed candidate DDS/metadata files retired after all resolved-parent/SHA256 checks; source artwork and prompts preserved.
- Three comparisons, three repeat sheets, twelve enlarged details and four original-scale focus comparisons inspected. Vent retains two sets of six slots; tile divisions, riveted strips, masonry courses, cable routes and pipe supports, inset rectangles and wood strip hardware reviewed against originals. Sharper grain, relief edges and metallic glints remain subject to scene-distance validation.
- Silver reflection main mean-gray row peak remains row 96 in both original and compiled after original-scale resampling, measured across x40..239; full-image luminance +0.38%. Original broad lower reflection retained; fine brushed grain is stronger. Door lock plate focus preserves keyhole/round opening placement and principal plate layout; fine ornament detail reinterpreted. Plate mean grayscale 72.8448 to 73.6058; whole door +16.98% luminance. Brighter door paint requires final scene lighting review.
- Accepted luminance changes -7.01% to +16.98%. Rock edge MAE LR 14.05 to 25.45, TB 15.12 to 31.41; masonry LR 20.86 to 30.66, TB 31.88 to 46.51; door TB 62.27 to 83.83. Twin vent LR 5.73 to 6.31, TB 5.93 to 5.19. These edge and repeat diagnostics do not establish all mapped adjacency or seamlessness.
- Eight 1254-square and four 1774x887 generated source PNGs retained. Eleven accepted downsampled DXT1 full-mip outputs: seven 1024-square and four 1024x512. Original/compiled alpha opaque. Actual source sizes/resampling recorded; no native 4K claim.
- Retired 47 older redundant enlarged detail JPGs totaling 6986793 bytes, preserving final comparison sheets, source artwork, held-draft detail evidence, game/catalog references and recovery hashes. All resolved parents and file hashes verified before removal. No game assets, personal settings/saves or source art removed.
- All 918 installed/offline-approved replacements pass native DDS/worklist audits. 5843 need art and 51 held drafts require correction; four previously moderated generation gaps remain unfinished and were not retried. Planner/inventory refreshed. Audited scope remains 6812 supported unique keys / 7147 payloads plus noncatalog discovery/category/scene gaps.
- Zero game launches; no runtime/tool changes. Shared source retains 512 MiB replacement residency cap; actual scene residency/fallback remains pending final testing. Full overhaul unfinished and near-completion gate not reached. Other-device synchronization and scene/hardware validation unverified.
- Pack 862,638,473 bytes (822.68 MiB / 6144 MiB). Texture workspace at audit 4,450,922,047 logical/allocated bytes (4.15 GiB); temporary output 265,913,472 bytes (253.59 MiB / 256 MiB); free disk 107,755,659,264 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-130-*`, authored source PNGs and prompt records, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 131

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `6395e7c`.
- Inspected 32 catalog-verified opaque originals: eleven prop, ten environment, three reflection, three effect/utility, two interface, two uncertain material and one uncertain weapon/prop classification. 1817 provisional classifications now recorded; full category and scene review remains unfinished.
- Twelve independent built-in imagegen sources produced. Eleven accepted offline: striped dimpled steel plate, gray radial reflection, brass emblem, concrete wall, cyan-lit machinery atlas, brick/plaster, bark, inset wooden panel, pale trim atlas, riveted wood strap and teal bracket plate.
- Brown vent 4211c8da02ba7770 held: whole-image luminance +17.08%; left/right edge MAE 33.67 to 79.02; rightmost 8px mean grayscale 58.4189 to 73.2510. Broad framing is brighter and more contrasty than original. Four framed compartments remain present; compartment loss is not the rejection reason. Two failed candidate DDS/metadata files retired after all resolved-parent/SHA256 checks; source artwork and prompts preserved.
- Three final comparisons, three repeat sheets, twelve enlarged details and four original-scale focus comparisons reviewed. Dimple positions, safety stripe framing, grille spacing, central machinery cylinders, brick/plaster silhouettes, wood insets, trim joints, strap rivets and four teal brackets compared to original references. Reflection and emblem silhouettes retained; sharper relief, glints and material detail still require scene-distance review.
- Focused striped plate check confirms staggered dimples retained: original-scale inset grayscale mean 103.2030 to 102.9206. Its generation prompt incorrectly described four corner bolts; visible original and generated reference retain the two prominent diagonal fasteners. Record original references as authoritative, not the mistaken wording. Cyan machine grille row profiles retain repeated slot spacing; cropped grayscale mean 25.0163 to 27.2331. Opaque alpha does not establish emissive/blend semantics.
- Gray radial reflection focus retains converging rays and dark lower region, with stronger ray contrast; original-scale cropped grayscale mean 156.6169 to 154.7918. Overall luminance -1.66%, top/bottom edge MAE 84.99 to 96.43. Final scene/shader validation remains pending.
- Accepted whole-image luminance changes -6.68% to +8.75%. Striped panel edge MAE LR 0 to 3.18 and TB 0 to 2.11; bark LR 44.34 to 52.44, TB 34.69 to 30.39; pale trim LR 21.29 to 32.94, TB 57.24 to 52.13. Repeat/edge diagnostics are not proof of seamlessness or all mapped adjacency.
- Eight 1254-square and four 1774x887 generated sources retained. Eleven accepted downsampled opaque DXT1 full-mip outputs: eight 1024-square and three 1024x512. Actual source sizes and resampling recorded; no native 4K claim.
- Retired 96 older reproducible decoded reference PNGs totaling 6246465 bytes. Catalog payload and decoded RGBA hashes verified before all resolved-parent and file-SHA256 deletion checks. Retained original game/catalog, source artwork, selected review sheets, held-draft references and recovery metadata. No original game assets or personal settings/saves removed.
- All 929 installed/offline-approved replacements pass native DDS/worklist audits. 5831 need art and 52 held drafts require correction; four previously moderated generation gaps remain unfinished and were not retried. Planner/inventory refreshed. Scope remains 6812 supported unique keys / 7147 payloads plus noncatalog discovery/category/scene gaps.
- Zero game launches; no runtime/tool changes. Shared source retains 512 MiB replacement residency cap; actual scene residency/fallback remains pending. Full overhaul unfinished and near-completion gate not reached. Other-device synchronization and scene/hardware validation unverified.
- Pack 869,285,771 bytes (829.02 MiB / 6144 MiB). Texture workspace at audit 4,499,909,936 logical/allocated bytes (4.19 GiB); temporary output 267,382,842 bytes (255.00 MiB / 256 MiB); free disk 107,499,507,712 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-131-*`, authored source PNGs and prompt records, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 132

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `648e76a`.
- Inspected 32 catalog-verified opaque originals: twelve environment, twelve prop, five character, one uncertain weapon/prop, one effect/utility and one sky/environment classification. 1849 provisional classifications now recorded; full category and scene review remains unfinished.
- Twelve independent built-in imagegen sources produced. Eleven accepted offline: small hazard panel, gravel/rocky soil, carved stone relief, four-fastener rusty plate, sliding machine door, twin glowing lenses, dry grass, pipe opening/reflection atlas, tall dark rusty metal, diamond-striped hatch and stepped wood molding atlas.
- Ornamental carpet 42230f8daae97f32 held: full-image luminance -22.84%; original-scale red-field crop x80..159/y60..109 mean grayscale 113.3270 to 81.9665 (-27.67%). Original red field and ornament values become too dark and the weave too contrasty. Two candidate DDS/metadata files retired after all resolved-parent/SHA256 checks; source artwork and prompt retained for correction.
- Three final comparisons, three repeat sheets, twelve enlarged details and four original-scale focused comparisons inspected. Compared gravel pale-stone distribution, stone relief silhouettes/scaled curls and border, four corner fasteners, door atlas partitions and tabs, twin lens positions, pipe circles/reflection, grass distribution, hatch diamonds and bolts, and stepped wood molding/highlight against originals. Fine detail reinterpretation does not establish exact pixel identity or scene approval.
- Small hazard panel focus confirms stripe widths and existing faint vertical panel joints remain aligned; the sharper joints were checked against original-scale references. Hazard-band grayscale mean 77.6967 to 78.2891. Machine-door top-panel mean 137.0708 to 143.7318, with stronger brushed-metal highlights; top/bottom edge MAE 102.67 to 35.46. Broad side panels and central dark recess retained; scene lighting/adjacency remains pending.
- Twin lenses retain green-white centers and peripheral glow; focused mean grayscale 197.6291 to 181.6745. Lens edges are more defined and need final in-game emissive/blend review. Opaque alpha does not establish shader semantics. Pipe retains black opening and right-side curved reflection, with stronger ring highlights; full-image luminance +8.23%.
- Accepted full-image luminance changes -14.78% to +8.23%. Grass edge MAE LR 30.34 to 40.82, TB 31.24 to 43.87; gravel LR 37.59 to 40.42, TB 44.40 to 47.37; wood molding LR 43.37 to 66.87, TB 32.98 to 48.81. Repeat/edge diagnostics do not establish seamlessness or all mapped adjacency.
- Seven 1254-square, four 1774x887 and one 887x1774 generated sources retained. Eleven accepted downsampled opaque DXT1 full-mip outputs: six 1024-square, one 512-square, three 1024x512 and one 512x1024. Actual source sizes/resampling recorded; no native 4K claim.
- Recycled 65 reproducible decoded reference PNGs (4001629 bytes), 21 redundant enlarged/repeat review images (3605613 bytes), and 33 older repeat sheets (12634015 bytes). All resolved parents and SHA256 hashes checked before removals; reference pixels also verified against catalog-checked game decoding. Final comparisons, original game/catalog, authored sources, metrics and recovery records retained. Historical batch presentation retirements annotated; no personal settings/saves removed.
- All 940 installed/offline-approved replacements pass native DDS/worklist audits. 5819 need art and 53 held drafts require correction; four previously moderated generation gaps remain unfinished and were not retried. Planner/inventory refreshed. Scope remains 6812 supported unique keys / 7147 payloads plus noncatalog discovery/category/scene gaps.
- Zero game launches; no runtime/tool changes. Shared source retains 512 MiB replacement residency cap; actual scene residency/fallback remains pending. Full overhaul unfinished and near-completion gate not reached. Other-device synchronization and scene/hardware validation unverified.
- Pack 875,059,257 bytes (834.52 MiB / 6144 MiB). Texture workspace at audit 4,536,168,127 logical/allocated bytes (4.22 GiB); temporary output 255,835,116 bytes (243.98 MiB / 256 MiB); free disk 107,227,541,504 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-132-*`, authored source PNGs and prompt records, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 133 corrections

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `7ac61b8`.
- Eight previously held opaque drafts corrected and reviewed; no new classification. 1849 provisional classifications, 6812 supported unique keys and 7147 supported catalog payloads remain the audited scope. Noncatalog discovery/category/scene gaps remain.
- Four corrections accepted offline: golden metal panel restores its separate small far-right fastener; carved ocher stone restores the broad middle shadow; brown vent restores muted frame shading; carpet restores its lighter coral-red field. Prior source artwork, prompts and failure history preserved.
- Stone middle-band original-scale grayscale mean 39.5404 to 38.3184, versus previous failed 55.0421. Carpet red-field crop mean 113.3270 to 112.4300, versus previous failed 81.9665. Vent rightmost 8px mean 58.4189 to 52.5479, versus previous failed 73.2510. Fastener focus x210..255/y75..104 shows both the larger and separate smaller right fittings again. These focused checks supplement full comparison review.
- Four corrections remain held: missing/simplified intermediate radial lines, grip medallion stroke changes despite closer lower marks, and two misregistered reflection maps. Reflections measured explicitly with original-scale Pillow-L row profiles across x14..57: silver bright peak 38 versus 36 and lower trough 51 versus 48; brass bright peak 38 versus 33 and lower trough 52 versus 46. Current grayscale method supersedes earlier approximate brass peak 39 wording. Band width/curvature and lower structures still differ.
- Two final comparisons, two repeat sheets, eight enlarged details and eight focused original-scale comparisons inspected. Original references verified against catalog payload and decoded RGBA hashes before generation. No automated brightness/profile diagnostic substitutes for visual review or final scene validation.
- Accepted full-image luminance changes: golden fitting panel -4.81%, stone +5.23%, vent -9.80%, carpet +0.71%. Edge MAE LR/TB: fitting panel 20.10/36.15 to 17.30/29.87; stone 11.57/42.93 to 19.06/39.00; vent 33.67/10.25 to 40.40/13.06; carpet 104.14/126.09 to 132.43/129.46. Edge metrics and repeat sheets do not establish seamlessness or all mapped adjacency.
- Five 1254-square and three 1774x887 built-in imagegen source images retained. Four accepted opaque DXT1 full-mip outputs downsampled to one 1024-square and three 1024x512. Original/compiled alpha opaque. Actual source sizes/resampling recorded; no native 4K claim.
- Eight failed candidate DDS/metadata files retired after all resolved-parent/SHA256 checks. All source artwork, prior/current prompt provenance, original game and personal settings/saves preserved. One active pack; no duplicate extract, ZIP, staging tree or additional game build.
- All 944 installed/offline-approved replacements pass native DDS/worklist audits. 5819 need art and 49 held drafts require correction; four previously moderated gaps remain unfinished and were not retried. Planner/inventory refreshed. Zero game launches and no runtime/tool changes. 512 MiB replacement residency cap retained; actual scene residency/fallback unverified. Full overhaul and scene/platform validation remain incomplete; near-completion gate not reached.
- Pack 876,809,187 bytes (836.19 MiB / 6144 MiB). Texture workspace at audit 4,565,683,315 logical/allocated bytes (4.25 GiB); temporary output 259,340,920 bytes (247.33 MiB / 256 MiB); free disk 107,069,952,000 bytes.
- Private evidence: `review/hybrid-133-*`, canonical/versioned prompts, source PNGs and progress. Commit records progress only; other-device synchronization unverified.


## Texture overhaul issue #74: hybrid batch 134

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `f708ce3`.
- Inspected and provisionally classified 32 catalog-verified opaque originals across environment, prop, character, interface, effect/utility, reflection and uncertain-material categories. 1881 classified keys now recorded; this does not establish full category or scene review.
- Twelve independent built-in imagegen sources produced. Nine accepted offline: brown cliff, pale concrete, horizontal bark, twin-lamp industrial door, studded copper-brown inset, cracked scroll-bordered stone, nested stone molding, peeling green twin panels and brown machinery grille. Three held: red vent, eye/mouth atlas and light cone; originals remain active for held keys.
- Three final comparisons, three repeat sheets, twelve enlarged details and five original-scale focused comparisons inspected. Original source pixels reverified against catalog-checked game decoding. Preserved broad fractures, joint positions, grain direction, frame geometry, stud distribution, carved borders, chipped areas and panel/slot layout; fine detail reinterpretation does not establish exact pixel identity or scene approval.
- Red vent held for +27.08% full-image luminance and stronger bright rust/rib contrast. Original-scale grayscale 37.8427 to 47.7809 (+26.26%). Eye/mouth atlas held for broad new highlights, stronger veins and altered small lower gray UV island; eye-region grayscale 133.4171 to 146.8236 (+10.05%).
- Light cone held for changed footprint/falloff and +34.83% full-image luminance; original-scale grayscale 38.2202 to 51.4538 (+34.62%). The stronger diagonal core and compressed contour banding need correction and format reassessment. Opaque alpha does not establish shader/blend semantics.
- Twin-lamp door retains lamp positions, two long raised strips, seams and fasteners. Sharper frame and darker surrounding metal: full-image luminance -12.04%; top 48-row grayscale 199.6919 to 178.3464, body crop 110.8084 to 96.8300. Lamp centers remain bright; emissive response, baked shading and scene adjacency remain pending.
- Prompt correction: 540ccadc53c9f7ed was initially described as five bars. Original-scale review confirms FOUR complete bolted bars plus thin upper/lower channels. Generated grille follows the observed reference structure; recorded description corrected, no five-bar claim retained.
- Accepted full-image luminance changes -13.37% to +12.63%. Cliff edge MAE LR 12.14 to 18.15 and TB 37.27 to 47.78; repeat reviews retain broad repeated structure but do not prove seamlessness or mapped adjacency. Material identities for the studded inset and some triage assets remain provisional.
- Eight 1254-square and four 1774x887 generated sources retained. Nine accepted downsampled DXT1 full-mip outputs: seven 1024-square and two 1024x512. Actual source sizes and resampling recorded; no native 4K claim.
- Recycled 25 older redundant repeat-review JPGs (10570358 bytes) after all resolved-parent/SHA256 checks; final comparisons, sources, metrics and recovery records retained. Six held candidate DDS/metadata files also retired after all checks. No source art, game assets, personal settings or saves removed.
- All 953 installed/offline-approved replacements pass native DDS and worklist audits. 5807 still need art; 52 held drafts require correction. Four previously moderated generation gaps remain unfinished and were not retried. Planner/inventory refreshed. Scope remains 6812 supported unique keys / 7147 payloads plus noncatalog discovery, category and scene gaps.
- Zero game launches; no runtime/tool changes. Shared code retains the 512 MiB replacement residency limit; scene residency, fallback and final 1080p/4K/ultrawide validation remain pending. Full overhaul unfinished and near-completion gate not reached. Other-device synchronization unverified.
- Pack 882,406,759 bytes (841.53 MiB / 6144 MiB). Texture workspace at audit 4,606,174,120 logical/allocated bytes (4.29 GiB); temporary output 256,276,034 bytes (244.40 MiB / 256 MiB); free disk 106,445,672,448 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-134-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 135

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `d4cee5b`.
- Inspected and provisionally classified 32 catalog-verified opaque originals across environment, prop, character, interface, effects, reflections and uncertain materials. 1913 classified keys now recorded; category and scene review remains unfinished.
- Twelve independent built-in imagegen sources produced. Nine accepted offline: interlocking stone blocks, pale stained wall, cyan-lit industrial door, pale golden folds, black marble, wood/metal crate panel, worn pink-gray surface, mossy brick wall and blue-gray panel atlas. Three held: gray recessed-square material, ornate barred windows and blue energy ring; original textures remain active for held keys.
- Three final comparisons, three repeat sheets, twelve enlarged details and five original-scale focus comparisons inspected. Original pixels reverified against catalog-checked decoding. Reviewed joint topology, panel boundaries, markings, grain direction, vein structure, plank seams, fastener positions and repeated appearance. Fine detail reinterpretation does not establish exact pixel identity or scene approval.
- Gray recessed square 56b57510b7d51249 held: fine gray grain became coarse deep ridges and stronger bevels, with changed dark surround. Full-image luminance -13.15%; original-scale grayscale 95.4180 to 82.9414. Material identity remains uncertain and the draft must not define it.
- Ornate windows 592f40c07d941cfd held: lower small hooks/inner curls and bottom curved strokes differ from original ironwork. Top/lower focused grayscale +16.94%/+15.40%. Window openings and frames alone do not establish faithful ornament.
- Blue ring 5a77cb6ec7d1746a held: black center retained closely but bright knots intensified. Full-image luminance +12.17%; original-scale max-RGB >220 pixel count 1579 to 5779. Central black runs (max RGB <=3) x60..194 to59..195; y60..194 unchanged; central 64-square max RGB remains zero. These diagnostics do not prove full-shape or blend equivalence; restore hotspot distribution/falloff.
- Cyan-lit door accepted offline with light-region grayscale 123.0109 to 121.6002 (-1.15%); hazard crop 89.5485 to 103.0434 (+15.07%) retains diagonal marking layout with sharper wear. Full-image luminance +7.10%. Emissive response and final scene lighting remain pending.
- Golden-fold center crop grayscale 218.6627 to 210.3391 (-3.81%); vertical fold structure retained with stronger fine grain and a slightly wider dark center near the bottom. Exact material/scene use remains provisional. Accepted batch full-image luminance ranges -7.21% to +7.10%; repeat/edge metrics do not prove seamlessness or mapped adjacency.
- Eight 1254-square and four 1774x887 sources retained. Nine accepted downsampled DXT1 full-mip outputs: four 1024-square, one 512-square, four 1024x512. Actual source size and resampling recorded; no native 4K claim.
- Recycled 30 older redundant repeat-review JPGs (12584747 bytes) after all resolved-parent/SHA256 checks; final comparisons, sources, metrics and recovery records retained. Six held candidate DDS/metadata files retired after all checks. No source art, original game assets, personal settings or saves removed.
- All 962 installed/offline-approved replacements pass native DDS and worklist audits. 5795 still need art; 55 held drafts require correction. Four previously moderated generation gaps remain unfinished and were not retried. Planner/inventory refreshed. Audited scope remains 6812 supported unique keys / 7147 payloads plus noncatalog discovery, category and scene gaps.
- Zero game launches; no runtime/tool changes. Shared source retains the 512 MiB replacement residency cap; scene residency, fallback and 1080p/4K/ultrawide validation remain pending. Full overhaul unfinished and near-completion gate not reached. Other-device synchronization unverified.
- Pack 886,780,997 bytes (845.70 MiB / 6144 MiB). Texture workspace at audit 4,643,549,615 logical/allocated bytes (4.32 GiB); temporary output 250,898,167 bytes (239.28 MiB / 256 MiB); free disk 106,213,736,448 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-135-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 136 corrections

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `c0f4934`.
- Eight previously held opaque drafts corrected using built-in imagegen; no new visual classifications. Audited scope remains 6812 supported unique keys / 7147 catalog payloads, with 1913 provisional visual classifications. Noncatalog discovery, category and scene coverage remain incomplete.
- Two corrections accepted offline: rust-red vent retains its vertical members, dark recesses and cropped edge with finer oxidation; gray inset square restores fine low-relief grain, shallow outline and central black spot. Exact gray material identity remains provisional. Vent full-image luminance -13.52%; gray -2.74%, with original-scale grayscale mean 95.4180 to 92.7856. Scene appearance remains pending.
- Six corrections remain held. Eye brightness improves (eye crop grayscale -0.69%, full-image luminance -1.77%) but small flat atlas island gains a beveled rim and altered lower-left boundary. Window lower center stem and hooks/curls remain changed; focused grayscale +9.26%. These shape defects prevent installation.
- Light cone is now 19.72% dimmer, with contracted outer fan and visible compiled contour banding. Energy-ring bright max-RGB>220 pixels decrease from prior failed 5779 to 3791 but still exceed original 1579; central 64-square stays black. Full-image ring luminance +4.33% does not establish hotspot equivalence. Both remain held.
- Reflection maps remain held: original-scale grayscale profiles averaged across columns14..57 measure silver peak38 to34/trough51 to47 and brass peak38 to34/trough52 to46. Band width, curvature and left strips also differ. Repeated image-only attempts have not preserved registration; next correction must use a more constrained editing strategy rather than reuse this failed full-map prompt.
- Two comparison sheets, two repeat sheets, eight enlarged details and focused atlas-island/eye/window views inspected. Original references reverified against catalog decoding and hashes; all eight original and compiled alpha ranges opaque. Repeat sheets and edge statistics are diagnostics, not proof of seamlessness or mapped scene adjacency.
- Six 1254-square and two 1774x887 generated sources retained with canonical, prior and versioned prompt records. Two accepted DXT1 full-mip outputs downsampled to 1024x512 and 512-square. Actual source dimensions and resampling recorded; no native 4K claim.
- Twelve rejected candidate DDS/metadata files safely retired after all resolved-parent and SHA256 checks. Source artwork and correction history retained; stale held-draft latest-attempt fields updated. No original game assets, saves or settings changed; one active pack, no duplicate extraction/build/ZIP/staging.
- All 964 installed replacements pass native DDS/worklist audits. 5795 need art and 53 held drafts need correction. Planner and inventory refreshed; four previously moderated gaps were not retried. 512 MiB replacement residency cap retained, actual scene residency/fallback unverified.
- Zero game launches and no runtime/tool changes. Full overhaul and representative scene/platform/1080p/4K/ultrawide validation remain incomplete; near-completion gate not reached. Continue hybrid material production from existing triage cursor 5be5ec38bdd947b1, with specialist corrections kept separate.
- Pack 887,306,433 bytes (846.20 MiB / 6144 MiB). Texture workspace at audit 4,663,931,149 logical/allocated bytes (4.34 GiB); temporary output 253,431,836 bytes (241.69 MiB / 256 MiB); free disk 106,108,067,840 bytes.
- Private evidence: `review/hybrid-136-*`, canonical/versioned prompts, source PNGs and progress. Commit records progress only; other-device synchronization unverified.


## Texture overhaul issue #74: hybrid batch 137

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `6fbb27a`.
- Visually inspected and provisionally classified 32 catalog-verified opaque originals across environment, prop, character, weapon/equipment, interface, effects and uncertain material categories. 1945 keys now classified; category and scene review remains incomplete. Audited scope remains 6812 supported unique keys / 7147 catalog payloads plus noncatalog discovery gaps.
- Sixteen independent built-in imagegen sources accepted offline: paired pale panels, mottled olive surface, gray panel with lower track, cyan-lit cabinet, fractured rock, concrete slab strip, rusty corrugated sheets, scuffed blue-green panel, grainy tiled-top wall, broken plaster/lath, dark grille, riveted brown panel, gray louvered panel, rectangular stone border, circular metal/brick inset and pale folded material. No new held drafts in this batch; 53 existing held drafts remain unfinished.
- Inspected four final comparisons, four repeat sheets, sixteen enlarged details and five focused comparisons. Original decoded RGBA verified against catalog payloads. Reviewed seam/fold/fracture layout, atlas boundaries, panel crops, fasteners, vent banks, light bars, plaster islands and circular rim/brick joints. Fine surface detail is a modern reinterpretation; exact material identity remains provisional for the mottled and folded surfaces.
- Cyan cabinet retains five upper, eleven middle and five lower light bars on visual review. Center-strip grayscale mean 132.4610 to 118.5563 (-10.50%); glow is more restrained, full-image luminance -4.57%. Stored green-channel row profiles show original glow merges some middle bars at threshold 200, so binary threshold-run counts alone cannot establish bar count. Final scene emissive response remains pending.
- Focused checks: grille center/side grayscale +4.38%/-6.41%, gray upper louvers -5.38%, lower pale panel -7.61%, exposed plaster/lath center -8.11%. Structural boundaries remain visible; finer surface relief and contrast require final scene review. All original/compiled alpha opaque.
- Full-image luminance ranges -12.54% to +0.44%. Rock edge MAE LR/TB increases 15.07/15.17 to 30.80/34.35 with sharper mineral detail; repeat sheets inspected without obvious new coarse discontinuity. Other edge/brightness metrics retained per key. Offline repetition does not establish seamlessness or every mapped adjacency.
- Twelve 1254-square and four 1774x887 generated sources retained. Accepted DXT1 full-mip outputs downsampled to ten 1024-square, two 512-square and four 1024x512. Source sizes and resampling recorded; no native 4K claim.
- Recycled 30 older redundant repeat-review JPGs totaling 12494401 bytes (11.92 MiB), after all resolved-parent/reparse/SHA256 checks. Final comparison sheets, source artwork, prompts, metrics and recovery records retained. No source art, game assets, personal saves or settings removed. One active pack, no duplicate extracts/builds/ZIPs/staging.
- All 980 installed replacements pass native DDS/worklist audits. 5779 still need art; 53 held drafts require correction. Planner and inventory refreshed. Four previously moderated gaps were not retried. 512 MiB residency cap retained; actual scene residency and fallback remain unverified.
- Zero game launches; no runtime/tool changes. Full overhaul and scene/platform/1080p/4K/ultrawide validation remain incomplete; near-completion gate not reached. Continue hybrid production from opaque >=2-occurrence/min128 triage cursor 65ba7f86b3509361, with specialist corrections kept separate. Other-device synchronization unverified.
- Pack 896,053,749 bytes (854.54 MiB / 6144 MiB). Texture workspace at audit 4,728,121,489 logical/allocated bytes (4.40 GiB); temporary output 250,428,564 bytes (238.83 MiB / 256 MiB); free disk 105,723,666,432 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-137-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 138

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `c1ccbab`.
- Visually inspected and provisionally classified 32 catalog-verified opaque originals across environments, props, characters, weapon/equipment atlases, interface, reflection and effects/utility categories. 1977 keys now classified; full category and scene review remains incomplete. Audited scope remains 6812 supported unique keys / 7147 catalog payloads plus noncatalog discovery gaps.
- Fourteen independent built-in imagegen sources accepted offline: leafy stone wall, staggered vertical gray panels, mottled material, perforated equipment supports, paired pink panels, grainy wall with tiled top, pale upper-banded panel, 4x4 gray tiles, mossy craggy rock, bronze pipe panel, elongated diamond material, horizontal wood, pale plaster and dark fractured rock. Two new held drafts remain unfinished; no rejected candidate remains in active pack.
- Inspected four final comparisons, four repeat sheets, sixteen enlarged details and six focused comparisons. All sixteen original decoded RGBA images verified against catalog payloads; all original and compiled alpha opaque. Reviewed block/board seams, panel/pipe/vent alignment, fracture silhouettes, tile grid, repeated pattern pitch and broad shading. Fine material detail is a modern reinterpretation; uncertain material identities remain provisional.
- Equipment central panel retains four vent groups of four columns by four rows on focused comparison. Focus grayscale +10.84%; full luminance +14.38%. Bronze panel focused right plate retains four fasteners and louver placement, grayscale +15.02%; full luminance +15.43%. Gray tile full luminance +14.30%; 4x4 grid and dark upper gradient retained. Accepted with final scene contrast review pending.
- Held 692448037726fcb2: small control cluster merges separated red markings, shifts blue indicator to green/cyan and changes small left indicators. Held 6f689dc7c7a3c8fe: peeling-wall full luminance +36.27%, upper-half grayscale +84.55%; dark upper shading and restrained flake contrast lost. Preserve original gameplay cues and broad shading in corrections. Both source PNGs, prompts, metrics and comparisons retained; four rejected candidate DDS/JSON files safely retired after all parent/reparse/SHA256 checks.
- Accepted full-image luminance ranges -5.69% to +15.43%. Repetition and edge metrics inspected; existing shaded/bordered nonseamless textures remain explicitly nonseamless. No obvious new coarse edge discontinuity on accepted comparisons; repeat checks do not establish every mapped adjacency. Original-scale diamond grayscale -0.22%; elongated staggered rhythm retained, exact surface usage pending scene review.
- Nine 1254-square and seven 1774x887 generated sources retained. Accepted DXT1 full-mip outputs downsampled to six 1024-square, one 512-square and seven 1024x512. Actual source sizes and resampling recorded; no native 4K claim. No additional image generation in the final review turn.
- All 994 installed replacements pass native DDS/worklist audits. 5763 still need art; 55 held drafts require correction. Planner and inventory refreshed. Four previously moderated gaps were not retried. 512 MiB residency cap retained; actual scene residency and fallback remain unverified.
- No old presentation cleanup needed for this batch; temporary output remains under 256 MiB. One active pack; no duplicate extracts/builds/ZIPs/staging. Authoritative original game assets, source art, personal saves and settings preserved.
- Zero game launches; no runtime/tool changes. Full overhaul and scene/platform/1080p/4K/ultrawide validation remain incomplete; near-completion gate not reached. Continue hybrid production from opaque >=2-occurrence/min128 triage cursor 704e5bd94fb33437, with specialist corrections kept separate. Other-device synchronization unverified.
- Pack 902,877,541 bytes (861.05 MiB / 6144 MiB). Texture workspace at audit 4,798,714,251 logical/allocated bytes (4.47 GiB); temporary output 258,473,428 bytes (246.50 MiB / 256 MiB); free disk 105,469,222,912 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-138-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 139

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `b9a80b3`.
- Visually inspected and provisionally classified 32 catalog-verified opaque originals across environments, props, characters, weapon/equipment atlases, interface, reflection and effects/utility categories. 2009 keys now classified; full category and scene review remains incomplete. Audited scope remains 6812 supported unique keys / 7147 catalog payloads plus noncatalog discovery gaps.
- Fifteen independent built-in imagegen sources accepted offline: dark brick wall, riveted inset metal panel, gray recessed-panel atlas, gray wall/trim, four-socket plate, ornate bronze louvers, dark cliff with foliage, paired green-gray equipment panels, cracked concrete, worn pale plaster, gray vent/slot panel, blue-gray plaster exposing brick, vertically banded wall, streaked slab wall and ochre cliff. One new held draft remains unfinished.
- Inspected four final comparisons, four repeat sheets, sixteen enlarged details and six focused comparisons. All sixteen originals verified byte-for-byte as decoded RGBA against catalog payloads; original and compiled alpha opaque. Reviewed UV subdivisions, socket patterns, vent/louver spacing, rivet rows, ornament placement, crack/brick layout and broad shading. Fine materials and ornament details are modern reinterpretations; final scene fidelity remains pending.
- Four socket plates preserve three-over-two slots in each upper plate and two-over-three in each lower plate. Focused left upper/lower grayscale +2.73%/-7.32%. Gray equipment atlas circular fitting/recess grayscale +6.58%/+7.92%; positions and bevel boundaries retained. Ornate louver focused grayscale -11.96%; three paired horizontal forms and side ornament strips retained.
- Paired green-gray grille focus grayscale -13.47%; dividers, row spacing and dark cutouts retained. Gray twin-vent focus +8.27%; slots, screw positions and long lower slot retained. Full-image accepted luminance ranges -12.89% to +7.42%. Contrast and material response still require consolidated scene validation.
- Held 74743caa92022bf4: angled machine hardware layout remains recognizable, but pale worn metal darkens/browns excessively (full luminance -19.40%, upper plate grayscale -22.34%, rivet strip -21.84%). Restore broad pale wear and separation from dark recesses without shifting rivets or diagonal boundaries. Source PNG, prompt, metrics and comparisons retained; rejected DDS/JSON removed only after all parent/reparse/SHA256 checks.
- Repetition and edge metrics inspected. Gray equipment atlas left/right edge MAE rises 56.61 to 109.30; opposite atlas edges contain different islands and are not a seamless material. It is accepted for atlas layout, not seamless tiling; mapped edge adjacency remains pending. Green-gray paired panel left/right edge MAE 0 to 5.48 with fine wear; no obvious new coarse seam on repeat sheet. Other mapped adjacency and seam claims remain unverified.
- Twelve 1254-square and four 1774x887 generated sources retained. Accepted DXT1 full-mip outputs downsampled to twelve 1024-square and three 1024x512. Actual source sizes and resampling recorded; no native 4K claim. Built-in imagegen edit mode used independently for all sixteen assets.
- Recycled 19 older redundant repeat-review JPGs from batches 130-136 totaling 8461043 bytes (8.07 MiB), after all resolved-parent/reparse/SHA256 checks. Main comparison sheets, source artwork, prompts, metrics and recovery records retained. Authoritative original game assets, personal saves and settings preserved. One active pack; no duplicate extracts/builds/ZIPs/staging.
- All 1009 installed replacements pass native DDS/worklist audits. 5747 still need art; 56 held drafts require correction. Planner and inventory refreshed. Four previously moderated gaps were not retried. 512 MiB residency cap retained; actual scene residency and fallback remain unverified.
- Zero game launches; no runtime/tool changes. Full overhaul and scene/platform/1080p/4K/ultrawide validation remain incomplete; near-completion gate not reached. Continue hybrid production from opaque >=2-occurrence/min128 triage cursor 787969fba3da53ee, with specialist corrections kept separate. Other-device synchronization unverified.
- Pack 912,323,335 bytes (870.06 MiB / 6144 MiB). Texture workspace at audit 4,865,727,957 logical/allocated bytes (4.53 GiB); temporary output 260,242,780 bytes (248.19 MiB / 256 MiB); free disk 105,116,016,640 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-139-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 140 corrections

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `19ebc0f`.
- Eight previously held opaque drafts corrected using built-in imagegen edits of their prior high-resolution sources, with catalog originals and selected enlarged reference crops. Six corrections accepted offline; two remain held. No new visual classifications. Audited scope remains 6812 supported unique keys / 7147 catalog payloads and 2009 provisional classifications; noncatalog, category and scene coverage remain incomplete.
- Accepted peeling-wall correction restores the dark upper field and restrained flake brightness. Upper-half original-scale grayscale is now +16.16% (previous +84.55%), lower half -14.32%, full-image luminance -7.66%. Panel divisions and major peeling regions retained. Scene shading remains pending.
- Accepted angled machine metal restores pale wear and hardware separation. Upper plate grayscale +6.26%, rivet strip +23.19%, full-image luminance +16.78%; prior upper plate was -22.34%. Rivet rows, diagonal plate boundary, slot and circular fittings retained; contrast/specularity still require scene review.
- Accepted face/hair atlas correction removes invented deep radiating mouth creases while preserving UV islands, mouth/nose position, eye strip and hair layout. Focused mouth grayscale -1.40%, full-image luminance -9.97%. Accepted accessory/eye atlas restores a muted blue-gray iris and retains oval pupil, lower reflection and accessory islands; eye-region grayscale -10.48%, full-image luminance -2.97%. Character identity on the mapped models remains pending runtime review.
- Accepted gray panel restores irregular dense dark streaks instead of invented oval perforations. Narrow-strip grayscale -4.80%, full-image luminance -1.90%; rails, slabs and bolts retained. Accepted blue shutters retain six leaf outlines, divisions, louver counts and lower ornaments. Sampled upper/lower dark-gap rows across all six leaves differ by at most one original pixel in eleven sections and at most two in the fifth lower section. The initial shared-window probe duplicated one original gap in leaf5; follow-up used its actual shifted rows and a separately inspected crop. This is approximate registration, not pixel-perfect equivalence or scene proof.
- Held 692448037726fcb2: colors improve, but red strokes remain enlarged/merged and the blue vertical cue becomes a short horizontal mark; cluster grayscale +30.81%. Held 4ead7440c40cd852: flat island loses shiny bevel but changes brown-gray fill and lower stepped silhouette; crop grayscale +40.13%. Eye brightness is close (-2.54%); the remaining island defect prevents installation. Preserve source art and history for constrained correction; do not repeat unchanged full-map prompts.
- Inspected two final comparisons, two repeat sheets, eight enlarged details, eight focused comparisons and the extra fifth-leaf crop. All eight originals reverified byte-for-byte against catalog-decoded RGBA, with payload/decoded hashes and opaque alpha. All candidate compiled alpha ranges opaque. Repeat sheets and edge metrics inspected; existing atlas/nonseamless boundaries are retained, not advertised as seamless. Actual mapped adjacency and scene appearance remain unverified.
- Five 1254-square and three 1774x887 generated source PNGs retained. Accepted DXT1 full-mip outputs downsampled to three 1024-square, one 512-square and two 1024x512 files. Actual source dimensions, resampling, prompts and prior/versioned correction records saved. No native 4K artwork claim.
- Four rejected candidate DDS/metadata files retired only after all resolved-parent, reparse-point and SHA256 checks. No redundant old presentation cleanup needed within this batch. One active pack; no duplicate game extracts, builds, ZIPs or staging trees. Original game assets, personal saves and settings preserved.
- All 1015 installed replacements pass native DDS/worklist audits. 5747 still need art and 50 held drafts need correction. Planner and inventory refreshed. Four previously moderated gaps were not retried. The 512 MiB residency limit remains; actual scene residency and fallback remain unverified.
- Zero game launches; no runtime/tool changes. Full overhaul and representative scene/platform/1080p/4K/ultrawide validation remain incomplete. Near-completion gate is not reached. Continue hybrid material production from opaque >=2-occurrence/min128 triage cursor 787969fba3da53ee, with specialist corrections separate. Other-device synchronization unverified.
- Pack 915,297,745 bytes (872.90 MiB / 6144 MiB). Texture workspace at audit 4,894,138,420 logical/allocated bytes (4.56 GiB); temporary output 263,772,947 bytes (251.55 MiB / 256 MiB); free disk 105,585,082,368 bytes.
- Private evidence: `review/hybrid-140-*`, canonical/versioned prompts, source PNGs and progress. Commit records progress only; other-device synchronization unverified.


## Texture overhaul issue #74: hybrid batch 141

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `8e00139`.
- Visually inspected and provisionally classified 32 catalog-verified opaque originals across environment, prop, weapon, character/equipment, interface and effect/utility categories. 2041 keys now classified. Recomputed classification summary fields from all records: before this batch they still reported 1977 despite 2009 actual records. Counts now agree with the 2041 unique records; full category and scene review remains incomplete. Audited scope remains 6812 supported unique keys / 7147 catalog payloads plus noncatalog discovery gaps.
- Fifteen independent built-in imagegen sources accepted offline: gray-brown brick, gray mottled banded material, ochre sandstone masonry, wooden boards, stained gray block wall, turquoise-accented riveted panel, gray corner molding, pale worn horizontal-band material, pale vertical panels, olive metal grid, layered tan rock, gray-green streaked material, architectural rectangular trim, pale veined stone stripe and small two-panel surface. One new paired-plate draft held for shading correction.
- Inspected four final comparison sheets, four repeat sheets, sixteen enlarged details and six focused comparisons. All sixteen originals verified byte-for-byte as decoded RGBA against catalog payloads, with recorded payload/decoded hashes. Original and compiled alpha opaque. Reviewed UV boundaries, mortar and board courses, frame widths, hardware positions, panel divisions and major rock fissures. Fine material detail is a modern reinterpretation; mapped scene appearance remains pending.
- Wooden boards retain five upper and five lower nail locations; focused upper/lower grayscale -0.18%/+2.23%. Gray metal panel retains its staggered fastener layout, clipped corners and turquoise frame accents; central region grayscale +4.32%. Corner molding retains four round outer forms and central angular shapes in the small lower-left cluster, grayscale +0.25%; exact object function remains provisional.
- Olive metal grid retains two columns and four rows, seam positions and fine rivet rhythm; focused seam grayscale -6.17%. Small two-panel lower section preserves its divider and recess outlines, grayscale -7.54%. Accepted full-image luminance ranges -11.46% to +7.15%. Exact material response, contrast and fine detail still require consolidated scene validation.
- Held 7f54bfac6994bde2: paired metal plates lose their original lighting difference, upper crop grayscale -17.62% and lower +40.39%, despite full-image luminance +0.77%. Restore stronger original upper illumination and subdued lower plate without moving rivets, ovals or recesses. Source PNG, prompt, metrics and comparisons retained. Two rejected candidate DDS/JSON files removed only after all resolved-parent/reparse/SHA256 checks.
- Repeat sheets and edge statistics inspected. Olive grid left/right edge MAE increases 27.82 to 52.03 with stronger edge rivet contrast; the original already has asymmetric boundary rivets. Broad panel pitch is retained, but mapped boundary appearance remains pending. Turquoise panel edge MAE remains small (left/right 0 to 1.23, top/bottom 0 to 3.29). Existing atlas and shaded nonseamless boundaries are not claimed seamless. No obvious new coarse discontinuity in accepted repeat comparisons; actual scene adjacency remains unverified.
- Ten 1254-square and six 1774x887 generated sources retained. Accepted DXT1 full-mip outputs include seven 1024-square, one 512-square, six 1024x512 and one 2048-square. The layered rock request aimed for 4096 but returned 1254-square; its 2048 output is upsampled, explicitly recorded, and not native 2K/4K art. The other fourteen accepted outputs are downsampled. Actual source sizes, resampling and prompts saved.
- Recycled 20 redundant older final-repeat JPGs from batches 61, 62, 85, 95 and 137-139 totaling 7652112 bytes (7.30 MiB), after all resolved-parent/reparse/SHA256 checks and verification that primary comparison sheets remain. Source artwork, provenance, metrics and recovery records retained. One active pack; no duplicate game extracts/builds/ZIPs/staging. Original game assets, personal saves and settings preserved.
- All 1030 installed replacements pass native DDS/worklist audits. 5731 still need art; 51 held drafts require correction. Planner and inventory refreshed. Four previously moderated gaps were not retried. The 512 MiB replacement residency cap remains; actual scene residency and fallback unverified.
- Zero game launches; no runtime/tool changes. Full overhaul and scene/platform/1080p/4K/ultrawide validation remain incomplete; near-completion gate not reached. Continue hybrid production from opaque >=2-occurrence/min128 triage cursor 815f195432810024, with specialist corrections separate. Other-device synchronization unverified.
- Pack 925,267,836 bytes (882.40 MiB / 6144 MiB). Texture workspace at audit 4,966,377,587 logical/allocated bytes (4.63 GiB); temporary output 266,365,603 bytes (254.03 MiB / 256 MiB); free disk 105,200,091,136 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-141-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 142

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `252ac6a`.
- Visually classified 32 additional opaque candidates (at least two catalog occurrences, minimum 128 pixels) through cursor 8a7b680d08a46bca. Classification now has 2,073 unique records and matching summary counts. Text/UI, weapons, characters, sky/effects and uncertain atlases remain in specialist queues; routing is not completion.
- Generated 16 sources with built-in image_gen: nine 1254x1254 and seven 1774x887. Accepted 15 offline: seven 1024x1024, two 512x512 and six 1024x512 DXT1 mipmapped replacements. All accepted sources were downsampled; no native 4K source or 4K output claimed for this batch.
- Reviewed all four compiled comparison sheets, four repeat sheets, 16 individual comparisons and seven focused crop comparisons. Re-decoded all 16 originals from catalog payloads and matched dimensions/RGBA bytes; originals and compiled drafts remain fully opaque. Preserved broad material layouts, rail/seam positions, major crack paths, yellow stripe and hardware placement. Scene UV appearance and actual adjacency remain unverified.
- Accepted industrial panel 8300f7b5b20f956f retains its stepped plates, yellow stripe and hardware; stripe-region grayscale +0.33%. Accepted vertical machine panel 8352c99c5bd22b57 retains left fasteners, staggered center rivets, right long recess and two central fasteners; center strip grayscale -14.14%, right region -10.55%. Recessed slot 83a3d6c19c4e5772 retains four corner fasteners and lower staggered strip; focused grayscale -2.58% and -5.31%.
- Accepted olive wall 820aee6b20d9c826 retains central vertical division and horizontal rails, with stronger edge definition; upper rail grayscale -7.28%, sampled lower region -18.51%, full luminance -12.93%. Mossy boards retain joint layout and moss distribution; upper-region grayscale -2.78%. Cracked plaster 84f97ffc5f3973f0 retains major crack paths; focused upper crack region +17.85%, full luminance +15.52%. Accepted full-image luminance ranges from -12.93% to +15.52%.
- Held 831ceabae6c2d679 for palette/exposure correction: four staggered hole rows remain but pale gray/brass shifted to saturated yellow-gold and dark recesses. Full luminance -18.31%; upper row grayscale -20.61%, lower region -17.93%. Removed only its two compiled candidate files after resolved-parent/reparse/SHA256 checks; original, generated source and review/provenance retained. It is not installed or counted complete.
- Repeat reviews preserve broad rhythm, but sharper edge detail increases some edge differences: gray blocks left/right MAE 15.84 to 31.24 and vertical machine panel 49.46 to 72.67. These offline comparisons do not prove scene adjacency; mapped seams and lighting require consolidated runtime validation.
- Safely recycled 61 redundant older presentation images totaling 12,843,713 bytes (12.25 MiB), after resolved-parent/reparse/SHA256 checks. Kept primary comparisons, necessary held-draft evidence, metrics, source art and provenance. Cleanup manifest records retained primary sheets.
- All 1,045 installed replacements pass native DDS and worklist audits; planner/inventory refreshed. 5,715 need artwork and 52 drafts require correction. Audited supported scope remains 6,812 unique keys / 7,147 catalog payloads, with noncatalog discovery/category/scene gaps; full overhaul unfinished.
- Zero game launches; no runtime/tool changes. Shared code retains the 512 MiB replacement residency limit; actual scene residency/fallback and Windows/Mac scene/platform/1080p/4K/ultrawide validation remain pending. Near-completion gate is not reached. Other-device synchronization unverified.
- Pack 932,616,484 bytes (889.41 MiB / 6144 MiB). Texture workspace at audit 5,033,017,015 logical/allocated bytes (4.69 GiB); temporary output 263,517,805 bytes (251.31 MiB / 256 MiB); free disk 104,795,901,952 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-142-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 143

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `8598e6f`.
- Visually classified 32 additional opaque candidates (at least two occurrences, minimum 128 pixels) through cursor 94c815a9ceddb08e. Classification now contains 2,105 unique records with matching counts. Printed clippings/labels, character faces/eyes, skies, uncertain reflective materials and lit machinery remain in specialist queues. Classification is not completion.
- Generated 16 sources with built-in image_gen: nine 1254x1254 and seven 1774x887. Accepted 15 offline: eight 1024x1024, one 512x512 and six 1024x512 DXT1 mipmapped replacements. All accepted sources were downsampled; no native 4K source or 4K output claimed for this batch.
- Reviewed four compiled comparison sheets, four repeat sheets, all 16 detail comparisons and nine focused crop comparisons. All 16 originals were independently decoded from catalog payloads and matched saved original RGBA bytes and dimensions. Originals and compiled drafts have fully opaque alpha. Scene mapping and actual adjacency remain unverified.
- Accepted fluted bronze panel 8bedb92c03a38d49 preserves four long flutes on each side, five shorter center flutes, five central fasteners, square inset and perimeter fittings. Lower-flute grayscale -4.42%, center fastener region -6.46%, full luminance +0.14%. Dark double-inset panel retains inset rectangles and rivets. Brown wood panel retains nested bevels and corner miters.
- Accepted horizontal timber 8cdac4162dd9fb22 preserves upper knots, beam divisions and dark lower section; upper grayscale -9.42%, lower -17.91% (29.44 to 24.17), full luminance -11.40%. Concrete blocks preserve mortar layout; cobbles retain diagonal boundaries and chipped lower-right region; small concrete crease remains narrow and in place.
- Accepted blue hatch 8d9dd99e73459d00 preserves four faint outlined fittings, yellow stripe, upper slot, side hinges and bottom strip; focused grayscale -8.46%. Dark elongated panel 8e0fd9e0fba378a8 preserves highlight, left vertical line and sloped inset outline with stronger grain; highlight grayscale -9.58%, lower field -22.12%, full luminance -15.66%. Scene lighting still requires validation.
- Accepted rounded vent 9083473970bf9440 retains eight horizontal ribs and top/bottom rivet rows; focused grayscale -9.90%. Hinged panel 92776b031c2c39dd preserves two horizontal strap regions, four hinge fittings, small oval fitting, perimeter rivets and right square recess; hinge-region grayscale +5.25%, oval fitting -2.71%. Blue modular panel retains upper notches, vertical divisions, handles and dense lower grille (focused grayscale -2.09%); three-group vent retains two slots per group (focused -1.46%).
- Held 91b0e9401f9209c3 for exposure correction: broad board joints remain but full luminance +26.90%, pale strips +29.97%/+29.91% and dark wood sample +24.20%. Restore original dim gray/brown exposure and softer transitions while keeping seams and fasteners. Removed only its two compiled files after resolved-parent/reparse/SHA256 checks; source PNG, original, review and provenance retained. Not installed or counted complete.
- Accepted full-image luminance ranges from -15.66% to +1.96%. Repeat patterns were reviewed; stronger edge detail increases some boundary differences, including concrete-block left/right MAE 13.83 to 27.33 and rounded vent 50.38 to 76.45. These measurements and 2x2 sheets do not prove scene adjacency or final mapped seams.
- Safely recycled 65 accepted-only older detail enlargements from batches 125-136, totaling 10,972,170 bytes (10.46 MiB), with all resolved-parent/reparse/SHA256 and retained-primary checks completed before deletion. Primary comparison sheets, authored sources, metrics, provenance and held-draft evidence remain.
- All 1,060 installed replacements pass native DDS and worklist audits; planner/inventory refreshed. 5,699 need art and 53 drafts require correction. Audited scope remains 6,812 supported unique keys / 7,147 catalog payloads, with noncatalog discovery/category/scene gaps. Full overhaul unfinished.
- Zero game launches; no runtime/tool changes. Shared code retains the 512 MiB replacement residency cap; actual scene residency/fallback and representative Windows/Mac scene/platform/1080p/4K/ultrawide validation remain pending. Near-completion gate not reached. Other-device synchronization unverified.
- Pack 940,489,422 bytes (896.92 MiB / 6144 MiB). Texture workspace at audit 5,096,896,364 logical/allocated bytes (4.75 GiB); temporary output 261,378,971 bytes (249.27 MiB / 256 MiB); free disk 104,419,393,536 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-143-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 144 corrections

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `8821d73`.
- Corrected eight held opaque drafts with built-in image generation; accepted two after offline review and retained six as unresolved. Installed/offline-approved coverage is now 1,062 of 6,812 supported unique catalog keys; 51 held and 5,699 still need artwork. This is not a completed remaster.
- Accepted 831ceabae6c2d679: four staggered perforation rows, borders and highlights retained; desaturated pale gray/brass restored. Full luminance -3.45% versus original (previous -18.31%); upper/lower focus -3.51%/-4.40%.
- Accepted 91b0e9401f9209c3: vertical board/joint placement and pale weathered strips retained. Full luminance +14.70% versus original, improved from +26.90%; pale-strip samples +16.92%/+15.42%, dark strip +10.50%. Residual brightness is documented for scene review; broad muted weathered appearance accepted offline.
- Held paired riveted plates: full luminance +30.28%, lower plate +44.95% grayscale. Five crop-guided label/marking corrections remain held for changed stroke shapes; the weapon atlas also gained an unrelated white stripe. Enlarged original crop guidance did not reliably preserve markings, so do not repeat the same full-atlas correction strategy unchanged.
- Reviewed two primary sheets, two 2x2 repeat sheets, eight full detail comparisons and eight focused comparisons. Re-decoded all eight originals from catalog payloads and verified RGBA bytes and opacity. Repeat sheets and edge metrics do not prove in-scene adjacency.
- Generated five 1254x1254 and three 1774x887 sources. The two accepted 1774x887 sources were downsampled to 1024x512 DXT1 with mipmaps; these are not native 4K textures. Source artwork, prompts, previous prompt backups and correction evidence remain private.
- Native format/mip validation passed for all 1,062 installed DDS files; worklist, hybrid planner and inventory audits passed. Removed 12 rejected compiled files after resolved-parent and SHA256 verification, preserving source artwork.
- Retired 21 redundant accepted-only detail enlargements from batches 140/141 (3,475,033 bytes); primary comparison sheets, source artwork and provenance retained. One active pack; no game copies, ZIPs or staging trees created.
- No runtime/source behavior changed and no gameplay launched. Residency limit remains 512 MiB. Final scene/category/embedded-asset coverage, 1080p/4K/ultrawide, memory and fallback validation remain pending; broader inventory gaps remain in scope.
- Pack 941,189,626 bytes (897.59 MiB / 6144 MiB). Texture workspace at audit 5,122,794,576 logical/allocated bytes (4.77 GiB); temporary output 262,415,011 bytes (250.26 MiB / 256 MiB); free disk 104,233,201,664 bytes.
- Private evidence: `review/hybrid-144-*`, canonical/versioned prompts, source PNGs and progress. Commit records progress only; other-device synchronization unverified.


## Texture overhaul issue #74: hybrid batch 145

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `b87ef83`.
- Visually triaged 32 more opaque multi-use keys, advancing this queue cursor to 9dd333792cb1534c and classification to 2,137 unique keys. Character atlases, warning text, interface graphics and uncertain light/reflection assets remain routed for individual or specialist work. Classification is not completion.
- Generated 16 material/prop replacements; accepted 14 offline: rusted metal, architectural stone, painted wood door, two rock surfaces, sci-fi panel, four-band vent, mottled corrosion, pale fastened panel, tile grid, riveted sheet, bracket panel, concrete inset and carved stone frieze. Coverage is now 1,076 installed/offline-approved of 6,812 supported unique catalog keys; 53 held and 5,683 need art.
- Reviewed four primary sheets, four 2x2 repeats, 16 full detail comparisons and nine focused comparisons. Re-decoded all 16 originals from catalog payloads and verified stored RGBA bytes and opaque alpha. Edge/repeat checks are offline diagnostics, not proof of in-scene adjacency.
- Focused review retained stone molding recess cadence, three orange hazard strokes and two five-hole clusters, four vent bands, central fastener pattern, eight-by-four tile grid, sixteen bracket sections and carved relief motifs. Accepted full-image luminance changes range -7.24% to +11.67%; stone lower molding focus is +23.41% despite full +8.84%, recorded for scene review. Tile edge MAE rises from 3.75/2.50 to 10.32/12.84; grid alignment retained, scene tiling still pending.
- Held 99d721182139c2c8: teal door hardware focus +50.43% grayscale and altered pale lower-edge pixels. Held 9a6926b7f278e627: diagonal machinery full luminance +19.34% and indicator focus +32.38%, with overemphasized pale scratches. Four rejected compiled files removed after resolved-parent and SHA256 verification; artwork and failure records retained.
- Actual generated sources: twelve 1254x1254 and four 1774x887. Installed ten 1024x1024 and four 1024x512 DXT1 textures with mipmaps, all downsampled. The held machinery trial was resampled from 1254x1254 to 2048x2048 and removed; it is not native 2K or 4K artwork.
- Native format/mip validation passed for all 1,076 installed DDS files; worklist, hybrid planner and inventory audits passed. Retired 48 redundant accepted detail enlargements/repeat sheets (11,226,077 bytes), preserving primary comparisons, failed-draft details, source artwork and provenance.
- No runtime behavior changes or gameplay launches. One active pack, 6144 MiB pack cap and 512 MiB residency limit preserved. Category/scene coverage, embedded/procedural/unclassified inventory gaps and final 1080p/4K/ultrawide, memory and fallback validation remain unfinished. No duplicate game extracts, ZIPs or staging trees created; other-device synchronization unverified.
- Pack 949,586,274 bytes (905.60 MiB / 6144 MiB). Texture workspace at audit 5,188,433,699 logical/allocated bytes (4.83 GiB); temporary output 261,155,217 bytes (249.06 MiB / 256 MiB); free disk 103,592,374,272 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-145-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 146

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `8a9da2f`.
- Visually classified the next 32 catalog keys; 2,169 unique keys are now provisionally classified. Corrected the stale classification scope string to match its records. Text-heavy labels/posters, character atlases, consoles, reflections, sky/effect and utility textures remain in specialist review; this routing does not count as completion.
- Generated 16 independent sources with built-in image generation: nine 1,254 x 1,254 and seven 1,774 x 887. Accepted 13 offline: five 1,024-square, one 512-square and seven 1,024 x 512 DXT1 replacements, all downsampled from the recorded sources. These are not native 4K assets.
- Accepted stone/concrete trim, stepped gray and blue-gray panels, fastened wall panels, riveted trim, stone tile grid, rusty grille, diagonal-riveted corrosion, dark granular surface, pale framed panel, grooved green-gray panel, industrial grille structure and cobblestones. Original palette, principal UV divisions, openings and structural patterns retained in offline review; scene appearance remains unverified.
- Held three drafts outside the active pack: a22f516f7128e3fa shadowed columns (+37.23% overall luminance), a41b635043754d5b scorched panel (+76.69%; opening-surround grayscale +114.49%), and a509aa32186892ff louver panel (+17.21%; central-frame grayscale +25.70%). Restore subdued shading/metal highlights before accepting them. All source artwork, prompts and failure evidence retained; six compiled DDS/metadata files retired after resolved-parent and SHA-256 verification.
- Reviewed four original/DDS comparison sheets, four 2 x 2 repeat sheets, all sixteen detailed comparisons and seven focused comparisons. All sixteen originals were re-decoded from authoritative catalog payloads and matched cached RGBA bytes, dimensions and opaque alpha exactly. Opaque alpha does not prove shader blend semantics.
- Accepted full-image luminance changes range from -14.14% to +2.82%. The rusty grille retains its slot pattern after detailed comparison (focus grayscale -11.02%); industrial grille focus +14.69% remains a scene-check item. Stone trim left/right edge MAE rises 9.30 to 25.68 and cobbles top/bottom 9.29 to 19.15; repeat layouts remain aligned offline but actual adjacency and seam visibility need scene validation. Dark granular surface remains -14.14% darker and visibly repeats, as its original does; review in its actual scene.
- Reclaimed 10,088,396 bytes (9.62 MiB) by retiring 152 regenerable cached original PNG exports of previously installed textures. Each was re-decoded and RGBA-verified against authoritative game data before all resolved-parent/SHA-256 checks and deletion. Game files, source artwork, review evidence, saves and settings were preserved; the retirement manifest records recovery.
- Native DDS audit passed for all 1,089 installed replacements; worklist, hybrid queue and broader asset inventory refresh passed. Coverage is 1,089 installed/offline-approved, 56 held drafts and 5,667 still needing artwork out of 6,812 supported unique keys. All 7,147 cataloged payloads are format-supported, but broader embedded/noncatalog/procedural discovery and complete category review remain unfinished.
- No game launches or runtime changes. The existing 512 MiB replacement residency limit is unchanged. Consolidated scene, memory, fallback and 1080p/4K/ultrawide validation remain deferred until the coverage/category gate; no full-remaster or campaign-completion claim.
- Pack 955,710,442 bytes (911.44 MiB / 6144 MiB). Texture workspace at audit (Windows compressed-file-size allocation measurement) 5,246,690,166 logical/allocated bytes (4.89 GiB); temporary output 259,585,122 bytes (247.56 MiB / 256 MiB); free disk 103,248,834,560 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-146-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 147 corrections

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `3996edf`.
- Completed eight focused corrections of held drafts using built-in image generation, preserving the full overhaul scope. No new visual-triage keys this batch: 2,169 provisional classifications remain; category and scene coverage are unfinished.
- Four corrections accepted offline: a22f516f7128e3fa shadowed columns, a41b635043754d5b scorched riveted panel, d437d76b03372483 dark translucent glass and 1ac9102ec850a11f semi-transparent tile grid. Installed total is 1,093; held drafts fall from 56 to 52, with 5,667 keys still needing art. Four older draft records moved to resolved history (54 resolved total); all earlier prompts and source versions retained.
- All eight generated source sizes recorded: seven 1,254 x 1,254 and one 1,774 x 887. Four accepted assets are downsampled to 1,024 x 1,024 (two DXT1, two DXT5 with verified original alpha). No native 4K claim; source detail and resampling remain explicit.
- Compared two full original/DDS sheets, two 2x2 repeat sheets, all eight details, three focused grayscale comparisons and five original-alpha composites on both dark and light backgrounds. Re-decoded all eight originals from authoritative catalog payloads and verified exact RGBA bytes and dimensions before authoring.
- Column luminance is now -9.76% versus original (previous +37.23%); focus grayscale -5.91%. Scorched panel is +7.35% (previous +76.69%), upper focus +6.75%, opening-surround +20.31% (previous +114.49%). Structural divisions, black opening and subdued hardware retained offline; residual local brightness remains a final scene-check item.
- Glass original-alpha-weighted luminance improves from the previous 26.14 to 12.08 versus original 10.50 (+15.09%); four-pane layout and opacity pattern retained. Tile-grid weighted luminance 108.48 versus original 113.07 (-4.06%); the full-canvas RGB coverage failure is resolved. Neither accepted alpha source has generated near-zero alpha overlapping original opaque pixels. DXT5 original-mask mean absolute alpha error is 0.61/255 for glass and 1.27/255 for tiles (max 9/13); original-alpha preservation includes bilinear resampling and lossy DDS compression, not bit-exact alpha.
- Held the louver correction because recesses became too dark (-25.28% left, -27.75% right grayscale); center frame is closer at -9.66%. Held bronze frame for +28.84% alpha-weighted brightness and remaining cutout contamination. The plaster/lath and brick patches also remain held for colored feathered-edge RGB after original-alpha compilation.
- Diagnosed colored edges at compiled resolution against equally resized original references: saturated red/yellow pixels above alpha 64 are 38 for bronze frame, 431 for plaster/lath and 28 for brick patch; all original references have zero under the same diagnostic. Counts above alpha 8 are 38/1,794/628. These diagnostics support the visual fringe finding, not a generic automatic acceptance threshold. Explicit edge-preservation prompts still failed; do not repeat this full-atlas transparent regeneration strategy unchanged.
- Removed eight rejected trial DDS/metadata files only after all resolved-parent, byte-size and SHA-256 checks. Kept source artwork, canonical/versioned prompts, original-alpha provenance and failure comparisons. No duplicate game extracts, ZIPs, staging trees or game launches.
- Native DDS validation passed for all 1,093 active replacements. Worklist, hybrid queue and broader inventory refreshed; catalog scope remains 6,812 supported unique keys/7,147 payloads with embedded/noncatalog/procedural discovery gaps. Pack cap 6,144 MiB and replacement residency 512 MiB unchanged. Consolidated scenes, memory, fallback and 1080p/4K/ultrawide testing remain deferred; no full remaster or campaign completion claim.
- Pack 959,907,981 bytes (915.44 MiB / 6144 MiB). Texture workspace at audit (Windows compressed-file-size allocation measurement) 5,276,122,854 logical/allocated bytes (4.91 GiB); temporary output 264,002,089 bytes (251.77 MiB / 256 MiB); free disk 102,210,797,568 bytes.
- Private evidence: `review/hybrid-147-*`, canonical/versioned prompts, source PNGs and progress. Commit records progress only; other-device synchronization unverified.


## Texture overhaul issue #74: hybrid batch 148

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `428fa92`.
- Visually classified 32 additional opaque textures, reaching 2201 distinct reviewed keys. Character, reflection, effect and detailed atlas candidates remain separate specialist work; routing does not count as completion.
- Eight built-in image-generation edits produced seven offline-approved replacements and one held bronze-trim draft. Installed coverage is 1100/6812 supported keys (16.15%); 5659 need art and 53 drafts require correction. Broader embedded/procedural discovery and category/scene coverage remain unfinished.
- Actual generated sources: five 1254x1254 and three 1774x887 images. Accepted DDS outputs: four 1024x1024, two 1024x512 and one 512x512, all DXT1 with complete mip chains. Resampling is recorded; these are not native 4K sources.
- Compared original and compiled primary/repeated sheets plus targeted detail views. Preserved plaster/baseboard divisions, rock massing, three board seams, stone channels and rivet layout. Riveted-panel top/bottom row counts remain 16 each; square-plate edge and interior rivet placement retained. Accepted brightness change ranges from -14.37% to +13.32%; wider rust-panel top strip is -20.70%, requiring scene review alongside the brighter boards and rock contrast.
- Held b0b30cafb2de7114: lower bronze band is 25.15% darker and browner at original scale; full texture is 20.48% darker. Its compiled DDS/metadata were retired with resolved-parent, size and SHA256 checks; source and correction evidence retained.
- Original catalog RGBA verified for all eight inputs. All 1100 installed DDS files passed native loader-format checks and worklist hashes/dimensions/mip-chain/metadata checks; planner and inventory refreshed. Opaque alpha remains 255. Tiling sheets preserve broad edge layout, but actual scene adjacency remains untested.
- Retired 14 verified regenerable original PNG cache files (1017807 bytes) and 31 redundant accepted detail enlargements (4635933 bytes), retaining original game data, source art, provenance, primary comparisons, metrics and recovery manifests.
- Zero game launches. The 90% plus all-categories-reviewed runtime gate remains unmet; 512 MiB runtime residency limit unchanged. No runtime source changes or other-device verification.
- Pack 963,582,015 bytes (918.94 MiB / 6144 MiB). Texture workspace at audit (Windows compressed-file-size allocation measurement) 5,309,431,252 logical/allocated bytes (4.94 GiB); temporary output 264,565,209 bytes (252.31 MiB / 256 MiB); free disk 102,021,873,664 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-148-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 149

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `8cf672d`.
- Worked eight previously classified specialist candidates from hybrid-148: carved wood, stained glass, ornate armor, character equipment, two instrument atlases, keyboard panels and a stone window facade. Classification remains 2201 distinct keys; other categories and scenes remain unfinished.
- Six new replacements passed offline review, with two held drafts. Installed coverage is 1106/6812 supported keys (16.24%); 5651 still need artwork and 55 drafts require correction. Supported catalog remains 7147 payloads; broader embedded/procedural discovery gaps remain explicit.
- Actual built-in generated sources: six 1254x1254 and two 1774x887. Accepted DXT1 outputs are four 1024x1024 and two 1024x512, all resampled with complete mip chains. Held ornate armor trial used a 2048x2048 output resampled from 1254x1254; it was removed and is not native 2K artwork. No native 4K claim.
- Primary, repeated and focused original-scale comparisons checked carved motif placement, stained-glass leading and colored regions, character face/eye/lip placement and eye emblems, equipment island silhouettes, yellow/orange cues, label/photo composition, keyboard grid and facade geometry. Opaque alpha remains 255 for all eight sources and trial DDS files.
- Keyboard diagnostic at original-scale rows 42, 46, 50 and 54 found identical bright-run boundaries to the original at threshold 60. Bottom-row threshold differences reflect local shading and remain covered by manual review; this narrow scan is not proof of whole-atlas equivalence. The 3-column by 5-row side keypad and main grid were reviewed visually.
- Accepted mean-brightness changes range from -7.53% to +10.75%. Stone-facade dentil strip is +23.59% and window surround +21.41%; geometry and dark aperture remain aligned, with those brighter highlights explicitly queued for scene review. Character eye/lip and emblem regions remain aligned offline; model mapping and all scene appearances are untested.
- Held a9c600b2a328659c for altered medallion engraving and increased metallic highlights (full +16.57%, right ornament +19.87%). Held aa71b08852505897 for reinterpreting the right instrument inset as a circular bright-rimmed opening; preserve the original inner contour before installation. Four failed DDS/metadata files retired with resolved-parent, size and SHA256 checks; private sources and evidence retained.
- All eight input originals were re-decoded from the catalog and matched byte-for-byte RGBA. All 1106 installed DDS files passed native format and worklist hash/dimension/mip/metadata checks. Refreshed inventory and hybrid plan. No gameplay launch or runtime code change; 512 MiB residency unchanged and the 90% plus all-categories-reviewed runtime gate remains unmet.
- Retired ten redundant repeat-review sheets from batches 144-146 (4237816 bytes), keeping primary comparisons, metrics, source art and provenance. Preflight storage measured separately from pack size with Windows GetCompressedFileSizeW. Artwork remains private; no other-device verification.
- Pack 967,080,715 bytes (922.28 MiB / 6144 MiB). Texture workspace at audit (Windows compressed-file-size allocation measurement) 5,338,372,329 logical/allocated bytes (4.97 GiB); temporary output 264,912,071 bytes (252.64 MiB / 256 MiB); free disk 101,850,939,392 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-149-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: inventory batch 150

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `fd1782c`.
- Reviewed 64 additional originals: 32 small assets and 32 single-use 128-256px assets. Classification now covers 2265 distinct keys. The new set includes 18 interface, 8 effect, 7 prop and 4 character records plus explicitly uncertain/utility, environment, reflection, weapon and alpha-bearing cases. Classification is not artwork completion.
- New routing keeps reticles, skull/status icons, glow/noise/gradient utilities, weapon-package text, transparent diagrams, character faces and cutout surfaces out of ordinary material enhancement. Ambiguous identity/function remains explicit; functional flat-color and alignment-template assets are not silently exempted or counted as complete.
- Checked exact decoded base-level RGBA (dimensions and alpha included) for all 6812 supported keys after verifying source payload SHA256. Found 6811 distinct decoded images: only keys 07668686f216d3d9 and e79e131a7d27d3d9 match. Both remain uninstalled; their encoded DXT1 base data differs. No runtime keys were collapsed and no artwork reuse or extra coverage was claimed. Mip/per-use semantics remain outside this equality audit.
- Retired 72 original-only triage sheets from older batches after re-decoding 576 corresponding catalog keys and checking available cached exports and recovery hashes. Reclaimed 9797547 bytes (9.34 MiB). All paths were resolved under review and size/SHA256 checked before deletion; selections, classifications, game assets, source artwork and comparative reviews remain.
- Worklist hash/dimension/mip/metadata checks, hybrid queue and full inventory refreshed. Queue now has 37 material-batch candidates, 432 individual-review candidates, 649 specialist-review candidates, 4533 awaiting visual triage, 55 corrections and 1106 installed replacements.
- No new artwork or pack changes in this inventory pass. Installed/offline-approved coverage remains 1106/6812 (16.24%); 5651 keys need art and 55 drafts need correction. Catalog has 7147 supported payloads, but 48572 unclassified archive entries and broader embedded/procedural coverage remain open.
- Zero gameplay launches. Native loader-format result from hybrid-149 remains the latest native audit of this unchanged pack; this pass did not rerun it. The 90% plus all-categories-reviewed gate is unmet; 6144 MiB pack budget and 512 MiB runtime residency remain unchanged. Other-device synchronization unverified.
- Pack 967,080,715 bytes (922.28 MiB / 6144 MiB). Texture workspace at audit 5,331,433,168 logical bytes and 5,331,433,168 Windows GetCompressedFileSizeW allocated bytes (4.97 GiB); temporary output 257,921,010 bytes (245.97 MiB / 256 MiB); free disk 101,882,597,376 bytes.
- Private evidence: `review/hybrid-150-*`, visual classification, inventory and progress. Commit records progress only; no game or artwork files published.


## Texture overhaul issue #74: hybrid batch 151

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `3fbea3d`.
- Continued the in-progress hybrid-151 batch after revalidating the clean #74 branch at 3fbea3d. Previous texture work made progress by producing 11 drafts; reused those existing outputs without regenerating or recompiling them.
- Twelve built-in image-generation attempts yielded 11 sources: eight 1254x1254, two 1774x887 and one 887x1774. All actual dimensions and resampling are retained in private prompts and pack metadata. No native-4K claim: accepted output is three 1024x1024, three 512x512, two 1024x512 and one 512x1024 DXT1 texture with full mip chains.
- Accepted nine offline: pale peeling plaster 024df14b1fa8aa42, paired cross-grooved shields 02793f8d46ab7f1c, checker ribbed metal 02b7d6140f042067, paired lockers 02cd8fdee2fcf669, olive equipment pouch 02d03568bd5ca1fe, rusted narrow strip 0445f59aad76ba2d, blue-green equipment atlas 0591e288fb55726f, industrial hazard-striped door 05c642e63fb33576, and aged green ribbed-side panel 05dba64c7d9dbf5f.
- Reviewed all 11 original/compiled primary comparisons, all three 2x2 repeat sheets, all 11 original-scale focus comparisons, and the control-panel detail. Repetition checks do not establish actual scene adjacency. Preserved shield orientations and crosses, alternating rib grid, four vents per locker, pouch strap/snaps, dark strip stain, equipment shadow islands, two rows of five door holes, hazard direction and hinge layout. The hazard-strip region is +21.14% mean grayscale with unchanged warning direction; require scene review of contrast. Ribbed tile -12.55% and green panel -12.99% overall luminance retain the original broad value pattern; runtime appearance remains pending.
- Held two drafts: 01b143734763cc7e studded door is +27.10% overall luminance and +49.04% grayscale in the hinge/wood region; 0386c56fd58df6b4 control panel rewrites digits and switch symbols. Removed only their four trial DDS/metadata files after checking every resolved parent, byte size and SHA-256. Retained generated source art, prompts and review evidence. Held drafts count as incomplete.
- Character face 00c739bc487633f6 remains unfinished: image tool rejected the input (moderation_blocked, request 6ee2b427-f653-405e-a11c-b109b99cc151). No retry or alternative route; no replacement or completion credit.
- All 11 reviewed originals match verified catalog payloads pixel-for-pixel as decoded RGBA and have fully opaque alpha. All 1115 installed DDS files passed the native format audit; worklist, hybrid planner and full inventory refreshed successfully with hashes, dimensions, formats, mip chains and metadata checks.
- Coverage: 1115 installed/offline-approved of 6812 supported unique catalog keys (16.37%), 57 held drafts, 5640 needing artwork; 7147 catalog payloads and zero unsupported catalog payloads. Broader embedded/non-texture archive classification remains unfinished. Visual classification remains 2265 keys; existing exact-base-RGBA duplicate audit is not proof of mip or per-use equivalence.
- No gameplay launch. Full scene, transparency, fallback, memory/residency and 1080p/4K/ultrawide runtime validation remain pending; 512 MiB residency cap is unchanged. Continue bounded hybrid production and focused corrections; all categories and the >=90% offline gate remain required before consolidated playtesting.
- Pack 970,755,895 bytes (925.78 MiB / 6144 MiB). Texture workspace at audit (Windows compressed-file-size allocation measurement) 5,375,083,287 logical/allocated bytes (5.01 GiB); temporary output 263,004,525 bytes (250.82 MiB / 256 MiB); free disk 101,656,809,472 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-151-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 152

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `4002735`.
- Previous goal turn made progress: hybrid-151 installed nine reviewed replacements and passed the all-pack audit. Revalidated clean #74 source at 4002735 before hybrid-152.
- Produced 12 independent built-in image-generation drafts grouped around stone/concrete; inspected every original and alpha first. Nine simple materials used the material lane; stair-edge, carved wall and cliff atlases received individual geometry constraints. Eleven actual source images are 1254x1254 and one is 1774x887. All sources and prompts remain private, with actual dimensions and resampling recorded.
- Accepted eight offline: pale green fractured rock 0abdef281e8a4c85, mossy fractured rock 106cc2d78485a7bd, stone stair-edge atlas 3875b01d1d04430d, mottled gray stone 413b2c0a7514694c, paired concrete slabs 453437e7e84deff9, craggy rock 45a00ba2405827e4, beige stone with shading transition 476c90b520c5cd21 and pale speckled stone 50cae08b2f38b57e. Seven 1024x1024 and one 512x512 DXT1 outputs with full mip chains; all downsampled from generated sources, not native 4K.
- Reviewed all 12 primary original/compiled comparisons and all 12 original-scale focus comparisons, plus all three 2x2 repetition sheets. Preserved stair step layout, gold edging and baked light bands, concrete slab joint locations, large rock fractures/cavities and the beige horizontal shading transition. Repeat sheets are offline diagnostics, not proof of scene adjacency. Green fractured rock is -15.48% overall luminance and -18.53% grayscale in the lower region; original green identity and fracture pattern remain, with contrast flagged for scene validation. Mossy rock is -11.78%; other accepted full-image changes range from -3.84% to +7.26%.
- Held four: af0f95b675bd46ea charcoal rock is -29.43% overall luminance; 0af9d3b7076524b4 rewrites a rectilinear central band into mineral striations; 489d0f5a6c76f884 carved wall is +28.81% overall and about +30% across ornament/middle shading; 6b5a16e136fd46da cliff is +24.29% overall and +58.14% grayscale at the upper silhouette region. Their eight DDS/metadata trial files were removed only after all resolved-parent, size and SHA256 checks; source artwork and evidence remain for correction.
- Reclassified 0af9d3b7076524b4 from environment to material_uncertain based on the original-scale central-band review. Material identity/scene function remain unresolved. Classification still covers 2265 unique keys; this is a correction to an existing record, not additional coverage.
- Before production, verified 325 cached original PNG exports against freshly decoded catalog RGBA and source-payload SHA256, then safely retired those regenerable exports with a resolved-parent/hash manifest. Reclaimed 12756916 bytes (12.17 MiB) before exporting current references as needed. Original game archives, catalog, authored source art and primary comparison evidence remain untouched.
- All 12 current references match verified catalog payloads pixel-for-pixel as RGBA, with opaque alpha. All 1123 installed DDS files passed the native format audit; refreshed worklist hashes/dimensions/mips/metadata, hybrid queue and full inventory passed.
- Coverage is 1123/6812 supported catalog keys (16.49%), with 61 held drafts and 5628 needing artwork. Catalog has 7147 supported payloads; broader embedded/non-texture discovery remains unfinished. No new moderation failures this batch; previously blocked keys remain unfinished and were not retried.
- Zero game launches. All scene-specific visual, transparency, fallback, memory and render-mode validation remains pending. Keep the 6144 MiB pack budget, 512 MiB residency limit and 256 MiB temporary budget. The >=90% offline coverage and all-categories-reviewed gate remains unmet. Other-device synchronization unverified.
- Pack 975,828,597 bytes (930.62 MiB / 6144 MiB). Texture workspace at audit (Windows compressed-file-size allocation measurement) 5,421,159,797 logical/allocated bytes (5.05 GiB); temporary output 258,664,450 bytes (246.68 MiB / 256 MiB); free disk 101,182,087,168 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-152-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 153

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `aa2cac5`.
- Previous goal turn made progress: hybrid-152 installed eight reviewed replacements, corrected a classification and recycled verified original-image cache exports. Revalidated the clean #74 branch at aa2cac5 before hybrid-153; checked Windows allocated/logical storage and free space before generation.
- Inspected eight opaque character/weapon candidates and attempted eight independent built-in image-generation edits. Seven sources were produced, all 1254x1254; compiled output is 1024x1024 DXT1 with full mip chains. Actual source sizes and downsampling are retained in private prompts and metadata; no native-4K claim.
- Accepted four offline: clothing/equipment atlas 04ccc072c089488b, white-haired character/equipment atlas 46a776963e8eb294, uniform/face atlas 62bfdc97a80cad04 and creature clothing/skin atlas 2ade1756aaffab73. Their overall luminance changes are -4.41%, +0.88%, -6.76% and -5.49% respectively.
- Reviewed all seven primary comparisons and all seven original-scale focus comparisons, plus both 2x2 repeat sheets. Focus checks covered garment seams and flat unused fields, rotated face/lip/eyelid/ear features, hairline, chest emblems, the creature eye and existing tattoo/skin marks. Original-scale features and broad UV layout passed offline review; model-specific identity, seams and scene appearance remain unverified. Repeat diagnostics do not establish actual adjacency on a model.
- Held three weapon drafts: 0d3d07703d111e1b changes inverted stamp glyphs/spacing and darkens the label-band grayscale by 25.78%; 0f33916275785636 changes subtle stamped text into different raised-looking strokes; 578e13e21ff348a7 turns small rectilinear grille openings into larger round holes and darkens the lower-right reflective region by 28.30%. Retired six trial DDS/metadata files only after all resolved-parent, size and SHA256 checks. Their authored sources and comparisons remain for correction.
- Reclassified 04ccc072c089488b from weapon to character_or_prop after inspecting its garment seams and clothing islands. Character/equipment association remains provisional until scene/model usage is confirmed. Classification still covers 2265 distinct keys; no new coverage claimed from this correction.
- Partial face 26588bddfa0086d5 remains unfinished: image generation rejected output with violence moderation category, request 79e0fdb9-dd63-47dc-9f77-d30f9c5ba358. No retry or alternate route; no artwork installed and no completion credit. Other previously moderated keys were excluded from selection.
- All seven generated-asset references were re-decoded from catalog payloads and verified pixel-for-pixel as RGBA, with fully opaque alpha. All 1127 installed DDS files passed native format checks. Refreshed worklist hashes/dimensions/mips/metadata, hybrid queue and full inventory passed.
- Coverage is 1127/6812 supported catalog keys (16.54%), with 64 held drafts and 5621 needing artwork. Catalog includes 7147 supported payloads; broader embedded/non-texture discovery and category review remain unfinished. Source artwork, original game files, settings and saves remain private.
- No gameplay launch. Consolidated model/scene, transparency, memory/residency, fallback and 1080p/4K/ultrawide validation remains pending. The >=90% offline coverage plus all-categories-reviewed gate is unmet; 6144 MiB pack, 512 MiB residency and 256 MiB temporary limits remain unchanged. No cleanup was needed during this batch. Other-device synchronization remains unverified.
- Pack 978,627,093 bytes (933.29 MiB / 6144 MiB). Texture workspace at audit (Windows compressed-file-size allocation measurement) 5,449,537,345 logical/allocated bytes (5.08 GiB); temporary output 262,547,983 bytes (250.39 MiB / 256 MiB); free disk 101,033,865,216 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-153-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: inventory batch 154

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `83a4d50`.
- Reviewed 64 additional originals: 32 small frequently reused assets and 32 single-use 128-256px assets. Classification now covers 2329 distinct keys. Catalog payload and decoded RGBA provenance remain in the selection manifest; classification is not artwork completion.
- New classifications: {'interface': 7, 'material_uncertain': 5, 'effect_or_utility': 6, 'prop_or_effect': 2, 'prop': 13, 'reflection': 1, 'effect': 5, 'character_or_prop': 2, 'environment': 11, 'interface_or_utility': 3, 'prop_or_weapon': 2, 'weapon': 2, 'character': 3, 'interface_or_prop': 1, 'environment_alpha': 1}. Warning/sign text, ammunition/status icons, flat-color utility tiles, gradients, fire, reflections, scene images and identity-sensitive character/weapon atlases retain specialist or individual review. Uncertain functions and mappings remain explicit; no functional assets were exempted or counted complete.
- Retired 12 redundant older repeat sheets from batches 147, 148, 149, 151 and 152, reclaiming 5622171 bytes (5.36 MiB). The cleanup manifest records resolved-parent and SHA256 checks; primary comparisons, metrics, selections, source artwork and game assets remain. No additional deletion during finalization.
- Worklist hash/dimension/mip/metadata checks, hybrid queue and archive inventory refreshed. Queue: {'batch_candidate': 33, 'correction': 64, 'individual_review': 443, 'installed': 1127, 'specialist_review': 676, 'visual_triage': 4469}. Broad discovery remains open: 48572 unclassified archive entries, plus unresolved embedded/other encoding and procedural coverage.
- No new artwork or pack changes in this inventory pass. Installed/offline-approved coverage remains 1127/6812 (16.54%); 5621 keys need art and 64 drafts need correction. The catalog contains 7147 supported payloads; broader coverage is not complete.
- Zero gameplay launches. Hybrid-153 remains the latest native loader-format audit of this unchanged pack; it was not rerun this pass. The 90% plus all-categories-reviewed gate remains unmet. Pack budget 6144 MiB and runtime residency limit 512 MiB remain unchanged. Other-device synchronization and runtime scenes remain unverified.
- Pack 978,627,093 bytes (933.29 MiB / 6144 MiB). Texture workspace at audit 5,447,268,442 logical bytes and 5,447,268,442 Windows GetCompressedFileSizeW allocated bytes (5.07 GiB); temporary output 260,225,779 bytes (248.17 MiB / 256 MiB); free disk 101,049,389,056 bytes.
- Private evidence: `review/hybrid-154-*`, visual classification, inventory and progress. Commit records progress only; no artwork/game files published. No runtime or platform implementation change; Mac adaptation is not needed for this inventory-only record, and Mac validation remains pending.


## Texture overhaul issue #74: hybrid batch 155

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `a82d269`.
- Produced eight independent environment-material edits with built-in image generation from catalog-verified originals. Seven accepted offline, one held for correction; no generation failures. Classified inventory remains 2329 distinct keys. Previous goal turn made inventory and cleanup progress; this pass advances installed artwork.
- Generated source sizes: five 1254x1254 squares and three 1774x887 wide textures. Compiler resampled the accepted sources to five 1024x1024 and two 1024x512 DXT1 textures with full mip chains. These are generated-source dimensions, not native 4K artwork. Canonical prompts, source PNGs and resampling metadata remain private.
- Accepted 07127c0fddc10231 tan brick wall (-4.63% mean luminance), 0a10913ab7032fc6 pink-gray fractured rock (+2.95%), 0a5d646a70bc00bb corrugated rusty metal (-1.98%), 0a66b4cf78594934 gray stone slabs (+2.48%), 0acd29e96a5e4753 blue-gray brick strip (-7.56%), 0b1ba5aaaf22a224 gray-green wall with rusty top trim (-3.45%), and 0811570442da97c9 rubble/pale-strip atlas (-1.62%). Preserved brick courses, main cracks, rib spacing, slab joints, pale horizontal band, upper trim border and large-rubble/small-gravel regions in offline comparisons.
- Reviewed both primary before/after sheets, both 2x2 repeat sheets and eight focused crop sheets. Focus checks: brick recess -7.25%, main rock crack +1.18%, corrugated upper ribs -0.22%, stone crack region +6.07%, blue-brick pale band -7.38%, wall top trim +1.48%, rubble pale strip -5.69%. Fully opaque alpha retained. Existing repeated trim/atlas boundaries are retained; repetition diagnostics are not proof of actual scene adjacency. Finer grain raises some edge-difference metrics; scene checks remain required.
- Held 0bbbff4e2abf27e5 weathered metal strip: overall luminance -19.13%, pale band -18.12%, bottom trim -17.55%, with darker rusty color and stronger repeated vertical edge lines. Removed only its compiled DDS and metadata after all resolved-parent, non-reparse, size and SHA256 checks. Source artwork, prompt and review evidence remain for correction; original game texture remains fallback.
- Re-decoded all eight originals from catalog payloads, verified SHA256 and exact RGBA against reference exports. Native loader-format audit passed all 1134 installed DDS files; refreshed worklist hash/dimension/mip/metadata checks, hybrid queue and full inventory. Coverage is 1134/6812 supported keys (16.65%), with 5613 needing art and 65 held drafts. The 7147 catalog payloads are supported, but broader embedded/procedural discovery and 48572 unclassified archive entries remain open.
- No gameplay launch. Consolidated scene/model, transparency, residency, fallback and 1080p/4K/ultrawide validation remains pending. The >=90% offline coverage plus all-categories-reviewed gate is unmet; 6144 MiB pack, 512 MiB residency and 256 MiB temporary limits remain unchanged. No additional review-cache cleanup needed this batch. No runtime/platform implementation change; other-device artwork synchronization and scene validation remain unverified.
- Pack 982,825,417 bytes (937.30 MiB / 6144 MiB). Texture workspace at audit (Windows compressed-file-size allocation measurement) 5,486,777,071 logical/allocated bytes (5.11 GiB); temporary output 264,378,805 bytes (252.13 MiB / 256 MiB); free disk 100,815,908,864 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-155-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 156

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `d67214b`.
- Produced eight independent built-in image edits of catalog-verified panel, door and trim originals. Five accepted offline, three held for correction. Seven drafts compiled for review; the eighth was held before compilation because its source aspect and padding violated the original UV layout. Classified inventory remains 2329 keys.
- Actual generated sources: five 1254x1254 squares, two 1774x887 wide images, and one 2170x725 trim image. Accepted sources were resampled to four 1024x1024 and one 512x256 DXT1 files with full mip chains. Small gray panel was compiled at 256x128 only for review, then removed. The incorrect trim was never resampled or installed. No source is described as native 4K art; prompts, source PNGs and resampling records remain private.
- Accepted 063fea74536bf822 framed metal/brick panel (+2.04% mean luminance), 06f590f98c394c16 riveted gray panel with vent (+0.59%), 0710f88ee0df73f2 brown riveted plates (-4.80%), 077e856cfa766f39 ornate metal door (+6.82%) and 599750d3162e65c4 radial-braced panel (-12.80%). Primary and focused comparisons preserve frame/brace placement, rivet rows, vent, ornamental scrollwork/flower, side hardware and central boss/ribs. The radial panel remains darker and needs scene-context contrast review.
- Reviewed two primary comparison sheets, two 2x2 repeat sheets and seven focused crop sheets. Focus luminance changes: framed panel -0.02%, vent -8.52%, gray-panel rivet strip -0.50%, brown rivet strip -3.70%, ornate central decoration +3.77% and side detail +6.10%, radial boss -11.34%. Fully opaque alpha retained. Repeated panel/door boundaries are intentional source features; offline repeats do not prove scene adjacency or full runtime fidelity.
- Held 0b3b536722b9cbcf wooden door: +78.89% overall luminance and +92.66% central focus, substantially lifting original dark stained shading. Held 1d9cc085d9626931 small gray panel: inset border shifts at right/lower UV landmarks and original broad tonal bands become mottling (-12.85% mean luminance). Held 78e78637715da38f trim: generated 2170x725 with black padding instead of an edge-to-edge 4:1 layout. Original game textures remain fallback; none count as complete.
- Removed only the two failed compiled drafts and their metadata (four files) after checking all resolved parents, non-reparse attributes, sizes and SHA256 hashes. Retained all three source drafts/prompts/review evidence. Before production, also retired 30 redundant full-texture detail views from completed batches 151-153, reclaiming 5389170 bytes (5.14 MiB); original/DDS primary sheets, focus crops, metrics, selections, source artwork and game data remain.
- All eight references were catalog-verified at preflight; seven compiled candidates were re-decoded and exact-RGBA checked again during final review. Native loader-format audit passed all 1139 installed DDS files; worklist hash/dimension/mip/metadata checks, hybrid queue and inventory passed. Installed/offline-approved coverage is 1139/6812 supported keys (16.72%); 5605 need art and 68 drafts need correction. Catalog has 7147 supported payloads, while 48572 unclassified archive entries and broader embedded/procedural discovery remain open.
- No gameplay launch. Consolidated scene/model, transparency, residency, fallback and 1080p/4K/ultrawide validation remains pending; >=90% offline coverage plus all-categories-reviewed gate is unmet. Pack budget 6144 MiB, residency limit 512 MiB and temporary limit 256 MiB remain unchanged. No runtime/platform implementation change; Mac scene validation and other-device artwork synchronization remain unverified.
- Pack 985,711,869 bytes (940.05 MiB / 6144 MiB). Texture workspace at audit (Windows compressed-file-size allocation measurement) 5,514,984,490 logical/allocated bytes (5.14 GiB); temporary output 262,153,667 bytes (250.01 MiB / 256 MiB); free disk 100,612,423,680 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-156-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: inventory batch 157

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `6ec96cc`.
- Reviewed 64 additional catalog-verified originals: 32 small reused assets and 32 single-use 128-256px assets. Classification now covers 2393 distinct keys. Each selection retains catalog payload SHA256, decoded RGBA hash and alpha range. Classification is routing evidence, not completed artwork.
- New classifications: {'prop': 13, 'effect': 4, 'prop_or_weapon': 4, 'interface_or_utility': 5, 'environment': 13, 'character': 3, 'interface': 8, 'prop_or_effect': 1, 'character_or_prop': 3, 'environment_alpha': 2, 'material_uncertain': 2, 'reflection_or_effect': 1, 'effect_or_utility': 2, 'interface_or_effect': 1, 'reflection_or_environment': 1, 'interface_or_prop': 1}. Ordinary stone, metal and wood surfaces expand the production queue; warning stencils, weapon outlines, targeting symbols, menu text, monitor diagrams, character atlases, light/glow masks and flat-color or gradient utilities remain under specialist/individual review. Ambiguous identity and function remain explicit.
- Refreshed worklist hash/dimension/mip/metadata checks, hybrid queue and archive inventory. Queue: {'batch_candidate': 36, 'correction': 68, 'individual_review': 467, 'installed': 1139, 'specialist_review': 697, 'visual_triage': 4405}. The catalog contains 6812 supported keys and 7147 supported payloads, but 48572 unclassified archive entries and broader embedded/other-encoding/procedural discovery remain open.
- No new artwork or pack changes in this inventory pass. Installed/offline-approved coverage remains 1139/6812 (16.72%); 5605 keys need art and 68 drafts need correction. Native loader-format audit from hybrid-156 remains applicable to the unchanged pack and was not rerun.
- No gameplay launch or cleanup this pass. Temporary original/review output remained below 256 MiB, one active pack remains below 6144 MiB, and the 512 MiB runtime residency limit is unchanged. The >=90% offline coverage plus all-categories-reviewed gate is unmet. Scene, hardware and other-device validation remain pending.
- Pack 985,711,869 bytes (940.05 MiB / 6144 MiB). Texture workspace at audit 5,517,976,124 logical bytes and 5,517,976,124 Windows GetCompressedFileSizeW allocated bytes (5.14 GiB); temporary output 265,094,308 bytes (252.81 MiB / 256 MiB); free disk 100,597,227,520 bytes.
- Private evidence: `review/hybrid-157-*`, visual classification, inventory and progress. Commit records progress only; no artwork/game files published. No runtime or platform implementation change; Mac adaptation is not needed for this inventory-only record, and Mac validation remains pending.


## Texture overhaul issue #74: hybrid batch 158

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `3831f5f`.
- Produced and accepted eight independent environment-material edits using built-in image generation from catalog-verified originals. No held drafts or generation failures in this batch. Classification remains 2393 reviewed keys; previous inventory pass expanded this production queue.
- All eight generated source images are 1254x1254. Compiler resampled five to 1024x1024 and three to 512x512 DXT1 replacements with full mip chains. Actual source sizes and resampling are recorded; these are not native 4K images. Authored PNGs, exact prompts and provenance remain private.
- Accepted 0cb61dc37f34da08 dark mossy rock (-4.97% mean luminance), 0cfa3e647a8face5 pale crystalline rock (+3.77%), 0d611601954847ec rusty green-gray panel (-13.29%), 0d7ad5091e4e1435 pale metal with recessed seam (+0.71%), 0dc94ff187685bf6 paired speckled metal strips (-2.78%), 0e4b4080f66af995 cracked plaster (-3.25%), 0efdb06a1c1bb067 mossy stone brick wall (-2.84%) and 1007e2bbd4a0a45a shaded metal surface (-0.99%).
- Reviewed both primary before/after sheets, both 2x2 repeat sheets and eight focused crop sheets. Major rock fissures, panel seam/notch, lower trim, dents, black vertical gap, plaster crack network, brick courses and broad diagonal shading remain aligned offline. Opaque alpha retained. The rusty panel is darker and more brown in the draft than its original; retained crack/trim layout passes offline review but contrast/color must be checked in its scene. Fine detail and edge contrast changes remain subject to runtime appearance review.
- Focused luminance changes: mossy-rock top -4.13% and bottom -6.19%; pale rock fracture +3.68%; rusty panel lower trim -9.97%; pale metal seam +3.63% and dents -0.22%; black-gap strip -2.81%; plaster cracks -4.88%; mossy brick top/bottom approximately -1.86%; shaded metal dark region +5.90% and pale region -4.49%. Repetition tests retain original trim/shading boundaries and do not prove actual scene adjacency.
- Before generation retired 19 redundant review images: 15 full-texture detail views from completed batches 155-156 and four repeat sheets from 153/155. Reclaimed 4394300 bytes (4.19 MiB), after all resolved-parent, non-reparse, size and SHA256 checks. Primary comparisons, focus crops, metrics, selections, source artwork, game assets and user data remain; no compiled draft was removed this batch.
- Re-decoded all eight originals from catalog payloads and verified exact reference RGBA and payload SHA256. Native loader-format audit passed all 1147 installed DDS files; refreshed worklist hash/dimension/mip/metadata checks, hybrid queue and archive inventory. Installed/offline-approved coverage is 1147/6812 supported keys (16.84%); 5597 need art and 68 drafts need correction. The catalog has 7147 supported payloads; 48572 unclassified archive entries and broader embedded/procedural discovery remain open.
- No gameplay launch. Consolidated scene/model, transparency, memory/residency, fallback and 1080p/4K/ultrawide validation remains pending. The >=90% offline coverage plus all-categories-reviewed gate is unmet. Pack budget 6144 MiB, residency limit 512 MiB and temporary limit 256 MiB remain unchanged. No runtime/platform implementation change; Mac scene validation and other-device artwork synchronization remain unverified.
- Pack 989,735,991 bytes (943.89 MiB / 6144 MiB). Texture workspace at audit (Windows compressed-file-size allocation measurement) 5,553,340,253 logical/allocated bytes (5.17 GiB); temporary output 265,578,625 bytes (253.28 MiB / 256 MiB); free disk 99,796,377,600 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-158-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: hybrid batch 159 archive coverage audit

- Continued from `ffe24d3` on `74-enhancement-gfx-overhaul-to-modernize-textures`.
- Added a read-only standard-signature scan for uncataloged P5CK entries. All 403 archive hashes and sizes match the prior embedded-scan corpus; exact cataloged top-level identities are skipped. Embedded containers remain in scope. No extractions or catalog mutations.
- Scanned 48,572 uncataloged entries containing 3,250,041,122 bytes; skipped 29,509 cataloged top-level entries. No compressed entries were encountered. 383 entries contain 1,486 signatures: 29 DDS matches all fail header checks, and 1,457 JPEG matches all fail entry-bounded Pillow decoding. No PNG, XPR0 or XPR1 signatures found. No valid new candidate or catalog key was established.
- All entries and all signature/decoder outcomes were counted. Every hit entry is retained; five per-signature sample lists exceed the 32-sample display limit. Aggregate decoder counts include these unsampled offsets. Decoder resource limits are explicitly unverified rather than rejected.
- 17 targeted regression tests pass: entry boundaries, DDS payload bounds, JPEG success/truncation, decoder resource limits, exact catalog exclusions, embedded containers, compressed-entry skips, report truncation, hash/set drift and catalog preservation. Python 3.12.14; Pillow 12.2.0; Windows-11-10.0.26300-SP0.
- Refreshed installed-file hashes, dimensions, full mip chains and metadata through texture-worklist.py. All 1,147 installed replacements remain unchanged. The previous native loader-format audit from batch 158 remains the native evidence; no new native audit or gameplay launch was needed for this research-only tool.
- Catalog remains 6,812 supported keys / 7,147 supported payloads: 1,147 installed, 68 held for correction, 5,597 needing artwork. Signature absence and decoder rejection do not prove entries texture-free. Other encodings, embedded/procedural coverage and scene-specific validation remain open; no scope was excluded or marked complete.
- One active pack, 6144 MiB pack cap, 512 MiB residency limit and 256 MiB temporary cap retained. No game copy, generated artwork, save, settings or build change. Research script is platform-neutral Python; tested on Windows only. No PC runtime feature or Mac adapter was introduced.
- Pack 989,735,991 bytes (943.89 MiB); texture workspace at worklist audit 5,553,876,401 logical / 5,553,876,401 allocated bytes (Windows GetCompressedFileSizeW). Temporary output 265,584,596 bytes (253.28 MiB); free disk 99,882,307,584 bytes. Private research report 507,197 bytes, SHA256 `31c7c5d68c1199db98277fec91b4f5eb706344aa320cf96d4e0e36cd595dee15`.
- Reproduce with `python tools/texture-archive-signatures.py --game <game-directory> --root <texture-remaster-directory>`. Requires the existing catalog and embedded-scan corpus baseline. Private evidence remains local; source and tests are tracked.


## Texture overhaul issue #74: hybrid batch 160

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `a68400d`.
- Produced eight independent stone, metal and wood material edits using built-in image generation from catalog-verified originals; accepted six offline and held two for brightness/tone correction. Classification remains 2393 reviewed keys. Previous batch 159 closed the standard-signature archive scan gap without adding catalog keys; other encodings and procedural discovery remain open.
- Generated source sizes are six 1254x1254 and two 1774x887 images. Accepted six square sources compiled as five 1024x1024 and one 512x512 DXT1 replacements with full mip chains. The two wide sources compiled for review at 512x256 and 1024x512, then were removed from the active pack by reversible quarantine. Actual source sizes, exact prompts and resampling are recorded; these are not native 4K images.
- Accepted 46f91fb440dc4cf6 rough wall with metal rails (+2.33% mean luminance), 0ff029773b766770 riveted silver panel (-4.74%), 6fcc7675dd0c1079 cracked stone with carved frieze (+10.00%), a2d0f61de05068d3 carved ancient stone atlas (-2.10%), 04ae01452d8b90de sixteen-tile stone grid (-1.78%) and 409ba2b0a96e21d8 inset wood panel (-7.84%).
- Reviewed both primary before/after sheets, both 2x2 repeat sheets and eight focused crop sheets. Accepted assets retain rail positions, rivet borders, carved motifs, principal crack junctions, the atlas black corner and relief divisions, the four-by-four tile grid, and inset wood borders. Opaque alpha retained. Carved stone has stronger fine-grain contrast and a brighter central field; retain a scene-contrast check. Repeated sheets do not prove actual scene adjacency or campaign appearance.
- Focused grayscale changes: wall rail +4.85%; riveted borders -7.00% and -5.88%; carved frieze -0.90% with central cracked area +11.67%; relief atlas upper figure -2.26% and lower motif -5.28%; tile grout junction -0.95%; wood inset border -7.78% and lower gap -10.93%. These diagnostics support manual comparisons and do not replace scene review.
- Held b3f834e8f09aa4d1 ribbed slats: -20.97% overall luminance, -21.07% focused grayscale, too dark with stronger ridge contrast. Held 1060966d5584f4d6 curved metal strip: -15.98% overall, upper-edge focus -20.12%, lower edge -15.95%, with muted blue-gray shifting brown/green. Both retain source art and review evidence for correction and are not counted installed.
- Before generation safely retired 24 redundant review images (20 detail images from batches 148, 149 and 158 that have preserved focus crops, and four repeat sheets from 156/158), reclaiming 5543084 bytes. All resolved-parent, non-reparse, size and SHA256 checks passed before removal. Primary comparisons, focus crops, metrics, selections, source art and user data were preserved.
- Automatic approval review blocked the combined rejected-DDS deletion command without a more specific reason; it did not execute. No deletion retry was made. A separately validated reversible move preserved the four rejected DDS/metadata files (438058 bytes) in private review, with destination SHA256 checks. Their storage counts toward the 256 MiB temporary budget; future cleanup requires permitted action.
- Re-decoded all eight originals from catalog payloads and verified reference RGBA and payload SHA256. Native loader-format audit passed all 1153 installed DDS files. Refreshed worklist hash/dimension/full-mip/metadata checks, hybrid queue and archive inventory. Installed/offline-approved coverage is 1153/6812 supported keys (16.93%); 5589 need art and 70 drafts need correction. The catalog has 7147 supported payloads; 48572 unclassified archive entries and broader discovery remain open.
- No gameplay launch. Consolidated scene/model, transparency, memory/residency, fallback and 1080p/4K/ultrawide validation remains pending. The >=90% offline coverage plus all-categories-reviewed gate is unmet. Pack budget 6144 MiB, residency limit 512 MiB and temporary limit 256 MiB remain unchanged. No runtime/platform implementation change; Mac scene validation and other-device artwork synchronization remain unverified.
- Pack 993,409,445 bytes (947.39 MiB / 6144 MiB). Texture workspace at audit (Windows compressed-file-size allocation measurement) 5,585,259,406 logical/allocated bytes (5.20 GiB); temporary output 265,113,926 bytes (252.83 MiB / 256 MiB); free disk 99,725,651,968 bytes. One active pack; no duplicate extracts/ZIPs/staging.
- Private evidence: `review/hybrid-160-*`, authored source PNGs and prompts, and progress. Commit records progress only.


## Texture overhaul issue #74: inventory batch 161

- Continued on `74-enhancement-gfx-overhaul-to-modernize-textures` from `4b93ad9`.
- Reviewed 64 additional catalog-verified originals: 32 small reused assets and 32 single-use 128-256px assets. Classification now covers 2457 distinct keys. Each selection retains catalog payload SHA256, decoded RGBA hash and alpha range. Contact sheets were decoded directly from authoritative archives; no standalone original PNG cache was written this pass. Classification is routing evidence, not completed artwork.
- New classifications: {'prop': 11, 'interface': 12, 'prop_or_weapon': 5, 'effect_or_utility': 4, 'environment_alpha': 4, 'effect': 9, 'reflection_or_effect': 2, 'interface_or_utility': 2, 'material_uncertain': 2, 'character': 1, 'environment': 8, 'interface_or_prop': 4}. Ordinary stone, metal and wood surfaces expand the production queue; warning stencils, weapon outlines, targeting symbols, text, technical diagrams, character atlases, light/glow masks and flat-color or gradient utilities remain under specialist/individual review. Ambiguous identity and function remain explicit.
- Refreshed worklist hash/dimension/mip/metadata checks, hybrid queue and archive inventory. Queue: {'batch_candidate': 28, 'correction': 70, 'individual_review': 494, 'installed': 1153, 'specialist_review': 726, 'visual_triage': 4341}. The catalog contains 6812 supported keys and 7147 supported payloads, but 48572 unclassified archive entries and broader embedded/other-encoding/procedural discovery remain open.
- No new artwork or pack changes in this inventory pass. Installed/offline-approved coverage remains 1153/6812 (16.93%); 5589 keys need art and 70 drafts need correction. Native loader-format audit from hybrid-160 remains applicable to the unchanged pack and was not rerun.
- Previous goal turn 160 made concrete progress with six accepted textures. This inventory turn continues that production workflow. No gameplay launch or cleanup this pass. Temporary original/review output remained below 256 MiB, one active pack remains below 6144 MiB, and the 512 MiB runtime residency limit is unchanged. The >=90% offline coverage plus all-categories-reviewed gate is unmet. Scene, hardware and other-device validation remain pending.
- Pack 993,409,445 bytes (947.39 MiB / 6144 MiB). Texture workspace at audit 5,586,460,567 logical bytes and 5,586,460,567 Windows GetCompressedFileSizeW allocated bytes (5.20 GiB); temporary output 266,262,863 bytes (253.93 MiB / 256 MiB); free disk 99,724,779,520 bytes.
- Private evidence: `review/hybrid-161-*`, visual classification, inventory and progress. Commit records progress only; no artwork/game files published. No runtime or platform implementation change; Mac adaptation is not needed for this inventory-only record, and Mac validation remains pending.
