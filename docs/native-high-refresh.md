# Native high refresh development

The `feature/native-144-fps` branch adds an opt-in path that runs the title's
own update and rendering loop at a 60, 120 or 144 FPS target. Normal play keeps
main's fixed-60 default. The old interpolated presentation engine and Smooth
motion setting remain retired.

Build this branch with the normal `tsfp.ps1 -Action Build` workflow. On Windows,
open `Play-Native.cmd` for the 144 FPS target. `Play-Native.cmd 120` selects 120;
`Play-Native.cmd 60` provides a native timing comparison at the original rate.
These launch overrides do not rewrite saved graphics preferences. Normal
`Play.cmd` retains the existing behavior. This is a development checkout entry
point, not a launcher update for an installed release.

The FPS overlay shows `target 144` (or the selected native rate) alongside
measured FPS. Original timing still shows `cap 60`. Network RTT appears only
when a network mode is active; offline play hides the latency readout.

For a direct game launch on either host, set `TSFP_NATIVE_FPS=144` or put
`native_fps = 144` in its isolated runtime display configuration. Supported
values are 60, 120 and 144; absent, zero and invalid values retain original
timing. Legacy presentation settings retain main's fixed migration values.
Leave the console interrupt clock at 60 Hz. Do not use
`RECOMP_VBLANK_HZ=144` to enable this feature.

## Timing and motion

The original US timing routine waits for an integer console tick and clamps
the frame delta to at least one. Native timing replaces that wait with a
rational host deadline and measures elapsed time with the host performance
counter. It gives continuous movement fractional deltas in the original
60-Hz units and carries remainders for integer timers. Pause stops game time
while unscaled time advances; loading stalls retain the original maximum
three-tick catch-up. The console ISR, kernel counters and audio clocks keep
their original frequency.

The original frontend consumes button edges on whole ticks. Native mode keeps
button-state sampling at that cadence and clears consumed edges between those
ticks, preventing a press from disappearing before the frontend can consume it.
Raw mouse aiming and continuous rendering do not use that button gate.

The entity post-update also uses live fractional time when deriving velocity
from position changes. Its original whole-tick guard cleared the result on
fractional updates even after an entity moved. This changes that guard only;
collision response and remaining fixed-tick physics integrators still need
separate validation.

Weapon-sound requests also accept a live fractional interval. This prevents
the sound helper from dropping an already-issued event on a zero-whole-tick
frame, and lets its repeat timer consume fractional time when called every
update. The enclosing firing/state-machine gates still need separate audit;
this helper change does not establish complete weapon cadence or audio parity.

The audited generic firing routine retains its authored whole-tick projectile
gate. Its event timers and nested sound requests use that same whole-tick span
inside the call, then restore fractional time for other updates. Opening this
gate on every fractional frame would overproduce zero-interval projectiles.
This preserves discrete event cadence while continuous animation/movement
remain on fractional updates; other weapon caller families need their own audit.

The recompiler also supports an optional title numeric-conversion header.
Future Perfect uses it to read the adjacent fractional delta when an original
instruction converts an integer frame delta to floating point. Ordinary
integer counters retain whole ticks. Other addresses and the disabled mode
keep their original conversion. A frozen animation-pose scope restores the
incoming fractional delta rather than rounding it through an integer.

The US level-script dispatcher's pause gate now tests whether fractional game
time advanced. Previously it skipped zero-whole-tick frames even though an
animation had advanced; the next animation update could overwrite an event
crossing before the script saw it. NPC and player dispatchers' similar gates
discarded fractional movement and elapsed time. A site-specific global-load
policy changes those gates at `0x228880`, `0x15EB0C` and `0xD5EBF`. The integer
delta in guest memory retains its original value. The
player's per-update damping also uses fractional elapsed time to retain its
original half-life. The original dispatchers and event readers are exercised
by `tests/native_script_events.c`, `tests/native_npc_dispatch.c` and
`tests/native_player_dispatch.c`.

Scripted camera translation and direction use the same live-time gate at
`0xC73C0` and `0xC7610`. Direction easing at `0xC7651` scales its original
per-update decay by elapsed game time, retaining its one-tick coefficient.
`tests/native_camera_motion.c` executes both original routines at 60/120/144,
including slow motion, pause, angular wrap and direct target snaps. Collision
services are mocked; complete camera modes and moving targets require scene
acceptance.

The camera-effect caller also runs on live fractional intervals. Its effect
timer and collision evaluation can therefore advance each native frame. The
old whole-tick kick-decay loop runs once with time-scaled factors, preserving
its one-tick result and cutoff threshold. `tests/native_camera_effects.c`
exercises the original effect routine, matrix helpers and caller gate, with
an unobstructed collision fixture. Real geometry and complete camera modes
remain separate acceptance work.

Camera mode 27 evaluates its authored track on each advancing fractional
interval. Its next sample must be at least one keyframe ahead even when the
whole-tick delta is zero; otherwise the existing blend repeatedly samples the
same pose. The native policy changes the gate at `0xCD70F`, that zero-span
case at `0xCD851`, and the divisor at `0xCD88E` to match the selected integer
span during fractional catch-up. Multi-tick spans, external track clocks, looping,
completion and large discontinuity cuts retain their original logic.

These changes target responsive input, consistent frame pacing and continuous
motion. They do not guarantee a performance increase: running more genuine
updates and draws costs CPU and GPU time. The renderer's normal scene remains
in use, including its effects and UI.

## Validation and remaining work

### Goal and acceptance criteria

The goal is a smooth, modern feel across the full game at a native 144 FPS
target, with correct gameplay speed. The user's expanded scope includes texture
fluidity, animation smoothness, and keeping previously addressed bugs resolved.
These requirements remain part of the active goal until verified:

- Animation: compare player hands, weapon equip/fire/reload, character movement,
  camera motion, effects and cinematics at original 60 FPS and native 60/120/144.
  They must progress smoothly, finish correctly and retain their original
  duration, event timing and interaction with pause/resume.
- Texture fluidity: check animated and scrolling textures, water, moving surface
  effects and texture transitions while moving the camera. Verify continuous
  motion at the correct speed, stable mip/filtering behavior, and existing
  texture replacement, alpha and fallback behavior. Record any visible hitching,
  flicker or shimmer and its cause. Artwork production remains on its existing
  texture branch; this goal covers runtime motion and rendering behavior.
- Previous fixes: build a regression checklist from the base revision's resolved
  issue/PR notes, existing regression tests and STATUS evidence. Link each
  affected fix to a test or a specific manual check, with source revision,
  result and remaining coverage gaps. Check input/menu navigation, camera,
  weapon behavior, timing, audio, display/aspect ratios, texture loading,
  settings/save preservation, networking and existing platform support.
  Run relevant checks with native mode enabled and with the original path.
  A historical passing result is not a current-branch verification.
- Completion: resolve observed regressions, preserve previous fixes, and record
  representative level/mode and visual comparisons plus measured frame pacing.
  An FPS counter, successful build, scene flag, or aggregate test count alone
  does not meet this goal. Unavailable host checks and untested gameplay remain
  explicit; do not claim all bugs are resolved without supporting evidence.

The opening timing regression required repairs to both script-event and NPC
dispatch gates. Repairing the script-event gate allowed native 144 to equip
the pistol and show the HUD/next objective, but the NPC still approached too
slowly. Correcting its movement gate restored walking speed to within 0.3%
of the original 60 FPS reference in the measured opening segment. The player
fix subsequently restores steady walking speed to within 0.05% of a
same-build original-60 reference, with matching sampled ammunition totals.
The latest bounded runs pass handoff, firing, hand-animation and pause/resume
checks. The unskipped cinematic also reaches the handoff on the prior NPC
build; exact natural-intro timing, other gameplay systems and full-game
progression remain unverified. See the
[regression record](native-high-refresh-regressions.md) for current evidence
and the prior-fix checklist.

Run `python -m pytest tests/test_native_timing.py` after applying the complete
patch stack. It checks elapsed-time integration at all three target rates,
integer timers, pause/resume, stalls, ABI stack balance, numeric conversions,
the original fallback, and fractional animation-time restoration.

The bounded Windows integration runner accepts:

```powershell
.venv\Scripts\python.exe tests/native_refresh_game.py --native --fps 144 --seconds 120 --stress --pause-check --windowed
```

It uses only disposable Test Lab data, caps its logs and recycles its virtual
disks. It records native update counts and game-clock ticks separately from
host presentation counts. A closed test window makes the run incomplete;
partial timing evidence is retained with that status. `--capture` keeps up to
24 renderer BMPs, with its final three slots rotating. The optional `TSFP_NATIVE_TRACE` CSV
contains at most 20,000 update timestamps and deltas. These measure CPU update
cadence, not GPU completion, monitor scanout or end-to-end input latency.

Measured Windows results and their source/build scope are in `STATUS.md`.
Native Mac build/runtime validation, additional levels and modes, vehicles,
recorded demos, multiplayer synchronization, full campaign timing, and
perceptual smoothness remain acceptance work. A passing first-mission test
does not establish native 144 FPS correctness across the full game. The mode
remains opt-in while that work is incomplete.

## NPC firing timing

The shared native timing path also scopes the NPC firing transaction
`sub_001BB5C0` to its carried whole-game-tick span, restoring fractional time
before returning. Its original projectile gate and ammunition operations
remain intact. This avoids losing event-timer and repeating-sound time on
the fractional updates skipped by that gate. See the
[NPC validation record](native-high-refresh-regressions.md#npc-firing-event-tick-scope)
for source/build status and the remaining caller/damage coverage.
