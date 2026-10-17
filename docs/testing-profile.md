# Reusable gameplay test profile

The local **Test Lab** profile unlocks story levels, Arcade content/characters
and challenges. Collectible/pickup progress is preserved from a fresh profile,
so collection can still be tested. Story completion masks and challenge awards
are already filled; use a separate fresh profile when testing those unlocks.

On Windows, launch a disposable copy for up to ten minutes:

```powershell
python tools/testing_profile.py --play --seconds 120
```

This restores the canonical profile into `private/test-saves/quick-test/saves`,
uses isolated controls and both display configurations, and skips startup videos
through the existing launcher switch `RECOMP_XMV_PLAY=0`. Select **Test Lab** in
the game's profile picker. The session ends at the chosen duration or when you
close its window. Logs are capped at 4 MiB each; known disposable partition
images are recycled after exit. Profile changes made during this session are
discarded on the next preparation. Never prepare the same directory while a
game is using it. An interrupted runner can leave `run.lock`; check that its game
has stopped before removing that marker.

A separate Test Lab copy is also installed in this PC's personal save location,
`private/saves/UserData/45410066/544553544C41`, for manual testing through the
normal launcher. Select **Test Lab** in the profile picker. Progress made there
persists like any personal profile; the bounded runner instead resets its own
disposable copy. This local installation does not travel through Git.

For a custom test harness:

```powershell
python tools/testing_profile.py --prepare private/test-saves/my-test
```

This prints only the required runtime environment settings. Apply them to that
test's child process; it does not change your terminal environment. Python
harnesses can import `prepare_run` from `tools/testing_profile.py` to obtain the
complete child environment. Custom harnesses must bound execution/logging and
recycle their known partition images after exit. Use a dedicated directory;
the tool refuses unowned nonempty directories and personal save seeds.

The canonical fixture is ignored local data at `private/test-fixtures/unlocked`.
It is already created on this PC. On another checkout, create one fresh profile
using an isolated save and isolated input/runtime/display configurations, exit
the game, then run:

```powershell
python tools/testing_profile.py --seed private/test-saves/fresh-profile/saves
python tools/testing_profile.py
```

The seed must be a fresh disposable profile, never `private/saves`. The tool
verifies the supported US XBE hash, save version and runtime HMAC signature,
applies the US game's unlock masks/grades, and re-signs the save. All other
profile bytes, including collectible/pickup data, remain unchanged. The fixture
manifest detects accidental edits; creation refuses to replace an existing
fixture. Saves and original game assets are never committed. Mac runtime
validation and runner integration remain queued in MAC-025.
