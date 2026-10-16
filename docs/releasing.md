# Packaging and publishing Windows releases

[Back to the README](../README.md)

Run these commands from the repository root. Players can use the ready-made ZIP linked in the README.

## Package locally

To produce the same public package after building:

```powershell
.\.venv\Scripts\python.exe tools/package-release.py
```

This uses an explicit file list and default settings. It excludes all game data,
personal settings, and saves, and writes the ZIP plus SHA-256 under `build/releases`.
The package includes an isolated Windows Python/Tk runtime and the Online
companion, including automatic hosting certificate support; players do not need
Python or Git installed. Packaging downloads the pinned Windows wheels in
`requirements-release.txt` and includes their licenses. A matching successful
Release build is required. `Source/pc-port-source.zip` records the local project
source and runtime patch, including changes not yet committed.
Use `.venv\Scripts\python.exe tests/portable_online.py --stage <extracted-package>`
to test the bundled runtime, HTTPS hosting, invitations, and three-peer UDP relay
without system Python/Git on PATH. Real router forwarding still needs validation.

## Publish a release from main

Commit the intended `VERSION` and merge the changes, including these release
scripts, into `main`. Install [GitHub CLI](https://cli.github.com/) and authenticate
once with `gh auth login` using an account allowed to publish this repository.
Use the existing workspace and game data; setup/build dependencies must already
be installed. Then run in PowerShell:

```powershell
git switch main
git pull --ff-only origin main
.\Publish-Release.ps1
```

The script requires clean source and an exact match with `origin/main`. It reads
`VERSION`, validates the runtime patch stack, rebuilds Release with native checks,
runs Python tests, packages the public ZIP and verifies its manifest/checksum.
It tests the bundled Online runtime with the existing game data, without copying
the game or using personal saves. It creates a GitHub draft for the exact tested
commit, uploads the ZIP, checksum and build metadata, checks the uploaded asset
names/sizes, then publishes it. Versions containing a prerelease suffix (such as
`0.2.0-alpha.3`) become prereleases and are not marked Latest.

The description includes GitHub-generated release notes, source commit descriptions
since the most recently **published** release (including prereleases), installation
instructions, validation performed and experimental limitations. It does not use
the last Git tag or skip prereleases by asking for GitHub's latest stable release.
GitHub's [release notes API](https://docs.github.com/en/rest/releases/releases#generate-release-notes-content-for-a-release)
provides the PR/contributor notes.

To prepare and inspect the ZIP/notes locally without creating a tag or release:

```powershell
.\Publish-Release.ps1 -PrepareOnly
```

Run normally afterward to publish the same validated package. `-Draft` instead
uploads a draft for review on GitHub. Notes are regenerated on each invocation.
Artifacts and the preparation record stay under ignored `build/releases`;
Disposable staging and Python test fixtures are permanently removed using
validated paths; packaged archives and release metadata are retained. Test
fixtures are also cleaned when tests fail. No Recycle Bin support is required.
No branch switching, version bump, overwrite of existing release files, or tag
rewriting occurs. Changed prepared assets or a changed main commit stop the run.
Existing drafts/releases with the version also stop it; inspect a failed upload's
draft on GitHub before retrying. Failed package stages are retained for diagnosis.
Do not rename a package to reuse an already published version: update `VERSION`,
commit and merge it before preparing the next release.
