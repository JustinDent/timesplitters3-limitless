# Windows release launcher

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
artwork and its prompt/provenance stay ignored under `private/launcher-art`.
The banner fills the header with top alignment to preserve the title and face.

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
assembly/file version attributes, then build. The build produces these assets
in `build/launcher`; for version 1.2.6, publish all three together:

- `TimeSplitters-Launcher-v1.2.6-windows-x64.zip`
- `TimeSplitters-Launcher-v1.2.6-windows-x64.zip.sha256`
- `launcher-update.json`

Use a GitHub Release such as `launcher-v1.2.6`. Mark test builds as prereleases;
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
dimensions), borderless fullscreen, supersampling (1–8), anisotropic filtering
(1–16), frame pacing, interpolation (1–4), VRR and the FPS overlay. Invalid
resolutions and non-finite sensitivity are refused. Desktop refresh is unchanged.

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

Starting with v1.1.2, Play keeps the launcher visible during startup and
minimizes it once the installed game's window appears. It restores the previous
normal/maximized launcher state when the launch script finishes. Startup failures
before a game window appears leave the launcher visible with status/log access.
Window detection matches this installation's executable path.
`tools/test-launcher-lifecycle.ps1 -LauncherRoot build/launcher-v1.1.2`
checks normal/maximized minimize-and-restore and failed startup using a bounded
synthetic executable/window. It uses no game data or personal files. The broader
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
