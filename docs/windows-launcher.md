# Windows release launcher

Game settings now includes a **Discord** tab for optional Rich Presence.
The public project Application ID is prefilled; enabling it takes effect on the
next game launch. Both launcher and game need builds containing issue #63.
See [Discord setup and configuration](discord-presence.md).

`TimeSplitters Launcher.exe` is the portable Windows x64 bootstrap. Extract the
whole launcher ZIP into a writable installation folder, keeping the application
`Bin/TimeSplitters.App.exe` and `Support/Launcher/manifest.json` beside it in
their subfolders. It installs game runtime packages from the
public `TheDoffman/timesplitters-future-perfect-pc` GitHub Releases page. It never
pulls a branch, builds game code, requests a GitHub login or downloads original
game data. It reuses `tools/release/ImportGame.cs` for local ISO/folder import.

## Build and validate

Run `powershell -NoProfile -ExecutionPolicy Bypass -File tools/build-launcher.ps1 -Test`.
The build uses the Windows .NET Framework compiler, WPF, and built-in libraries;
no NuGet packages or .NET SDK are required. Supported systems are Windows 10/11
x64 with .NET Framework 4.8. The test executable uses tiny synthetic ZIPs in
`build/launcher/updater-tests`, never real game data, settings or saves, and
removes that isolated directory after the pass.

`build/launcher/launcher-tests.exe --live` reads GitHub release metadata without
installing anything. `launcher-tests.exe --verify-release` downloads, verifies,
and installs the published Preview ZIP into a disposable test folder, then
removes it; it never executes downloaded code or copies game data.
`launcher-tests.exe --live-launcher` reads launcher-only metadata without
installing anything. `"build/launcher/Bin/TimeSplitters.App.exe" --render-preview
preview.png` renders a sample installed-state UI without showing a window or
accessing the network. The sample version in that image is illustrative.
`"build/launcher/TimeSplitters Launcher.exe" --render-splash splash.png` renders
the startup splash without opening a window or checking the network.

`tools/package-release.py` builds and includes the launcher each time it creates
a Windows runtime ZIP. The checkout's developer `Play.cmd` is unchanged. The
packaged `Play.cmd` opens the launcher, with its previous direct-launch path as
a compatibility fallback when the EXE is absent.

## Launcher artwork and colors

The v1.1.1 interface uses dark blue steel panels, silver text and icy blue
accents, with a matching startup splash and silver TS/blue portal icon. The primary Play button
uses a silver-blue metallic gradient.

`tools/build-launcher.ps1` embeds the optional
`private/launcher-art/future-perfect-banner-v1.png` into the main application.
Use `-BannerPath <image.png>` to supply a different banner (maximum 8 MiB).
The image travels inside `Bin/TimeSplitters.App.exe` for launcher-only updates;
no extra runtime image file or download is required. If the private image is
absent, the build retains the blue/silver vector header. Generated source
artwork and its provenance stay ignored under `private/launcher-art`.
The current private banner is the supplied 1600 x 1200 Future Perfect key art,
embedded unchanged. The full photo fits proportionally inside the right side
of the header, with no cropping or redraw. A narrow transparency blend along
the photo's left background edge joins it to a generated blue/silver time-fracture
environment; Cortez, both weapons and the boot remain visible. The larger original
game wordmark sits over a darker area of that environment. Source hashes and the
previous generated banner's recovery copy are recorded in `banner-provenance.json`.

The companion environment is `private/launcher-art/future-perfect-backdrop-v1.png`
(2172 x 724), embedded separately as `launcher-backdrop.png`. Use
`-BackdropPath <image.png>` to override it (maximum 8 MiB). Its generation prompt,
source and dimensions are recorded privately in `backdrop-provenance.json`.
Source builds without it use the existing gradient. Release builds with
`-RequireArtwork` require the banner, logo and backdrop so artwork is not silently
omitted. No external image files are needed after installation.

The optional `private/launcher-art/timesplitters-future-perfect-logo.png`
supplies the original title wordmark and subtitle lettering in the sidebar.
`-LogoPath <image.png>` accepts
the 500 x 187 title lockup (maximum 1 MiB); the sidebar subtitle crop depends
on that layout. The PNG is embedded unchanged, with its subtitle displayed
separately at a readable size instead of substituting a font. Without this private
asset, source builds retain the sidebar text fallback. Source URL and SHA-256 stay in
`private/launcher-art/logo-provenance.json`; no external image is needed at runtime.

Both EXEs use a dedicated compact silver TS monogram inside an icy blue temporal
aperture, independent of private artwork. `tools/launcher/IconBuilder.cs` draws
each transparent ICO entry directly at 16, 24, 32, 48, 64, 128 and 256 pixels;
the small sizes use heavier portal strokes for clarity. The build also writes
`build/launcher/icon-preview.png` to show all sizes. No image generator or
external asset is required to reproduce the icon.

### Windows game icon

New Windows game builds embed a separate game icon in `futureperfect_recomp.exe`.
It uses the same blue/silver temporal frame, with the original TSFP title at
128 and 256 pixels and the original Future Perfect lettering on its own larger
centered line. The subtitle is about twice its previous size. At 16, 24, 32,
48 and 64 pixels, a compact serif TSFP monogram replaces the wide title, centered
with space inside the rings. Source builds
without private artwork use the TSFP monogram at every size. The launcher keeps
its existing TS icon.

The monogram uses outlined letter shapes typeset with Future TimeSplitters by
Chequered Ink, a series-inspired typeface whose author permits all use. It is
not claimed to be the official logo font. Geometry and attribution are in
`GameMonogram.cs` / `GameMonogram.NOTICE.txt`; no installed or bundled font is
needed. Both build helpers compile this geometry alongside `IconBuilder.cs`.
Game packages include the lettering attribution in `Licenses/Game-icon-lettering.txt`.

`tsfp.ps1 -Action Build` runs `tools/build-game-icon.ps1` before CMake and passes
its generated `game-icon.rc` through `TSFP_GAME_ICON_RESOURCE`. The Windows-only
CMake patch embeds resource group 1 before executable identity sealing. SDL's
Windows backend uses the first executable group icon for its default window
class, so no separate runtime image or SDL gameplay change is needed. Packaging
continues copying the sealed executable unchanged, including its embedded icon.
The icon builder and shared icon renderer are game inputs in release notes.

Run `tools/build-game-icon.ps1 -RequireLogo` to require the local official
wordmark, `-LogoPath <image.png>` to supply it explicitly, or `-Fallback` to
preview the compact mark at every size. Outputs stay ignored in `build/game-icon`:
the seven-size `game.ico`, resource source, size preview, and provenance JSON
containing source/icon hashes and Git revision/changes. The existing PNG remains
unchanged; no generated redraw or font substitution is used for its lettering.
Public source includes only the renderer and build integration, with private
original art supplied locally as with the launcher banner/logo.

This requires a new game build/release. A launcher-only update does not replace
an older game's executable or change the icon in existing published packages.
Actual Windows taskbar/Explorer appearance should be checked with that game
release; resource-link tests use a tiny executable without running the game.

## Publishing game updates

Publish a GitHub Release with a semantic version tag (`v1.2.3`, or
`v1.2.3-alpha.1` for a prerelease) and these assets from the existing packager:

- `TimeSplitters-Future-Perfect-PC-<tag>-windows-x64.zip`
- The corresponding `.zip.sha256` sidecar (publish both).

Do not attach game data, saves or personal settings. The ZIP contains one package
root and a `BUILD.json` with a matching `version` plus SHA-256 for every file
except the manifest itself. Existing release packages are supported even if
they predate the launcher. The standalone launcher remains installed in that case.

Both channels check the most recent 100 published release records and choose
the highest supported game version with the matching Windows asset. Stable
excludes prereleases; Preview includes them. Launcher-only releases, drafts and source-only releases are
ignored. Updates never downgrade the installed version; rollback is explicit.
An unpublished or checksum-less release is not offered. Network failures and
rate limits leave installed play available.

The launcher uses an asset's GitHub-provided SHA-256 digest when available,
otherwise its `.sha256` sidecar. This verifies download integrity from the
project's release channel; it is not an independent publisher signature.
Production code signing and Windows SmartScreen reputation are not included.

## Publishing launcher-only updates

Bump `LauncherVersion.Value` in `tools/launcher/LauncherVersion.cs` and its
assembly/file version attributes, then build with `-RequireArtwork`. The build produces these assets
in `build/launcher`; for version 1.2.17, publish all three together:

- `TimeSplitters-Launcher-v1.2.17-windows-x64.zip`
- `TimeSplitters-Launcher-v1.2.17-windows-x64.zip.sha256`
- `launcher-update.json`

Use a GitHub Release such as `launcher-v1.2.17`. Mark test builds as prereleases;
stable launcher versions must also have no semver prerelease suffix. Releases
may contain a game ZIP too; launcher versions and game tags are independent.
The main application lists/filter releases so a launcher-only release cannot
hide a game release. Nothing is read from a branch or an unpublished draft.

The 200-byte update manifest has `schema: 1`, `version`, `asset_name`, `size`
and the ZIP's `sha256`. The bootstrap derives the download URL from the matching
GitHub asset in that same release. If GitHub publishes an asset digest, it must
agree with the manifest. Every file is checked again after extraction and
immediately before installation. The launcher-only ZIP contains exactly the
bootstrap, application, and `Support/Launcher/manifest.json`; the latter hashes
the two executables. Extra runtime/user files and unsafe paths are rejected.

At startup a small themed window checks launcher metadata before opening the
main UI. The eight-second deadline also bounds an OS request that ignores
cancellation. The Open installed launcher button cancels the check/download;
closing the splash cancels startup. Downloads have a two-minute limit. Offline,
missing-manifest, rate-limit and verification failures fall back to the installed
application. An incomplete installation still requires the complete ZIP if no
usable release is available. Launcher updates never downgrade across channels.

If Windows refuses the final handoff to `Bin/TimeSplitters.App.exe`, the startup
window now remains open instead of showing a raw exception and closing. Native
Win32 error 1223 gets conditional SmartScreen/security-prompt guidance; access
denied and missing application errors get distinct instructions. Classification
uses `Win32Exception.NativeErrorCode`, not its generic HRESULT or localized text.
The log records the native code and exception. **Retry** repeats only the
application launch; **Open launcher folder** opens this installation root, and
**View logs** opens its `.launcher` folder. Failed folder actions keep the original
startup guidance. Successful launch or closing the window ends the bootstrap
normally and releases its installation mutex.

`tests/launcher_startup.cs` injects synthetic launch/folder actions to verify
failure, retry success, repeated cancellation, folder/log actions, close behavior,
layout and personal-file preservation without executing applications or displaying
an actual security prompt. Run it through `tools/build-launcher.ps1 -Test` or
`launcher-tests.exe --startup`. The build remains unsigned; this handles recovery
and does not bypass SmartScreen or configure Authenticode signing.

Updates are automatic for the selected Stable/Preview channel. No game update
is implied. The bootstrap waits for neither a game build nor a large runtime ZIP.
When ready, `.launcher/self-helper.exe` waits for the bootstrap process to exit,
acquires the shared installation mutex, and replaces the three launcher files.
The new bootstrap skips its network check once after a helper handoff to avoid
retry loops, then starts the main application. The application waits for the
bootstrap to exit before acquiring the same mutex. Opening the companion EXE
directly routes back through the bootstrap.

Launcher updates use separate `.launcher/self-transaction`, `self-pending` and
`self-rollback` folders. Startup recovers an interrupted launcher transaction
before opening the main application. The game BUILD.json, game rollback snapshot,
Data, Settings, Saves and Textures are never launcher-update targets. Game
updates/rollback preserve a newer independently installed launcher. Keep the
complete `.launcher` folder for recovery. No launcher-only release is published
automatically by building these files.

Starting with launcher v1.2.4, after a normal launcher window loads successfully and both installed launcher
executables match their manifest, automatic cleanup removes the previous
launcher backup (`self-rollback`), obsolete or incomplete launcher staging
(`self-pending`), and unused `self-helper.exe` / `update-helper.exe` copies.
There is no retained history of older launcher versions after successful startup.
The backup remains available until that startup succeeds. An unfinished launcher
transaction or an invalid installation prevents cleanup. A newer prepared update,
an unreadable staging marker, and helpers needed by pending updates are retained.
Locked files are logged and retried at the next normal startup. Failed or cancelled
launcher downloads also discard their staging files immediately when possible.

Cleanup only targets these launcher-owned paths. It preserves game rollback,
game data, settings, saves, preferences, logs, and unrelated/manual downloads.
Developer render previews never perform startup cleanup. Run the isolated
cleanup regressions with `build/launcher/launcher-tests.exe --cleanup` after
building the test executable with `tools/build-launcher.ps1 -Test`.

## Startup argument troubleshooting

`Unknown bootstrap arguments` means the startup EXE received unsupported
command-line arguments before opening the main application. Normal startup uses
`TimeSplitters Launcher.exe` with no arguments. In launcher v1.2.2, developer
commands such as `--render-settings-graphics output.png` belong to
`Bin/TimeSplitters.App.exe`; passing them to the startup EXE causes this error.

The local v1.2.3 fix also accepts `--render-preview`, `--render-settings-preview`
and `--render-settings-graphics` at the startup EXE and forwards them to the
application without an update check. Relative output paths resolve against the
caller's current directory; the child exit code is preserved and preview time is
bounded to 30 seconds. `--render-splash` remains the startup EXE's own preview.
Unknown/malformed commands are still rejected. The startup log now records the
launcher version, argument count and bounded, JSON-escaped arguments, rather
than losing the arguments that caused a rejection. It remains capped at 1 MiB.

`launcher-tests.exe --bootstrap` runs the isolated preview-routing/diagnostic
checks without running game-update installation tests. Actual preview commands
operate on disposable output paths and do not launch the game or save settings.

## Pre-game settings

The sidebar **Game settings** page edits Player 1's keyboard/mouse bindings
(four alternatives per action), input device, sensitivity, mouse look and invert Y.
Click a binding to capture a key, mouse button or wheel direction. Reserved game
keys are refused. Reusing another action's key requires confirming the move.
Restore default controls affects Player 1 only and does not write until Save.

Launcher v1.2.2 opens both tabs inside the main launcher, with the sidebar
visible and usable. Binding capture, conflict reassignment and restore-defaults
confirmations also appear inside that page. **Home** retains your draft
and dismisses any active binding prompt; reopening **Game settings** restores the
same tab and edits. **Save settings** keeps the page open and reports validation
or save results in its footer. **Cancel** discards unsaved edits and returns to
Home; closing the launcher also discards unsaved edits. Existing
backup, rollback and external-edit protection are unchanged.

Graphics include output resolution (presets, attached display sizes and custom
dimensions), borderless fullscreen, console supersampling (1–8), anisotropic filtering
(1–16), frame pacing, interpolation (1–4), VRR and the FPS overlay. Invalid
resolutions and non-finite sensitivity are refused. Desktop refresh is unchanged
unless a supported fullscreen monitor rate is explicitly selected.

The development build adds **Smooth motion → 120 FPS / 144 FPS** under
**Quality & performance**. These experimental targets require a game build
newer than v0.3.0-alpha.2; older installed releases keep the legacy multiplier.
Selecting a target enables fullscreen and VRR, and selects 60 Hz game pacing
when necessary. The game-pacing and legacy-multiplier controls are disabled
while a presentation target is selected to prevent conflicting choices.
Save and restart the game. **Use interpolation setting** restores the existing `frame_interp` preference;
existing installations are not switched automatically.

For an easier starting point, open **Game settings → Graphics & resolution →
Smoothness setup**:

1. Select **120 FPS · smooth** first. This sets fullscreen, VRR, 60 Hz game
   pacing, 1× console render scale and the FPS overlay. It requests 120 Hz if
   the primary display reports support; otherwise it keeps the desktop rate.
   Your output resolution and filtering stay as selected.
2. Check **Monitor refresh · fullscreen** and the displayed current primary
   monitor rate. A supported **144 FPS · smooth** setup works the same way
   with a 144 Hz request. If the display rate is too low, check the game's
   monitor in **Windows Settings → System → Display → Advanced display**.
   Windows only offers rates supported by that display and current resolution.
   See [Microsoft's refresh-rate instructions](https://support.microsoft.com/en-us/windows/hardware/display-graphics/change-the-refresh-rate-on-your-monitor-in-windows).
3. Choose **Save settings**, then restart the game. Buttons update a draft;
   Cancel discards it. Compare the same scene while moving the camera.
4. If 144 is uneven, try 120. If GPU load causes drops, lower **Output
   resolution**. Console scale only affects **Original console resolution**:
   2× draws four times as many scene pixels, so reduce it to 1× first in that
   mode. With an explicit resolution, the renderer uses that scene size and
   console scale is disabled in the UI while retaining its saved value.
5. **60 FPS · original** explicitly turns interpolation off and uses 60 Hz
   game pacing, 1× console scale, fullscreen, VRR and the FPS overlay, with the
   current desktop refresh rate. It retains output resolution and filtering.

The guidance updates when resolution, target, fullscreen, VRR or refresh
changes. The monitor inventory reflects the primary display when the settings
page opens; reopen it after changing displays. The runtime validates the
requested rate against the monitor actually hosting the game. Selecting VRR
here does not enable the monitor/driver feature for you. The Windows presentation
path uses the tearing-allowed flag needed for VRR; a fixed-refresh screen can
still tear. See [Microsoft's VRR requirements](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/variable-refresh-rate-displays).

Use **Game** FPS to check the original game updates and **Shown** to check host
submissions. Game FPS can fall during loading or CPU-heavy work even at a low
resolution. Shown FPS can fall when there is insufficient rendering time or
when prediction cannot safely match a frame. Neither counter measures monitor
scanout, GPU execution time or input latency. The existing disk shader cache is
already enabled by default; clearing it is not a general FPS improvement.

The target controls host presentation, while the console clock stays at 60 Hz.
Camera and object transforms are blended between guest frames. During CPU work,
the renderer can predict up to one game frame ahead using the two completed
poses while the next frame is still being built. Separate color/depth targets
and a host-state snapshot preserve that partial frame. Feedback and offscreen
effects use their latest real-frame image during prediction, so those passes
do not interrupt the main scene's extra frames. Use an appropriate
high refresh monitor with VRR enabled in its driver and borderless fullscreen.
This setting does not change the desktop refresh rate. GPU load, loading,
camera cuts, movies and frames that cannot be matched can reduce the rate;
144 FPS is a target, not a constant-rate guarantee. Screen-space UI and some
CPU-animated effects still update at the game's own rate.

If the extra rendering target is temporarily unavailable, the 120/144 FPS
mode continues with normal game frames and retries at most once per second.
It can resume smooth motion without restarting after resources become
available. Persistent GPU pressure may still require a lower output resolution.

**Camera & window → Monitor refresh · fullscreen** separately offers the
current desktop rate and supported **120 Hz / 144 Hz** rates at the desktop's
current resolution. The FPS target and the monitor rate are separate controls.
Choose matching values for a high refresh display. The game applies the monitor
rate only while it owns the foreground fullscreen window; it restores the
previous rate on focus loss, leaving fullscreen, or exit. An independent helper
also restores it after a process crash or forced termination. Unsupported modes
or unavailable recovery support leave the display unchanged. Independent user
display-mode changes are preserved. Existing settings keep the desktop rate.
The implementation tests driver-reported modes with Windows'
[display-mode API](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-changedisplaysettingsexw).

For a direct development run, put `frame_target = 144` and `frame_cap = 60`
in the runtime display configuration, or set `RECOMP_FRAME_TARGET=144` and
`RECOMP_FPS_CAP=60` for that process. `frame_target = 0` keeps legacy behavior.
`refresh_rate = 120` / `144` (or `RECOMP_REFRESH_RATE`) selects the optional
fullscreen monitor rate; `0` preserves the desktop rate.
Alternatively, double-click `Play-Smooth.cmd` for a temporary 144 FPS/fullscreen
override, or run `Play-Smooth.cmd 120`. It requests the matching monitor rate
when supported and leaves saved preferences unchanged.
`RECOMP_INTERP_TRACE=<csv path>` optionally records up to 20,000 interpolated
CPU submission timestamps and blend factors; it does not measure scanout.

The running Windows game also offers **Settings → Display settings →
Smoothness / refresh** (or paused **Options → Display settings**). Its FPS
and monitor-rate choices save for the next normal launch; Back discards edits.
Unlike the launcher's setup buttons, this page preserves render scale and
the FPS-overlay preference. See the [in-game controls](windows-settings.md#in-game-display-settings).

For development investigations, `TSFP_GPU_TRACE=<csv path>` enables a bounded
D3D11 timestamp probe around interpolated redraws and their resolve, plus CPU
redraw and Present-call timings. Queries are polled without waiting or flushing;
the CSV stops after 20,000 samples. GPU elapsed spans can include idle bubbles
and exclude the guest frame, so they are not whole-frame GPU busy time.
DXGI display statistics are recorded when available; zero statistics do not
establish scanout timing. The probe is off by default. The isolated
`tests/high_refresh_game.py --fps 144 --seconds 120 --stress --pause-check
--gpu-timing` runner bounds and collects this trace. Its optional `--presentmon`
argument accepts a locally installed PresentMon console executable and traces
only that child game's PID; unavailable ETW events are reported explicitly.
For a longer stability check, `--seconds 300 --resources` samples the isolated
child's private memory, working set and handle count every five seconds into
a separate `*-stability-resources.json` file. Runs are capped at ten minutes;
the interpolation CSV retains its independent 20,000-sample cap. The existing
stress inputs run early in the session; later time observes continued gameplay.

Launcher v1.2.1 adds
**Game settings → Graphics & resolution → Startup → Skip startup splash screens**:
skips the opening EA and Free Radical logo movies on the next launch. It also
disables the idle attract video; story cutscenes rendered by the game still play.
The checkbox defaults to off. Save settings to keep the choice, or uncheck and
save to restore the videos. The choice is stored under
`launcher.skip_startup_splashes` in `Settings/pc-controls.json`, preserved across
game and launcher updates. Launching sets the existing runtime's
`RECOMP_XMV_PLAY` switch explicitly to `0` (skip) or `1` (play), after clearing
inherited overrides. This works with the published alpha.4 game without a
runtime rebuild. New packages' `Launch.ps1` also honor this preference.

Controls and resolution use the same `Settings/pc-controls.json` and
`Settings/pc-display.ini` formats as the in-game settings. Graphics use a portable
`Settings/runtime.conf`; when it is absent, the UI reads the existing per-user
`xboxrecomp/titles/45410066.conf` as its starting point. Saving copies those
settings into the installation and never changes that per-user source. Unknown
JSON fields, other controller profiles, unknown runtime keys and comments survive.
Save detects external changes, makes `.bak` copies, writes atomically per file,
and restores earlier files if a later write fails. A running game blocks saves.

After portable graphics settings have been saved, Play starts the game directly
with explicit data/save/input/display/texture/online paths, sanitized environment,
mouse preferences and timestamped `Logs/*.out`/`.err`. This is necessary because
older `Launch.ps1` packages discard inherited display overrides. Installations
without portable graphics settings retain the existing script launch path.
Game updates preserve all three settings files. No runtime/game-code change is
needed for the existing supported Windows package.

`launcher-tests.exe --settings` runs isolated settings/preservation/rollback and
child-process environment checks. `TimeSplitters.App.exe --render-settings-preview
preview.png` and `--render-settings-graphics graphics.png` render the two tabs
without saving settings or starting a game. These previews now include the whole
launcher and its sidebar, at 1152 × 772, using the same embedded settings page.

## Installation and recovery

### Home and settings layout

Home uses matching vector sidebar icons, readable blue/silver controls and a
themed update-channel picker. The banner displays the complete original artwork
with uniform scaling inside a softly lit frame; the source image is unchanged.
After game data is verified, setup actions collapse into **Manage game data**.
Expand it to open the installed Data folder. Missing data shows the ISO/folder
import actions; existing incomplete data shows an inspection action instead of
offering to overwrite it. A routine refresh preserves the expanded/collapsed
state unless verification status changes. Home cards scroll when expanded
content exceeds the available window height.

Settings use tighter section spacing and adjacent FOV/window controls while
retaining the fixed Save/Cancel footer, key bindings and draft behavior. These
layout changes do not modify saved preferences or the install/update process.

The game card groups the installed version, readiness, Play and data management.
Home highlights one next action: Play for a ready game, install/import/inspection
during setup, or Retry/Resume during recovery. Secondary actions use the same
quieter style as embedded-page controls. The banner contracts in shorter windows
to preserve space for these actions while keeping the complete artwork visible.

The update card identifies checking, up-to-date, available, downloading,
verifying, paused, attention-needed and no-release states with both text and
vector icons. It shows local progress during checks/downloads/preparation and
stops that indicator on pause, failure or completion. Reserved message/progress/
action space keeps routine transitions steady; unusually long messages and
expanded data details can still scroll rather than being truncated. These cues
describe existing update operations and do not change their safety checks.


Play minimizes the launcher to the Windows taskbar as soon as the game process
starts, including while its window is still loading. The taskbar button remains
available. When the process exits, the launcher restores its previous normal or
maximized state. If the process cannot start, the launcher stays visible with
status/log access. Early game exits restore it immediately. Successful-session
cleanup still requires an observed game window and a zero exit code; minimizing
alone never confirms an update. Window detection matches this installation's
executable path.
`tools/test-launcher-lifecycle.ps1 -LauncherRoot build/launcher-v1.1.2`
checks immediate normal/maximized minimize-and-restore, taskbar availability,
early exits, cleanup eligibility and failed startup using a bounded synthetic
executable/window. It uses no game data or personal files. The broader
updater suite requires real game processes to be closed before its install tests.

The installation directory is the folder containing the launcher. The updater
accepts only known root files, `Bin/*.exe`, `Bin/*.dll`, `Licenses/*.txt`, the
release-managed `Source` and `Support` trees, and the initial
`Settings/pc-controls.json` default. Existing settings are skipped.
`Data`, `Saves`, `Textures`, other settings and developer/source directories are
never release targets. ZIP traversal, duplicate/case-colliding paths, alternate
data streams, linked paths and unlisted/missing files are rejected. Archive
count/expanded-size limits and disk-space checks bound staging.

The application checks for game releases automatically but game installation requires an
Install/Update click. Downloading and verification happen before the launcher
closes. A copy of the launcher runs as `.launcher/update-helper.exe`, waits for
the old process, acquires the per-installation mutex, verifies the staged files
again, and installs. Both update and rollback refuse while any game process is
running. The launcher also blocks a second instance for the same folder.

Every replaced file is backed up before any installed file is changed. A
`.launcher/transaction/journal.json` records which files originally existed.
Failures restore old files and remove newly introduced files. On the next
startup an interrupted transaction is recovered before play/update is enabled.
If recovery files are missing or locked, the launcher stops and retains recovery
evidence. Successful updates retain one `.launcher/rollback` snapshot until the
new game opens a window and exits normally. Until then, Restore previous swaps
releases without replacing user settings. The downloaded ZIP is
discarded after successful extraction, and staging is discarded after install.

When Windows temporarily blocks renaming the completed transaction into the
rollback folder, the updater retries that rename every 250 ms for up to five
seconds. This applies to game installs, repairs and launcher updates; file writes
are not replayed. A persistent access/sharing/lock error still fails the update
and attempts restoration, with instructions to close programs using the install
folder and retry. Other filesystem errors fail immediately. If recovery also
fails, the log retains both errors and the message tells users to preserve
`.launcher` and reopen the launcher after releasing the lock.

### Automatic game cleanup (launcher v1.2.5)

After a game session opens a window and exits with code zero, the launcher checks
that the installed runtime still matches its release manifest and removes the
previous game snapshot. The Restore previous button then becomes unavailable.
A failed launch, crash, incomplete installation or unfinished update keeps recovery
available. This check establishes startup/clean-exit behavior, not full gameplay
compatibility. Cleanup runs in the background; a locked file is retried on later
launcher startups only for the same confirmed build. Developer previews do not
confirm a game session or run startup cleanup.

Game updates also remove unchanged files listed in the old release manifest when
the incoming release no longer uses them, including old source archives and DLLs.
Those removals are transactional and restored by rollback before confirmation.
Modified files, untracked files, personal settings and independent launcher files
are retained. Missing or invalid old manifests skip this optional pruning.

Cleanup only removes updater-managed game backups and obsolete/incomplete staging.
Current game data, saves, settings, texture packs, and the original ISO remain in
place. An ISO found inside a managed folder prevents that folder from being
deleted; linked paths and personal backup directories are also refused. A newer
or unreadable staged release is retained. Normal ISO storage and manually
extracted game folders are never searched for cleanup candidates.

Run `build/launcher/launcher-tests.exe --game-cleanup` for the focused synthetic
game cleanup checks. The full updater suite additionally covers manifest-based
file removal, rollback restoration, and protected ISO handling during updates.
Do not delete `.launcher` while an operation is active.

The interface imports only into an absent `Data` directory. Existing data is
validated and retained; it is never overwritten or automatically deleted.
An incomplete/wrong existing Data folder requires the user to inspect/relocate
it before retrying import. Failed imports retain a named `Data-import-*` folder
for inspection. The ISO does not need to remain available after successful import.

Launcher errors appear in the status line and `.launcher/launcher.log` (bounded
to roughly 1 MiB). Game output continues through the release's `Launch.ps1` and
its `Logs` folder. Saves and config are not used for automated launcher checks.

Windows-specific source lives in `tools/launcher`; Mac release updating remains
queued as `MAC-014` in `MAC_PORT_BACKLOG.md`.

Launcher v1.2.3 keeps custom FOV disabled for installed game releases through
v0.2.0-alpha.4, which only read V1 display settings. Resolution changes continue
to save in that compatible format. FOV becomes available with a newer game
release; unpackaged development builds retain access for testing.

Launcher v1.2.6 filters resolution presets for the installed game release and
validates custom sizes before saving. Game releases through v0.2.0-alpha.4
support heights up to 2160 (maximum 7680×2160); newer games and unpackaged
development builds support up to 7680×4320. Both retain landscape and 4:1 limits.

### Preserve launcher artwork during game updates

A game package replaces the independent launcher only when its bundled version
is newer, or when an equal installed version fails its manifest checks. A valid
equal/newer launcher keeps its binaries and manifest together, including embedded
banner/logo artwork. Explicit launcher updates use their separate transaction.

Public game packaging requires the private banner, backdrop and title logo via
`tools/build-launcher.ps1 -RequireArtwork`; missing artwork stops packaging.
Developer/CI builds may still omit the switch to use the fallback interface.
For an installation already affected by an older updater, close the launcher
and extract the complete verified launcher release ZIP into that installation.
An equal version with internally valid fallback files is not automatically
reinstalled by the existing launcher self-updater.

## Resumable downloads and recovery (current source)

Game updates, launcher updates and repair downloads keep matching saved bytes
across retries and launcher restarts. Home offers **Resume download** for a saved
game transfer with a matching published digest. **Pause download** keeps the saved
bytes; closing during a game download requests a pause and waits for the current
file operation to stop safely. **Cancel preparation** also retains a completed
cached download so the next attempt can verify it again without downloading it.

Progress shows bytes received, total size, measured download speed and estimated
time remaining. Speed measures this attempt's new bytes; previously saved bytes
are excluded from the rate. The launcher says **Estimating time remaining** until
it has a useful measurement, and shows verification/preparation separately.

Home failures offer **Retry** (or **Resume download** after a pause),
**Repair / Verify** and **View logs**. Failed repair operations offer retry and
logs in the same page. Interrupted startup launcher updates offer **Retry update**,
**View logs** and **Open installed launcher** when one is present. A failed initial
release check still opens the installed launcher automatically for offline use.

The cache is separate from installation staging, with one game slot and one
launcher slot under `.launcher/downloads`. It is retained for interruptions and
cancelled preparation, then cleaned when staging succeeds. A different release
URL, size or SHA-256 replaces its previous recognized cache. Unknown cache files
and linked paths are refused and kept; cleanup only removes the named owned
cache files and never recursively removes user folders.

Resume uses HTTP byte ranges and a strong If-Range validator when available.
Range boundaries and lengths must agree with the published asset. A server that
ignores Range restarts the file; an invalid range is rejected, and an unsatisfiable
saved range gets one fresh attempt. The complete file always passes the published
SHA-256 before it reaches staging, and package manifests are checked as before.
Checksum failures discard the damaged cache. A connection or stream that stalls
is bounded to 30 seconds; retry can resume the saved bytes.

Developer checks: `launcher-tests.exe --downloads` runs synthetic HTTP and
scratch-file recovery tests without requiring the game to close.
`--downloads-live` interrupts a small public standalone launcher ZIP after 64 KiB,
resumes it in a fresh client and verifies the checksum; it never installs or runs
the downloaded program and cleans its disposable files. Protocol reference:
[HTTP range semantics](https://www.rfc-editor.org/rfc/rfc9110.html#name-range).

## Game versions (launcher v1.2.14)

Select **Home → Updates → Game versions** to install a specific published
Windows runtime for comparison or bug reproduction. This main-window page
keeps the sidebar visible and shows the installed version, release selector,
stable/preview classification, publication date, download size and release notes.
The picker includes both stable and preview builds regardless of the normal
update channel. **Load older releases** reads subsequent GitHub release pages;
launcher-only, draft, source-only and unverified packages are excluded.

Choose **Install selected version** (or **Reinstall selected version**) and review
the confirmation. It replaces the installed game runtime only. It does not
change Git branches, rebuild a revision, or replace the launcher, even if the
selected game package bundles a different launcher version. Imported `Data`,
the ISO, `Saves`, existing `Settings` and textures are preserved by the existing
transactional installer. Save formats can differ across game versions: back up
your saves before comparing older builds. Unpublished development commits are
not downloadable from this list; the source checkout remains unchanged.

The same checksums, manifest validation, resumable downloads, pause/retry actions,
disk-space checks and rollback rules apply. Download progress appears on Home.
Retry retains the explicitly selected version even when it is older than the
normal update candidate. Close the game before switching; a running game blocks
installation. A failed refresh preserves loaded choices and leaves installed
files alone. Normal **Check updates** still offers newer releases only and never
silently reverts an installed game. No version is installed without an action.

`launcher-tests.exe --versions` runs the isolated catalog and native-page checks.
`TimeSplitters Launcher.exe --render-versions-preview preview.png` renders the
sample version picker without accessing GitHub or an installed game.

## What's new (launcher v1.2.11)

**What's new** in the sidebar and on Home opens an embedded release-notes page;
the sidebar stays visible. Choose **Game releases** or **Launcher releases**,
then **Available release** or **Installed release**. The version overview compares
the installed game or running launcher with the latest compatible release on the
Home update channel. It distinguishes newer updates, up-to-date installations,
versions newer than that channel and missing/unversioned installations. If Stable
has no game release, notes for an installed preview remain available separately.

The page reads published GitHub release descriptions; it never reads main or
invents a change summary. Latest game/launcher selection follows the existing
updater rules, including launcher metadata verification. Installed notes use the
exact version, looking up an older release directly when it is outside the latest
100 records. Unavailable or unpublished versions are not replaced with another
version's notes. Release dates and titles appear when published.

Launcher v1.2.15 gives Game versions and What's new a shared selectable native Markdown view. Headings,
bold/italic and struck-out text, ordered/unordered lists, quotes, inline code and
fenced code blocks are formatted in the blue/silver theme. Markdown links show
their label (for example, `#106`), and bare HTTP/HTTPS URLs are clickable. Links
open in the default browser only when activated; their destination is available
as a tooltip. Copying selected text remains available. This is a compact Markdown
subset: advanced GitHub extensions and raw HTML are displayed as text rather
than rendered as web content. Images are omitted, and scripts/embedded XAML are
never interpreted. File, script and other non-HTTP(S) links are not navigable.
Descriptions are limited to 64,000 characters / 512 lines, with an explicit link
to read the complete release if shortened. **View release on GitHub** opens the
selected release only when clicked. **Known issues** displays a published heading
such as `## Known issues` or `## Known limitations`; if none exists, the page says
so rather than claiming there are no issues. Other sections remain in the change
notes. Open GitHub for the complete original release and unsupported formatting.

**Refresh notes** has a 25-second cancellation deadline. A failed refresh retains
previously loaded notes for the same version/channel for this session and marks
them with their last checked time; a first offline load shows retry guidance.
These notes are held in memory, not saved for the next launcher session. Switching
channel or installation version resets that view, and cancelled older requests
cannot replace its newer result. Updates are still installed from Home or the
startup updater; reading notes does not install anything or change settings.

Developer preview: `--render-news-preview output.png` works through the bootstrap
or app, with labeled sample releases and no network/filesystem maintenance.
`launcher-tests.exe --news` exercises synthetic release selection, exact-version
lookup, failures, safe text handling and embedded navigation without real game
files. The standard `build-launcher.ps1 -Test` includes these checks.

## Repair / Verify tabs (launcher v1.2.12)

The sidebar has one **Repair / Verify** entry with two tabs in the main window:
**Repair / Verify** for file checks and repairs, and **Diagnostics** for system
information, report copying, bug reports and enhancement requests. There is no
separate Diagnostics sidebar button. Opening the sidebar entry or a recovery
shortcut selects Repair / Verify by default; diagnostic developer previews still
open Diagnostics directly.

Both tabs keep **View logs** visible in the footer, including during an operation.
Switching tabs preserves verification results and the diagnostic report. The
report is collected on the first Diagnostics selection; use **Refresh diagnostics**
to update it after a file check or system change. File operations lock tab switching
while keeping **Cancel operation** and **View logs** usable. The sidebar remains
visible, and navigating away preserves unsaved Game settings drafts.

## Repair / Verify (launcher v1.2.8)

The **Repair / Verify** sidebar page stays inside the main window. Its **Open game
folder** button sits below the verification and repair cards and opens the
installation folder for inspection. Choose
**Verify files** separately for the game runtime or launcher. Verification works
offline, compares SHA-256 hashes with the installed manifest, reports missing,
changed or unreadable release-owned files, and creates or changes no files.
Game checks exclude personal settings and independent launcher files. A matching
installed manifest is an integrity check; it is not a fresh publisher signature.
Imported game data, ISOs, saves and texture packs are not scanned or repaired.

The current source layout uses separate game and launcher status cards. Each shows
**Not checked**, **Verified** or **Needs attention**, with version and checked-file
counts after a scan. **Verification details** stays collapsed until needed; a check
that finds a problem opens it automatically. Expand it at any time to inspect the
file-level results. The page scrolls at smaller window sizes and keeps operation
status and cancellation available below the content.

After verification, **Repair files** downloads the exact installed game release,
or the exact running launcher release, from GitHub. Draft, unavailable or
unpublished releases cannot be substituted with a newer/older version. The
archive size and published SHA-256, package layout and every staged file hash
must verify before installation. An unpublished development version cannot repair itself
from GitHub until its exact version is published. Repair is explicit, including for
internally consistent fallback artwork builds.

Close the game before repair. Downloads, verification and extraction can be
cancelled; installed files stay unchanged until the existing updater helper
applies the verified transaction and reopens the launcher. Game repair replaces
only changed/missing owned runtime files and the official BUILD manifest; it
preserves all settings, saves, extracted data, ISOs, textures, independent
launcher files and untracked files. Modified release-owned runtime files may be
replaced. The previous game rollback is retained. Repair has separate
`.launcher/repair-transaction` and `repair-rollback` folders; interrupted repairs
recover on startup, and repair backups use the existing confirmed-game cleanup
rules. Existing unknown/pending updates are retained, rather than overwritten.

## Diagnostics (launcher v1.2.8)

**Repair / Verify → Diagnostics** shows running/installed versions, Windows and process architecture,
CLR version, GPU/driver information when available, free space, embedded artwork,
saved graphics settings, last verification results and recent structured launcher
exception types. Graphics discovery runs in the background with a bounded wait;
missing system information does not block the launcher. Use **Refresh diagnostics**
after verifying files, then **Copy diagnostic report** to put the displayed text
on the clipboard. It is never uploaded automatically.

Choose **Report a bug** or **Request enhancement** to open the matching GitHub
issue form. The launcher converts the displayed filtered report into the optional
**Launcher diagnostics** field and also copies the complete text to the clipboard.
You fill in your description, select the requested version/platform choices,
review the diagnostics and submit on GitHub using your account. The launcher
needs no GitHub token and never creates or submits an issue automatically.
Clicking a report button opens a browser draft and sends the prefilled diagnostic
text to GitHub; review the displayed report before choosing it.

Long reports use a short browser link with a clearly marked summary. Paste the
complete copied text into **Launcher diagnostics (optional)** before submitting;
the full report is never silently truncated. If clipboard access is unavailable,
the report stays visible for manual copying. Refresh diagnostics after collecting
new verification results. Bug/enhancement description fields remain blank so the
launcher does not invent symptoms, reproduction steps or feature requests.
Both issue templates include the same optional plain-text diagnostics field;
they must be on the repository's default branch before publishing the launcher.
The prefill uses GitHub's documented [issue-form URL query fields](https://docs.github.com/en/issues/tracking-your-work-with-issues/using-issues/creating-an-issue#creating-an-issue-from-a-url-query).

The report excludes raw logs, personal/installation paths, arbitrary error
messages, file contents, save data and ISO information. Recent errors contain
only recognized UTC timestamps and exception types. **View logs** remains
available on Home and in both Repair / Verify tabs for local investigation.

Developer previews: `--render-repair-preview output.png` and
`--render-diagnostics-preview output.png` work through the bootstrap or app EXE.
They show sample UI state without network requests, repairs or hardware queries.
`launcher-tests.exe --maintenance` runs disposable maintenance fixtures;
`--maintenance-readonly` runs only the read-only/model/UI subset while an
interactive game must remain running. The normal `build-launcher.ps1 -Test`
includes the full maintenance suite and retains the production game-running guard.

## Game crash reports (launcher v1.2.16)

Launcher v1.2.16 adds report viewing/export and unfinished-session tracking.
Unexpected-exit summaries work with existing game builds. Native minidumps and
frame-progress freeze detection require a game build containing the Windows
crash helper; installing this launcher alone does not add them to an older game.

New game crash reports restore the launcher and open Diagnostics. The page offers
Open latest crash report and Export crash report actions. Export creates a local
ZIP with the minidump, summaries and bounded log excerpt; nothing uploads. Dumps
and logs may contain personal information. Existing reports remain available
from Diagnostics after restarting the launcher. See [Windows crash logging](windows-crash-logger.md).

Suspected hangs also restore the launcher and open Diagnostics while the game
remains running. Open latest game report / Export game report work without
closing a hung game. Game updates and repair remain blocked during play.
Frame-progress, grace-period and false-positive limits are documented in
[Windows crash logging](windows-crash-logger.md#freeze-detection).

Games started through the launcher also have a persistent session record. A
nonzero exit without a native fatal report produces an exportable unexpected-exit
summary. On reopening, the launcher flags an unfinished session after confirming
its recorded game process is gone; a surviving game is left running. Diagnostics
states when no dump or exit code was captured. An unfinished record cannot prove
a crash cause: forced closure, reboot, and interrupted launcher monitoring can
also leave one. See [session recovery and limits](windows-crash-logger.md#unfinished-sessions-and-exits-without-a-dump).
