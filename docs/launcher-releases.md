# One launcher release for Windows and Mac

The **Launcher release (Windows + Mac)** workflow builds the Windows x64 launcher
and native Apple Silicon Mac app from the same source revision and shared version.
It publishes both packages in one GitHub release only after both hosts pass their
launcher regressions and package checks. Each installed launcher selects its own
platform's update manifest. Game releases remain independent.

## Publish an update

1. Change `LauncherVersion.Value` in `tools/launcher/LauncherVersion.cs`, plus its
   assembly/file attributes, to a new unused version. Keep the attributes numeric
   (`X.Y.Z.0`); a preview may use `X.Y.Z-rc.1` for `Value`.
2. Merge that change and the desired launcher updates into `main` through a PR.
3. In **Actions → Launcher release (Windows + Mac) → Run workflow**, select `main`,
   enable **publish**, and run it once. The workflow creates `launcher-vX.Y.Z` at
   the tested commit and publishes both packages. There is no second Mac release
   step and no locally installed SDK or artwork needed on the maintainer's device.

Alternatively, push a `launcher-vX.Y.Z` tag on the merged commit. Its version must
match the source, and the commit must be contained in the default branch. Do not
create a separate manual GitHub release for that tag: let the workflow own its
draft and upload sequence. GitHub's standard Actions token supplies publication
access; no personal access token is needed.

Leave **publish** off for a build/test run with downloadable Actions artifacts.
Relevant launcher PRs run this validation automatically, with no publication
credentials. Windows is built first so both hosts receive the same original
artwork and newly generated shared icon. Mac uses the pinned SDK and locked NuGet
dependencies. Source artwork remains ignored; `config/launcher-artwork.json` pins
the archive and three embedded PNG checksums from the existing Windows release.
The artwork extractor reads that assembly in reflection-only mode. Changing art
requires updating those shared pins after the new assets are available.

## Published assets and failure handling

Each release contains two versioned ZIPs, two permanent-name ZIP aliases, their
four SHA-256 sidecars, two platform build receipts and these update manifests:

- Windows: `launcher-update.json`, selecting `windows-x64`.
- Mac: `launcher-update-macos-arm64.json`, selecting `macos-arm64`.

The workflow verifies internal file manifests, archive checksums, permissions,
source receipts and the uploaded GitHub asset digests before making its draft
public. A failed build leaves no release. A failed upload leaves an unpublished
draft; rerun the failed job to finish it. It can repair its own incomplete draft,
but refuses to overwrite an unrelated release, move a tag or replace published
assets. Fix a published package by releasing a new version.

Stable launcher releases become **Latest**, preserving the Windows README download
URL and providing the equivalent Mac alias. Previews do not replace Latest.
Publication explicitly refreshes the issue-form version workflow because releases
created using `GITHUB_TOKEN` do not trigger its release event.

The Mac target is ARM64, not Intel. The app retains its existing ad-hoc signing;
Developer ID signing/notarization and clean-machine Gatekeeper acceptance remain
separate delivery work. This workflow releases the launcher only; native game
packages and original game data are not bundled. CI package tests use disposable
fixtures and never use personal saves or settings.
