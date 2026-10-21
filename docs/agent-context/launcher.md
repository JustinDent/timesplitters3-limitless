# Launcher internals

<!-- packet:start -->
The desktop launcher is C# under `tools/launcher/`, NOT the guest recompilation
runtime. Windows uses WPF; native Mac uses Avalonia plus thin platform adapters.
Shared policy belongs in the existing C# models, not a second Mac copy.
`Launcher.cs` coordinates UI; `Launcher.xaml`, page classes and themes define
shared layout; `Ui.cs` maps controls/events. `LauncherPlatform.cs` owns OS/package
names and development-checkout versus installed paths. Read it before assuming
that `Data`, `Settings`, `Saves`, `Bin` and root checkout paths are interchangeable.

`ReleaseClient.cs`/`GameVersionsPage.cs`: published game releases and selection;
`SelfUpdate.cs`/`UpdateRestart.cs`: launcher updates and restart;
`DownloadTransfer.cs`: transfers; `InstallationMaintenance.cs`: repairs;
`GameSessions.cs`: game launch/lifetime; `GameSettings.cs`/`SettingsPage.cs`:
settings; `TexturePack.cs`/`TextureDownload.cs`: optional textures.
`GameSettings.NativeFps` stores original(0)/120/144 beta selection in a separate
`pc-frame-rate.ini` and exports `TSFP_NATIVE_FPS`; it needs a compatible game
build and does not revive interpolation or change the console interrupt clock.
Launcher version and game version are separate. A source commit is not a
published release. Use `tools/launcher-compatibility.py --local` for API-free
shared checks, plus affected host lifecycle/build checks. Never start publishing
or change credentials just to validate a launcher edit.
<!-- packet:end -->

## Flow and ownership

Read [launcher usage](../launcher.md), [Windows launcher](../windows-launcher.md),
[Mac launcher](../mac-launcher.md) and [release process](../launcher-releases.md)
only as needed. `Bootstrap.cs` handles startup; `Core.cs` contains shared
installation/path/transaction primitives. `LauncherPlatform.cs` chooses host
executable names, locks, restart/open behavior and file operations.

Release selection and UI do not own installation safety. Follow the existing
validation/staging/rollback code when updating downloaded content. Keep package
integrity, local data, settings and saves intact. Do not replace a whole settings
file to change one option. `ReleaseNews.cs`, release-note classes and
`WhatsNewPage.cs` handle displayed news; `CrashReports.cs` and
`DiagnosticIssueDraft.cs` handle diagnostics. Review redaction before collecting
or sending diagnostic data.

Root `Play.cmd` variants and `Play-Native-Mac.command` launch development builds;
they are not the packaged launcher's installer. `tools/build-launcher.ps1` and
`tools/build-mac-launcher.py` build the desktop UI. `tools/launcher/macos/` holds
native project/adapters; shared behavior stays above it. Do not run the
interactive launcher as an automated gameplay test.

## Checks

`tests/test_launcher_compatibility.py` checks local selection and the API-free
workflow; C# behavioral fixtures under `tests/launcher_*.cs` cover updates,
settings, sessions, startup, downloads and lifecycle. Python harnesses choose
appropriate compilation/host checks. See `.github/workflows/launcher-build.yml`,
`launcher-compatibility.yml` and [compatibility docs](../launcher-compatibility.md).
Use the harness's explicit result; a skipped compiler/native UI test is not a
pass on that host. A Windows build does not prove Mac UI or process behavior.

## GitHub issue automation

Issue forms: `.github/ISSUE_TEMPLATE/`; normalization/labels/version options:
`tools/issue-forms.py` and issue workflows under
`.github/workflows/`. PR resolution notes: `tools/issue-resolution.py`,
`.github/pull_request_template.md` and `issue-resolution-comments.yml`.
Use live GitHub issue/release metadata for a request that requires current data.
Local Git history and stale advisory files do not replace a live issue body.
Publishing a fix, merging, releasing and creating issues are different actions;
preserve the user's actual authorization.
