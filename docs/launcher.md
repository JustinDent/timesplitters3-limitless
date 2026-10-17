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

## What's new (current source)

The sidebar's **What's new** page shows game and launcher release notes inside
the same window. It compares installed and available versions for your selected
channel. Switch between **Game releases** and **Launcher releases**, then choose
**Available release** or **Installed release** to read that exact version's notes.
Published known issues appear in their own section when supplied by the release.
**Refresh notes** checks GitHub again; a failed refresh keeps notes already loaded
for this session and shows their last checked time. **View release on GitHub**
opens the original release, and **Open Home** returns to the update controls.
This page reads release information; installing an update remains a separate action.

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

For a bug report, pick your **game version** from the installed version shown in the launcher,
then pick your **launcher version** from **LAUNCHER v...** at the top of its
window. These can be different. If you cannot find either one, choose
**I don't know**. Choose **Other** for an unlisted version and mention it with
your report. The version choices refresh automatically as releases are published.

Tell us what happened and how to make it happen again, or describe your idea.
Screenshots and logs are optional; **View logs** in the
launcher opens its log folder. Remove personal information before attaching
anything, and keep game files and saves private.

Improvement suggestions only ask for your idea and optional
extra details. Versions and platform are not required. Reports are labeled
automatically by type; bug reports also use the selected platform.

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
