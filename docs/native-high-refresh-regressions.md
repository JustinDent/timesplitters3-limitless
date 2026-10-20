# Native high refresh regression record

The user requires previously addressed bugs to stay resolved while native
144 FPS, animations and texture fluidity are developed. This record starts from
main `5db4efc` and the existing issue-resolution evidence in `STATUS.md`.
It is an initial coverage inventory, not an exhaustive certification of every
historical issue. Expand it as timing-sensitive fixes are audited.

## Current evidence

Windows x64, supported US XBE, pinned runtime, local development branch
`feature/native-144-fps`, 2026-10-20. The initial Windows gameplay comparisons
used executable SHA-256
`80771b53aaa3a15b04530a4b7427106e7516093402816e27a29d5d6dd1bfc31c`.
Later script-event and NPC repairs use separate builds and stronger runner
observations; their hashes and results are recorded below. The latest NPC
comparison repairs the skipped-intro opening regression in a bounded test.

The following focused source checks passed **91 tests** in 61.83 seconds.
Local evidence: `logs/native-144-preserved-fixes.log`. They exercise the
existing fix mechanisms; they do not execute all affected levels with the
native clock enabled. Mac execution remains unavailable here.

| Previously addressed behavior | Current source checks | Native 144 FPS scene acceptance |
| --- | --- | --- |
| Companion jeep dismount loop, issue #100; floating-point/control-flow joins | `test_recomp_vehicle_joins.py`, `test_recomp_float_joins.py` | Scotland jeep encounter, dismount/remount and combat transitions pending |
| Terminal depth clears, issue #47 | `test_terminal_depth.py` | Possession and return to player camera pending |
| U-Genix monitor composition, issue #49 | `test_camera_feed.py` | Crow/logo/scanlines, camera motion and aspect-ratio comparisons pending |
| Story Objectives health and shield, issue #11 | `test_objectives_health.py` | Damage, shield change and objective-screen updates pending |
| Frontend profile acceptance and menu mouse behavior | `test_profile_accept.py`, `test_menu_mouse.py` | Startup/profile and pause navigation observed; full menus, sliders, physical input and settings preservation pending |
| Audio timing and buffering, issue #8 | `test_audio_timing.py` | Playback cadence and synchronization during cinematics/gameplay/pause pending |
| Automatic look behavior | `test_auto_look.py` | Aiming, scripted look and vehicle camera checks pending |
| Texture loading, DDS handling, mode toggles and fallback | `test_texture_runtime.py` | Animated/scrolling surfaces, replacement textures, alpha and transitions pending |
| Cutscene letterboxing and channel clear rules | `test_cutscene_letterbox.py`, `test_channel_clears.py` | Representative cinematics, ultrawide and texture-camera transitions pending |

Outstanding inventory includes Ghost Gun/spirit rendering, gameplay/frontend
FOV isolation, terminal mouse routing, multiplayer movement/networking,
launcher/install/update behavior and platform-specific fixes. Use their
existing test suites and issue-resolution notes before assigning a result.
An unchanged file, an old passing result, or another platform's successful run
does not replace the relevant current check.

## Opening-sequence regression

The original timing and native timing at 60 FPS reach firearm/HUD gameplay.
Read-only sampling of the original 60 FPS reference shows weapon ID 14 equipped
by about 80 seconds and retained through the movement/firing replay.

At native 144 FPS, captures show unarmed hands and no firearm/HUD. Additional
sampling established that movement and hand animation can progress: the
problem is more specific than a completely frozen animator. The opening weapon
handoff/progression remains unresolved. Do not apply a forced weapon grant as a
substitute for repairing the original behavior.

The runner now records player position, hand/body/weapon clip times, equipped
weapon IDs and weapon actions. Its first-mission stress checks require actual
movement, advancing/changing hand animation and the expected firearm equipped.
The checks reject a frozen scene and an animated but unarmed player. They are
bounded smoke checks, not measurements of correct weapon cadence or full-game
progression. Unit checks for the timing hook and observations pass eight cases.

A 150-second run without requesting an intro skip remained in the opening
cinematic and did not reach Story gameplay. It is insufficient to isolate a
skip-specific regression. A subsequent 300-second native 144 run reached Story
at 157.609 seconds without skipping and remained equipped with unarmed weapon
ID 37 through 299.734 seconds. Thus the failure also occurs when the cinematic
finishes naturally. It is not explained solely by the skip action.

The older runner's 300-second metadata passes only its scene/no-crash/native
hook assertions. A separate post-run handoff check evaluates the actual
observations: original 60 FPS passes, native 144 FPS fails. Local records are
`60-handoff-analysis.json` and `native-144-natural-intro-handoff-analysis.json`
under `private/test-saves/high-refresh`. New runner versions require the
firearm once at least a minute of Story-player observations is available,
including runs without the movement/firing stress replay.

Natural-intro runs have a separate filename tag to preserve their scope.
Validation records include the executable hash, runner hash, selected mode and
explicit checks. Test data stays isolated from personal saves/settings, and
the canonical Test Lab fixture is preserved.

### Fractional script-event repair

The original US level-script dispatcher `sub_00228870` returns immediately
when the integer game delta is zero. Its first-mission callback
`sub_00218910` grants loadout 25 (firearm ID 14) when the RBS1 actor's animation
`0xA1C` crosses the `0x2000` event. The event reader `sub_00061570` examines
only the current and previous animation times via `sub_000585E0`. Skipping a
fractional frame can therefore lose a crossing permanently.

The shared load policy now changes the dispatcher load at `0x228880` to test
the live fractional game delta. It does not modify guest integer timers or
other reads of the same delta. A compiled test executes the original
dispatcher, handoff callback and animation-event readers with mocked asset
lookup and inventory side effects. It reproduces missed events with the
old gate, catches the event exactly once at every phase of the 144:60 cadence,
and checks pause, original-mode behavior and stack balance. Nine timing tests
pass. The recompiler suite passes 352 tests and 87 subtests, with 41 skips.

Windows build SHA-256
`f74f5b15364a1d742a37c6e7cd8500cf665c19ab949127329954b174e6ee6006`
includes this repair. Its 120-second native 144 stress/pause/capture run now
reaches the pistol, HUD and next objective, but first equips at **93.579 s**.
The original 60 reference equips at **73.047 s**. Movement, changing and
advancing hands, pause/resume and eventual handoff pass; the stress-window
requirement to already have the firearm fails. Correct opening timing remains
unresolved. These diagnostic runs are not controlled performance benchmarks.

This run also exposed a runner reporting defect: its final equipped check
reused the stress-check key, overwriting a failure in metadata even though the
terminal assertion failed. New runners use `opening_firearm_equipped_at_end`
separately and test this late-handoff case. Preserve the old metadata as
historical output, but use `native-script-events-analysis.json` for its
corrected post-run assessment. Local evidence is under
`private/test-saves/high-refresh`; selected images are
`native-144-unarmed-before.bmp` and `native-144-handoff-after.bmp`.
Build, unit and runtime logs use `logs/native-script-events-*`.

Two further runs on the same repaired executable omit stress and pause inputs
and sample the RBS1 actor. Original 60 equips at **73.063 s**; native 144
equips at **89.578 s**. During comparable portions of walk clip 42, the
reference covers about **3.020 world units/s** and native covers **1.240**,
a **2.436×** difference. The native animation itself advances about one second
per wall second. This separates slow actor translation from the repaired
event delivery and identifies the next motion audit. The compared spans,
positions and source hashes are saved in `native-script-events-npc-comparison.json`.
The actor observations come from `60-handoff-loadout-gameplay.json` and
`native-144-handoff-gameplay.json`; these are bounded first-mission diagnostics,
not full-game timing acceptance.

### NPC movement dispatcher

The next audit found a second whole-tick pause gate: `sub_0015EAE0` loads
the integer game delta at `0x15EB0C`, returns when it is less than one, and
otherwise overwrites that register. This dispatcher advances AI timers by
the fractional delta and calls the movement state dispatcher. Animation
root displacement has already been generated for each fractional update;
skipping its consumer loses those updates.

The shared policy now treats this specific load as a live fractional-time
gate too. A compiled test executes the original dispatcher and verifies its
actual elapsed-time counters and movement callback cadence at 60, 120 and
144 updates/s, plus pause/resume, bounded catch-up, register/stack preservation
and unchanged integer fields. It reproduces the old counter slowdown.
All **10 native timing tests** pass. Windows Release and its built-in checks
pass. The new executable SHA-256 is
`614eec11e1896273a4424988893b0a298122149c494136c159c26f9f7fb9c5d5`.

Its 120-second native 144 run passes all current runner checks, including
equipped-before-stress, movement, advancing/changing hands and pause/resume.
First firearm equip is **72.063 s**, compared with **73.063 s** in the earlier
original-60 reference. Matching clip-42 spans from 56.047 to 58.047 seconds
measure **3.02986 versus 3.03783 world units/s**, a **0.26%** difference.
These spans precede stress/pause inputs. This is evidence of restored opening
movement, not exact event parity or full-game timing acceptance.

A subsequent **300-second unskipped** run on this same build passes the
eventual-handoff check: Story observations begin at 157.609 s, firearm ID 14
first appears at **182.125 s** and is retained through 299.750 s. The earlier
unskipped failure remained unarmed through the end of its 300-second test.
This confirms restored progression without the skip action; matching
cinematic duration against original 60 FPS still needs a reference run.
Evidence uses the `native-144-natural-intro-handoff-none` prefix and
`logs/native-npc-natural-intro.log`. No stress/pause replay was used.

The first attempted diagnostic stopped at 50 seconds because its fixed NPC
watch address no longer matched the actor allocation. It did not pass.
The successful rerun uses `--trace-handoff --handoff-watch none`: bounded
read-only actor sampling through the live tag lookup, without a fixed memory
watch. Evidence and source hashes are in `native-npc-dispatch-comparison.json`
and `native-npc-144-handoff-{gameplay,validation}.json` under the isolated
high-refresh test directory. Logs use `logs/native-npc-dispatch-*`.

Before this NPC change, the combined existing-fix, timing and patch-transition
suite passed **120 tests in 404.79 s**. The command includes the suites listed
above plus `test_native_timing.py` and `test_patch_updates.py`; output is in
`logs/native-script-events-regressions.log`. This remains bounded source
coverage, not full-game or Mac validation.

### Player movement and damping

The preserved original-60 stress replay moves the player at about 6.25 world
units/s during steady forward motion, while the NPC-repaired native 144 replay
still moves the player at about 2.6. The player dispatcher `sub_000D5EB0`
has its own whole-tick pause gate at `0xD5EBF`, followed by fractional movement
and timer consumers. Its integer load at `0xD6251` is a separate state update
and must retain its original value.

The player gate now uses fractional time. The same routine halves
`player+0x1008` at `0xD5F60` once per update; running that unchanged at 144 Hz
would alter its half-life. A generic optional scalar-multiply hook keeps
ordinary multiplication as its default. The title policy changes only this
audited site to use `factor ** fractional_delta`, preserving the original
decay per game second. Other constants and multiplications remain exact.

The original player routine is compiled and executed in a test that checks
its real timer and damping arithmetic, pause/resume and bounded catch-up at
60/120/144. World/AI helpers are mocked on the inactive-player branch; this
does not establish full player physics. **11 native timing tests** and
**352 recompiler tests with 87 subtests** pass (41 skips). Windows build and
its built-in checks pass. The runner now samples loaded/reserve
ammunition and requires consumption during its firing replay; reload-only
transfers cannot satisfy that check.

Build SHA-256
`a52dc8bbd81f92f27d203cce27ac05fec8ee9f507863106ad7831493660736b6`
passes 120-second stress/pause runs at native 144 and original 60 FPS.
In the unobstructed forward segment around 86.56–87.56 seconds, speed is
**6.25801 versus 6.25497 world units/s** (0.049% difference). The previous
NPC-only build moved at 2.61088 in that segment. Both current runs start with
16 loaded and 112 reserve rounds, have 12/112 near 90 s and 0/112 near 100 s,
and finish reloaded at 16/80 near 115 s: **32 rounds consumed**. First handoff
is 72.047 s native versus 73.063 s reference. Every runner check passes.

The longer walking path diverges as the runs take slightly different headings
and approach obstacles; it is not used as an unobstructed speed comparison.
Exact acceleration, collision behavior, shot-event timing and other weapons
remain unverified. These are behavioral diagnostics with other test processes
on the host, not controlled performance benchmarks.
Evidence/source hashes are in `native-player-comparison.json`, current
`native-144-handoff-none` and `60-handoff-none` records; logs use
`logs/native-player-*`. The earlier NPC-only records were retained separately
as `native-npc-144-handoff-*`.

The same executable/runner also passes native **60 and 120 FPS** stress/pause
replays. Measured steady speeds are 6.25804 and 6.25385 world units/s
respectively, both within 0.05% of the original reference; both finish at
16 loaded / 80 reserve rounds. First handoff observations are 72.547 s and
72.047 s. The four-mode evidence and hashes are in
`native-player-rate-matrix.json`. The bounded runner terminates its child at
the deadline, producing exit code 1 on Windows; `completed` plus the explicit
checks distinguish these planned stops from early exits.

Refreshed current-source suites pass **91 prior-fix tests in 62.53 s**,
**20 patch-transition tests in 320.26 s** and **11 timing tests**. Logs are
`native-player-preserved-fixes.log`, `native-player-patch-regressions.log`
and `native-player-unit.log`. Their combined scope is 122 tests; the
recompiler's 352 tests/87 subtests are recorded separately.

## Completion gate

Keep each failed or unrun scene check open. Repair new regressions, then rerun
the relevant prior-fix check in original and native modes. Keep frame pacing,
animation/event timing, animated texture behavior, visual fidelity and
gameplay progression separate in the evidence. Completion requires the full
[goal and acceptance criteria](native-high-refresh.md#goal-and-acceptance-criteria).

At the 2026-10-20 NPC audit, the local `origin/main` snapshot was 41 commits
ahead of this branch's base. Those later changes have not been integrated or
validated here. Reconcile them without losing their fixes before claiming
preservation of all currently resolved bugs; the passing suites above cover
the source actually on this branch.
