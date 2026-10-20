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

The broader camera audit remains open. The mode-27 branch at `0xCD70F`
is covered by the later camera-track checkpoint below. Do not broaden the load policy without following register uses,
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

### Authored camera-track sampling

The original switch table maps camera mode 27 to `0xCD691`. Its float track
clock advances before the whole-tick gate at `0xCD70F`, so native fractional
frames previously left the pose unchanged. The branch also selects its second
keyframe using the integer delta loaded at `0xCD851`. Removing only the gate
still samples the same keyframe twice on zero-whole-tick intervals.

The native policy evaluates the track on positive live game time and supplies
a one-keyframe span only when that span would otherwise be zero. Existing
fractional blend weights then evaluate the authored track at the current time.
The divisor at `0xCD88E` also uses the selected integer span, rather than
assuming it equals the float elapsed delta. A 2.5-tick interval with a two-frame
span exposed a separate error after the gate/span repair; a 1.8-tick interval
with a two-frame span covers the opposite mismatch. Multi-tick spans are
retained, as are track-end, loop, external-clock and
large-discontinuity cut behavior. This uses the game's existing keyframe
evaluation, not synthesized intermediate presentation frames.

`native_camera_track.c` executes the original complete mode branch, with its
three exits redirected to a terminal RET in the in-memory fixture only.
Synthetic linear keyframe decoding, matrix/sector/projection services and
player input are stubbed. The enclosing camera routine and real asset decoder
are not covered by that fixture. The original policy fails the fractional
pose comparison; a gate-only change also fails it. The complete change passes
60/120/144, quarter speed, pause, externally synchronized clocks, three-tick
and fractional catch-up, loop/completion, discontinuity cuts, stack and
integer-delta checks. Native-disabled division and neighboring sites stay exact.

The frontend audit also traces `sub_002DA0E0` to its raw fractional delta at
`0x4E7948`, its rotating-layer helper `sub_002DAB30`, and sheet/texture-index
helper `sub_002DAF90`. No timing rewrite is needed for these paths. Their
source tests use original code with matrix/menu services stubbed; broader
world texture/UV/water motion, rendered filtering and replacement-texture
coverage remain pending.

The combined timing/camera/frontend/auto-look/float/vehicle suite passes
**55 tests**, and Windows Release plus its built-in checks pass.
Final executable SHA-256:
`ea81dc2b9e148d2e71063e2e7ea64a2221cb61b8abed5a95825f4844298097dc`.
Logs: `native-camera-track-regressions.log`,
`native-camera-track-acceptance-build.log`, and `native-menu-motion-tests.log`.

One intermediate build's debug-settings test encountered system error 5 while
saving its disposable settings and blocked behind nested error dialogs. Its
timer repeatedly queued Save, and an attempted IDOK dismissal did not match
the observed dialog's OK control ID (IDCANCEL). `tests/pc_debug.c` now posts
Save once and dismisses save-error dialogs with WM_CLOSE. A forced read-only
save verifies bounded dismissal and unchanged settings; the final build passes
that case and ordinary Save/Cancel. The original unexpected access-denied
event's underlying filesystem cause is unconfirmed. No production persistence
behavior or personal files were changed, and interrupted runs are not counted
as passing checks.

The final 240-second unskipped native run passes scene, opening-firearm,
guest-only-presentation and no-failed-present checks. It observes 45 mode-27
samples for track 2485, from 157.625 to 179.656 seconds. The track clock
advances at 0.999546 seconds per wall second; the original-60 reference's 45
samples measure 1.000023. First firearm is observed at 182.156 versus 183.656
seconds. Subtracting the median estimated track start gives 24.830 versus
25.115 seconds to that observation. Half-second sampling and loading offsets
prevent exact event or pose-parity claims.

The reference executable is the intermediate
`0797de6d828bd395acbb5c13036d8e4deb59cbbd1ca3a9e2f6c9d865b2a0c167`
build, before the final native-only divisor correction; the final native
executable is the `ea81dc2...` build above. The original-path division remains
exact in the source test. Both runs use runner SHA-256
`16065fa19dff50563a80b802d7b6f15789dbc032a3e0690131a1e4f9a68a35d0`.
No captures or profiler were enabled. The final native trace reaches its
20,000-row limit at 154.936 s, before the track begins, so there is no
cinematic/gameplay interval distribution from that run. Extend the bounded
trace window before using another long-intro run for pacing acceptance.

Local source hashes and comparison calculations are in the ignored
`private/test-saves/high-refresh/native-camera-track-comparison.json`.
Run labels are `60-natural-intro-handoff-none-camera-track` and
`native-144-natural-intro-handoff-none-camera-track-final`. These half-second
observations establish exercised track progression and completion, not
per-frame visual smoothness, full cinematics or full-game acceptance.

## Offline network callback overhead

The previous CPU profile found repeated environment lookups in
`pc_network_enabled`. Bot animation, bounds, activity, damage and frame hooks
queried it before checking guest eligibility, including ordinary offline
actors. `pc_network_bots_guest.inc` and `pc_network_bots_lobby.inc` now reject
ineligible states/actors first. Eligible callbacks still query live settings;
the transport and ready-file polling implementation are unchanged.

The guest-dispatch fixture counts network queries across offline animation
and combat callbacks while checking original-call count, return value, stack
and pose argument preservation. It also switches the enabled result between
eligible actor calls to check that it remains live. Existing host/guest bot
behavior, movement cadence, original result-builder and native fallback tests
pass. The compiler fixture now uses the common MSVC discovery helper rather
than silently skipping when `cl` is absent from the shell PATH. These tests
are registered in the Windows/macOS runtime workflow; actual Mac execution
and network transport remain pending. The combined network, CI-scope and
backlog selection has 71 passes in `logs/native-network-guards-tests.log`.
The complete Windows build and built-in checks pass in
`logs/native-network-guards-build.log`.

Two isolated 120-second native-144 runs use the same runner
`16065fa19dff50563a80b802d7b6f15789dbc032a3e0690131a1e4f9a68a35d0`,
with walking, firing, turning, pause/resume and handoff sampling. Neither
uses captures, profiler, resource sampling or PresentMon. Both pass all
reported behavioral checks. Unpaused Story intervals from 85 seconds:

| Measure | Before | After |
| --- | ---: | ---: |
| Observed duration | 34.3333 s | 34.4999 s |
| CPU updates | 4,740 | 4,837 |
| Mean updates/s | 138.059 | 140.203 |
| p50 interval | 7.0444 ms | 7.0153 ms |
| p95 interval | 9.0831 ms | 8.4934 ms |
| p99 interval | 10.1881 ms | 9.3970 ms |
| Maximum interval | 13.3604 ms | 10.9974 ms |
| Game seconds / wall second | 1.0 | 1.0 |

The baseline executable is
`2389e61f7d36d789984ff3575ed26040953118c1a258ac12ae361b96e8ef0b46`
(overlay checkpoint `a83cf05`); the revised executable is
`a920df05eb37f18c2247ebb233a074d98cb591f8672eb7104961573705f612c7`.
Run labels are `native-144-handoff-none-network-guards-before` and
`native-144-handoff-none-network-guards-after`. Trace hashes, source hashes
and validation results are retained in
`private/test-saves/high-refresh/native-network-guards-comparison.json`.
This single comparison is encouraging but does not establish repeatability,
GPU/scanout timing, full-game coverage or sustained 144 FPS. The direct XNet
dispatch still checks host settings; no persistent environment cache was added.

## Entity velocity on fractional updates

US entity post-update `sub_000A7360` derives entity+0xF8/0xFC/0x100 velocity
from the position difference between +0x64 and +0xEC, divided by fractional
game delta. Its guard at 0xA75E7 instead checked the integer delta. On a live
interval without a whole tick, it wrote zero velocity and still updated the
previous position, losing that interval's motion from the derived velocity.
`pc_native_global_load` now supplies a live-game-time boolean at this site
only. The original path, other loads, integer timers and division formula
are unchanged; a paused interval does not divide by zero.

`tests/native_entity_velocity.c` executes the complete original routine
(0xA7360–0xA7666) and its linked-entity dispatcher (0xA7870–0xA79A2), translated
from the hash-verified US XBE. Two entities receive prescribed positions;
collision, sector and unrelated services are mocked and unused indirect
callbacks fail the test. This is a velocity-derivation test, not a physics
integration or collision-response test. It checks each update at 60/120/144,
quarter speed, pause (including a changed position with a carried integer
tick), stationary objects, three-tick and fractional catch-up, register/stack
balance and preservation of integer/float delta fields. It also covers the
original behavior and neighboring/unrelated loads.

Before the guard change, the test fails with native=1, whole=0, delta=0.25:
velocity is zero instead of 0.125 (`logs/native-entity-velocity-red.log`).
With the fix, the timing/camera/auto-look/float/vehicle/frontend/entity suite
has 56 passes (`logs/native-entity-velocity-regressions.log`). The final focused
test also checks finite results and passes. The full Windows Release build,
patch validation and built-in checks pass in
`logs/native-entity-velocity-build.log`.

Same-build 120-second original-60 and native-144 runs use walking, turning,
firing, handoff and pause/resume replay. Both pass their reported behavioral
checks and end with pistol ammunition 16 loaded / 80 reserve. The runner now
samples entity previous-position and velocity alongside animation; these are
asynchronous half-second reads, not atomic per-update observations.
Six samples in the steady forward window 86.5–89.5 seconds have median body
speeds 6.25886 / 6.25914 units per second. Position-derived forward speeds are
6.26610 / 6.21808, about 0.77% apart. This narrow observation does not establish
full movement/physics parity.

The native CPU update trace after 85 seconds reports 75.247 updates/s,
p50/p95/p99 intervals 9.9746/27.0758/31.4509 ms and maximum 84.0939 ms.
Game and raw time advance at 0.998992 seconds per wall second; long intervals
still encounter the original bounded catch-up policy. **Do not compare this
as a clean performance regression against the prior 140.203 run.**
Another game process (`sp26-cod`, PID 34128) started at 10:53:37 local time,
before both tests. A follow-up measurement found 42.875 CPU seconds over
5.837 wall seconds and 99.76% GPU 3D-engine utilization. That measurement is
after the tests, not a synchronized GPU trace; the overlap makes performance
acceptance inconclusive. The process was left unchanged. A clean pacing
retest remains required.

Both runs use executable SHA-256
`076eec0128aa55f977ee5475701797b0342c7a5cab3936e3921efb0bf7b03b34`
and runner
`65b94927ec6bed6e603ec219d2274e3ce3dd2a280ef023550196b20ac0486e2a`.
Labels are `60-handoff-none-entity-velocity` and
`native-144-handoff-none-entity-velocity`. No captures, profiler, resource
sampler or PresentMon were enabled. Validation, source/trace hashes and the
contention note are in
`private/test-saves/high-refresh/native-entity-velocity-comparison.json`.
Mac execution, collisions, other physics/weapon paths and full-game acceptance
remain unverified.

Further audit candidates remain unchanged: 0xA84F0 contains a whole-tick
physics loop whose per-step integration must be understood before conversion;
weapon routines have whole-tick gates at 0x1B6FF0, 0x1BB5FD and 0x1BD044.
The first also advances a float duration at 0x1B7194. These observations are
an audit queue, not proven fixes or permission to replace every integer gate.

## Weapon sound events on fractional updates

The original US helper `sub_001B6FF0` returns before selecting or emitting a
sound when its integer game delta is zero, even if fractional game time is
advancing. The native-only numeric guard now tests live fractional time at
0x1B6FF0. The return register is overwritten before subsequent use. Paused
updates still return; other reads and the original timing path retain their
behavior. The repeat timer at weapon+0xFC already integrates fractional
seconds, and its original reset-to-zero threshold behavior is preserved.

`tests/native_weapon_sound.c` executes the hash-verified original helper,
both sound parameter builders (0x21780 and 0x21FB0), and their initializer
(0x45390). Constants come from the same verified XBE. The audio backend and
RNG are mocked; unexpected indirect weapon callbacks fail the test. Checks
cover fractional one-shot dispatch at 60/120/144, one backend event per
accepted call, positional coordinates and nonpositional flags, sound ID and
handle, original one-tick cadence, repeat elapsed time at normal/quarter
speed, pause including a carried integer tick, catch-up and missing sounds.

The original guard fails `fractional one-shot dispatch` in
`logs/native-weapon-sound-red.log`. With the fix, 57 timing/camera/auto-look/
float/vehicle checks pass in `logs/native-weapon-sound-regressions.log`.
The 17 audio-adapter checks pass in `logs/native-weapon-sound-audio.log`;
they exercise deterministic fake-device stream/buffer behavior, not audible
output. Full patch validation, Windows x64 Release and its built-in checks
pass in `logs/native-weapon-sound-build.log`. The compiler is MSVC
14.44.35207 with Windows SDK 10.0.26100.0 and SDL 3.4.16; upstream remains
pinned to d90c92988a9f99d0580d2ca3f7efd6ec3a7b7845.

The isolated 120-second native-144 run
`native-144-handoff-none-weapon-sound` passes every reported check: Story,
native hook, guest presents with interpolation disabled, no present failures
or crash, movement, changing/advancing hand animation, firearm equipped,
ammunition consumed and pause/resume. Its exit code 1 is the runner's expected
bounded termination (`completed`, `terminated_by_runner` and `passed` are
all true). No capture, profiler, resource sampler or PresentMon was enabled.
The other game remained active; no FPS improvement or clean performance
comparison is claimed. Evidence lives in `private/test-saves/high-refresh`
and `logs/native-weapon-sound-144.log`.

This build uses base source ce77aa6 plus the weapon-sound change; executable
SHA-256 is `0f4bc5c9b26fce5b262c18ffa53a4602d3c89fdf111fa504c56a7d7f7d6ebe3a`.
Runner SHA-256 is
`65b94927ec6bed6e603ec219d2274e3ce3dd2a280ef023550196b20ac0486e2a`.
The same executable was subsequently opened for the user's manual test;
startup confirms the 144 native-update target and 3440x1440 at 144 Hz.
That startup is not evidence of sustained 144 FPS.

The helper consumes RNG before checking its repeat threshold. More live
invocations can therefore consume more random values; cross-rate RNG sequence
and audible parity are unverified. All three direct caller families require
separate cadence coverage. `sub_001BB5C0` has a whole-tick gate at 0x1BB5FD,
and `sub_00244F30` has one at 0x244FCD before duration accumulation at
0x245094 and projectile work. `sub_001C0200` is also called by several
weapon states; its caller's gates at 0x1C38CC/0x1C397F protect firing work.
Do not open these gates indiscriminately: ammunition, projectile count,
damage and random consumption need equal-duration comparisons first.
The discrete cooldown at 0x1C3243 subtracts carried whole ticks and must not
be replaced with a boolean. Complete weapon mechanics, other levels/modes,
multiplayer and actual Mac execution remain pending.

### Deferred weapon cooldown preservation

The original branch 0x1C320C–0x1C3273 subtracts the carried whole-game-tick
count from state+0x110, then invokes 0x1C0110 once when the counter expires
and callback metadata is present. `tests/native_weapon_cooldown.c` executes
these original bytes, replacing only the instruction at the final join with
RET and mocking the completion callback. The fixture supplies the enclosing
frame/register context; it does not run the whole player weapon update.

The original and native policies both pass at 60/120/144 for delays of
1/7/30/60 ticks, normal and quarter speed, and pause between updates. Each
counter expires at the expected accumulated game tick and dispatches one
completion, with no repeats afterward. Three-tick catch-up subtracts three,
while a live zero-whole-tick update leaves this discrete counter unchanged.
Alternate mode selection, primary fallback, missing metadata and absent
weapons are covered. No runtime change is needed at this site. This is a
check of a deferred action counter, not a complete firing/reload timing claim.
The focused sound/cooldown pair passes in `logs/native-weapon-branches.log`.
The final expanded timing/camera/auto-look/float/vehicle suite passes all 58
checks in `logs/native-weapon-final-regressions.log` (42.49 seconds). The 66
CI-scope/backlog contracts pass in `logs/native-weapon-sound-contracts.log`.
The cooldown addition changes tests only, so the already-built executable
and its runtime/build validation remain applicable.
Source and smoke-evidence hashes are retained in
`private/test-saves/high-refresh/native-weapon-sound-evidence.json`.

## Generic firing mixed-clock regression

This section records the pre-fix failure. The following
[event-tick scope](#generic-firing-event-tick-scope) addresses the reproduced
case; broader weapon acceptance remains open.

Audit checkpoint: d6e8fe4 plus the new test; Windows x64/MSVC 14.44.35207.
No production code changes or game rebuild are part of this audit. The
existing user session was left running.

`tests/native_weapon_fire.c` translates the complete original routines
0x244F30–0x24598E (generic firing), 0x1BC890–0x1BC988 (integer weapon-counter
update) and 0x1B5660–0x1B567B (mode lookup) from the hash-verified US XBE.
It invokes counter update then firing on every synthetic update, with the
native policy enabled at 120/144 and the original policy at 60. The fixture
uses the actor-flag 0x40 socket path, a single projectile per event and
controlled weapon definitions. Actor lookup/socket/direction, eligibility,
RNG, projectile construction, effects and sound dispatch are mocked. Unused
services and indirect callbacks assert. It verifies return-stack/register
preservation, paused updates and expected request counts. It does not run
whole actor/player callers or real collisions, damage, ammunition and audio.

Over one second, with the original gate intact:

| Configured interval | FPS | Projectile requests | Weapon+0x104 |
| --- | ---: | ---: | ---: |
| 0 s | 60 original | 60 | 0.999999702 |
| 0 s | 120 native | 60 | 0.499999851 |
| 0 s | 144 native | 60 | 0.416666836 |
| 0.1 s | 60 original | 10 | 0.166666672 |
| 0.1 s | 120 native | 10 | 0.083333336 |
| 0.1 s | 144 native | 10 | 0.069444448 |

The gate at 0x244FCD only accepts whole-tick frames. The accepted path adds
`game_float / 60` to weapon+0x104 at 0x245094; fractional time from skipped
frames never reaches that addition. The field also has consumers/reset/decay
paths in other weapon routines, so this fixture does not assign it a single
user-visible meaning such as firing cooldown or damage.

A test-only override that opens the gate on every live fractional update
raises zero-interval projectile requests to 120/144 and RNG requests to
360/432, versus 60 and 180 originally. With a 0.1-second interval, integer
cooldown thresholds still limit shots to 10, and the timer discrepancy
remains. The experiment is isolated inside the test numeric hook; **the
runtime gate is unchanged**. This demonstrates why merely replacing this
integer gate with a live-time boolean is insufficient.

The test is marked strict expected failure for `WeaponTimingMismatch` only.
That exception is raised after successful compilation/execution when the
unmodified native path differs from the original reference's timer or shot
count. Build failures, ABI assertions and other errors remain real failures;
an unexpected parity pass requires removing the marker. This known failure
is not included in passing timing counts. Measurements are retained in
`logs/native-weapon-fire-measurements.log`, with the focused pytest result in
`logs/native-weapon-fire-audit.log`. The combined timing/camera/auto-look/
float/vehicle/CI-scope/backlog run reports **124 passed, 1 expected failure**
in 46.54 seconds (`logs/native-weapon-fire-regressions.log`). No new runtime
build or game smoke was needed for this test/documentation-only checkpoint.

The next fix must separate discrete firing cadence from fractional timer
consumption, characterize the field's other writers/decay and preserve
nested sound/event timing. Acceptance must include zero and positive fire
intervals, variable/catch-up steps, slow motion, pause, the alternate mode,
multiple projectiles, actual caller states and representative weapons.
A complete damage/ammo comparison remains necessary before claiming weapon
mechanics parity. Actual Mac execution remains unverified under MAC-058.

## Generic firing event-tick scope

`pc_native_weapon_guest.h` wraps only the audited US generic firing routine
0x244F30. Native mode supplies its carried whole-game-tick span as the float
delta for this discrete transaction and restores both fields on return.
The original gate, cooldown counters, projectile loop and reset behavior
remain original code. A locally paused float interval suppresses the
transaction even if the caller retained a whole tick. The non-native path
calls the original body without changing either field.

This gives the routine's event-timer additions and its nested sound helper
the same elapsed tick span as the discrete firing gate. It does not run
additional firing calls or round every native update up to a tick. Continuous
animation/movement outside this routine retains fractional time. The additive
`native-weapon-event-scope.patch` supplies the manual wrapper; the common
patch installer orders it after native timing/input wrappers. Generation
retains `sub_00244F30_gen` reproducibly; generated C is not hand-edited.
The original clock 0x1538C0 supplies matching integer/float tick values in
its normal path, which is the reference for this discrete transaction.

The original-code fixture now executes the sound helper 0x1B6FF0, parameter
builders 0x21780/0x21FB0 and initializer 0x45390 in addition to firing, mode
lookup and counter update. Only the final audio backend is mocked. Nested
projectile/audio callbacks assert the scoped delta, and every call checks
that caller time fields are restored. Actor/socket services, projectile
construction, effects, eligibility and RNG remain mocked; this does not
prove damage, ammunition, audible output or complete caller behavior.

There are 96 configuration combinations: primary/secondary/fallback mode,
normal/repeating sound, one/three projectiles, the definition's 0x200 phase
gate off/on, zero/0.1-second interval, and normal/quarter-speed schedules.
Each compares 60/120/144, including pause between updates, integer cooldown
remainders, emitted sound count and repeat remainder, projectile/effect/RNG
counts and both event-timer fields. Quarter-speed scheduling supplies 15
whole game ticks per simulated second, comparing against original calls
with matching integer/float deltas; it is not a playtest of every in-game
slow-motion mode. Separate repeated variable-step sequences include zero,
fractional and three-tick catch-up intervals. A retained whole tick with a
zero float delta produces no firing/audio events. Actual actor callers and
arbitrary-rate physics remain separate acceptance work.

The focused test passes in `logs/native-weapon-fire-scope-focused.log`.
The original timer/shot mismatch now passes without an expected-failure
marker. The intentionally unsafe gate-opening alternative remains confined
to the test to demonstrate overfire. Full stack application passes in
`logs/native-weapon-fire-scope-patches.log`; source generation passes in
`logs/native-weapon-fire-scope-generate.log`. Windows Release and its built-in
checks pass (`logs/native-weapon-fire-scope-build.log`). The initial broader
suite passed 173 checks and exposed four patch-transition failures. The
overlay link patch unnecessarily included optional texture-source context;
its hunk now depends only on the common input target. Historical fixtures now
exclude the new firing wrapper when they exclude its prerequisite native
wrappers. Reapplying the complete corrected stack updates zero runtime files
(`logs/native-weapon-fire-scope-patches-recheck.log`), so these compatibility
repairs do not alter the tested executable. All 35 patch-transition/tracking
rechecks pass in 395.08 seconds
(`logs/native-weapon-fire-scope-patch-recheck.log`), including all four
previous failures. The other 142 checks passed in the original broader run.

The Windows x64 Release executable SHA-256 is
`1edf3c62f7f648448e8d8a3fc8050d53f00cfe2aff509bd694d349653ad55ded`.
The isolated 120-second native-144 run
`native-144-handoff-none-weapon-event-scope` passes all reported checks:
Story gameplay, fresh guest presentation with interpolation disabled, movement,
hand-animation progression, opening weapon handoff and ammunition consumption,
pause/resume, and no reported crash or presentation failure. The runner ends
the process deliberately (exit 1, `terminated_by_runner: true`). It uses the
verified US XBE and runner SHA-256
`65b94927ec6bed6e603ec219d2274e3ce3dd2a280ef023550196b20ac0486e2a`.
Evidence is retained under `private/test-saves/high-refresh/` and in
`logs/native-weapon-fire-scope-144.log`. This behavior check does not establish
sustained 144 FPS: the runtime reported a 69.28 FPS mean over its first 115
seconds, including startup. No GPU/display timing was collected.

The interactive game was reopened with `Play-Native.cmd 144`; its
`logs/play-20261020-115115.err` confirms the native 144 fresh-update target,
interpolation disabled and 3440x1440 fullscreen at 144 Hz. The earlier overlay
fix remains included: native target replaces the misleading cap label, and
offline sessions hide RTT. The shared firing wrapper has no host adapter;
actual Mac compilation/execution remains unverified under MAC-058.

## NPC firing event-tick scope

The complete original NPC firing routine 0x1BB5C0–0x1BC64F has a direct
whole-game-tick comparison at 0x1BB5FD and an event-timer addition using the
float delta at 0x1BBADE. It resets its integer weapon counters and invokes
projectile, ammunition and sound operations inside this gated transaction.
Opening its gate would change the number of transactions; leaving fractional
time in this gate loses elapsed event-timer and repeating-sound time.

The original-code fixture reproduced these one-second measurements before
the fix, with one projectile per invocation and a one-shot sound:

| Update rate | Projectiles | Sounds | RNG calls | Weapon event timer |
| --- | ---: | ---: | ---: | ---: |
| Original 60 | 60 | 60 | 240 | 0.999999702 |
| Native 120, unscoped | 60 | 60 | 240 | 0.499999851 |
| Native 144, unscoped | 60 | 60 | 240 | 0.416666836 |

Using the shared `pc_native_weapon_fire_guest` scope restores 0.999999702
at all three rates without changing these event counts. The additive
`native-npc-weapon-event-scope.patch` now supplies the production wrapper
for 0x1BB5C0, ordered after the generic firing wrapper. The scope restores
both game-time fields on return; continuous caller updates retain their
fractional time. Original timing calls the original body unchanged.

The routine already has a multiplayer bot-shot wrapper. The timing scope
is its inner callback, preserving network capture before firing and event
publication after the original return. The fresh-stack test now rejects
duplicate guest function definitions, which generation alone does not
reject. The network dispatch fixture checks the composed hooks with
simulated Windows transport and native fallback: a carried tick publishes
one shot, a fractional update without a whole tick and a paused interval
publish none, and caller time/register/stack state is restored.
All five network dispatch/results checks pass in 6.26 seconds
(`logs/native-npc-weapon-scope-network.log`). This is an ABI/event test,
not an actual multiplayer session.

`tests/native_npc_weapon_fire.c` executes the complete original routine,
mode lookup, sound selector and parameter builders, both ammunition helpers
(0x1B6790/0x1B66E0), sound-position helpers, actor flag helpers, effect-state
query and statistics eligibility/update routines from the verified US XBE.
Projectile construction, weapon-property services, effects and final audio
output remain mocked; the fixture uses a deterministic RNG. Unexpected
services/indirect callbacks fail. It checks guest ABI, stack/FPU balance,
finite projectile direction and restoration of both clock fields.

The 96 configurations cover left/right weapon slots, primary/secondary/
fallback definitions, one-shot/repeating sound, one/three projectiles,
normal/quarter-speed schedules and actor/weapon ammunition pools. Original
60 FPS and scoped native 120/144 agree on event count, ammo decrement,
projectile/audio/effect/RNG counts and event/sound timer remainders. Pauses
are interleaved; separate repeated variable-step schedules include zero,
fractional and three-tick catch-up intervals. Invalid target/body/weapon
exits remain inert. A retained whole tick with a zero float delta emits no
events. Quarter-speed schedules test 15 carried game ticks per second,
not the full in-game slow-motion state machine.

The unscoped regression reproducer remains in the fixture. Its measured
failure was recorded in `logs/native-npc-weapon-measurements.log` before
adding the production wrapper. The final scoped parity test passes without
an expected-failure marker (`logs/native-npc-weapon-scope-focused.log`).
The broader timing/audio/camera/animation/CI/backlog suite reports 143
passed in 54.62 seconds (`logs/native-npc-weapon-scope-regressions.log`).
Full stack application passes (`logs/native-npc-weapon-scope-patches.log`).
Generation completes in 117.2 seconds
(`logs/native-npc-weapon-scope-generate.log`), preserving
`sub_001BB5C0_gen` for the manual wrapper. Five targeted fresh/repeated-build,
historical controls/Mac-stack and texture/main branch-transition checks
pass in 139.17 seconds
(`logs/native-npc-weapon-scope-patch-transitions.log`).

The `cfaef73` checkpoint now passes the Windows x64 Release build and
built-in checks with MSVC 14.44.35207
(`logs/native-npc-weapon-scope-build.log`). Executable SHA-256:
`afb5ed6a71c05f3c8a3ddc1ede825e8f2f07f4a05d39dc3901de665a35840d26`.
The isolated 120-second `native-144-handoff-none-npc-weapon-event-scope`
run passes all reported checks: Story movement, hand animation, firearm
handoff and ammunition consumption, pause/resume, native fresh guest
presentation with interpolation disabled, and no reported crash or failed
presentation. The runner deliberately terminates the process at its limit
(exit 1, `terminated_by_runner: true`). Logs are in
`logs/native-npc-weapon-scope-144.log`; structured evidence is under
`private/test-saves/high-refresh/`. The original verified XBE and runner
hash `65b94927ec6bed6e603ec219d2274e3ce3dd2a280ef023550196b20ac0486e2a`
are unchanged. The game was reopened; `logs/play-20261020-121550.err`
confirms native 144 and fullscreen 3440x1440 at 144 Hz.

The runtime reported 62.76 FPS over its first 115 seconds, including startup.
The user reported that COD was open during this run. Treat it as contended
behavior validation, not a clean performance benchmark or proof of a
regression. No GPU completion or monitor scanout was measured.

A second isolated 120-second run, `native-144-handoff-none-npc-weapon-cod-closed`,
used the identical executable, XBE, runner and flags. COD was absent from
process checks at the start and during gameplay. All reported behavior
checks passed again. `tools/native_update_stats.py --from-seconds 85`
compared unpaused Story CPU update intervals:

| Run | Intervals | Window (s) | Mean updates/s | p50 (ms) | p95 (ms) | p99 (ms) | Worst (ms) | Game seconds / wall second |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| COD reported open | 2131 | 34.3704 | 62.001 | 14.1186 | 30.4936 | 33.3740 | 37.5897 | 1.0 |
| COD absent repeat | 4798 | 34.6179 | 138.599 | 7.0151 | 8.9571 | 9.8945 | 19.7135 | 1.0 |

The repeat's final five-second FPS samples reported 144.1 and 144.0.
This supports resource contention as the explanation for the earlier low
rate, without isolating every background variable or proving full-game
performance. These are CPU update intervals in the windowed test; they
do not measure GPU completion, monitor scanout or input latency.
Evidence is in `logs/native-npc-weapon-cod-closed-144.log` and the matching
`private/test-saves/high-refresh/` artifacts. No compiler or pytest work
ran during the measured window. The interactive game was reopened afterward;
`logs/play-20261020-122121.err` confirms native 144, interpolation disabled
and fullscreen 3440x1440 at 144 Hz.

Full actor caller cadence, damage/collision, audible output, other weapon
families, multiplayer and actual Mac compilation/execution remain open.

## Object rocking and bounce audit

The callback at 0xA84F0–0xA8A97 updates an object's vertical rocking/bounce
offset and two tilt components, then builds its transform. Its initializer
0xA8AA0 selects a model matrix, applies an impulse and installs 0xA84F0 as
the entity+0x16C callback (assignment at 0xA8BC5). This is a specific object
motion path, not the general player or world collision integrator.

The callback executes one fixed step per carried whole game tick. Both
loop bounds read 0x4E7944 at 0xA8595/0xA8980; it never consumes the fractional
game delta. Each loop uses 1/60 in vertical integration and adds fixed
angular increments. Opening these bounds alone would accelerate the
simulation rather than making its current motion smooth.

`tests/native_object_rocking.c` executes the complete original callback,
Euler/translation matrix builders and matrix multiplication routines from
the verified US XBE. The fixture supplies an identity base transform per
update and checks the resulting vertical translation against the simulated
offset. Its free-flight start is offset 10, vertical velocity 5 and zero
tilt. Bounce/math and impact-audio services fail if reached; this deliberately
does not mock a collision outcome. This setup stays above the routine's
0.3 near-ground threshold. Stack/FPU balance, non-frame callee-saved
registers, unchanged clock fields and finite matrices are checked.
The translator publishes framed helper EBP as frame TLS; it is not treated
as a restored guest register by this fixture.

| Updates per second | Schedule | Changed transforms | Repeated transforms | Final offset | Final velocity |
| ---: | --- | ---: | ---: | ---: | ---: |
| 60 | Normal | 60 | 0 | 9.546200752 | -4.523219109 |
| 120 | Normal | 60 | 60 | 9.546200752 | -4.523219109 |
| 144 | Normal | 60 | 84 | 9.546200752 | -4.523219109 |
| 60 | Quarter-speed | 15 | 45 | 10.686166763 | 1.669251561 |
| 120 | Quarter-speed | 15 | 105 | 10.686166763 | 1.669251561 |
| 144 | Quarter-speed | 15 | 129 | 10.686166763 | 1.669251561 |

The endpoints match exactly because the same number of fixed steps run.
Ordinary pause is inert, and one three-tick catch-up call matches three
one-tick calls. The missing native progression is recorded as a strict
expected failure that accepts only `ObjectRockingQuantization`; compile,
ABI and other fixture errors remain failures. Measurements are in
`logs/native-object-rocking-measurements.log`. The broader timing, auto-look,
float/vehicle translation, audio, CI scope and Mac backlog suite reports
143 passed and one expected failure in 44.75 seconds, recorded in
`logs/native-object-rocking-regressions.log`.

No production change is made to this routine. A fix must preserve the
original step's damping/acceleration, characterize tilt clamping, impact
timing/audio, bounce restitution and settling, and produce a fresh transform
for each advancing native interval. Whole-game collision coverage, actual
scene review and Mac execution remain open. This audit does not change
the already rebuilt NPC-weapon executable.

The additional `native_object_bounce.c` fixture now follows the original
callback through near-ground tilt clamping, floor impacts and callback
retirement. It runs the original matrix routines, sound defaults builder
0x45390 and positional sound builder 0x21910. The audio-device boundary is
captured rather than played. The CRT helper 0x3C9DF8 is adapted with host
`asin`; its entry and x87 asin-kernel bytes are verified against the XBE.
This is not a test of the complete CRT exceptional-input machinery or
audible output.

Across 72 combinations of 60/120/144 updates, normal/quarter-speed time,
zero/nonzero tilt and six impact flags, the original code produces the
same impact tick sequence and byte-identical 48-byte sound parameters.
Starting at height 0.2 and velocity -3 produces three audible impact
requests for each enabled material, with sound IDs CDA/CDB/CDC and the
original flag priority. Position and gain are checked against the entity.
Six single steps and two three-tick catch-up calls also produce identical
physics and sound payloads. A paused moving object emits no sound and
retains its state.

Settling is checked before the whole-tick loop: the normal 60 FPS case
retires at game tick 63, while extra zero-tick callbacks retire the same
state at tick 62 at 120/144 FPS and under quarter-speed schedules. This
one-tick timestamp distinction is recorded, not hidden by a blanket
cross-rate equality assertion. The fixture preserves this original
behavior while documenting what a fractional replacement must address.
Measurements are in `logs/native-object-bounce-measurements.log`; the
focused checks are in `logs/native-object-bounce-audit.log`. No production
motion conversion is included in this audit.

## Bridge-crash placement investigation

The user reports a later ship crash intruding into the playable canyon,
on the way to the base entrance immediately after the bridge blows up.
Two user screenshots show the obstruction beside the destroyed bridge.
The initial investigation below established the scene evidence. A subsequent
original-code reproduction identifies a timing defect and source correction,
recorded at the end of this section; rebuilt scene verification remains open.

The running Windows executable remains the tested `cfaef73` build with
SHA-256 `afb5ed6a71c05f3c8a3ddc1ede825e8f2f07f4a05d39dc3901de665a35840d26`.
The user requested continued gameplay during investigation. The saved
`private/pipeline/ship-bridge-user-snapshot.json`, `ship-bridge-models.json`
and `ship-bridge-static-models.json` are bounded, read-only process-memory
observations; they do not send game input, change memory, or change saves.
The first snapshot contains 66 entities and player position approximately
(-88.60, -11.08, -91.54).

Two debris objects have clips 2536/2537 (`crash_debris01/02`) with unchanged
previous/current times of 1.814850 seconds. The bridge object has clip
2550 (`bridge_collapse_`) at 4.062600 seconds and callback 0x216E00.
Their sampled bone bases have unit lengths. These observations rule out
neither an incorrect terminal pose nor a separate misplaced ship.
The four sampled type-0x80 ships use ambient clips 2512–2515 and continue
moving. A separate type-1 object at approximately (18.08, -26.12, -30.28)
has a large bounding extent, clip zero, and model definition 0x81416728;
its relationship to the visible obstruction still needs confirmation.
Do not identify it solely by entity address 0x8204BB80: an earlier snapshot
of this same pool slot contained an NPC. Pool addresses are recycled.
The generic tracer now records type, tag and model definition explicitly,
and the runner records the tracer's source hash for future runs.

The two completed 120-second isolated opening runs,
`60-ship-path-reference` and `native-144-ship-path-native`, each contain
73 complete entity-list samples. Their existing smoke checks pass, but
neither reaches the bridge event. Ambient positions at equal wall times
have different clip phases; those differences are not evidence of a path
regression. Both used runner SHA-256
`1cdfe69268a9bdb55ace432c47c94503437f8ff10ed74a74c5770b615a187626`;
they predate the tracer-hash metadata field. The subsequent
`60-bridge-reference` run was stopped to return the game to the user and
is incomplete, not a passing bridge comparison.

At this stage, identifying the ship's model and spawn/movement/settling path
was still pending. The later investigation below resolves that identification.
A bridge-event comparison at original 60 Hz and native 144 Hz with isolated
data remains required. No speculative position offset or asset edit has been
applied. Mac behavior is unverified.

The offline timing, entity-trace, float/vehicle translation, audio and CI
scope checks report 144 passed and one expected failure in 44.21 seconds
(`logs/native-ship-audit-regressions.log`). The expected failure is the
previously measured object-rocking cadence gap, not this ship report.
The Mac backlog integrity check also passes. These tests validate the
diagnostics and existing covered behaviors; they do not resolve the scene.

### Mothership identification and source correction

The later read-only template snapshot (`private/pipeline/ship-templates.json`)
identifies entity 0x8204BB80 as model 0x53A. Its template name/path pointers
resolve into the level's object-name table at offsets 0x10753 / 0x1077A:
`INTRO_TIMESPLITTERS_MOTHERSHIP_CRASHED` / `intro/intro_mscrash`.
The original updater at 0x217050 spawns this model at 0x2185A7 with the
live mothership's position and orientation, then removes the moving object.
This supersedes the earlier uncertainty about which object was the wreck.

The updater's entry load at 0x217059 is only a pause gate, with its EAX
value replaced at 0x217090. Its forces, position integration, angular motion,
crash timer and associated shared timers use 0x4E794C, the fractional delta.
Skipping zero-whole-tick frames therefore loses motion time. Meanwhile,
the angular and linear damping loops consume the carried whole ticks,
so damping becomes too strong relative to elapsed motion time. This changes
the crash trajectory, not just its apparent frame rate.

`src/pc_native_numeric.h` now allows this gate on every positive fractional
interval in native mode. Its two damping loops and all other loads remain
unchanged. Pause still stops the callback, original 60 Hz behavior is
unchanged, and the original multi-tick catch-up path stays intact.

`tests/native_mothership.c` runs the complete original 0x217050–0x21862F
updater, original shared effect-timer helper 0x2157B0, vector transforms and
scalar approach helper. It supplies a controlled airborne state, centered
random wobble, a sine table and an effect-matrix adapter. Sound, particles,
damage/player feedback, asset creation and deletion are bounded fixture
services; the actual wreck-creation request is captured and checked.
It does not exercise collision/rendering, audio output, the preceding
combat/trigger sequence, or the full random-number stream.

Normal-speed results from the same starting state:

| Path | Updates/s | Changed transforms in first second | Crash seconds | Wreck X | Wreck Y | Wreck Z |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Original reference | 60 | 60 | 9.66667 | 48.60029 | -44.30268 | -119.41282 |
| Original gate at native cadence | 144 | 60 | 26.58333 | 43.02375 | -26.25528 | -128.49136 |
| Corrected native gate | 120 | 120 | 9.64167 | 48.28055 | -44.04354 | -119.62134 |
| Corrected native gate | 144 | 144 | 9.64583 | 48.36892 | -44.14344 | -119.55445 |

Assertions cover six 60/120/144 × normal/quarter-speed combinations:
fresh position each advancing callback, one-second velocity within 0.005,
position within 0.03, complete crash time within 0.05 game seconds, landing
within 0.75 units per axis and angles within 0.05 degrees of the original.
These tolerances allow sampled height/time thresholds at state transitions.
The original 60 Hz result is byte-identical with the native gate enabled,
as are separate one- through six-tick catch-up calls. Paused entity state,
callee-saved registers, stack, FPU balance and restored clock fields are
checked. A counterfactual run without the gate correction must still
reproduce the late/high crash. Measurements:
`logs/native-mothership-regression.log`.

The patch stack validates without runtime-file changes, and the existing
generated production routine already uses the scoped global-load hook at
0x217059. No hand-edited generated C or modified game asset is involved.
The broader native timing, entity-trace, float/vehicle translation, audio,
CI-scope and Mac-backlog checks pass: 146 passed and one expected failure
in 49.07 seconds (`logs/native-mothership-shared-regressions.log`).
The expected failure remains the separate object-rocking cadence gap.
An additional schedule alternates intervals from 2/12 to 10/12 of a
game tick and repeats at quarter speed, carrying whole ticks independently.
It pauses twice in each of crash states 3, 4 and 5, including immediately
before wreck creation; entity state, spawn count and retirement count stay
unchanged during every pause. Both schedules create exactly one wreck and
retire the ship once. Crash duration is 9.6333/9.6347 game seconds, with
landing coordinates within 0.39 units per axis of the original 60 FPS
reference. The focused original-code test passes in 42.25 seconds while
the Windows build runs (`logs/native-mothership-variable-regression.log`).
These checks retain centered random wobble and bounded effect stubs;
they do not replace a bridge-scene playthrough.
After the user's interactive process had closed (both process-name and
Win32 process queries found no game process), the supported
`tsfp.ps1 -Action Build` completed successfully, including its built-in
Windows tests. The runtime source is `d4c5ee8`; the subsequent changes
only extend this fixture and documentation. The new Release executable
SHA-256 is `08ba9a6be1d6f078123e53e69a00254e05b6fc7f131f4e896ec4f3124c250c61`.
Build output is `logs/native-mothership-windows-build.log`.
The rebuilt executable subsequently passed a 120-second isolated Test Lab
run at native 144 with movement/firing and pause/resume replay, using
`tests/native_refresh_game.py --fps 144 --native --label ship-fix-build
--stress --pause-check --seconds 120 --windowed`. All recorded checks pass,
including opening firearm handoff, advancing/changing hand animation,
ammunition consumption, fresh guest presents and no interpolation.
The runner terminated its child at the duration limit; recorded exit code 1
is that intentional termination, not a normal game exit.
Evidence is `private/test-saves/high-refresh/native-144-ship-fix-build-validation.json`
and `logs/native-mothership-built-smoke.log`.

The unpaused CPU-update sample starting at 85 seconds contains 4,743
intervals across 34.3823 seconds: 137.949 updates/second, p50 7.0546 ms,
p95 9.0974 ms and p99 10.1704 ms, with game/raw time both 1.0 seconds per
wall second. This windowed 1280×720 opening-area test does not establish
144 FPS throughout the game or monitor scanout. It does not reach the bridge.
Audio initialized through XAudio2 at master gain 1 and submitted nonzero
samples; this does not establish audible output at the user's device.
An isolated bridge-scene comparison is still required before calling the
reported obstruction resolved; actual Mac execution and full mission/
random-effect coverage remain pending.

### Ship debris inherited velocity

A separate original initializer, 0x2168C0–0x216CA5, creates 30 debris
fragments and a final effect. It derives parent velocity from the difference
between entity+0xB4/B8/BC and entity+0xE0/E4/E8, multiplied by
60 / elapsed game ticks. Its gate at 0x216958 skipped that division whenever
the carried whole-tick count was zero. At 144 FPS, a parent velocity of
(12, -24, 36) consequently contributed only (5, -10, 15) on those frames.
This makes fragment motion depend on which frame triggers destruction.

The shared numeric policy now uses positive fractional time for that
division guard. EAX is replaced at 0x216987 before any other use.
This does not open a periodic spawn gate: fragment count, model selection,
random calls, effect arguments and allocation-failure behavior stay original.
Zero-time initialization keeps the original no-division behavior; this
one-shot event still creates fragments when invoked with zero elapsed time.

`tests/native_ship_debris.c` runs the complete original routine with its
original scalar constants and model table. The allocator and final effect
service capture requests, and a repeatable xorshift stream supplies both
positive and negative random values. The fixture covers both model-selection
branches, inheritance enabled/disabled, successful/all-failed/alternating-failed
allocations, fractional normal/quarter-speed intervals, carried zero/one ticks,
two/six-tick catch-up and zero time. Native one-tick output is byte-identical
to the original. Other schedules match inherited velocity within 0.0001;
all remaining output bytes, including RNG call count and generated rotations,
must match exactly. Parent data, guest time fields and ABI are preserved.
The original fractional counterexample remains in the test.

The new test failed at the inherited-velocity assertion before the correction
(`logs/native-ship-debris-before.log`) and passes afterward
(`logs/native-ship-debris-after.log`). This correction is in shared source;
the interactive executable built from `d4c5ee8` does not yet include it.
Full Windows rebuild, rendered debris review and actual Mac execution remain
pending. The separately tested mothership landing fix remains in that build.
The full shared timing, entity-trace, float/vehicle translation, audio,
CI-scope and Mac-backlog suite passes: 147 passed and one expected failure
in 50.69 seconds (`logs/native-ship-debris-shared.log`). The expected failure
is still object rocking. The complete patch stack validates with zero runtime
files changed; generated production C already has the scoped load at 0x216958.

### Additional first-mission reports awaiting reproduction

The user reports enemies before the bridge that react to damage and die
but neither move nor shoot; trails from any fired gun appear about three
seconds late, increase with additional shots, and eventually disappear;
and the weapon described as the plasma sniper does no damage.
These are open behavior regressions, not covered by the passing opening-area
smoke check. Existing NPC fixtures isolate dispatcher/firing arithmetic and
mock world/actor/projectile services; they do not prove live AI transitions,
projectile collision, damage or trail timing.

The reported build is the interactive executable with SHA-256
`08ba9a6be1d6f078123e53e69a00254e05b6fc7f131f4e896ec4f3124c250c61`.
The screenshot `codex-clipboard-d06e74af-098b-4c32-9dff-eaf353fcf3b4.png`
shows several red trails near the bridge. A bounded read-only 30-second
entity observation is saved in
`private/pipeline/npc-trails-user-observation.jsonl`. At least one NPC moves
during this observation; other actors include a fixed `portalnonjumper02`
pose and `zomdeath01` poses. Actor addresses can be recycled and this
post-bridge observation does not identify the particular pre-bridge enemy.
Do not infer a root cause or resolution from those pose names alone.


### Projectile travel on fractional frames

Source baseline `b551804`; shared gate change at 0xF4A66 and
`tests/native_projectile_motion.c` are the next local checkpoint. Windows x64,
pinned US XBE, repository Python/MSVC test toolchain. No personal saves,
settings or live process memory were modified. The interactive process later
closed independently; both Get-Process and Win32_Process queries found none.

The bounded read-only 45-second observation in
`private/pipeline/npc-projectile-observation.jsonl` contains 179 snapshots
(10,148,429 bytes). Captured kinds 28/31/45 use updater 0xF4A50. A continuous
kind-31 segment with speed 240 travels about 24.9 units in 0.25 seconds while
its age advances by 0.25 seconds: approximately 100 units/s rather than 240.
The original routine skips its fractional displacement/collision sweep when
integer delta is zero, but outer 0xDC7B0 advances age on every callback.
The shared change opens only that entry gate for positive live game time.

The new fixture translates the complete original 0xF4A50..0xF51AF routine.
A deterministic collision plane replaces the world query; impact, damage,
rendering and effect services are captured rather than executed. Before the
fix it fails the requirement for one sweep per advancing interval
(`logs/native-projectile-before.log`). After the fix it passes
(`logs/native-projectile-after.log`):

- Kinds 28/31/45, speeds 180/240/480, 60/120/144 FPS and quarter speed.
- At speed 240, a plane 180 units away is reached at about 0.75 seconds,
  versus 1.80555 seconds on the unpatched native-144 schedule.
- A plane 360 units away receives one impact/damage callback at about
  1.50694 seconds; the unpatched shot expires after traveling only 200 units
  and never reaches it.
- No-hit expiry and cleanup, exactly one hit/retirement, zero-time pause on
  every frame, uneven intervals, two-/six-tick catch-up, ABI/timing integrity,
  and byte-identical native-60 free-flight state.

Limits: these callbacks do not prove enemy health decreases, plasma-sniper
identity/damage, trail appearance or pre-bridge AI recovery. The observed
kind-35 updater 0xF08D0 has a similar gate plus homing/decay behavior that needs
separate coverage. The sum-of-squared-step accumulator at projectile +0xF4 is
frame dependent and can feed the original damage-falloff helper 0xB9920 via
0xDCEF0. This change does not redefine that mechanic; cross-rate falloff and
partial-hit behavior remain explicit follow-up work. The focused shared suite passes 148 checks with one existing object-rocking
expected failure (`logs/native-projectile-shared.log`). Full patch-stack
validation passes with zero runtime files updated; generation is current.
Full Windows rebuild, scene replay and actual Mac execution are pending at
this source checkpoint.


The supported Windows Release build of `ef2ace3` and built-in tests pass
(`logs/native-projectile-windows-build.log`). Executable SHA-256:
`9c9f801015f988ad4ce952d54136324c6117e2dd2324802484ef9c3b7114d77d`.
The 120-second isolated Test Lab native-144 run passes movement, equipped
firearm/ammunition, hand animation, pause/resume, native hook and successful
guest presentation checks, with interpolation disabled. Evidence is
`private/test-saves/high-refresh/native-144-projectile-travel-validation.json`
and `logs/native-projectile-built-smoke.log`. Runner termination is intentional
(exit 1); completed/passed are true. The 73 bounded entity snapshots are in
the companion entities JSON and do not reach the reported bridge encounter.

From 85 seconds onward: 4,825 unpaused Story CPU intervals over 34.7362
seconds, mean 138.904 updates/s, p50/p95/p99 7.0446/8.9396/9.9019 ms,
worst 12.3229 ms, game/raw clocks both 1.0 seconds per wall second. This
windowed 1280x720 opening check is neither scanout measurement nor a full
campaign/damage test. The interactive launcher was started again at native
144 with personal settings/saves after the isolated child exited.

A read-only inspection of the owned isolated child's weapon table located
it at guest 0x4EEF74 (pointer 0x4F9BAC). Weapon record stride is 0x29C;
fire-mode flags start at +0x68 and projectile kind is +0x6C (secondary +0xE8).
Weapon ID 17, recorded throughout the earlier user observation, uses kind 24
for both modes. Its descriptor selects initializer 0xE36D0 and updater
0xF5FF0, which calls the immediate trace/hit helper 0xF5D30. Therefore the
0xF4A50 travel fix does **not** resolve that weapon's damage report.
Kind 24's original hit path reaches damage setup 0xB91F0/0xB9220 and
application 0xBD0F0; actual health behavior is still pending reproduction.
The small table snapshot is ignored at
`private/pipeline/projectile-weapon-table.bin`. No game memory was written.


### Sniper damage on fractional frames

Source baseline `2d90c6c`, Windows x64, pinned US XBE and repository MSVC/Python.
The previous projectile-travel fix does not affect weapon 17 / shot kind 24.
Tracing its damage chain found a second independent timing gate:
`sub_000BB6C0` returns immediately when integer delta 0x4E7944 is zero.
The sniper's immediate ray can hit on such a frame, calculate positive damage,
then retire before a later whole-tick frame. Its health change is lost.
The gate's EAX is replaced at 0xBB6D2; no counter consumes its value. The
shared `pc_native_numeric.h` policy now tests positive live game time at that
load only. Damage amounts, health arithmetic, network policy and continuous-
damage scaling are unchanged. Zero live time still blocks application.

`tests/native_sniper_damage.c` translates the complete original routines:
0xE36D0 initialization, 0xF5FF0 update, 0xF5D30 hit processing,
0xB91F0/0xB9220 damage setup, 0xBD0F0/0xB9790/0xB9720 damage calculation,
and 0xBB6C0/0xBB490/0xBA310 damage dispatch and actor health subtraction.
Original scalar and damage-table constants come from the checksum-verified XBE.
The world ray result, region choice, network/eligibility-service response,
modifiers, statistics, sound/render effects and reactions are controlled
fixture services. The actual subtraction of entity +0x1CC runs original code;
it is no longer replaced by an impact-call counter.

The initial arithmetic-only fixture passed with damage 0.65 at every rate;
that did not establish health loss. Extending through application reproduces
the failed health effect (`logs/native-sniper-health-before.log`). With the
source correction, `logs/native-sniper-health-after.log` verifies:

- An accepted hit reduces health 1.00 to 0.35, matching original 60 FPS,
  with deltas 1, 1/2, 5/12, 1/4, 5/48, 2 and 6 ticks.
- Fractional intervals work with either zero or one carried integer tick.
- Misses, untargetable actors, rejected damage and zero-time pause leave
  health unchanged. The unpatched zero-whole positive-time case also leaves
  health at 1.00, explicitly preserving the counterfactual reproduction.
- Each shot performs one trace and one beam emission, then retires once;
  the next update does not apply duplicate damage. ABI, x87 stack balance
  and live timing fields are preserved.

A fixture boundary typo initially omitted the miss-path RET from 0xF5D30;
correcting its range to end at 0xF5FEC resolved that fixture-only stack mismatch.
It was not a production translation defect. This source change is shared
with Mac. Full Windows rebuild, actual user-scene sniper damage confirmation
and actual Mac execution remain pending while the interactive PID 2212 runs.
The delayed trails and pre-bridge NPC behavior remain separate open reports.


The focused shared regression suite passes 149 checks with one existing
object-rocking expected failure in 52.91 seconds (`logs/native-sniper-shared.log`).
Full patch validation passes with zero runtime files updated
(`logs/native-sniper-patches.log`); generation is current. The production
translation already calls the shared policy at 0xBB6C0, so no generated C was
edited. These are offline checks, not a rebuilt or live-verified sniper fix.


### Beam-manager lifetime audit

Source baseline `e7da2e4`, Windows x64, pinned US XBE. The kind-24 sniper
updater emits type-32 effects through 0x104F30. That constructor sets age zero
and lifetime 1.2 seconds. The complete manager 0x10A760 uses float delta
0x4E794C, stores its effective delta in 0x73BBA8, and advances effect age
by delta/60. Type-32 dispatch calls 0x1053D0. The inspected gameplay caller
0x1CE2E0 reaches this manager without a whole-tick gate on the normal path.

`tests/native_beam_lifetime.c` executes the complete original manager and
original descriptor/switch tables. Type-32 geometry updates and unrelated
subsystem services are spies; expiry captures the original decision while
replacing allocator cleanup. This isolates manager scheduling, not complete
beam construction, rendering or GPU resource lifetime. It does not prove
which effect produced the user's red trails.

`logs/native-beam-lifetime.log` passes 13 schedules: original 60, native
60/120/144 at normal/quarter speed, and explicit 0.3-second deferred starts.
With no explicit delay, the updater runs on the first advancing frame.
Expiry is 1.216667 game seconds at original/native 60 and 1.201389 at native
144, within normal step/float boundary rounding of 1.2 seconds. Deferred
starts remain within one step of their requested time, and total expiry
within two steps of delay plus lifetime. A zero-time callback before each
live frame leaves age unchanged; register preservation, x87 stack balance
and timing globals are checked. The manager may visit its geometry updater
at zero time; this fixture does not assert that all visual state is frozen.
An initial fixture omission left the constructor's lifetime constant unloaded;
loading its verified original value resolved the fixture-only failure.
No production timing change was justified by these results.

A bounded read-only observation of interactive PID 2212 collected 596
snapshots over 30 seconds (`private/pipeline/fx-trails-observation.jsonl`,
72,977 bytes), with advancing time but no active effect heads. It did not
capture firing or the reported delayed trails. No input, memory writes,
restart or personal-settings changes were performed. Further reproduction
and render-path investigation remain necessary; actual Mac execution is
also pending. Sniper source fix `e7da2e4` still needs a Windows rebuild and
live gameplay verification; the current interactive executable remains
`ef2ace3`.

The timing, CI runtime-scope and Mac-backlog checks pass 94 tests with one
existing object-rocking expected failure in 33.61 seconds
(`logs/native-beam-shared.log`). The new harness is included in the existing
CI timing suite. This is audit coverage, not a delayed-trail fix.


### NPC animation-marker shots

Source baseline `b6b9311`. While tracing the non-shooting NPC report, two
animation-event callers were found: 0x27AE14..0x27AE83 and
0x2B9BCE..0x2B9C0C. Both query marker mask 0x200 through original 0x61570,
then call 0x1BB5C0 and clear temporary firing flags. The marker selector
recognizes crossed animation bounds; a rejected shot cannot be retried after
the bounds advance. The weapon's whole-tick gate rejects a positive fractional
interval with zero integer ticks, even when an actual one-time event reached
it. These callers are reached through 0x27D590 in mode 0x0A; this does not
identify the user's particular first-mission enemy or explain no movement.
Other one-time attack transitions (0x2612FB/0x2B739D) need separate auditing.

The extended `tests/native_npc_weapon_fire.c` executes original 0x61570,
both complete event-test/call/flag-cleanup branches, and the existing full
firing/sound chain. Only each caller branch's following state transition is
replaced by RET. Animation lookup/table/bounds are controlled services;
projectile construction, collision, live AI and audio output remain mocked.
Calls enter the same manual wrapper as production, while the original firing
body remains `_gen`. The first fixture attempt reversed the two animation
bound outputs; correcting them restores the original-60 reference. Production
code was not responsible for that fixture setup error.

The pre-fix original-60 event produces one shot and sound; the fractional
0.5-tick event produces neither (`logs/native-npc-animation-event-before.log`).
A retained counterfactual using the previous transaction wrapper confirms
that the following whole-tick frame cannot recover the event.

Shared `pc_native_npc_weapon_fire_guest` recognizes only the two audited
return sites (0x27AE66/0x2B9BF8). A positive zero-whole event gets one scoped
firing transaction, using the existing event timer/sound scope; original live
time is restored on return. It does not open the gate for held-fire callers,
change the global clock, queue stale actors or alter the original damage
amounts. Pause, even with a stale nonzero integer tick, remains blocked. The
network wrapper remains outside this helper and receives the original caller
stack and restored timing. The additive `native-npc-animation-shots` patch is
ordered after the existing NPC event-scope patch; historical-stack fixtures
exclude it along with its prerequisite when recreating old states.

`logs/native-npc-animation-event-after.log` verifies one shot, one sound,
one ammo debit and one event-timer increment for both callers with deltas
1, 1/2, 5/12 and 5/48. An unchanged bound on the following frame produces no
duplicate. Zero-time and invalid-target/weapon cases produce no firing or
timer changes. Existing 96 held-fire configurations, 60/120/144, quarter speed,
variable/catch-up schedules and pause checks continue to pass. Full stack
application passes and updates only the manual runtime wrapper
(`logs/native-npc-animation-event-patches.log`). Existing production generated
C already calls the manual NPC entry with the two original return addresses;
no generated C was edited. Windows rebuild and live confirmation, including
the reported pre-bridge behavior, and actual Mac execution remain pending.


### NPC and sniper build validation

The animation-event source change is `81c213c`; manual guest-memory access is
corrected in `fc3a7ce`. The full Windows build initially caught an integration
mistake: MEM32 is defined for generated routines, not this manual runtime
file. The wrapper now uses the same direct guest-memory read convention as
neighboring wrappers. The first build invocation also used PowerShell 7,
whose drawing assembly did not satisfy the icon helper; the supported Windows
PowerShell invocation avoids that shell mismatch. These failures were resolved
before launching the new executable.

The broader suite passes **169 tests, one existing object-rocking xfail** in
475.07 seconds (`logs/native-npc-animation-shared.log`), covering native timing,
float/vehicle joins, network guest/fallback dispatch, CI scope, Mac backlog,
and patch updates/tracking. After the manual access correction, fresh/repeated
patch application and the original NPC firing fixture pass two focused checks
(`logs/native-npc-animation-final-integration.log`). Windows Release and all
build-command tests pass (`logs/native-npc-animation-windows-rebuild.log`).
Executable SHA-256: `912f416b5f6eaeabf545c960c255491907988eae9f031ead4231b24832cbc333`.

The isolated 120-second `native-144-npc-animation-sniper` test is complete and
passed. All crash, Story, native timing, guest presentation, interpolation-off,
presentation-failure, movement, hand-animation/handoff, ammunition and
pause/resume checks pass. It collected 73 entity snapshots and 140 gameplay
samples. Exit code 1 is the runner's intentional bounded termination, not a
crash. Logs: `logs/native-npc-animation-built-smoke.log`; structured evidence:
`private/test-saves/high-refresh/native-144-npc-animation-sniper-validation.json`.
This opening-area replay does not equip or test a live sniper hit and does not
reach the reported bridge enemies/crash; those targeted checks remain open.

From trace second 85, 4,755 complete intervals span 34.5557 seconds: **137.60
CPU updates/s**, p50/p95/p99 **7.0507/9.2407/10.3390 ms**, worst **11.9642 ms**.
Integrated game time/wall time is 1.0000000001. The single truncated final CSV
row from bounded termination was excluded. These are CPU timing samples,
not scanout or a stable-144 certification; the text FPS log also records dips.
No prior performance regression cause is established by this one run.

Personal controls/display hashes match their pre-test values. Normal
native-144 play was reopened as PID 25304, responding, from the same executable.
`logs/play-20261020-141507.err` confirms native timing/interpolation off and
XAudio2 initialization; that is not an audible-output test. An initial launcher
attempt inherited an incompatible PowerShell module path and failed before
game creation (`logs/launch-20261020-141431-445.err`). Its owned launcher was
closed, and a child-process-only Windows module path allowed the normal
`Play-Native.cmd 144` launch; no persistent environment/settings edit was made.
The delayed trails, specific pre-bridge enemy behavior, remaining physics/
weapon/campaign audits, full-game performance and actual Mac validation remain
open. The goal is active.


### NPC aim recovery and tracking

Baseline `87ed587`, pinned US XBE, Windows x64. The aim portion of 0x2B8F90
loads integer delta at 0x2B93DA and skips both recovery and tracking on a
zero-whole interval. For non-actor targets, 0x2B9720 consumes fractional time
when reducing actor-state +0x9FC toward zero; lost intervals slow recovery.
For actor targets, the original interpolation helper 0x126C0 instead consumes
a fixed 5% argument pushed at 0x2B963F. A target-state flag also enables a
random decision at 0x2B9478, with possible additional random draws. Opening
the gate alone would accelerate blending and random decisions.

`tests/native_npc_aim.c` executes original 0x2B93D5..0x2B9792, the zero-result
store at 0x2B9381, and the original clamp/sign/interpolation helpers at
0x124F0/0x123D0/0x126C0. Outgoing joins end with a fixture RET before subsequent
eligibility/firing work. Actor geometry and random outputs are controlled;
the CRT float-to-integer boundary is modeled, and two actor-state services
return false. This tests the original aim branch, not pathfinding, target
selection, firing eligibility, rendered appearance or a live mission.

Before correction (`logs/native-npc-aim-audit.log`), a recovery value of 0.75
ends at 0.0833327 after one game second at original 60, versus 0.4722231 at
native 144, and 0.6805558 at native 144 quarter speed. The actor-target path
reaches the same final value as original 60 but changes only 60 of 144
advancing frames. An isolated gate-only counterfactual changes every frame
but shifts tracking from 0.0250130 to -0.0095290 and increases RNG calls from
60 to 144. That counterfactual remains in the fixture to demonstrate the
coupled behavior; it is not a production mode.

The shared correction has three parts:

- The pause-only load policy at 0x2B93DA accepts every positive live interval.
  Its ECX is only compared before replacement on either path.
- The immediate 5% argument is replaced by `1 - (1 - 0.05)^delta_ticks` at
  0x2B963F only. One-tick values retain their original bits. Other callers of
  the shared interpolation helper and other immediate arguments stay exact.
- On zero-whole live frames, only the chance call at 0x2B9478 selects a helper
  returning a guaranteed nonzero chance outcome without consuming RNG. The
  target's velocity adjustment still executes. Whole-tick chance calls and
  their rare additional draws execute the original RNG. Guest return-stack
  handling remains inside the existing ABI-call boundary.

The additive `native-npc-aim-timing` patch adds generic immediate-argument and
direct-callee policy hooks with identity defaults, plus the small shared
manual RNG-skip helper. Title addresses stay in `pc_native_numeric.h`, not the
lifter. Constant site tests occur outside the function-pointer helper so other
calls retain their original callees and do not acquire an RNG-helper linkage
dependency. The initial generic helper form exposed such a linkage dependency
in an unrelated fixture; the constant-site macro form resolved it. No generated
C was edited by hand. Historical patch fixtures exclude the new patch with its
native timing prerequisites.

`logs/native-npc-aim-fixed.log` and final expanded
`logs/native-npc-aim-final.log` verify normal/quarter-speed stable-input results
within 1e-5 of original 60, including fresh changes on all native 120/144
frames. Rare-event schedules preserve 60 chance decisions, 66 actual RNG draws
and events at game ticks 20/40/60. Their post-event blend phase is allowed to
differ by at most one original blend step (5% of the full [-1,1] range, 0.1);
observed final differences are at most about 0.034. This is not bit-identical
trajectory parity across an instantaneous random change. Zero-time callbacks,
including stale nonzero integer ticks, preserve state. Uneven intervals and
2-/6-tick catch-up match the elapsed-time recovery/blend result while retaining
the original per-callback chance cadence. Positive/negative recovery clamps
exactly at zero, and a missing target changes neither aim nor RNG.

The wider suite passes 484 checks and 87 subtests, with 41 skips and one existing
object-rocking expected failure in 57.36 seconds
(`logs/native-npc-aim-shared.log`). It includes native timing, scalar/vehicle
joins, CI scope, Mac backlog and the upstream recompiler tests. The later rare/
boundary additions pass separately. Fresh/repeated application, historical Mac
and controls upgrades, and patch tracking pass 17 checks in 48.38 seconds
(`logs/native-npc-aim-patch-transitions.log`). Full stack application updates
three runtime source/template files (`logs/native-npc-aim-patches.log`).

The interactive executable remains `fc3a7ce` / PID 25304 and was left running.
A bounded read-only observation produced no Story snapshots during its window
(`private/pipeline/npc-current-build-observation.jsonl`); it did not reproduce
the reported pre-bridge enemy behavior. This source correction has not yet
been rebuilt or verified in that scene. Delayed trails, live enemy movement,
remaining campaign/physics audits and actual Mac execution remain open.

Full generation completes in 73.6 seconds (`logs/native-npc-aim-generate.log`),
and the generation-current check passes. The production translation contains
the intended gate at 0x2B93DA, RNG-callee policy at 0x2B9478 and coefficient
argument policy at 0x2B963F. The shared manual file contains the matching
RNG-skip helper. This verifies integration into generated source, not a linked
or running executable; the interactive process remains unchanged.
