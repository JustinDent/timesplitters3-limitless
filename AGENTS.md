# Project storage and texture work

The user explicitly requires a lean project folder. Preserve this preference on
all further work, including texture remastering and testing.

- Working root: this folder on the Desktop, not the old Documents location.
- Keep one game-data copy (`private/game`), one working build (`build/windows`),
  the user's `private/saves`, `config/pc-controls.json` and `config/pc-display.ini`, source, patches, and
  required build dependencies. Never use personal saves for automated tests.
- Keep one active texture pack in `private/texture-remaster/pack`. Its compiler
  enforces a 6144 MiB (6 GiB) user-requested budget. Do not silently raise the budget. Use 4K
  selectively for major surfaces and smaller sizes for other assets.
- Check physical disk usage before each large texture batch. Count source art,
  not just compressed DDS files. Keep temporary texture outputs below 256 MiB;
  inspect and clean disposable outputs before proceeding beyond that limit.
- Do not create duplicate game extracts, full game ZIPs, release staging trees,
  or isolated worktrees unless needed for the current request.
- Reuse one isolated texture validation save directory. At the end of a test
  pass, remove its disposable virtual disk images so their sparse 4.9 GiB
  capacity does not inflate the project folder's apparent size again.
- Retain selected before/after screenshots and validation summaries; remove
  redundant frame dumps and runtime texture exports once mapped to the catalog.
- Recycle disposable files with explicit, resolved paths contained in the
  intended test directory. Keep authoritative game files, user saves/settings,
  source artwork, and recovery records. Never bypass a denied deletion.
- Generated images and original game assets stay under ignored `private/`.
  Keep prompts, actual source resolution, resampling, and coverage recorded.
  Do not describe resampled 4K files as native 4K art or a partial pack as a full
  game remaster. Use built-in image generation for artwork.
