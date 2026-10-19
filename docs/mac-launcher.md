# Native Mac launcher

The native Apple Silicon launcher ports the Windows v1.2.18 interface using
Avalonia. Its home layout is generated directly from `tools/launcher/Launcher.xaml`;
artwork, placement, colors and labels come from that Windows source. The native
settings, release notes, version browser, maintenance and startup windows use
Mac controls. This is a local development port, not a published Mac release or
certification of complete visual/behavioral parity.

## Build and run

From the checkout root on Apple Silicon:

```sh
python3 tools/build-mac-launcher.py --install-sdk --test
open 'build/macos-launcher/TimeSplitters Launcher.app'
```

The SDK installs only under ignored `external/dotnet`, using the SHA-512 pin in
`config/mac-launcher-sdk.json`. Avalonia and transitive packages are locked in
`tools/launcher/macos/packages.lock.json`. The standalone app includes .NET;
users of the built app do not need to install the SDK, Python or CrossOver.
Source-checkout Vulkan builds additionally require their existing Python/SDK
verification helper. The SDK, NuGet cache, generated app and intermediate output
are build dependencies, not files to transfer between Windows and Mac.

Use `--dotnet /path/to/pinned/dotnet`, `--output path`, and `--artwork-dir path`
to select existing dependencies or the output location. The artwork directory
accepts the unchanged embedded resources `launcher-banner.png`,
`launcher-backdrop.png`, `launcher-logo.png`, and optional `launcher.ico`.
`--require-artwork` rejects missing artwork. Source builds without private assets
use the original vector fallback. The icon converter retains the original PNG
entries without redrawing or upscaling them. Keep original assets and provenance
under ignored `private/launcher-art`.

In a development checkout the app discovers the project by walking its parent
folders, or accepts `--root /path/to/checkout`. It runs
`build/macos/futureperfect_recomp` and preserves `private/game`, `private/saves`,
`config/pc-controls.json`, `config/pc-display.ini`, and
`private/native-settings/runtime.conf`. Startup does not create defaults or touch
those personal files. Play creates missing defaults only; settings saves are
explicit, backed up, checked for external edits, and rolled back on failure.
The app minimizes after process creation and restores after exit. It bounds the
two reused game logs to 4 MiB each. Existing 60 FPS interpolation preferences
are retained; absent native preferences default to 2x interpolation.

The native executable hash, Mach-O ARM64 identity, supported US XBE hash, and
nonempty `pak`/`xmv` folders are checked before launch. Vulkan checkout builds use
`tools/vulkan_sdk.py` to verify the existing pinned SDK receipt and select its
KosmicKrisp loader. No SDK download, renderer rebuild or switch occurs at launch.
A successful prerequisite check is not gameplay or graphics validation.

```sh
'build/macos-launcher/TimeSplitters Launcher.app/Contents/MacOS/TimeSplitters.Launcher' --check --root .
```

## Shared behavior and distribution

`LauncherPlatform.cs` contains package names, paths and Mac atomic replacement.
Version ordering, Stable/Preview filtering, original-game import, checksum/path
validation, resumed downloads, update journals, rollback, settings models,
diagnostics filtering and issue-draft preparation use the existing Windows
source. Native keyboard/mouse events map to the shared binding codes, including
134/135 for the wheel. Unsupported Discord preferences remain saved; the native
Discord IPC game adapter is still pending and its controls are read-only.

The app requests only `macos-arm64` assets. Windows releases are never installed
or run by this launcher. The existing checkout stays usable when no compatible
published Mac release exists. Release installs and self-updates are disabled for
development checkouts so they cannot overwrite source or native build work.

Portable game packages use `Bin/futureperfect_recomp`, `Launch.command`,
`Settings/pc-controls.json`, `Licenses`, and a hashed `BUILD.json`; personal data
uses `Data`, `Saves`, `Settings`, and `Textures`. A new native runtime package,
including redistributable dependencies and provenance, still needs the MAC-012
release pipeline. The launcher alone cannot supply a game runtime.

`--package` additionally creates a launcher ZIP, SHA-256 sidecar and
`launcher-update-macos-arm64.json`. Keep the `.app` beside `Support/Launcher` in
a portable installation. Mac launcher updates use an independent release asset
and manifest, an eight-second startup check and bounded resumable downloads.
The previous launcher remains until a successful verified startup; helpers apply
updates after the old process exits. Original game data, saves and config are
never launcher-update targets. Building does not publish a release.

The executable has a local ad-hoc signature. Developer ID signing, notarization,
clean-machine installation, real published upgrade/repair/rollback and native
crash-dump capture have not been validated. Windows minidumps cannot be copied
as a Mac crash-capture implementation. Unexpected process exits still use the
shared session records and privacy-filtered diagnostic summaries.

## Validation

`--test` executes synthetic settings, native package, integrity, rollback,
download and platform-selection checks in a disposable directory alongside the
app. It never starts the game or uses personal saves/settings. The workflow runs
these checks on an ARM64 Mac runner alongside existing Windows launcher checks.

The executable also supports `--render-preview output.png`,
`--render-settings-preview output.png`, `--render-settings-graphics output.png`,
`--render-settings-discord output.png`, and `--render-repair-preview output.png`.
Pass an explicitly disposable `--root` when rendering settings. Preview mode
performs no network requests, startup cleanup or saves. Run actual game tests
through `tools/test-native-mac.py` with its isolated paths and bounded duration;
do not automate Play against personal saves.

The first measured native launcher port is recorded in
[STATUS.md](../STATUS.md#native-mac-launcher-port-2026-10-19).
