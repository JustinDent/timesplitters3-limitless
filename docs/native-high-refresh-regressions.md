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

## Scotland aiming and shore movement

Windows x64/D3D11, the unchanged `6540b5c` HUD-fix source binary SHA-256
`167e9bce3b96d4ff8ceaab378d52aaadb9d0b923fa3a7a42374a1364db413397`.
The bounded transition runner now accepts one sequenced synthetic mouse packet
per command (`mouse: [dx, dy]`, each count bounded to +/-4096) and optional
50 ms position/clock observations during its existing <=3-second key holds
(`sample_hold: true`). Packets use the game's existing test input adapter;
this is not physical mouse delivery or input-to-display latency measurement.
The runner also records the absolute level tick and whether it is minimized.
There is no new gameplay, timing, input policy or executable change here.

The isolated `scotland-locomotion-min-{60,120,144}` comparisons select Scotland,
skip the boat intro and wait for the pistol handoff. From the identical starting
look `[110.005348, 0.001624]`, 900 horizontal counts turn to yaw 20.005346;
-450 counts then set yaw 65.005348 toward the shore path. Both results match
at original 60 and native 120/144. After two seconds forward and two backward,
`[100,50]` changes aim to `[55.005348,-4.998376]`; `[-100,-50]` restores it to
`[65.005348,0.001624]` at all rates. A paused `[900,500]` packet changes neither
position nor aim, and resume produces no retained-motion jump. The same
isolated mouse sensitivity (0.1) and original unzoomed FOV apply throughout.

Reviewed renderer captures establish that this path is unobstructed over the
short walk. It advances from the dock onto the shore, unlike the earlier
wall-facing comparison. Linear least-squares fits to 50 ms position samples
between held seconds 0.5 and 1.8 give the following world-unit/second estimates:

| Mode | Forward | Backward |
| --- | ---: | ---: |
| Original 60 | 6.189483 | 6.260947 |
| Native 120 | 6.248019 | 6.206928 |
| Native 144 | 6.281620 | 6.166030 |

These capture-heavy short estimates vary by about 1.5% between rates and
have up to 0.168 world-unit fit residuals. They do not establish exact speed
parity. Separate three-second holds without renderer dumps use the same
binary, route, isolated controls and minimized window. Fitting held seconds
0.5 through 2.8 (46 samples, about 2.26 seconds per segment) to both observer
wall time and the absolute level clock gives:

| Mode | Forward / wall second | Backward / wall second | Forward / game second | Backward / game second |
| --- | ---: | ---: | ---: | ---: |
| Original 60 | 6.256263 | 6.150714 | 6.250089 | 6.179163 |
| Native 120 | 6.255006 | 6.182455 | 6.248912 | 6.167250 |
| Native 144 | 6.243758 | 6.185598 | 6.235904 | 6.186768 |

Compared to original 60, game-time movement estimates differ by at most
0.227%. The original path itself has slightly different forward/backward
speeds over this dock/shore transition. Game/wall clock fit ratios range from
0.995362 to 1.002430; position residuals reach 0.115 world units. These short,
asynchronous observations of a discrete level clock establish close behavior
on this route, not bit-exact physics or a universal speed tolerance. Initial
acceleration, key-delivery delay and release are outside the fitted window;
final return positions are therefore not used as a speed-parity assertion.
The additional runs also reproduce the same aim angles and pause discard,
and position remains fixed after releasing movement through pause/resume.

Evidence lives in `private/test-saves/scotland-locomotion-timing-{60,120,144}`
and matching logs. Each provenance records `captures: false`, `minimized: true`,
and runner SHA-256
`393a52cd23ea1c5f5a92eee3e8edcde0990ec6baaa16b5516ca7fdfce356af71`.
No renderer BMP dumps are produced. The local
`private/pipeline/locomotion-timing-validation.json` records both fits;
`locomotion-validation.json` retains the separate captured comparison.
All six completed sessions exit normally with unchanged valid test profiles.

All three completed captured sessions exit normally and preserve their
isolated profile signatures. Tests run minimized to avoid interrupting the
user; renderer captures are not monitor scanout. An earlier exploratory run
only validated the first camera turn before its duration limit; a separate
startup run received a normal window-close exit before gameplay. Neither is
counted as a completed movement comparison. Full acceleration/collision,
vehicles, physical input, multiplayer, complete levels and Mac execution
remain unverified by these checks.

## HUD message timing

The opening comparison led to an audit of delayed objectives and HUD notices.
Original US `sub_0014A5A0` computes fractional elapsed game seconds but gates
its entire queue update on the whole game delta at `0x14A5CD`. At native
120/144, zero-whole-tick intervals are discarded. This slows both initial
message delays and their expiry, despite correct overall game-clock speed.
The gate register is replaced at `0x14A670`; its value is never used as a
counter. The shared numeric policy now opens only this audited load for
positive live game time and preserves pause and the disabled-native path.
No message duration, queue priority or sound timing constant is changed.

`tests/native_hud_messages.c` executes the complete original update,
eligibility, conflict-table and queue-limit initializer routines translated
from the checksum-pinned US XBE. Only sound services/player count are spies.
Before correction, a one-second notice lasts 2.016667 seconds at native 120
and a half-second-delayed notice has not expired after three seconds
(`logs/native-hud-before-test.log`). After correction, original 60 and
native 60/120/144 expire the first notice in 1.016667/1.016667/1.008333/
1.006944 game seconds, within one update of its authored duration. The
half-second delay, single-message queue limit, one-shot sound trigger,
sound-bound hold, pause, quarter-speed intervals and adjacent-site isolation
also pass (`logs/native-hud-after-test.log`). A separate type-6 case uses a
representative 20-second delay and the five-second lifetime observed in
Scotland; it appears/expires within one update of 20/25 game seconds at
each rate. This does not assert that every live script authors a 20-second
delay. This fixture is part of the
existing native timing suite registered in shared Windows/Mac CI; execution
requires the pinned local XBE, and local Mac execution remains unavailable.

The pre-fix live `scotland-hud-before` run uses the previous binary SHA-256
`41ca454f88d9d11279fe57cbf24fbd56dbca9b72a5d5550c154c20898a91fa43`.
The read-only `--watch-hud` observer records the sixteen bounded player-one
message slots. Between elapsed wall seconds 114.188 and 124.360, the delayed
type-6 notice advances only 5.0163 game seconds (-15.9631 to -10.9468).
The captured run includes renderer-dump overhead; it establishes lost message
time, not a precise target-rate benchmark. The isolated profile is preserved
and the session exits normally.

Full Windows Release build and built-in checks pass
(`logs/native-hud-build.log`); 133 timing/translation/CI/backlog regressions
pass (`logs/native-hud-regressions.log`). The expanded objective fixture
also passes separately. The new binary SHA-256 is
`167e9bce3b96d4ff8ceaab378d52aaadb9d0b923fa3a7a42374a1364db413397`;
its compiled label is `7783767`, built with this HUD source correction before
commit. The original-60/native-120/native-144 bounded Scotland comparisons
use that same binary and the identical command sequence in isolated profiles.
Evidence is `private/test-saves/scotland-hud-after-{60,120,144}`, matching
logs, and `private/pipeline/hud-message-validation.json`.

| Mode | Observed message seconds / wall second | Observed unpaused span |
| --- | ---: | ---: |
| Original 60 | 1.000024 | 23.266 seconds |
| Native 120 | 0.999715 | 23.265 seconds |
| Native 144 | 1.000389 | 25.421 seconds |

These endpoint measurements span the scripted delay and visible banner, with
small asynchronous sampling error. Reviewed captures show the Scotland
objective banner fading; queue observations and later captures confirm its
removal. Pause takes effect after key delivery, holds the remaining lifetime,
and resume lets the notice expire. Pickup notices also retire in each run.
All three sessions exit normally with valid, unchanged test profiles; personal
controls/display hashes remain unchanged. These checks establish this HUD
clock behavior, not full cutscene/subtitle audio synchronization, all mission
scripts, multiplayer UI, physical input, or Mac execution. The Scotland jeep
encounter remains untested at native rates.

## Scotland opening comparison

The same Windows x64/D3D11 countdown-fix binary (`a0a7214` source, SHA-256
`41ca454f88d9d11279fe57cbf24fbd56dbca9b72a5d5550c154c20898a91fa43`)
completed bounded original-60 and native-144 runs using separate disposable
Test Lab profiles. Evidence is in `private/test-saves/scotland-original60`
and `scotland-native144`, with matching logs under `logs/`.

Both runs select Scotland the Brave, reach the boat cinematic (scene 30),
skip into dock gameplay (scene 4, mission 1), fire, move, pause and resume.
The initial position and look values match exactly. A 1.5-second synthetic
fire hold consumes three pistol rounds at both rates (8 to 5, reserve 52).
A three-second forward hold ends at `[1.616877, -46.362591, 103.591850]`
at original 60 and `[1.603127, -46.362701, 103.577972]` at native 144.
The endpoints differ by approximately 0.020 world units; input delivery and
sampling are not frame-identical. Both pause states suppress input and keep
position fixed, and both resume with suppression cleared. Reviewed opening
and post-movement captures show the same dock/wall location and ammunition.
Objective-message timing and animation phase are not established by these
non-synchronized captures. Both runs exit normally with unchanged, valid
profile signatures.

This covers basic opening controls only. Movement ends at the nearby wall;
it does not establish unobstructed movement speed or collision behavior
throughout the level. The jeep encounter, companion dismount/remount, combat,
mission completion and audio synchronization remain unverified at native 144.
The native trace cap was reached before gameplay, so this run supplies no
Scotland frame-pacing result. Native 120 Scotland and Mac execution remain
unverified.

## Arcade pacing without renderer captures

After the countdown correction, both native targets were measured on the same
Windows x64/D3D11 executable (`a0a7214` source, binary SHA-256
`41ca454f88d9d11279fe57cbf24fbd56dbca9b72a5d5550c154c20898a91fa43`).
`tests/stability_transitions_game.py --no-captures` omits all renderer dump
settings and skips screenshot labels, while preserving state observations,
bounded CPU timing CSV, logs and isolated Test Lab saves. Its provenance
records `captures: false`; neither run produced renderer BMPs. No game
source or pacing policy changed for this comparison.

| Target | Measured CPU updates/s | Intervals / seconds | p50 / p95 / p99 ms | Worst ms | Game/wall time |
| --- | ---: | --- | --- | ---: | ---: |
| 120 | 119.958 | 5,752 / 47.9502 | 8.3309 / 8.7218 / 8.9563 | 12.6748 | 1.000000 |
| 144 | 143.936 | 6,869 / 47.7225 | 6.9785 / 7.3793 / 7.9785 | 11.5586 | 1.000000 |

Evidence directories are `private/test-saves/arcade-pacing-120` and
`arcade-pacing-144`; logs use the corresponding names under `logs/`.
`update-stats.json` summarizes unpaused scene-48 intervals after trace second
62 / 58 respectively. The runs include combat/death/respawn states and are
not identical camera/AI workloads. Initial firing consumes six rounds and
forward movement changes position at each rate. At 120, later observations
confirm respawn with dual pistols and a further ten rounds consumed. The 144
late weapon-37 state does not prove a second firearm check. Both sessions
exit normally and retain unchanged, valid profile signatures. Personal
controls/display hashes remain unchanged.

Earlier capture-heavy runs had p99 near 33 ms. The much lower no-capture
intervals support screenshot overhead as a substantial contributor, not a
proven isolated causal measurement. Short scene CPU samples do not establish
full-game 144 FPS, GPU completion, monitor scanout, physical input latency,
4K/ultrawide performance, or other hardware/OS behavior. Retain captures for
visual checks separately from primary pacing measurements.

## Arcade Vietnam comparison

### Match-start countdown diagnosis

A bounded replay on Windows build SHA-256
`6934be7528dfe9635b34c0ca9579f3a93e5183e2373291dec2ad9ff7a88770f2`
reproduces stationary position and unchanged ammunition at native 144
(`private/test-saves/arcade-input-trace`, `logs/arcade-input-trace.log`).
The input watch observes the suppression mask reset to zero each frame,
then set to `0xFFFFFDFF` before the next suppression call, and finally
`0xFFFFFFFF` before `sub_0025F0C0`. This rules out a missing controller-mask
reset. Original dispatcher `0x68830` applies `0xFFFFFDFF` to all four ports
while the match-start countdown is active.

At `0x69043` it adds raw fractional time to integer counter `0x4D287C`,
then truncates back to integer at `0x69051`. Sub-tick additions disappear
at 120/144 FPS, so the start state cannot finish. The site-specific shared
numeric policy consumes the timing system's accumulated whole raw ticks
for this counter. Other additions keep their fractional values; the
original-60 path is unchanged. No input mask or match state is forcibly
cleared.

`tests/native_arcade_countdown.c` executes the complete checksum-verified US
state dispatcher and original mask writer. Rendering/messages/sound services
are spies and unrelated branches abort. Before the correction, original 60
and native 60 reach state 7 at tick 184 with four cues; native 120 remains in
state 5 with zero cues after 300 ticks. After the correction, original 60 and
native 60/120/144 reach state 7 at tick 184 with the same four cue IDs. The
fixture also checks all four input masks, guest return-stack preservation,
zero raw time, accumulated multi-tick input, adjacent-site scope and disabled
native policy. The full Windows Release build and its built-in checks pass, as do 132
shared timing/translation/CI checks. Final scope/Arcade-statistics checks
pass separately. These original-code checks alone do not establish playable
Arcade.

The first corrected live run (`arcade-countdown-120`) uses executable SHA-256
`41ca454f88d9d11279fe57cbf24fbd56dbca9b72a5d5550c154c20898a91fa43`.
The match reaches state 7 and the input suppression mask clears. After the
three-second forward command the player moves from `[48.9512, 0.3954, 66.0679]`
to `[66.2618, -0.4588, 52.9205]`. Firing reduces the dual magazines from 18/18
to 13/13 with shared reserve 24; the reviewed fire capture shows these counts
and bullet impacts. Pause/resume toggles the pause state correctly. The later
weapon-37/death state means the post-resume command is not a second firearm
acceptance check. Test profiles remain unchanged; the runner exits cleanly.

`native_update_stats.py --scene 48` filters Arcade intervals explicitly; the
default remains Story scene 2. The 120 trace after second 58 includes 2,057
unpaused intervals over 19.0999 seconds: 107.697 updates/s, p50/p95/p99
8.3202/9.8263/32.5744 ms, worst 318.0451 ms. Raw/game time ratios are both
0.979667; the existing three-tick catch-up clamp discards time during long
stalls. Renderer capture and scripted input were active. This diagnoses
remaining pacing limits, not stable target performance or monitor scanout.

The final native-144 replay (`arcade-countdown-144-final`) passes match start,
movement, firing and pause/resume on the same executable. The player moves
from `[48.9512, 0.3954, 66.0679]` to `[68.0795, -0.1995, 51.7039]` during the
three-second forward command. The earlier 1.5-second fire command reduces
both magazines from 18 to 15, with reserve 24; reviewed captures agree. The
input mask clears after start and resume. The run exits normally, preserves
profile signatures and contents, and reports no crash/unresolved call. The
first 144 attempt is partial evidence only: it confirms movement, then the
player is killed by Goddard and a disappearing rotating BMP stops the test
helper. The helper now tolerates that enumeration race; no game crash occurred.

The final 144 trace after second 56 includes 1,344 unpaused Arcade intervals
over 10.3235 seconds: 130.188 updates/s, p50/p95/p99 6.9124/8.2455/33.0204 ms,
worst 37.9402 ms. Raw/game time both advance at 1.0 seconds per wall second.
These short, capture-heavy scene samples establish neither sustained 144 FPS
nor full-game correctness. Full matches, other arenas/modes, online peers,
physical aiming, stable pacing and Mac execution remain open. Personal
controls/display SHA-256 values remain unchanged and no personal FPS sidecar
was created. Disposable test disks are recycled by the runner.

### Earlier baseline

Windows x64 build `c4171db` was tested through the normal Arcade menus using
isolated Test Lab profiles: Deathmatch, Vietnam, recommended weapons/bots,
Cortez. Executable SHA-256:
`85ebe7f5665eb2426b7d80438abfe59d8fe7c70584082268ae54122b57baa2ad`.
The extended `tests/stability_transitions_game.py` records read-only player
position/look/equipment/ammunition, bounded held-key commands, command times
and executable/runner hashes. The runner hash for both sessions is
`362787dc1a8eaf727985bd05b67ef4c4fd800eb8a4e445be0e0bc1536af26e6b`.
Commands and screenshots are retained in `private/test-saves/native-arcade`
and `private/test-saves/original-arcade`; logs are `logs/native-arcade.log`
and `logs/original-arcade.log`.

The native-144 session was bounded to 300 seconds and stopped normally after
the observations; the original-60 session was bounded to 150 seconds. They
use the same helper and binary, with the explicit native-rate override only
in the native session. Both preserve the fixture and exit normally; neither
reports a crash/unresolved indirect call. The runner's generic completion
message certifies process/fixture handling, not the scenario's gameplay.

Native-144 **fails Arcade gameplay acceptance**. At 179.391 seconds the arena
is visible and scene is 48, flags 34571298, player at
`[48.951206, 0.395421, 66.067909]`, dual weapon ID 2 with 18 rounds in each
magazine and shared reserve 24 (total 60). Three-second forward, strafe and
fire commands, including another fire attempt after pause/resume, leave the
same position and ammunition at 244.969 seconds. Reviewed captures
`match-start.png` and `refire.png` show the same view and full magazines.
The pause menu opens and closes normally. A separate read-only clock sample
(`live-clock-observation.json`) finds update IDs 39097/39169/39238 and level
ticks 4764/4794/4824 at roughly half-second spacing, positive fractional game
deltas near 0.42 and pause count zero. The title is not globally paused;
advancing its clock has not produced responsive player gameplay here.

Original-60 shows combat, death and respawn (`spawn-confirmed.png` includes
the dead player). During the later three-second fire command, the same ammo
type's total falls from 60 to 51, with 14/13 rounds loaded and shared reserve
24; the visible `game-fire.png` matches those values. Positions change and
pause/resume succeeds. Respawns and combat prevent treating these positions
as a controlled movement-distance comparison. Menu timing also differs: the
initial queued original commands stopped at the Ready screen, so their early
move/fire labels are excluded; subsequent confirmed scene-48 observations
provide the comparison. No network peers were used: this is local Arcade,
not online multiplayer validation.

The native CPU trace exhausts its 20,000-row cap at 143.542 seconds, before
the match, so it cannot supply gameplay interval percentiles for this run.
Late FPS reports show 140.4 swaps/s in native mode and 59.4 in original mode;
these are not evidence of correct simulation or an isolated performance
comparison. The separate user session remained open and responsive. Personal
controls/display hashes stayed unchanged; each runner recycled its disposable
disks/lock and preserved the canonical profile.

Next investigation: reproduce with faster state-confirmed menu navigation,
compare native 60 against native 144, and inspect the Arcade simulation/input
dispatch and any temporary delta scopes using original code. The frontend
input gate is a candidate, not a confirmed cause. No production correction,
Arcade pass, full-match completion or Mac result is claimed from this work.

## Terminal mouse fractional cadence

The follow-up to camera input found two independent losses in
`src/pc_terminal.h`: its separate consumption did not rebase the mouse batch,
and its pause guard rejected zero whole ticks even when native fractional
time advanced. Rejected terminal motion is deliberately consumed to prevent
it turning the hidden player or leaking on terminal exit, so those packets
were lost rather than deferred to the next tick.

`tests/native_terminal_mouse.c` executes the shared helper against guarded
guest memory. In the original helper, a one-second 1,000 Hz stream requesting
100 horizontal and 25 vertical pointer units produces only 39.1 and 9.775
units at native 144. At quarter speed the horizontal move falls to 9.5 units.
Original fixed-60 produces 87.4 horizontal units because of the separate
buffer-age bug. Evidence: `logs/native-terminal-before.log`.

Shared `pc_input_consume_motion` now returns the previous batch age while
clearing counts and rebasing the empty batch. Both camera aiming and terminal
movement use it. The terminal helper takes explicit native-mode selection from
the existing shared rate lookup in each host adapter. Native mode requires
positive finite fractional game time at `0x4E794C`; original mode retains its
whole gate at `0x4E7944`. A zero fractional delta rejects movement even when a
stale whole counter is positive. Counts remain unscaled by delta, zoom or
inversion, with the original pointer bounds and inactivity reset.

Forty-two cases combine original 60 and native 60/120/144, normal/quarter-speed
native clocks, 125/1,000/8,000 Hz packet schedules and uniform/uneven frame
intervals. Every corrected path retains the requested displacement within
0.003 pointer units. Movement occurs on exactly the frames receiving packets;
low-poll-rate input can legitimately leave some uneven frames without motion.
Additional cases cover native pause, invalid/nonfinite time, stale positive
whole ticks, original disabled-native behavior, all four local input indices,
catch-up without multiplying counts, scripted/disabled pointer descriptors,
ownership mismatch, stale input and return to the gameplay camera without a
stored jump. The original terminal routing tests and all 48 camera packet
cases still pass (`logs/native-terminal-fixed.log`).

Windows and SDL adapters also have synthetic integration cases that select
native 144, exercise a zero-whole live interval and stale-whole pause, then
switch back to original mode. Mac execution remains unverified. These fixtures
do not drive a live mission terminal, measure hardware input latency, or
establish pixel-level pointer smoothness. All 161 shared timing,
terminal/automatic-look/menu input, float/vehicle-join, CI-scope, backlog and
XMV-stop checks pass in 77.78 seconds (`logs/native-terminal-shared.log`).
The complete unchanged patch stack verifies with zero updated runtime files
(`logs/native-terminal-patches.log`).

The full supported Windows MSVC x64 Release build of `c4171db` passes its
built-in checks, including the real Windows adapter's five native/original
selection, pause and resume cases (`logs/native-terminal-build.log` / `.err`).
Executable SHA-256:
`85ebe7f5665eb2426b7d80438abfe59d8fe7c70584082268ae54122b57baa2ad`.
Matching crash symbols were generated. The SDL adapter integration case is
present in source but has not executed on Mac. No new live terminal scene or
gameplay timing measurement was performed for this source checkpoint; the
previous opening replay did not exercise a terminal and is not acceptance
evidence for it.

The user's PID 38248 remains responsive on the retained `3c2fcc2` executable;
the normal executable is ready for the next launch. Controls/display hashes
match `private/pipeline/terminal-protected-before.json`. No interactive game
was closed, and automated checks used fixture/disposable data.

## Raw mouse packet cadence

The native-refresh aiming audit found an input-buffer age bug in shared
`src/pc_input.c`. `pc_input_motion` drops an unconsumed batch after 50 ms to
avoid storing a long turn while gameplay/camera updates are suspended.
`pc_input_look` consumed its counts without resetting that age. At ordinary
frame rates, a new packet could therefore clear other fresh packets already
queued for the same frame using a timestamp from a previously consumed batch.
The resulting count loss depended on packet timing and camera update rate.

The correction saves the incoming timestamp for the existing stale-batch
check, then rebases the empty buffer to the current consumption time. Normal
camera updates retain all packets received between them. When no camera
consumes input, the original 50 ms discard still bounds accumulated movement;
the existing 100 ms stale-look rejection and focus/capture clearing remain.
Mouse sensitivity, FOV scaling, pitch limits and counts-based yaw are unchanged.
This helper is used by both Windows `pc_controls.c` and native SDL
`input_sdl.c`, including the original fixed-60 mode. No platform fork or new
timing gate is added.

`tests/native_mouse_cadence.c` executes that actual shared input code, with
packet arrivals independent of camera updates. For a one-second 1,000 Hz
stream containing 1,000 horizontal counts at sensitivity 0.1, the original
60/120/144 update paths turn 87.4/93.8/93.3 degrees instead of 100. The fixed
paths turn 100 within 0.0002 degrees of float accumulation error. Twelve
uniform unzoomed rate combinations expose loss in eleven original cases;
the original 125 Hz mouse / 144 Hz camera happens to retain all counts.
Evidence: `logs/native-mouse-cadence-before.log`.

The expanded fixture passes 48 combinations: 60/120/144 camera updates;
125/500/1,000/8,000 Hz packet arrivals; uniform and alternating +/-2 ms frame
offsets; normal view and half-FOV inverted pitch. Each run delivers the same
total horizontal/vertical counts, checks the analytical end angles and verifies
that a second consumption cannot replay the same motion. Additional checks
cover stale buffered motion, fresh input after a stall, bounded continuous
motion while no camera consumes it, focus loss and capture transitions.
Evidence: `logs/native-mouse-cadence-fixed.log`. The test is registered in
the existing shared native timing suite. These are synthetic packet schedules,
not measured physical-device delivery, end-to-end input latency or live
camera-pose acceptance. Actual Mac execution remains pending.

All 153 shared native timing, float/vehicle-join, CI-scope, backlog and XMV-stop
checks pass in 69.51 seconds (`logs/native-mouse-shared.log`). The unchanged
complete patch stack verifies with zero updated runtime files
(`logs/native-mouse-patches.log`). Seven terminal routing, automatic-look and
menu-mouse checks pass (`logs/native-mouse-context-checks.log`). The full
supported Windows Release build of `149bef9` and its built-in checks pass,
including the real host input adapter/guest-camera integration checks
(`logs/native-mouse-build.log` / `.err`). Executable SHA-256:
`530861ca93dd9241a1d9247a338e40495e307810b71b828c118cb3e0b9f88a94`.

A 120-second native-144 isolated replay passes the opening firearm handoff,
hand animation, movement, firing and pause/resume checks. Evidence is
`logs/native-mouse-gameplay.log` and the `native-144-mouse-cadence` records in
`private/test-saves/high-refresh`. Its runner hash is
`f328225561af2c719afe184bb0299417f367572ea043cbc7636354ff187764e1`.
After 60 seconds, 7,934 active Story CPU intervals span 56.524 seconds: mean
140.365 updates/s, p50/p95/p99 7.0401/8.4030/9.4087 ms, maximum 35.5394 ms.
Game and raw time both advance at 1.000000 seconds per wall second. This run
uses sparse synthetic replay events; the independent packet fixture above,
not this gameplay replay, establishes the dense-packet count-loss correction.
The retained end capture shows the equipped pistol and 80 reserve / 16 loaded
rounds. It also contains an orange streak at the top right; the deferred trail
report is not cleared by this smoke test.

The user's previous `3c2fcc2` session (PID 38248) remained responsive on its
retained executable throughout build and validation. Thus the timing sample
includes another running game and is not an isolated performance benchmark.
Controls/display hashes are unchanged and the runner cleaned its disposable
disks/lock. The normal executable is ready for the next launch; no forced
restart occurred.

Follow-up input audit: `pc_terminal_mouse` consumed motion separately in
`src/pc_terminal.h`. Its buffer-age handling and integer game-delta gate are
unchanged by this camera fix. Existing terminal routing checks passed, but did
not cover native fractional pointer cadence. The separate reproduction and
pause-preserving correction are recorded above; live terminal visual
acceptance remains open.

## Latest-build opening comparison

On Windows x64, the latest production source `3c2fcc2` was compared using
the same executable and runner in two 120-second isolated Test Lab replays:

```
python tests/native_refresh_game.py --fps 60 --label movie-strip-regression --stress --pause-check --seconds 120 --windowed --capture
python tests/native_refresh_game.py --fps 144 --native --label movie-strip-regression --stress --pause-check --seconds 120 --windowed --capture
```

Executable SHA-256:
`16ddf892fc209737f682345c5d7203945bc531a52dadb471e9f231c180c753f7`.
Runner SHA-256 at both executions:
`060b1424dab281a9cbaffd85c0eb9609fd0666f5fa50b254cbbab1c1cea98492`.
The supported US XBE hash is checked by the runner before launch. Both runs
completed, passed their gameplay/pause checks and were terminated at the
requested bound by the runner (child exit 1 is the Windows termination result,
not an observed crash). Logs are `logs/native-latest-original60.log` and
`logs/native-latest-144.log`; full records use the corresponding
`60-movie-strip-regression` / `native-144-movie-strip-regression` prefixes in
`private/test-saves/high-refresh`.

| Observation | Original 60 | Native target 144 |
| --- | ---: | ---: |
| First sampled firearm handoff, seconds after launch | 72.844 | 72.328 |
| Total ammunition, start/end of 85–115 s action window | 128 / 96 | 128 / 96 |
| Observed hand animation clips in action window | 1180, 1188, 1190 | 1180, 1188, 1190 |
| Movement, hand advancement, pause/resume | Pass | Pass |

The half-second observer and wall-time input replay do not prove identical
handoff timing or trajectories. Final sampled positions differ by about
0.4 game units; movement parity remains broader than these smoke checks.
The selected original frame `60-movie-strip-regression-frame_s00006900.bmp`
and native frame `native-144-movie-strip-regression-frame_s00016560.bmp` were
reviewed: both show the opening canyon, an equipped visible pistol and
80 reserve / 16 loaded ammunition. They are not synchronized frame pairs or
proof of perceptual smoothness.

`tools/native_update_stats.py` measures 7,936 unpaused Story CPU update
intervals after 60 seconds, totaling 56.5248 seconds: mean 140.399 updates/s,
p50 7.0356 ms, p95 8.3620 ms, p99 9.3273 ms, maximum 35.1005 ms. Summed game
and raw deltas both yield 1.000000 seconds per wall second. The original path's
last five-second FPS report is 59.7; it has no equivalent detailed CPU trace
in this run, so this is not a percentile comparison. A separate interactive
game was open during each test (older `b3c30cf` during original-60, latest
`3c2fcc2` during native-144); this is not an isolated performance benchmark.

Presentation records show guest-tagged API submissions, no interpolation and
no reported presentation failures. They cannot alone prove fresh content:
the movie investigation below demonstrates repeated guest-tagged pictures.
The runner's assertion wording and its test name now reflect that limitation;
the checks themselves are unchanged. Fresh update cadence is measured
separately by the native CPU trace.

The presentation-check unit test passes, as do all 66 backlog/CI-scope checks
(`logs/native-latest-evidence-checks.log`, `logs/native-latest-doc-checks.log`).
Only test wording and evidence documentation changed after these live runs;
no production rebuild is required for this checkpoint.

The user-requested latest-build session (PID 38248 at validation) remained
responsive throughout the native run. Personal controls/display hashes match
`private/pipeline/gameplay-comparison-protected-before.json`; disposable
test disks and the run lock were cleaned by the runner. These replays disable
startup movies and skip the intro. They do not trigger/accept the new strip
distortion visually, test the later bridge encounter, resolve the deferred
enemy/trail/sniper reports, establish input latency or GPU/scanout timing,
or validate later levels, other modes, multiplayer or native Mac execution.

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

This section preserves the baseline and earlier experiments. The current
source correction is described under [Native object rocking integration](#native-object-rocking-integration).

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

The initial audit made no production change to this routine. A motion fix must preserve the
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

### Native pause guard and rejected fractional prototype

Extending the pause check to an already-settled object reproduced a separate
failure: with height 0 and velocity 0.2, the original callback clears +0x16C,
clears the rocking flag and stamps +0x90 even when the live game delta is zero.
The existing moving-object pause check did not enter that pre-loop branch.
`logs/native-rocking-pause-before.log` records the failing entity-state check.

The shared policy now takes the bypass branch at 0xA8526 only during native
zero-time calls, then supplies zero to this callback's two whole-tick loop
bounds. It still executes the original matrix builders: returning at function
entry would omit the rocking transform when the caller supplies a fresh base
matrix. No guest field is temporarily changed. Advancing intervals and the
native-off path retain the original comparison and loop counts. The additive
`native-object-pause` patch exposes an optional float-branch policy in the
lifter; the generic default preserves the original condition, and operands
remain captured at the comparison instruction.

The original-code fixture checks byte-identical entity state and matrices
for a tilted moving object with zero live delta and carried counts 0 through
6. It separately checks settled-object pause, positive-fraction resumption,
native-off zero-tick compatibility, and the existing 72 advancing bounce/sound
configurations. The strict rocking-cadence expected failure remains open.
This pause correction does not make bounce motion advance at 144 Hz.

A separate, unshipped prototype tested a fractional extension of the original
free-flight affine update. For one original step, write `v' = r*v - q` and
`h' = h + a*v - b`, where `dt=1/60`, `r=1-damping*dt`, `q=gravity*dt`,
`a=0.8333333134651184*dt`, and `b=4.158332824707031*dt*dt`. For a fraction `d`,
`s=(1-r^d)/(1-r)`, `v(d)=r^d*v-q*s`, and
`h(d)=h+a*s*v-b*d-a*q/(1-r)*(d-s)`. Angular increments were scaled by `d`;
the original floor, tilt and sound logic ran after each fractional step.

This produced 144 changed free-flight transforms out of 144 updates, with
one-second endpoints within 0.00002 of the original in the measured normal
and quarter-speed cases. It did **not** preserve the sampled impact response:

| Case (height 0.2, velocity -3, impact flag 0x40) | Audible requests | Settling game tick |
| --- | ---: | ---: |
| Original 60, normal | 3 | 63 |
| Prototype 120, normal | 3 | 55 |
| Prototype 144, normal | 3 | 53 |
| Original 60, quarter speed | 3 | 62 |
| Prototype 120, quarter speed | 3 | 52 |
| Prototype 144, quarter speed | 3 | 52 |

At normal 144 the impact tick sequence changed from 5/29/43 to 4/28/41,
and the last sound gain changed from 0.359771818 to 0.313612521. The prototype
measured these differences by relaxing cross-rate assertions only in its
temporary in-memory fixture; the tracked acceptance assertions were not
weakened. All 72 prototype configurations completed, but completion is not
acceptance. See `logs/native-rocking-fractional-experiment.log`; the ignored
driver is `private/pipeline/rocking-fractional-experiment.py`. No prototype
integration, floor response or sound change is present in production source.
Preserving original impact/settling behavior while advancing fractional motion
remains the next design problem; free-flight endpoint agreement is insufficient.

A second ignored prototype splits the affine motion at original one-tick
boundaries and evaluates near-floor constraints only at those boundaries.
In the same fixture it retained impact ticks 5/29/43 and sound gains within
0.000001, while still producing 144/144 changed free-flight transforms.
However, it can carry negative height between boundary checks, and its
phase-gated retirement waits until tick 65 at 144 FPS rather than 62/63.
The nonnegative-height assertion was relaxed only in that temporary
experiment; this is another incomplete design, not an accepted fix.
`logs/native-rocking-contact-boundary-experiment.log` and the ignored
`private/pipeline/rocking-contact-boundary-experiment.py` preserve the result.
Object initialization/reuse, interleaved callbacks, floor presentation and
settling across partial steps must be resolved before integrating such a path.

For the pause correction, the shared timing, float/vehicle translation,
CI-scope and backlog suite passes **135 tests with the existing rocking
expected failure** in 64.37 seconds (`logs/native-rocking-pause-shared.log`).
All **21 patch-upgrade checks pass** in 418.91 seconds, including older
Windows/Mac stacks and repeated application (`logs/native-rocking-pause-upgrades.log`).
The supported Windows `tsfp.ps1 Generate` pipeline completes in 83.4 seconds,
and its generation fingerprint is current. A Release/x64 `ClCompile` pass
with MSBuild 17.14.40 and MSVC 14.44.35207 succeeds for the generated units,
manual code and display menu. It uses two compiler workers at below-normal
priority. Logs are `logs/native-rocking-pause-generate.log` and
`logs/native-rocking-pause-compile-only.log`. MSB8028 again reports shared
intermediate-project files; there are no compilation errors.
This was compile-only, with no relink or launch: the responsive interactive
PID 25304 still runs `fc3a7ce`, whose executable SHA-256 remains
`912f416b5f6eaeabf545c960c255491907988eae9f031ead4231b24832cbc333`.
The pause correction, NPC aim correction and kind-35 projectile motion
changes still need a complete supported build and targeted live checks.
Actual Mac compilation/runtime and full-game acceptance remain unverified.

## Native object rocking integration

The shared `pc_native_rocking_guest.h` implementation now advances the
0xA84F0 callback on fractional intervals. It evaluates the affine
gravity/damping flow at the current time, using the last original contact
state as its integration origin. It does not blend two rendered poses or
require a future frame. At an original one-tick boundary it uses the exact
original float operation order, then executes the original floor response,
tilt limits and sound builders. Keeping those contact intervals avoids the
earlier prototype's changed impacts and settling. The original matrix
builders run for every advancing native update and during pause.

| Native update rate | Fresh transforms, normal speed | Fresh transforms, quarter speed |
| ---: | ---: | ---: |
| 60 | 60/60 | 60/60 |
| 120 | 120/120 | 120/120 |
| 144 | 144/144 | 144/144 |

The one-second height/velocity endpoints match the original **exactly** in
all six native cases: 9.546200752 / -4.523219109 at normal speed and
10.686166763 / 1.669251561 at quarter speed. The native-off quarter-speed
reference still changes 15/60 transforms. The strict expected-failure mark
has been removed; repeated native transforms now fail the test normally.

The bounce fixture expands to 96 combinations: native-off 60 plus native
60/120/144, normal/quarter speed, two initial tilts and six material flags.
All preserve the original impact ticks and byte-identical sound payloads,
including gain, material priority and position. Native settling retains the
previously documented next-callback distinction (tick 62 versus original
normal-speed 60-FPS tick 63). Zero-time calls preserve state and matrices.
A pause immediately before the first floor-impact boundary retains latent
penetration without drawing below-floor height, emitting early audio, or
losing the eventual original impact velocity. The sound boundary is captured,
not played; the previously verified CRT asin service remains a host adapter.

Timing state is kept per slot of the original object pool, whose base and
capacity are 0x72F5E8/0x72F5E4 and whose stride is 0x2E8. The original pool
reset at 0xA5AF0 explicitly clears that array; its shared wrapper invalidates
the host state too. The 0xA8AA0 initializer either starts a fresh rocking
instance or adds an impulse to an existing instance, including recursive
child impulses. New instances reset their slot; live impulses rebase the
motion at the current phase without restarting the contact clock. A hidden
negative offset is restored only for physical calculations, while exposed
height remains nonnegative between contact boundaries. No guest padding is
used. Pool base/capacity changes clear the host state; native-off, invalid pool
membership, allocation failure, non-60 base rate and intervals outside the
audited 0–6-tick range retain the original updater.

The original-code lifecycle checks compare separate and interleaved runs,
nested update contexts, 60 single ticks versus uneven fractional/catch-up
steps, impulses applied partway through a tick, rejected impulses, recursive
initialization, address reuse, pool reset/base/capacity changes and unknown
entities. The original initializer's RNG call count is retained. These checks
exercise both the shared header and translated original routines, rather than
replacing the bounce response with a test approximation. The pool-reset
wrapper's original-call boundary is spied; the full level-reset chain is not
executed by this fixture. Whole-game callback/lifecycle coverage remains open.

The additive patch provides optional scalar add/subtract policy hooks and
wraps the updater, initializer and pool reset. Generic arithmetic is unchanged;
the title policy selects only the audited rocking instructions. Full generation
confirms the recursive initializer calls the wrapper, not its generated body.
Measurements and lifecycle results are in
`logs/native-rocking-integration-measurements.log` (two focused checks).
The broader timing, float/vehicle translation, CI-scope and backlog suite
passes **136 tests with no expected failures** in 67.23 seconds
(`logs/native-rocking-integration-shared.log`). Supported Windows generation
passes in 91.0 seconds (`logs/native-rocking-integration-generate.log`).
All **21 patch-upgrade checks pass** in 424.54 seconds
(`logs/native-rocking-integration-upgrades.log`). Release/x64 `ClCompile`
succeeds with MSBuild 17.14.40 / MSVC 14.44.35207, using two below-normal
priority compiler workers (`logs/native-rocking-integration-compile-only.log`).
The existing MSB8028 intermediate-directory warning remains; no compile errors
occurred. This check compiles the generated/manual code and display menu but
does not relink or replace the running executable.

A ten-second read-only observation of the unchanged interactive `fc3a7ce`
build found state 101, a pool capacity of 20 and no listed entities in all six
samples (`private/pipeline/rocking-pool-observation.json`). Those non-atomic
samples establish no live rocking behavior or scene acceptance.

The subsequent full supported Windows PowerShell 5 `tsfp.ps1 Build` succeeds
on `dee6170`, including its built-in checks, network identity seal and matching
crash symbols (`logs/native-rocking-full-build.log`). The loaded `fc3a7ce`
executable was renamed in place to
`build/windows/Release/futureperfect_recomp.running-fc3a7ce.exe`, preserving
the responsive interactive PID 25304. Its SHA-256 remains
`912f416b5f6eaeabf545c960c255491907988eae9f031ead4231b24832cbc333`.
The new normal Release executable is `dee6170`, SHA-256
`3e5932fb336506015b35e83312a8a3c9c8cd5eee2a855843e916462e0bb454e6`.
The retained loaded image is needed until that interactive process exits;
its original reported process path does not establish that it runs the new
file. Ignored `private/pipeline/live-build-handoff.json` records the handoff.
No second game-data copy or build tree was created. Representative live scenes
and actual Mac compilation/runtime remain pending.

The first effects-enabled run exposed an observer startup race: it read the
scene global before the guest address space was mapped. The runner now probes
that global before any guest observation, tolerates startup unavailability for
at most five seconds, and still fails on missing/partial memory at the deadline.
The game-exit check remains active throughout. A focused regression exercises
unmapped, partial, mapped menu-state and later unreadable memory. Five observer/
smoke-check tests and five effects/entity tracing tests pass; 66 CI-scope/backlog
checks also pass. The runner change does not alter game code or require relinking.

The repeated 120-second native-144 run on the new executable passes all checks:
Story, firearm handoff, advancing/changing hand animations, movement, ammunition
consumption, pause/resume, fresh guest-only successful presents and no logged
crash. It uses isolated Test Lab saves/configuration and internal replay input,
with its window created behind the user's existing window without activation.
The owned child alone is terminated at the deadline; the resulting child exit
code 1 is intentional, recorded separately from the successful runner result.
Logs are `logs/native-rocking-live-144.log` and
`private/test-saves/high-refresh/native-144-rocking-build-*`.

The unpaused Story window after trace second 60 contains 7,811 CPU update
intervals over 56.4306 seconds: mean **138.418 updates/s**, p50 **7.0673 ms**,
p95 **8.8808 ms**, p99 **10.1254 ms**, worst **32.1661 ms**, and game/raw time
ratios both **1.000000**. The old interactive renderer remains running, so this
is a shared-load measurement, not an isolated performance benchmark or proof
of steady 144 Hz, GPU completion, monitor scanout or input latency.

The effects observer retains 147 snapshots, 146 complete; one detects changing
list heads. Observed effect kinds are 2/5/9/11/12/29/39, with **no type-32 beam**.
All 74 entity traversals complete, but none observes callback 0xA84F0. These
half-second/one-second non-atomic samples can miss short-lived effects and
cannot verify live rocking or identify the reported delayed trail. A reviewed
late capture shows the starting canyon, weapon 14, crosshair, ammunition HUD
and surrounding geometry. The replay does not reach the pre-bridge encounter,
plasma-sniper hit scenario or post-bridge ship landing; those reports remain
open despite the passing smoke check and original-code fixtures.

The matching 120-second original-60 replay also passes all gameplay and pause
checks on the same executable (`logs/native-rocking-live-60.log`, evidence
prefix `60-rocking-reference`). All 146 effects and 73 entity traversals
complete; it observes the same effect-kind set, again no type-32 beams or
rocking callbacks. These are repeated samples, not equal-spawn-count or
per-frame motion comparisons. The reviewed final capture shows the same
starting canyon/weapon/HUD, with 16 rounds loaded and 80 reserve in both runs.
Each run retains three BMPs; all evidence for the pair totals about 25.1 MiB.
The disposable disk cleanup completes and the run lock is removed. Personal
controls/display SHA-256 values match their pre-build values, and the old
interactive PID 25304 remains responsive. The user subsequently deferred
the encounter-specific reports and asked to prioritize overall experience;
they remain recorded as unverified rather than resolved.

## Performance follow-up and rejected GPU-poll experiment

After the user deferred the encounter-specific reports, the `dee6170` build
passed a 120-second native-144 replay with 200 Hz main-thread profiling
(`native-144-experience-profile-*`). The whole-run sample includes startup,
menus and gameplay: 5,182 of 20,573 samples ended in `NtQueryVirtualMemory`
and 2,531 in `sub_003D6690`. Folded wrapper symbols can be misleading;
these counts neither isolate active gameplay cost nor establish GPU time.

Experimental commit `aef8d29` immediately acknowledged the audited PFB-flush
poll at 0x3D66D0. A 32-case original-submit fixture, 143 shared checks,
21 patch-upgrade checks and a full Windows build passed. The actual two-minute
replay **failed** to reach Story or perform movement/firing/pause checks.
Its profile instead accumulated 14,390 samples in `sub_003D65B0`; the last
buffered frame timestamp was 34.3128 seconds. Sampling does not identify the
precise stalled instruction or prove the dependency between the two waits.
This is evidence that the isolated fixture did not model full device progress,
not permission to bypass another driver wait.

The entire optimization was reverted in `eae18a3`, the complete patch stack
reapplied, generated source refreshed and the supported Windows Release build
rerun successfully. The rejected executable was removed by an exact hash
check. The restored executable SHA-256 is
`12863fe128dfde83e4a726c5feb924a0ba361e77ae8aae9dcf8caeeb69d4ada2`.
All 137 shared regressions pass. The 120-second restored native-144 replay
passes Story, movement, firearm handoff/animation, ammunition consumption,
pause/resume and fresh guest-only presents. A late capture shows the starting
canyon, weapon and HUD. This does not revisit the user's deferred encounter.
Evidence: `logs/native-gpu-flush-revert-build.log`,
`logs/native-gpu-flush-revert-shared.log`, `logs/native-gpu-flush-restored-live.log`,
`private/test-saves/high-refresh/native-144-gpu-flush-restored-*` and
`private/pipeline/gpu-flush-rejected.json`. The user's old PID 25304 exited
independently before the experimental test; no user session was terminated.

The subsequent, narrower prompt change keeps one fresh mapping snapshot per
wrapper invocation. It reuses the OS-reported bounds and write permission
for nearby text and scratch, without retaining them across guest execution.
On Windows a non-page-aligned stack and nearby rewritten text require one
VM query instead of two (or three for text below ESP). Page-aligned stacks,
other regions, protection boundaries and unwritable scratch still receive
their necessary queries. Windows real-VM tests exercise those cases, changed
permissions on successive calls, no-access and guard pages. The native Mac
self-test adds read-only scratch and page-boundary cases but is not run here.
The prompt change is committed locally as `6fc4d87`. Its full supported Windows
Release build and built-in tests pass, along with 66 CI-scope/backlog checks.
The controller-routing subtest explicitly skips when foreground activation is
refused; this is not physical-controller acceptance. The generated-code
fingerprint remains current and the pinned US XBE hash is unchanged. Executable
SHA-256: `1f59510ed7bc8ce088dd87238546bee89e65ac30abf46f8ce3f61d4700109b02`.

The 120-second native-144 replay with profiling and capture passes all Story,
movement, firearm handoff/animation, ammo, pause/resume and fresh-present checks.
The reviewed final capture shows the starting canyon, weapon and ammunition HUD
(16 loaded, 80 reserve). Its unpaused Story intervals after trace second 60
average **137.512 updates/s**, p50 **7.0515 ms**, p95 **9.1119 ms**, p99
**10.4290 ms**, worst **34.2793 ms**, with game/wall time ratio **1.000000**.
The restored build's earlier capture-only run averaged 138.285 updates/s;
profiling and background-load differences prevent a causal before/after
performance conclusion. No overall FPS gain or perceived-smoothness improvement
is claimed from the reduced query count. Stable 144 FPS and complete-game
coverage remain open. The matching 120-second original-60 replay also passes
all gameplay/pause checks on the same executable. Its reviewed final capture
shows the same canyon/weapon/HUD and 16 loaded, 80 reserve ammunition.
Evidence: `logs/native-prompt-region-live-60.log` and
`private/test-saves/high-refresh/60-prompt-region-reference-*`.

Evidence: `logs/native-prompt-region-full-build.log`,
`logs/native-prompt-region-test.log`, `logs/native-prompt-region-shared.log`,
`logs/native-prompt-region-live-144.log`,
`private/test-saves/high-refresh/native-144-prompt-region-*` and
`private/pipeline/prompt-region-validation.json`. Selected final captures are
retained; disposable test disks and run locks are cleaned, and no test child
remains. Redundant intermediate captures and the no-longer-loaded `fc3a7ce`
executable were removed after validation. Personal controls/display hashes
remain unchanged. Nothing has been pushed and actual Mac validation is pending.

## MapMaker random light preview

The visual timing audit identifies `sub_00116130` as a light-color preview
routine, not world texture scrolling. Its only direct call in the pinned US
disassembly is 0xA1E8B, gated by scene 100 (MapMaker) and entity type 2.
It selects constant, sinusoidal, square-wave or random light behavior from
the 40-byte parameter record, writes a packed color and a float RGB table,
then propagates that color through `sub_00115F50`.

The mode-3 branch tests whole game ticks at 0x11622E. A zero value jumps to
the color blend with XMM0 still zero, replacing the previous color with the
dark endpoint. The complete original routine reproduces white `ffffffff`
becoming black `ff000000` on a live 144-Hz fractional interval without any
RNG call or deadline change. Modes 0/1/2 retain their color for that same
clock sample. This identifies extra dark subframes, independently of any
authored random flicker; it is not a diagnosis of the deferred gun trails.

The shared wrapper in `pc_native_light_guest.h` leaves mode-3 color and its
already-propagated model state intact until the next whole-tick decision.
Native pause also retains it even when a stale positive whole count remains.
Other modes, positive-whole updates and the original-60 path call the original
routine unchanged. The caller does not consume a return value; the wrapper
preserves its cdecl return-slot ownership and callee-saved registers. There
is no host cache, so initialization/palette edits/slot reuse remain guest-owned;
a new or edited random light takes effect on the next original decision tick.

`tests/native_light_preview.c` executes the complete original routine and
switch table, with deterministic RNG and a spy for downstream model updates.
An eight-game-second reference supplies 480 decisions and 69 RNG calls.
Native 60/120/144 at normal/quarter speed matches every packed color, float RGB
table entry, deadline and cumulative RNG count. Native 144 holds 672 normal
or 4,128 quarter-speed intermediate frames without introducing dark subframes;
paused probes preserve the same state. The unpatched counterexample remains
part of the fixture. This proves the color decision/dispatch boundary, not
actual model upload, GPU output, complete MapMaker operation or Mac execution.
Runtime source is committed locally as `8893ba8`. The supported Windows Release
build and built-in checks pass; the physical-controller routing subtest skips
when Windows refuses foreground activation. All 138 shared checks and 21
patch-upgrade checks pass. An additional original-code check preserves the
complete parameter record, RGB table and RNG for one/two/three-tick catch-up.
The generated original/manual wrapper declarations and fingerprint are current.
Executable SHA-256:
`56c319e0f4cd6fba9d3e9a6cbea169f0927870222897f1c0a91e791808a18de7`.

The 120-second native-144 Story replay passes movement, firearm handoff/hand
animation, ammunition use, pause/resume and fresh guest-only presents. The
reviewed final capture shows the starting canyon, weapon and ammunition HUD.
Active CPU intervals average 136.724 updates/s, p95 9.2886 ms, p99 11.3105 ms,
worst 51.1706 ms; game/wall ratio is 0.999979 with the existing catch-up clamp.
This is not a performance comparison, steady-144 proof, or MapMaker acceptance.
Actual light/model rendering in MapMaker and actual Mac execution remain open.
Evidence: `logs/native-light-preview-{shared,upgrades,full-build,live}.log`,
`logs/native-light-preview-fixed.log` and
`private/test-saves/high-refresh/native-144-light-preview-*`.

The audit also records all nine direct writers of the absolute visual clock
at 0x7DE318 in `private/pipeline/visual-clock-writes.asm`: ordinary increments,
initialization, recorded playback and network synchronization. Smooth pulse
and screen-distortion consumers still sample whole absolute time. They need
fractional-phase support that respects those clock resets and overrides;
globally replacing integer absolute-time reads is not justified by this
evidence and is not part of this correction.

## Screen-distortion absolute-time audit

The full original US `sub_000F9700` (`0xF9700..0xF9B0B`) calculates a
screen-distortion grid from the whole absolute clock at `0x7DE318`. Its direct
caller is the per-player effects renderer `sub_000FED20` at `0xFEDA3`, after
processing five effect slots. This is a framebuffer distortion effect; it
does not establish the behavior of world texture scrolling or all animations.

`tests/native_screen_distortion.c`, registered in the existing shared native
timing suite, executes that entire routine, the original grid-size selector
`sub_000F8AA0`, and original sine-table initializer `sub_00153C20` plus its
FSIN helper `sub_00012010`. All code and constants are read from the pinned,
SHA-256-verified US executable. The fixture supplies a controlled 640x448
grid with amplitude 0.75, fixed edges, a viewport and active-player records.
Graphics services are spies: every update checks all 99 vertex positions,
all 80 draw packets of 21 words, the framebuffer-copy bounds, call counts,
stack/callee-saved registers, FPU depth and unchanged guest timing fields.
This tests guest geometry and dispatch, not GPU output or scene activation.

For eight game seconds, the current production numeric policy repeats the
whole original-60 mesh and draw packets on each intermediate native frame.
A **test-only** clock-read probe supplies the exact fractional absolute time
to the original calculation. It is not linked into the game and does not
implement clock accumulation, reset detection or network/replay integration.

| Target FPS | Speed | Updates | Current repeated outputs | Fractional probe repeated outputs |
|---|---|---:|---:|---:|
| 60 | Normal | 480 | 0 | 0 |
| 120 | Normal | 960 | 480 | 23 |
| 144 | Normal | 1,152 | 672 | 44 |
| 60 | Quarter | 1,920 | 1,440 | 279 |
| 120 | Quarter | 3,840 | 3,360 | 1,452 |
| 144 | Quarter | 4,608 | 4,128 | 2,059 |

Every probe sample at a whole-tick boundary matches the original mesh and
draw packets exactly. The remaining repeated outputs reflect the original
integer pixel conversion; fractional time alone cannot remove that geometry
quantization. The fixture also samples repeated paused times, forward/backward
rebases, zero-time resets and two player-view selections at the same time.
These verify consumer determinism, **not** execution of the actual pause,
reset, replay or network writer paths. Quarter speed is a controlled clock
input, not proof of the complete live slow-motion path.

The next production correction needs a fractional absolute clock tied to the
original clock writes. It must distinguish an advancing zero-whole-tick
interval from a branch that never updates the level clock, explicitly rebase
on initialization/demo/network writes (including same-value writes), and
allow several player views to sample one clock value without advancing it.
The six ordinary direct increment stores are `0x688BB`, `0x68F8E`,
`0x691F3`, `0x69239`, `0x69265` and `0x69325`; direct reset/override stores
are `0x670E8`, `0x43595` and `0x357939`. This inventory does not rule out
indirect writes. In particular, the `0x6924D` path can skip the increment
after calling `sub_00153AF0`, so elapsed host time alone is insufficient.

Evidence: `logs/native-screen-distortion-audit.log` and
`logs/native-screen-distortion-shared.log`. On Windows x64/MSVC, all 139 shared
timing, float/vehicle-join, CI-scope and backlog checks pass in 71.25 seconds.
Runtime build `8893ba8` is unchanged;
no rebuild or gameplay interruption is needed for this test-only audit.
Actual Mac execution and live distortion visuals remain unverified.

## Fractional screen-distortion clock

Following the preceding reproduction, shared source now tracks the fractional
difference between continuous elapsed level time and the original integer
clock. `src/pc_native_visual_time.h` owns that model;
`src/pc_native_visual_time_guest.inc` observes the six ordinary direct clock
increments and the three direct initialization/demo/network stores through
an additive `*-native-visual-time.patch`. Store values and integer mechanics
are unchanged. Ordinary increments consume the live float delta at the store;
explicit rebases clear the fractional difference, even for same-value writes.
A branch that does not write the clock does not advance this state.

The shared lifter adds a default identity policy for direct 32-bit global MOV
stores and an instruction-address-aware integer-to-float conversion policy.
Only stores to `0x7DE318` and the audited read at `0xF9726` use the new title
logic. Other stores/conversions and the original inactive-native path retain
their values. Render callbacks sample the clock; they do not accumulate time
per view or call. A different unobserved clock value falls back to its original
integer and rebases the next observed increment. Same-value indirect writes
cannot be distinguished this way; the known lifecycle sites are explicit
direct stores, but complete live lifecycle coverage remains required.

The complete original distortion fixture now compares three paths: original
whole time, an exact fractional probe, and the production store/sample policy.
The production policy matches the probe's repeated-output counts in all six
normal/quarter-speed 60/120/144 cases above, including **44 repeated outputs
instead of 672 at native 144** over eight game seconds. Original whole-tick
mesh/draw packets match exactly in those sequences. Integer pixel coordinates
still limit very small changes. Additional original-code checks cover paused
sampling, rebase/seek sampling and two player-view selections at the same time.

`tests/native_visual_time_guest.c` executes all nine original store
instructions from the pinned XBE, with a RET appended after each store. It
checks exact guest integer values, stack/callee-saved/FPU state, fractional
increments, catch-up, paused increments, same-value rebases, inactive-native
behavior and non-target numeric reads. These slices test store dispatch;
they do not execute the full enclosing state machines or network/replay code.
`tests/native_visual_time.c` additionally checks skipped-update/view sampling,
resets across a whole-tick carry, stale-positive-whole pause, mismatched or
unknown writes, signed-clock wrap and invalid-delta fallback. Accumulating
float deltas with a double offset has less than 0.006 tick drift over the
synthetic one-hour 144-Hz case (under 0.1 ms). That is a model measurement,
not a wall-clock runtime measurement.

All 146 shared timing, float/vehicle-join, CI-scope and backlog checks pass on
Windows x64/MSVC (`logs/native-visual-time-shared.log`, 71.87 seconds).
The store slice and distortion fixture are registered in the existing shared
native timing suite. All 21 patch-upgrade checks pass (399.99 seconds), and
the regenerated game contains exactly nine direct clock-store hooks and one
scoped distortion read. The generation fingerprint is current. Runtime source
is committed locally as `ae410c4`; the supported Windows Release build and
built-in checks pass, with matching crash symbols. The physical-controller
routing subtest skips because foreground activation was refused. Executable
SHA-256: `2c77a2aa320732d9a5b492c68c169d8104c446f5437cfc53626dbc9524dcbaab`.

The 120-second native-144 Story replay passes movement, hand animation,
firearm handoff, ammunition use, pause/resume and fresh guest-only presents.
The reviewed final capture shows the canyon, firing weapon and HUD. Its
7,846 active CPU intervals over 56.7008 seconds average 138.375 updates/s:
p50 7.056 ms, p95 8.8298 ms, p99 10.1903 ms, worst 34.8258 ms, game/wall and
raw/wall ratios 1.0. The older interactive session was still running during
this test. These are concurrent-load CPU measurements, not isolated FPS
improvement, stable-144, monitor scanout or live-distortion acceptance.

The subsequent original-60 replay reached Story but closed around 60 seconds,
before pause/resume and the scripted movement/fire checks. Its log records
`window closed by the user`, exit 0, and no runner termination. Validation
correctly reports incomplete/failed acceptance; this is not a passing
original-60 comparison or evidence of a crash. The older interactive PID
32624 also ended with a logged user window close. No window was relaunched.
The temporary original image was removed only after both its PID was absent
and the new native replay passed, with both image hashes checked. Personal
controls/display hashes stayed unchanged. Final captures and validation
records remain; redundant captures and disposable test disks were recycled.

Evidence: `logs/native-visual-time-{shared,upgrades,generated,build,live,original60}.log`,
the focused store/distortion logs, `private/pipeline/visual-time-build-handoff.json`,
and `private/test-saves/high-refresh/{native-144-,60-}visual-time-*` records.
Actual Mac execution, live distortion visuals, complete original-60 comparison,
full replay/network/level lifecycle behavior and world texture animation remain
unverified; this correction does not close the full high-refresh goal.

## Procedural-texture cache cadence

The original surface updater `sub_00127500` advances two wrapped texture
offsets at record `+0xB0/+0xB4`, velocities at `+0xA8/+0xAC` and another
phase at `+0xB8`. The downstream surface routine `sub_00127830` consumes
those offsets while constructing coordinate data, including at `0x127BD2`,
`0x127BF5`, `0x127D88` and `0x127DB1`. The float-delta offset integration
already advances on fractional intervals in the controlled zero-force/
zero-drag fixture. This does not establish all surface/material behaviors.

The same updater invokes procedural-texture dispatcher `sub_001956A0` using
a resource pointer at `+0xBC`. It compares a per-view timestamp against console
vblank `0x7497A8`, calls the dispatcher, then stores that vblank into the cache.
The dispatcher immediately returns when whole game ticks are zero. At native
rates a zero-whole frame can therefore populate the cache on a new vblank;
the following positive-whole frame shares that vblank and skips real work.
The two clocks have independent fractional phase, so this is not just an
incorrectly chosen FPS constant.

`tests/native_surface_cache.c` executes the complete pinned US caller
(`0x127500..0x12782C`) and dispatcher (`0x1956A0..0x19576D`). Two controlled
material records alias one procedural resource. Its graphics services and
five nested texture-pass bodies are spies; the outer call sequence and two
resource swaps execute original instructions. The fixture checks the full
sequence and restored handles, callee-saved registers/stack/FPU depth,
unchanged timing globals and integrated U/V/phase values. It does not execute
the nested GPU kernels, produce a procedural image, or prove material loading.

Over eight game seconds at native 144, the original cache produces respectively
480, 192, **0**, and 192 updates for display-clock phase offsets of 0, 1/4,
1/2 and 3/4 tick. At native 120, the latter two offsets also suppress all
updates. Native 60 has no such collision in these sequences. These controlled
phase offsets reproduce a timing failure; they are not measurements of its
frequency in live levels.

The shared numeric policy now maps only the two vblank loads at `0x12764C`
and `0x127694` to native frame/update counter `0x7497B8`. Every advancing
native update gets a fresh cache key, while aliases within a view still
deduplicate. The procedural callee keeps its authored whole-tick gate and
positive-interval pass sequence, including its original one-dispatch
catch-up behavior. Its load at `0x1956A0` now reads zero during a native
zero-float pause, preventing stale positive whole counts from evolving the
image. That register is replaced by a graphics-state return value before
subsequent use. Native-disabled behavior and neighboring loads stay original.

The fixed fixture restores 480 updates in each normal/quarter-speed
60/120/144 sequence across all four phase offsets and view slots 0 and 3.
It checks every individual frame, not just the final count. Additional cases
cover a second view's independent stamp, shared-resource aliases, ordinary
pause/resume, the stale-whole paused-image counterexample and multi-tick
catch-up. U/V/phase values retain their original constant-velocity integration.
The procedural GPU algorithm is still discrete; evaluating its image at
fractional intervals requires auditing its nested passes and coefficients.
This cache correction alone does not establish native-144 procedural simulation
or complete texture fluidity.

Evidence: `logs/native-surface-cache-before.log` reproduces the failure;
`logs/native-surface-cache-fixed.log` records the correction. The fixture is
registered in the existing shared native timing suite. All 147 shared timing,
float/vehicle-join, CI-scope and backlog checks pass in 61.92 seconds
(`logs/native-surface-cache-shared.log`). The unchanged complete patch stack
verifies with zero updated files (`logs/native-surface-cache-patches.log`);
no new patch or generator change is needed for these existing load-policy sites.
Windows source checkpoint `b3c30cf` builds successfully through the supported
MSVC x64 Release workflow and passes its built-in checks
(`logs/native-surface-cache-build.log` / `.err`). Executable SHA-256:
`3b73d1223ffc53841fb3431ccc3caf79dd4afd8829c576687151fcc9f2e73f1e`.
Personal controls and display files match their pre-build hashes. The user
requested an interactive launch of this build at native target 144; PID 22536
was responding, using the normal personal save route. Startup is not gameplay
or texture visual acceptance. The previous original-60 comparison remains
incomplete; actual Mac execution and live surface/texture acceptance are open.

The lifecycle extension executes original resource lookup/allocation
`0x127400..0x127494` and initialization `0x194B00..0x194BB0` as well. Five
controlled pool slots start with poisoned stamps. New allocation sets all six
stamp fields to `-1`; requesting an existing resource preserves its stamps,
and an exhausted pool returns null. Recycling one slot at the same native
update ID invalidates the previous resource's stamps and permits a fresh
update. Four player views each dispatch once, with duplicate calls suppressed.
Original initialization sets the resource tables, handles and image state;
render-target binding and the two 128-by-128 clears are argument-checking
spies, not GPU work. The focused fixture passes in 1.27 seconds
(`logs/native-surface-cache-lifecycle.log`). This tests allocator behavior with
a controlled recycled slot, not a complete level-unload/load sequence.

The nearby clock reads in `sub_00195810` select strings using shifted vblank
bits; they do not access these resource stamps and remain unchanged. A direct
call scan finds the procedural dispatcher only at `0x127684`; that does not
exclude indirect callers. The surface construction routine's broader material
and GPU behavior remains outside this fixture.

## Fractional strip distortion

The separate screen effect `sub_000F9B10` samples whole level time at `0xF9B4B`,
divides by the original ticks-per-second value and uses two sine waves to
offset screen-copy source coordinates. It emits two-row strips through
`sub_0001F090`; destination coordinates, row stepping and tint remain original.
The absolute-time read is continuous visual phase, not an event counter.

`tests/native_strip_distortion.c` executes the entire pinned US routine
(`0xF9B10..0xF9CAF`) and the original sine-table generator/FSIN helper. Graphics
services are spies that capture every nine-word copy packet. In a controlled
640-by-448 region the routine emits 224 strips. The fixture compares original
whole time, exact rational fractional time, and production visual-clock
sampling through the existing direct-store observer.

Over eight game seconds at normal speed, original native 120 repeats 480 of
960 frames, and native 144 repeats 672 of 1,152. The correction repeats zero
in both sequences. At quarter speed, original 60/120/144 repeat respectively
1,440/3,360/4,128 frames, versus zero with the correction. These are captured
draw-argument counts, not live frame-pacing or GPU measurements. Original
whole-tick packets match exactly at every common boundary. Native 60 normal
speed remains identical to original 60.

Every production frame is compared with the exact fractional reference.
All normal-speed cases and quarter-speed 60/120 match byte-for-byte. Five of
4,608 quarter-speed 144 frames differ by at most one source-coordinate pixel:
accumulated float deltas differ from rational time by at most 0.000031 ticks
at those samples, crossing a lookup-table/integer-pixel boundary. Destination
coordinates, extents and tint stay exact. The assertion explicitly bounds
this rounding difference rather than calling the outputs identical.

Additional cases check repeated paused draws, explicit clock rebases, four
views sampling without advancing time, clipped X/Y ranges with odd/even
heights, empty-range early-outs, neighboring unmodified sites and the original
native-disabled path. The only runtime change adds `0xF9B4B` to the existing
site-specific fractional visual sampling policy in `pc_native_numeric.h`.
Guest clocks and every other read remain unchanged; no new patch is needed.

Evidence: `logs/native-strip-before.log` records the original production-policy
failure at 120; `logs/native-strip-fixed.log` records all six corrected cases.
All 148 shared timing/float/vehicle/CI-scope/backlog checks pass in 70.07 seconds
(`logs/native-strip-shared.log`). The unchanged complete patch stack verifies
with zero updates (`logs/native-strip-patches.log`), and the generated caller
contains the site-aware conversion at `0xF9B4B` with a current generation
fingerprint. The full Windows MSVC x64 Release build of `a08f41a` and its
built-in checks pass (`logs/native-strip-build.log` / `.err`). Executable SHA-256:
`e4082f51b9d4fb9ee595f7eb06db8982973bb6bcfab4df8f60bb3dababc4e163`.

The user's existing PID 22536 remains on `b3c30cf`, retained as
`futureperfect_recomp.running-b3c30cf.exe` while the normal executable was
rebuilt. It was not terminated or restarted. Personal controls/display hashes
remain unchanged. Live strip-effect triggering, actual GPU visuals, complete
original-60 gameplay comparison and Mac execution remain pending. This does
not establish full-game native 144 FPS coverage.

A separate read-only observation of that older interactive session found
`attract.xmv` playback in the log while the guest update counter stayed at
18,092, console vblank at 38,742 and level clock at 7,550 for six seconds in
scene 101. RHI present counts continued growing. Those counters represent
accepted API submissions, including repeated movie pictures, not fresh game
updates or decoded movie frames. The shared recorder already excludes positive
skip statuses and negative failures. Movie-loop pacing and fresh-picture
accounting require a separate audit; this observation was made during a build
and is not a performance benchmark. Evidence is
`private/pipeline/strip-interactive-counter-audit.json` and the corresponding
`logs/play-20261020-180012.err` movie/presentation records.

## Native movie polling

The live `b3c30cf` attract-movie observation above exposed a separate loop:
native mode bypasses `xbox_Nv2aFlipGateArm()` at guest swaps, but movie playback
does not call the level-update pacer. `xmv_play_update` frequently returns
NOFRAME while the guest keeps submitting the same movie layer. Consequently
accepted present calls can increase far faster than decoded pictures or guest
updates. This is a pacing gap, not evidence of fresh high-refresh gameplay.

`src/pc_native_movie_pacing.h` adds a per-playback rational schedule using the
existing `PcNativeClock`, native-rate setting and platform clock/wait interface.
The first poll is immediate; subsequent polls wait until the next native
60/120/144 slot. Already-paced callers do not wait twice. Late work skips missed
slots without an immediate catch-up burst. Rate/clock changes rebase the poll
schedule, timer failures fall back to a short sleep, and playback close releases
the timer. The guest clock, video timestamps and movie wall clock are untouched.

The additive `*-native-movie-pacing.patch` places the wait in the existing host
player after handle validation and before processing its next poll. It stores
the pacing state in that player's movie object and closes it with playback.
Disabled native mode returns immediately. The existing host decoder is only
available on Windows; the Mac player remains a stub under MAC-009. The pacing
helper is shared and should be consumed by that backend when implemented.

`tests/native_movie_pacing.c` checks exact rational cadence at 60/120/144 with
two counter frequencies, independent playback schedules, already-paced calls,
late work, reset/disable, timer create/set/wait failures and timer lifetime.
`tests/native_movie_update.c` compiles the actual patched movie structure and
get/start/update/time/close/audio-feed bodies. Demux, decoder, audio output and
rendering are synthetic backends; the fixture does not decode a real file.
Its ninety 20-FPS packet timestamps and 2,880 PCM bytes are identical with and
without pacing, including rejected audio submissions retried without loss.
At a modeled 100 microseconds of guest work per poll, native 144 reduces calls
from 44,502 to 643. EOF is observed at 4,458 rather than 4,450 ms, within one
poll interval after the final picture; timestamp values are unchanged.
Invalid/closed handles do not wait, and close releases the timer exactly once.

Evidence: `logs/native-movie-pacing-model.log` and
`logs/native-movie-update.log`. These checks establish scheduling and update
protocol behavior, not decoded image quality, audible synchronization, real
CPU savings or human skip latency.

Source `3c2fcc2` passes all 152 shared timing/float/vehicle/CI-scope/backlog and
existing movie-stop checks in 76.77 seconds (`logs/native-movie-shared.log`),
plus all 21 complete patch-upgrade checks in 351.81 seconds
(`logs/native-movie-patch-upgrades.log`). The supported Windows MSVC x64 Release
build and built-in checks pass (`logs/native-movie-build.log` / `.err`).
Executable SHA-256:
`16ddf892fc209737f682345c5d7203945bc531a52dadb471e9f231c180c753f7`.
The interactive PID 22536 remains on its retained `b3c30cf` executable; no
restart or game input was performed for this validation. Personal controls and
display hashes match the pre-build record. A redundant 274,829-byte standalone
compiler folder remains at `build/path-tests/native-movie-file`: automatic
approval review blocked its cleanup; no alternate deletion was attempted.

`tests/test_native_movie_file.py` additionally compiles the complete Windows
host player plus the real XMV demuxer, FFmpeg decoder and ADPCM helper against
the local verified dependency. It reads `private/game/xmv/eag_e.xmv`, with a
controlled host clock/wait implementation and mocked GPU/audio-device output.
It runs windowlessly, does not touch saves/settings, and explicitly skips on
hosts without the Windows backend or local data/dependency. The shared CI
workflow includes this optional test; a skip does not establish real decoding.
The local test passes in 2.14 seconds (`logs/native-movie-real-file.log`).

Original 60-Hz polling and native 60/120/144 all emit 88 pictures with identical
full BGRA sequence hash `ab1283f013e63d41`, timestamp sequence hash
`6e7e769a7ca873f5`, and 511,912 PCM bytes with hash `84ca1802fb1feb6d`.
Poll counts are 177/177/351/420, and modeled EOF observation times are
2,933/2,933/2,916/2,909 ms respectively. The last picture and following EOF are
observed at polling boundaries; picture timestamps themselves remain exact.
This compares actual decoded content and update protocol, not real-time CPU
usage, OS timer accuracy, audible synchronization, on-screen presentation,
skip/input latency, attract-movie completion or full cinematic coverage.

The file fixture now also builds with `LIVE_HOST_CLOCK`, using real Windows
QueryPerformanceCounter and waitable timers without virtual-time overrides.
All native 60/120/144 runs emit the same 88 pictures, full image/timestamp
hashes and 511,912 PCM bytes as the controlled original-60 reference above.
After close, the saved timer handle fails `GetHandleInformation` with
`ERROR_INVALID_HANDLE`, and the player's timer/state fields are cleared.
No GPU window or audio device is opened; the user's interactive game remains
running on `b3c30cf` throughout. The two focused tests pass in 12.24 seconds
(`logs/native-movie-windows-timers.log`).

| Native target | Poll intervals | Mean (ms) | p50 (ms) | p95 (ms) | p99 (ms) | Max (ms) |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 60 | 176 | 16.6547 | 16.6287 | 19.1922 | 19.5307 | 20.0750 |
| 120 | 350 | 8.3262 | 8.5138 | 10.6537 | 10.9167 | 11.0859 |
| 144 | 419 | 6.9374 | 7.0187 | 9.2085 | 9.5942 | 9.9869 |

These are intervals between completed player polls, including actual decode
and full-frame test hashing, measured alongside the older interactive session.
They demonstrate bounded host polling and successful OS-timer integration,
not isolated wake-up latency, GPU presentation, scanout or full-game frame
pacing. This follow-up changes fixtures/documentation only; the ready game
executable remains the validated `3c2fcc2` build.

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

### Beam construction and vertex output audit

Source baseline `b45a799`, Windows x64, checksum-verified supported US XBE.
`tests/native_beam_geometry.c` extends the lifetime investigation by executing
the complete original type-32 constructor (0x104F30), geometry update
(0x1053D0), renderer (0x141A0) and manager (0x10A760). The original renderer
writes positions, colors/alpha and texture coordinates into a captured guest
vertex buffer. Allocation, texture/render services, quality selection and
random draws are controlled. A host-generated sine table substitutes the
initialized guest trig service; original scalar constants and manager
descriptor/switch tables come from the verified XBE.

The 84 schedules cover original 60 and native 60/120/144 at normal/quarter
speed, two/four configured strands, both 8/16-segment quality paths, and random
values 0, 0.5 and 0.999. Each live update is followed by a zero-time update:
age, all 0xA00 bytes of geometry and visible-vertex counts remain unchanged.
The original vertex count/format, finite positions, register preservation,
x87 balance and unchanged timing globals are asserted.

Every schedule emits nonzero vertex alpha on its first advancing frame.
The last nonzero-alpha frame varies with the original random strand delay:
about 0.68–1.00 game seconds. Across rates and slow motion it agrees within
one original 60-Hz frame. Manager retirement stays within two update intervals
of its 1.2-game-second lifetime. Original/native 60 final geometry is
byte-identical, and random draws occur only during construction with identical
counts. This supplies evidence against a three-second onset delay inside
these type-32 routines under the tested inputs; it does not identify the
effect in the user's screenshot or justify a production timing change.

The renderer is called directly in this fixture. The surrounding 0x10ADF0
dispatcher also requires scene visibility flags, zero deferred-start time
and the current player's visibility mask before calling it at 0x10BFBA.
Those visibility decisions and GPU submission/resource lifetime remain
outside this test. Other effect types and full firing-to-display behavior
still need reproduction. Actual Mac execution is unverified.

A read-only snapshot of interactive PID 25304 found zero entity/effect list
heads while its timing continued to advance. It did not capture a shot or
trail and does not prove gameplay correctness. The game was left running
on build `fc3a7ce`; no input, memory writes, restart or personal-settings
changes were performed. The separate NPC aim source fix remains unbuilt.

The focused geometry run passes (`logs/native-beam-geometry.log`). With the
additional cross-rate fade and exact native-60 assertions, timing,
recompiler float/vehicle, CI-scope and backlog checks pass 133 tests with the
existing object-rocking expected failure in 57.74 seconds
(`logs/native-beam-geometry-shared.log`). The fixture is registered in the
existing `test_native_timing.py` CI suite. No runtime code changed, so this
audit did not require replacing the running executable.

### Effect visibility and reproduction tracing

Source baseline `84b0157`, Windows x64, supported checksum-verified US XBE.
The beam fixture now also translates 0x10BF5C..0x10BFC9, the entire type-32
list branch in the original render dispatcher. Only its outgoing join into
the next unrelated effect list is replaced with a return, after checking
the original bytes. The branch receives the original current-player bit and
player-index field; all 84 construction/geometry schedules now reach vertex
output through that branch.

Six additional 60/120/144 normal/quarter-speed schedules exercise visibility.
The original manager reads controlled active-player/room-visibility inputs
and builds the effect's visibility flags. A beam hidden for 0.25 game seconds
becomes drawable on the next live fractional update when that room becomes
visible. The prepared flag, explicit-hidden flag, player view mask and room
visibility mask independently suppress rendering as intended. Zero-time
callbacks can refresh visibility without advancing effect age. Explicit
0.3-game-second deferred starts reach vertex output at 0.300000–0.305556
game seconds (within one update); retirement leaves an empty list that the
original branch traverses without drawing. `logs/native-beam-visibility.log`
passes these cases. Upstream camera/room queries remain controlled, so this
does not prove the actual bridge scene's visibility.

Read-only inspection of the host path finds that `host_DrawPrimitiveUP`
copies vertex bytes into a deferred payload when that mode is active;
otherwise it submits to the device immediately. The swap path flushes deferred
operations before capture/presentation. The device appends vertex bytes to a
dynamic ring and calls `rhi_draw`. That inspection does not establish GPU
completion, resource correctness or visible output, and supplies no runtime
fix for the user's delayed-trail report.

`tools/native_effect_trace.py` now provides bounded read-only snapshots for
reproduction. It traverses all 40 effect types with a global 256-node default
limit, rejects invalid pointers, type changes, cycles and unreadable/changed
heads, and marks traversal completeness separately from atomicity. Records
include age/lifetime/delay, position, flags, view mask, and optional type-32
colors/endpoint/strand delays. Its type-32 eligibility result describes only
the original dispatch gate, not a GPU draw. Missing player context yields
unknown eligibility; unreadable optional geometry is explicit. Nonfinite
floats become null rather than invalid JSON numbers.

The existing isolated Windows runner accepts `--trace-effects` and samples
its owned child approximately every 0.5 seconds during Story mode, with a
16 MiB output limit. It retains `<tag>-effects.json` and the tracer hash,
sample count and limit status in validation metadata. Short effects can fall
between samples and recycled addresses cannot establish continuous identity.
The new tests cover visibility metadata, missing/short reads, list mutation,
limits and optional payload failures and are registered in shared CI.
The runner option has passed its help/import check; a complete isolated
gameplay run with this option is still pending. Actual Mac execution remains
unverified. No production runtime code changed in this audit.

The targeted trace/CI checks pass 70 tests. The final timing, effect/entity
trace, CI-scope and backlog suite passes 101 tests with the existing rocking
expected failure in 36.45 seconds (`logs/native-effect-dispatch-shared.log`).
A 30-second read-only observation of interactive PID 25304 collected 299
snapshots / 69,818 bytes with no traversal faults. All were outside Story
(state 101) with empty effect heads, so this validates the read path but does
not reproduce the reported shot/trail. Evidence is retained in
`private/pipeline/effects-dispatch-observation.jsonl` and its `.json` summary.
The interactive process and its executable were left untouched.

### Second projectile updater coupled timing

This subsection records the pre-fix audit at `bf6b5d8`; the source correction
and its validation follow below.

Source baseline `38c3f4c`, Windows x64, supported checksum-verified US XBE.
`tests/native_projectile_coupled.c` translates the complete original
0xF08D0..0xF1A4B updater, kind-35 initializer 0xE78F0..0xE799A,
target sampler 0xE1FA0..0xE2370 and steering 0xE2370..0xE2514.
The original switch table and scalar constants come from the XBE. Collision
queries return no impact; line-of-sight queries accept a controlled target.
Effect allocation/position updates and deterministic RNG are captured
services. This executes their original call sites and arithmetic, but does
not render effects or apply damage.

The initializer allocates four effects through 0x101660 with configuration
IDs 83, 83, 84 and 85, assigns sizes 45/40/37/35, and clears the
target sampling counter. The updater's gate at 0xF08D0 rejects zero-whole
ticks although displacement uses live float delta at 0xF0D45. The
non-homing path also applies the original 1.02 speed multiplier in the
whole-tick loop at 0xF1513/0xF1544. This is acceleration, not damping.
For kind 35, the effect positioning code at 0xF1735..0xF1952 makes nine RNG
draws and updates four effect positions per accepted callback. With homing
flag 0x2000, the target helper decrements integer +0x1AC once per callback,
refreshes line of sight when negative and resets it to 10; steering adds a
fixed per-callback contribution before normalizing the direction.

`logs/native-projectile-coupled-audit.log` executes 16 one-game-second
schedules: original 60, native 60/120/144 at normal/quarter speed, and a
native-144 counterfactual that opens only the gate, for both homing modes.
Original/native 60 shot bytes are identical. Zero-time callbacks between all
live updates preserve projectile/effect bytes and all callback/RNG counts.
Guest registers, stack/x87 balance and timing globals are checked.

| Normal-speed case | X position after one game second | Fresh movement frames | Target checks | RNG draws | Effect position updates |
| --- | ---: | ---: | ---: | ---: | ---: |
| Original 60, unsteered | 456.205841 | 60/60 | 0 | 540 | 240 |
| Current native 144, unsteered | 190.085800 | 60/144 | 0 | 540 | 240 |
| Gate-only native 144, unsteered | 454.700317 | 144/144 | 0 | 1296 | 576 |
| Original 60, homing | 215.687500 | 60/60 | 6 | 540 | 240 |
| Current native 144, homing | 89.950760 | 60/144 | 6 | 540 | 240 |
| Gate-only native 144, homing | 215.079788 | 144/144 | 14 | 1296 | 576 |

The homing Y positions are 104.512825, 43.384315 and 106.175316 respectively.
Unsteered terminal speed remains 787.446533 in these schedules; homing speed
remains 240. Quarter-speed native 144 loses still more motion: X is
47.521450 unsteered and 22.498043 homing over the same game-time duration,
with only 60 fresh movement frames out of 576.

The new strict `ProjectileCoupledTiming` expected failure applies only after
the complete fixture runs successfully. Compilation, missing-service,
constant-table, ABI or fixture failures remain ordinary failures. Acceptance
requires fresh movement and effect positioning each advancing frame,
original RNG and target-check counts, matching terminal speed, and a final
position within one original frame's travel of the original 60 reference.
This is recorded missing behavior, not a passing native-timing claim.

Opening this gate alone is not a valid fix. The correction needs to separate
continuous travel/steering/effect anchoring from target-sampling and random
decisions, preserving initialization and recycled projectile identity.
The nine random trail values must not consume the shared RNG 2.4 times as
often, and the target counter must retain its game-time cadence. Other kinds
sharing the updater, collision/retirement branches, multi-tick catch-up and
distance-based damage falloff also need coverage before broader changes.
No production code changed, no game rebuild/restart was performed, and the
user's screenshot has not been identified as this projectile/effect type.
Actual Mac execution remains unverified.

The timing, CI-scope and backlog suite completes with 96 passes and two
strict expected failures: this newly measured coupling defect and the
existing object-rocking quantization (`logs/native-projectile-coupled-shared.log`,
40.17 seconds). A final focused run also verifies all four original effect
configuration IDs, owner slots and sizes before reproducing the timing
failure; the backlog format check passes. The test is in the existing shared
timing CI suite. These results do not mark the runtime defect resolved.


### Kind-35 projectile motion and trail correction

The shared kind-35 wrapper now enables 0xF08D0 on positive fractional time,
scales its homing contribution at 0xE237A by the live step, and updates all
four original trail anchors each advancing frame. The 1.02 acceleration
loop retains carried whole ticks. Target sampling retains the original
whole-update countdown. A newborn homing shot acquires a usable target on
its first fractional update, then skips the duplicate countdown at the next
whole boundary. The original target-vector calculation still runs each frame.

Nine random trail values are cached separately by their original call sites
and original projectile-pool slot. Fraction-only callbacks reuse these values
at fresh positions; they do not consume the shared RNG. Missing emitters do
not shift the other coordinates. Initialization resets the slot even with
native timing disabled. The pool bound is derived from the original 0xE8EB0
allocation: 0xB8A0 bytes with 0x22C stride, or 85 complete slots. Unknown slots
fall back to original behavior. Nested updater calls save/restore the active
scope; other projectile kinds and native-off retain the original policy.

The complete original-code fixture now passes its former strict expected
failure. At native 144, normal-speed unsteered X is 454.700317 after one game
second versus original 456.205841 (before fix 190.085800). Homing X/Y is
215.639862/104.503189 versus original 215.687500/104.512825. Every advancing
frame produces fresh movement and effect positions. All tested rates retain
540 trail RNG draws and six target checks per game second. Quarter speed
also passes at 60/120/144; native/original 60 normal-speed projectile bytes
remain identical. Initial fractional jitter is neutral until its first
whole-tick random sample, avoiding extra random draws at birth.

Additional checks execute the original collision and expiry branches against
a controlled X=100 plane and controlled allocation/sound/damage services.
Each impact dispatches one damage callback, creates one impact object and
cleans up all four attached effects; expiry retires once without an impact
hit callback. Arrival and expiry times differ from original-step timing by
less than one 60 Hz frame. This verifies dispatch, not actual health loss,
scene geometry or rendered impact effects. Pause checks include zero float
time with a stale whole-tick value of one. Missing-emitter tests verify other
anchors keep their own cached jitter. An uneven .125/.875/2/.25/.75/6-tick
schedule verifies integrated motion, the original acceleration loop and one
random set per accepted whole update. The separate sidecar fixture checks
interleaved and recycled slots, native-off initialization, unknown slots,
pool changes, nested unrelated kinds and uncached impact random calls.

This change is deliberately limited to kind 35. The user's red trails have
not yet been identified as this projectile/effect. Original distance
accumulation at +0xF4, its use by damage falloff, moving/occluded targets,
other updater kinds and full mission behavior remain open. Windows linkage
and an isolated live smoke check are pending; the user's interactive
fc3a7ce executable is left running unchanged. Actual Mac execution is pending;
the implementation and tests use shared source with no OS-specific gameplay
adapter. The wider native-144 goal remains active.


Validation on Windows/MSVC: the final shared timing/CI/backlog/entity/effect
suite passed **103 tests with one expected object-rocking failure**
(`logs/native-projectile-final-shared.log`, 44.94 s). The initial combined
timing and full patch-transition suite passed **52 tests with that same
expected failure** (`logs/native-projectile-coupled-shared.log`, 400.12 s).
Focused projectile integration and scope checks pass separately. The supported
`tsfp.ps1 Generate` command completed successfully, both manual wrappers appear
in the generated declarations, and the generator fingerprint is current
(`logs/native-projectile-generate.log`). The generation/backlog contract checks
passed two tests (`logs/native-projectile-generation-checks.log`). These are source,
translation and fixture results; they do not claim a new linked executable.


### Projectile distance accumulation and damage dispatch

At source baseline `5ece0b9`, the existing saved read-only capture
`private/pipeline/npc-projectile-observation.jsonl` contains 10,148,608 bytes
(SHA-256 `06fd0a428238d8ebfe0e118e360b905ddb408efae14d3c3a635faef7e8e5fe52`,
last modified 2026-10-20 13:25:49 local). Decoding each projectile's kind at
+0, damage-profile index at +8 and distance accumulator at +0xF4 gives:

| Kind | Profile | Updater | Observations |
| --- | --- | --- | ---: |
| 28 | 2 | 0xF4A50 | 47 |
| 31 | 8 | 0xF4A50 | 375 |
| 35 | -1 | 0xF08D0 | 56 |
| 45 | 2 | 0xF4A50 | 82 |
| 46 | 2 | 0xF4A50 | 200 |
| 24 | 9 | 0xF5FF0 | 3 |
| 23 | 0 | 0xEFF90 | 40 |

These are repeated observations, not unique projectiles or proven weapon
identities. The old capture is not evidence about the newly corrected build.
It does identify realistic kind/profile combinations for the original-code
fixtures, including kind 46 that the earlier movement test omitted.

Both moving-projectile fixtures now execute the complete original collision
record path 0xDCEF0, profile helper 0xB9920 and falloff helper 0xB9720. The
controlled scene hit populates the original updater's hit record; the original
caller forwards it to those routines before their final service boundary
0x12A170 is captured. Actual kind flags and profile values are loaded from the
checksum-verified US XBE. Kinds 28/31/45/46 have flag 4 in their descriptor
(0x9C) and bypass the falloff computation, forwarding scalar 1 at every tested
rate. For the captured kind-35 profile -1, the original helper forwards 0 at
every rate without evaluating an exponent. This is one collision-record
scalar, not the projectile's total damage: the original kind-35 impact still
creates its impact object and follows its separate impact callback.

The +0xF4 accumulator remains frame dependent. For example, the controlled
kind-28 impact has approximately 405 at 60 FPS and 202.5 at 120 FPS, while the
forwarded scalar remains exactly 1. Consequently, these captured paths do not
justify changing the accumulator globally or blaming it for missing sniper
health loss. The sniper's separate original damage/health fixture remains
necessary; it is not replaced by these projectile checks.

Fifteen additional caller-contract cases deliberately use kind 35 with
profiles -1, 2, 9, 11 and 12 and accumulator values 0, 16 and 4096. Positive
flat profiles preserve their table multiplier; non-flat profiles exercise the
actual falloff branch. The CRT pow adapter is replaced by host pow with its
original x87 argument/return convention; the original adapter bytes are
verified before translation. For profile 12 the scalar is 1, 0.732094169 and
0.006808810 respectively. Profiles 11/12 with kind 35 are controls, not observed
live combinations. This demonstrates the remaining conditional sensitivity
without inventing a confirmed live damage regression. Mapping all legal
profile/kind combinations, other accumulator consumers, target health and
full collision geometry remain open.

The movement fixture adds kind 46 to its normal/quarter-speed 60/120/144
travel, impact and expiry schedules. Captured profiles verify one collision
record, the expected scalar, one impact callback and one retirement/cleanup.
Focused projectile checks pass (`logs/native-projectile-falloff-focused.log`).
No gameplay code changed in this audit. The same portable original-code
fixtures run through the existing shared timing test entry; actual Mac
execution remains unverified.


The final timing/CI/backlog suite passed **98 tests with one expected
object-rocking failure** (`logs/native-projectile-falloff-shared.log`,
148.32 s while production compilation was active). The focused projectile
run passed three tests (`logs/native-projectile-falloff-focused.log`, 29.55 s),
and the final backlog format check passed. Original-code tests require the
pinned local XBE; these results do not imply an asset-free CI run executed it.

Production Windows compilation of the `5ece0b9` gameplay source completed
successfully using the existing Release project with MSBuild 17.14.40 and
MSVC 14.44.35207:

```
MSBuild.exe build/windows/futureperfect_recomp.vcxproj /t:ClCompile /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false /m:2 /v:minimal
```

This compiled the manual runtime wrappers and all generated game units
without linking/replacing the executable. The log
`logs/native-projectile-compile-only.log` contains warning MSB8028 about reused
intermediate project files, but no compilation errors. Full linking and the
supported `tsfp.ps1 Build` regression checks still remain. The interactive
PID 25304 remained responsive and its executable SHA-256 stayed
`912f416b5f6eaeabf545c960c255491907988eae9f031ead4231b24832cbc333`.
Thus the NPC aiming and kind-35 corrections have production compilation
evidence, while the user's running fc3a7ce game does not yet include them.
