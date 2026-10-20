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

### Main integration validation

Checkpoint `5830f12` has been integrated with fetched main `7b0838a`. Both sets
of validation notes and Mac backlog entries are retained; the native entry
is now **MAC-058**, since main already assigned MAC-057 to optional texture
delivery. Main's fixed-60 default, retired interpolation engine, launcher
repairs and packaging changes remain in place.

The native schedule now uses `pc_native_clock.h`, independent of the retired
presentation helper. An additive patch removes the old native interpolation
guard before main's retirement patch runs; the final renderer has inert
interpolation entry points. The native guest timing and flip-gate bypass
remain opt-in. Legacy timing settings still migrate to main's fixed values.
An upgrade test covers the pre-merge native checkpoint and idempotent reapply.

The bounded runner is now `tests/native_refresh_game.py`, with only original
60 or native 60/120/144 modes. Native acceptance requires recorded guest
presents and zero interpolated/other presents, rather than an initialization
message. `--label` keeps integration evidence separate from earlier captures.

Windows Release and its built-in checks pass. **14 timing/default/retired-engine
checks**, **119 patch/launcher/Mac-tool tests with 13 subtests** (two host-dependent
skips), and the backlog integrity test pass. Current integration build SHA-256:
`95660e9e93227cbb05e2f1b0a66bd97caa609c9c8cbef4ad2e68fc79f4bd23e2`.

The original-60 and native-144 120-second opening-level replays both pass
movement, hand animation, firearm handoff, ammunition use and pause/resume.
Native presentation records contain guest frames only, with no failed presents.
Matching one-second walking spans measure 6.256 and 6.262 world units/s
(0.10% difference). First firearm observations are 73.062 and 72.063 seconds;
both runs end with 16 loaded rounds and 80 in reserve. These observations
do not establish identical event cadence, collision behavior or full-level
correctness. The native run overlapped source tests, so it is not a controlled
performance benchmark.

Logs use `logs/native-main-*`; isolated evidence uses `main-integration`
labels, with source hashes in `native-main-comparison.json`. Real Mac execution,
broader levels and texture/camera animation acceptance remain pending. The
runtime overlay's cap label still reflects the fixed console flip-gate setting
even when native pacing bypasses that gate; its label needs a separate repair.

### Scripted camera fractional motion

The original US `sub_000C73C0` translates a camera using the fractional delta
but skips the entire operation when the whole-tick field is zero. It also
returns zero in that case; its caller at `0xCE076` can interpret this as
completion and clear a transition flag. The direction routine `sub_000C7610`
has a similar gate, followed by per-call angular easing.

The native load policy lets these two routines consume live fractional time.
The division at `0xC7651` converts the original easing coefficient `a` to
`1 - (1 - a)^delta`, using stable logarithm/exponential arithmetic and keeping
the original one-tick value. Other divisions and the disabled native path
retain their original arithmetic. The additive camera patch adds a
site-aware scalar-division hook; no generated game C is edited.

`tests/native_camera_motion.c` executes both original routines with mocked
collision/sector lookup. It reproduces translation loss under the old gate,
checks that a distant target does not report completion on a fractional step,
and compares camera translation and angular decay at 60/120/144. It also
covers pause, quarter-speed time, catch-up, angular wrap, direct target snaps,
stack/register preservation and unchanged integer deltas. A gate-only repair
failed the angular comparison before the easing hook was applied.

The native timing, bounded capture cleanup and existing auto-look tests pass
**15 cases**. Recompiler checks pass **352 tests and 87 subtests**, with 41
skips; **37 original floating-point/vehicle regression checks** and **22
patch-migration/backlog checks** also pass. Windows Release and its built-in
checks pass. Build SHA-256:
`6609f626278bc4ce6d77c9bd1597c74c7e4a748973a36ebcecc1ef6d3ad5acb1`.

Both 240-second natural-intro runs pass scene/handoff/no-crash checks, with
guest-only presents and no presentation failures in native mode. First
player/firearm observations are 158.531/183.625 s for original 60 and
157.625/182.141 s for native 144. Subtracting embedded animation time from
the first observed NPC idle and handoff clips estimates the intervening
span at 20.678 and 20.599 s (0.079 s difference). These estimates help
separate initial loading delay from clip progression, but do not prove exact
event timing or a complete animation-rate match.

The native trace retained 7.309 s of unpaused Story intervals after 160 s:
141.748 mean CPU updates/s, p50/p95/p99 intervals 7.002/7.781/8.428 ms, and
34.706 ms worst interval. Both game and raw deltas sum to 1.0 second per
wall second. The timing summary now reports those two clocks separately,
including quarter-speed test coverage. This is a bounded captured diagnostic
run, with source tests overlapping part of the intro; it is not sustained
144 FPS or monitor-scanout acceptance. The 20,000-row trace cap limits the
recorded gameplay interval after the long intro.

Local evidence is `native-camera-natural-comparison.json`, its source files
and their hashes under `private/test-saves/high-refresh`, plus
`logs/native-camera-*`. Reviewed end-of-run captures show the firearm/HUD and
rendered opening environment on both paths; they are different moments, not
matched visual-fidelity comparisons. The runner now explicitly recycles
early and rolling captures to retain only three images per run, rather than
relying on the runtime's KEEP_LAST option, which also retains early frames.
Camera samples from `--trace-camera` are asynchronous half-second reads and
can observe temporary rendering cameras. The comparison identifies the
dominant Story camera pointer; it cannot establish per-frame visual smoothness
or prove that every camera path exercised the repaired routines.

The broader camera audit remains open, including the mode-specific branch
at `0xCD70F`. Do not broaden the load policy without following register uses,
per-call constants and the affected original behavior. Moving targets, real
collision geometry, vehicle cameras, multiplayer cameras and full cinematics
need scene checks.

### Camera effect timer, collision cadence and kick decay

The follow-up covers the outer gate at `0xCEA8D` and `sub_000C79C0`.
The routine's effect timer and phase increments already use fractional game
time, but the caller skipped zero-whole-tick intervals. A kick could also be
absent from a newly supplied base pose on those skipped frames.

Inside the routine, `0xC7D73` and `0xC7E0C` bound a whole-tick decay loop for
camera fields `+0x3A4` and `+0x3AC`, with factors at `+0x3A8` and `+0x3B0`.
The original kick setter's observed caller supplies a 0.8 factor for both.
The native policy runs this loop once per advancing interval and raises those
factors to elapsed ticks at multiply sites `0xC7DA8` and `0xC7DC5`. It preserves
the original one-tick multiplication, threshold clamp and guest integer fields.
Pause skips the entire effect evaluation.

`tests/native_camera_effects.c` executes the complete original effect routine,
its three original matrix helpers, and the original caller's gate/call slice.
Only a terminal RET is added at that slice's join; the complete caller is not
executed. The fixture supplies a fresh base pose and unobstructed collision
services. It reproduces lost timer time with the old gate and verifies live
offset application, collision-call cadence, kick decay, thresholds, quarter
speed, three-tick catch-up, flag-driven reset, pause and stack preservation at
60/120/144. The two loop loads are tested together with the decay multipliers,
not treated as independent pause gates.

The combined native timing, camera, auto-look and original floating-point/
vehicle suites pass **53 tests**. Windows Release and built-in checks pass.
Build SHA-256:
`5820d4f6bd4dda225bf7181cb67f638e1712d7b1a2bcbbf5ddbdc57a0c307d95`.
Logs use `logs/native-camera-effects-*`; camera observations now include effect
time, kick amplitudes and decay factors. These asynchronous samples remain
diagnostics, not per-frame or complete collision acceptance.

The separate 120-second original-60 and native-144 movement/firing/pause runs
both pass every runner check. Walking measures 6.255391 and 6.259301 world
units/s (0.063% difference), and both finish with 16 loaded rounds/80 reserve.
There are no nonzero kick samples in either asynchronous camera trace; live
kick acceptance is therefore still pending. The unprofiled native interval
after 85 seconds contains 4,635 intervals over 34.465 seconds: 134.484 mean
updates/s, p50/p95/p99 7.078/10.285/11.433 ms and a 12.756 ms maximum. Both
game and raw deltas sum to one second per wall second. Five-second windows
fall to roughly 111 FPS during movement/firing, so sustained 144 is unmet.
Neither run captured frames or enabled CPU profiling. Source hashes and
comparison details are in the ignored `native-camera-effects-comparison.json`.

A separate 120-second `camera-profile` replay passes all checks. Its final
200-Hz sample contains 19,976 main-thread samples, 16,133 on CPU, including
4,706 leaves in `NtQueryVirtualMemory` and 2,513 in `sub_003D6690`. Repeated
memory queries in text/camera call chains warrant causal investigation.
Optimized static-function names may be folded; names such as `fprintf` or
`pc_online_disconnect` in a chain do not establish that those named operations
caused the cost. These totals include startup, loading and menus and do not
measure GPU completion. Runner metadata now explicitly records profiling,
capture, resource-sampling and PresentMon flags; the obsolete GPU-trace option
was removed because the current runtime no longer implements that hook.

### Text wrapping query overhead

Following the profile chain into original `sub_00040C10` shows that its loop
calls the prompt-aware width routine while building a stack-local fragment.
The prompt wrapper previously queried both its argument stack and that text,
even when one readable mapping covered both. `pc_prompt_guest.inc` now reuses
the validated forward stack span within that one wrapper invocation. Text
outside the span receives a fresh query; rewritten scratch storage retains
its separate write-protection check. Nothing is cached across guest calls.

Windows real-VM tests confirm one read query instead of two for an ordinary
stack-local fragment, translation/ABI preservation, fallback for text below
the stack, strings crossing differently protected regions, no-access fallback,
argument frames crossing a boundary, and fresh checks after protection changes.
Windows Release and all built-in checks pass. Equivalent stack-local cases
were added to the Mac self-test; that host remains unavailable.

The unprofiled 120-second native replay passes all gameplay checks and ends
with 16 loaded rounds/80 reserve. The interval after 85 seconds averages
138.101 updates/s across 34.728 seconds, compared with 134.484 before this
change. Its p50/p95/p99 are 7.023/9.040/10.121 ms, with a 12.819 ms maximum.
Both clocks retain a 1.0 game/raw second per wall second. This single before/
after comparison suggests improvement but is not a repeatability claim;
five-second windows still fall to 119.8 FPS during the stress replay.
Build SHA-256:
`f4800caaa98d593b8b62bc657c95081463a8a35d2a7fd1a139b3dc712bf19b04`.
Evidence: `logs/native-prompt-span-*` and ignored
`native-prompt-span-comparison.json`, with source trace hashes.

The separate 120-second `prompt-profile` run also passes all checks. It
contains 19,970 samples (15,748 on CPU). `NtQueryVirtualMemory` leaves fall
from 4,706 to 4,055; the text-wrap query chain falls from 1,881 to 1,093
samples. Total reported main-thread CPU time is 94.3 versus 92.8 seconds,
so this does not establish a reduction in overall CPU time. The narrower
chain reduction supports the redundant-query diagnosis, while whole-run
variation and remaining render/update costs still require investigation.
