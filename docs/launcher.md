# Play with the Windows launcher

[Back to the README](../README.md)

Open the [latest Windows launcher release](https://github.com/TheDoffman/timesplitters-future-perfect-pc/releases/latest)
and download its `TimeSplitters-Launcher-…-windows-x64.zip` asset. Extract it
completely into a writable folder and open **TimeSplitters Launcher.exe**.
Keep the companion files in `Bin` and `Support` with it. Windows 10/11 x64 and
.NET Framework 4.8 are required; no developer tools or GitHub login are needed.

## First-time setup

1. Choose **Preview** under **Update channel**. Current game releases are
   prereleases, so they do not appear in Stable.
2. Select **Install release**. The launcher downloads a published Windows game
   runtime from this project's GitHub Releases and checks its integrity before
   installing it. It does not download original game data.
3. Select **Locate ISO**, or **Use extracted files**, and choose your supported
   US Xbox game. An extracted folder must contain `default.xbe`, `pak` and `xmv`.
4. Wait for the import to finish and select **Play Game**.

The launcher verifies and imports the data once. The ISO is no longer needed
for subsequent play; the imported data must remain installed. The launcher
minimizes after detecting the game window and restores itself when the launch
finishes. A failed startup leaves status and logs accessible.

Use **F2 inside the game** to edit keyboard/mouse controls. Resolution is under
**Settings → Display settings**, or **Options → Display settings** while paused.
See the [Windows settings guide](windows-settings.md) for the full controls.

## Everyday play and updates

Open **TimeSplitters Launcher.exe** and select **Play Game**. An installed game
can still be launched when a release check fails or you are offline.

**Stable** offers stable game releases; **Preview** also includes published
prereleases. The launcher checks published releases, not development branches.
A newer GitHub commit does not become an installed game update until a compatible
release is published. Launcher and game versions are separate.

Use **Check updates**, review **Release notes**, then select **Update release**
when offered. Close the game before installing updates or restoring a previous
release. The launcher downloads and verifies an update before applying it and
preserves imported data, saves and existing settings. It may restart during
installation. Leave room for staging and one rollback snapshot in addition to
the installed game; optional textures also need their own space.

Launcher updates are checked separately at startup. **Open installed launcher**
lets you continue with the installed launcher while a startup check is underway.

## Saves, logs and recovery

- **Saves** opens the local save folder.
- **Game files** opens the installation directory.
- **View logs** opens launcher diagnostics; game logs are also under `Logs`.
- **Support** opens this project's GitHub issues.
- **Restore previous** swaps back to the retained game release when available,
  preserving your data, saves and settings.

Keep the whole installation together, especially `Data`, `Settings`, `Saves`
and `.launcher`. The latter contains updater recovery and rollback records;
retain it throughout updates and recovery. Normal updates do not require manually
copying game files or reimporting your ISO.

If **Play Game** is unavailable, check that both a game release and verified US
Xbox data are installed and that another game instance is not already running.
An unsupported or incomplete existing `Data` folder is retained for inspection,
not overwritten. Check the status message and logs before changing files.

## Report a problem or request an improvement

Use the [bug report](https://github.com/TheDoffman/timesplitters-future-perfect-pc/issues/new?template=01-bug-report.yml)
or [enhancement request](https://github.com/TheDoffman/timesplitters-future-perfect-pc/issues/new?template=02-enhancement.yml)
form. Search existing issues first and report one problem or suggestion at a time.

Game and launcher versions are separate, and an available update may differ from
the version you have installed:

- **Game version:** copy the installed game version from the launcher. Select
  **Game files** and open `BUILD.json` for its `version` and `source_commit`.
  Report `NOT INSTALLED` if setup failed before installing the game, or
  `UNVERSIONED` if that is what the launcher shows.
- **Launcher version:** copy **LAUNCHER v...** from the header. If it will not
  open, check `version` in `Support/Launcher/manifest.json` in the installation.
  The `launcher_version` in a game's `BUILD.json` describes the launcher bundled
  with that game package; a separately updated launcher can be newer.
- **Build source:** include Stable/Preview when used, the exact release link or
  ZIP filename, and whether you installed through the launcher or manually.
  Source builds should include `VERSION`, `git rev-parse HEAD`, and any local
  changes shown by `git status --short`. These commands work on Windows and Mac.
  If you cannot identify a version, say unknown and describe what you checked.
- **Evidence:** use **View logs** for `.launcher/launcher.log`, and check `Logs`
  for game output. Source builds, including native Mac builds, write diagnostics
  under the checkout's `logs` folder. Attach the relevant log and screenshots,
  with reproduction steps, OS version, hardware and relevant settings.

Remove personal paths, IP addresses, invitations and secrets before posting.
Keep original game files, ISOs, BIOS files and personal saves private. You do not
need to delete settings or saves to complete a report.

## Older portable packages and source builds

A standalone game runtime ZIP is an alternative for manual installation. Extract
the entire package and use its **Play.cmd** entry point; older packages may open
the importer and game directly instead of the graphical launcher. On first use,
choose the supported ISO or extracted game folder. Preserve `Data`, `Settings`
and `Saves` when manually upgrading, and keep `Support` and the licenses with
the package. This is separate from the recommended launcher-managed setup above.

The repository's developer **Play.cmd** starts an existing local build. Follow
the [build guide](building.md) if you are working from source; downloading the
source repository alone does not install a playable game.
