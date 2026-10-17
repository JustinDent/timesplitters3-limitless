# Issue #25: network bots investigation

Status: an experimental host editor and match-configuration prototype are
implemented locally. Fully synchronized Online/System Link bot gameplay is
**not established**; issue #25 is not complete.

## Test implementation

The host's lobby Options flow now exposes Arcade's original **BOT SET** row
below **WEAPON SET**, including its selector and edit callback. The original
editor retains bot sets, characters, teams, skill and Add/Remove behavior.
The temporary top-of-menu experimental button has been removed.
Host UI construction temporarily uses Arcade setup mode, restoring the real
network mode before input or network processing resumes. No transport flag is
changed. Entering host settings enables the roster adapter for that session.

`src/pc_network_bots.h` validates the editor's roster and combines it with the
active human IDs, preserving holes and the total sixteen-character limit.
`src/pc_network_bots_guest.inc` applies that configuration only on a PC network
host in the original network setup modes. Disabled, guest, offline, invalid,
empty and full-roster cases leave the original configuration unchanged.
`src/pc_display_menu.c` reuses the host-side Arcade menu construction. The separate
`pc-network-bots-test` patch adds reproducible wrappers; generated C is rebuilt
through the supported workflow.

This prototype does not implement a new AI replication protocol or establish
that the original network paths replicate local AI correctly. Bot movement,
weapons, damage, scoring, respawns and cross-peer agreement must be tested and
any missing replication implemented before treating this as working Online
bot support. See the remaining acceptance work below.

Investigated on Windows against source `33fc619`, branch
`25-add-bot-support-to-online-matches`. Addresses below apply only to the pinned
US title `45410066`, executable SHA-256
`3cfd001a84fc3e08175d6c4b2e42ebf49577a089b5d0bd87ac724f61a41816bc`.
The original executable was read and hash-checked, not modified.

## Verified original paths

- Online enters the original System Link frontend after configuring the bridge:
  `src/pc_display_menu.c::online_activate`, guest `sub_001A0A60`.
- `sub_003533F0` reads the network-active flag at `0x0076B108`.
- English localization IDs `0x36D`, `0x3B1`, `0x3B2`, and `0x3B8` are
  Setup Bots, Add Bot, Remove Bot, and Bot Set. The labels were read from an
  isolated running game's localization tables, rather than inferred from
  unrelated menu functions.
- `sub_002C2C20` constructs Setup Bots. Its child descriptors at `0x0052374C`,
  `0x0052373C`, and `0x0052372C` point to constructors `sub_002C1EA0`,
  `sub_002C2BC0`, and `sub_002C19F0` respectively.
- The editor maintains ten records of seven 32-bit words at
  `0x0075FD90..0x0075FEA8`. `sub_002C0310` populates these from a saved bot set;
  `sub_002C2CA0` and `sub_002C1370` inspect enabled entries.
- `sub_002D2C70` builds match configuration at `0x0078AD58`. Character, team,
  and skill arrays are written at `0x0078AD6C`, `0x0078AD7C`, and `0x0078AD8C`.
  The branch at `0x002D36C2` excludes modes 11, 12, and 13 from the local bot
  assembly loop. Its excluded path explicitly clears `0x0078AD5A` at
  `0x002D383F`. The local path instead fills the arrays and writes its bot count
  at `0x002D382F`.
- `sub_003568C0` checks network player activity using the 16 records rooted at
  `0x00777244`, stride `0x124`, including flags at record offset `0x48`.
- The existing movement sampler `sub_003588D0` iterates a peer's controller
  records. It reads a record index at offset `0x3C`, then addresses player
  state through `0x007B0C48 + index * 0x1584`. The existing
  `pc_network_guest_sample` wrapper retains that controller-oriented path.

These observations do not prove that enabling bot configuration produces
playable or synchronized bots. The initial prototype patched only the builder, which was insufficient:
`sub_002D38C0` clears byte `0x78AD5A` at `0x002D3947` after the builder
returns. This explains the reported empty match. The revised wrappers apply
the host roster BEFORE `sub_00356490` serializes it into `0x774044` and
BEFORE `sub_000658D0` copies it into pending load configuration `0x7DE340`.
Guests consume the host configuration without rebuilding their own roster.
The regression test reproduces the clear and checks bytes captured by the
original serializer, including register/stack preservation. Other original
network paths remain in place.

## Spawn and activity follow-up

The first repaired launch retained six bots in the wire, pending and live
configuration and allocated six actors (`0x7A2D00`), but all six actor positions
remained zero. Allocation is not evidence of visible, active bots.
`sub_003568C0` accepts only active network controller records. The revised
activity wrapper accepts validated AI slot indices only at the actor update
and HUD character-list call sites that already iterate humans plus AI. Controller
initialization (`0x0015C5A3`) and network serializers keep their original
predicate, avoiding null human-state access for a bot slot.
`sub_00358560` already gives host authority to AI indices; the wrapper preserves
that authority check. Replication still requires two-peer gameplay validation.

The original footer draw routine `sub_002C8370` independently excludes network
modes. It now uses the same scoped Arcade Custom UI construction/draw rules
as the Bot Set row. Bot editor callbacks restore the network mode before
returning, including when editing custom saved sets.

## Remaining implementation and acceptance work

1. Trace local bot spawn, AI ownership, and transforms through match loading;
   distinguish AI actor state from the network controller/player state above.
2. Establish how host-owned bot actors can be represented on guests, including
   movement, animation, weapons, shots, damage, death, respawn, teams and scores.
   Reuse original serializers only after verifying their actor/state assumptions.
3. Validate the implemented host editor/configuration hook and establish roster
   distribution with a match generation. Verify human joins, disconnects,
   match changes, and zero-bot behavior against the real game. Guests must not
   control the roster. Pure count/ID/role checks already have unit coverage.
4. Include protocol changes in compatibility identity so older builds cannot
   enter a bot-enabled match. Keep the directory and relay transport independent
   of gameplay AI; the same gameplay implementation should serve LAN and Online.
5. Validate two isolated processes entering the same match: identical roster,
   visible moving/firing bots on both, bidirectional human/bot damage, consistent
   deaths/respawns/scores, a rematch, and cleanup after a disconnect. Then test
   through the Online bridge. Unit tests and a lobby screenshot alone are not
   sufficient acceptance evidence.

Native Mac networking remains a separate deferred adaptation, tracked by
MAC-013 and MAC-028. Native wrappers keep their original behavior.

## Initial investigation scope

Two bounded Test Lab probes used disposable copies under
`private/test-saves/issue25`, with isolated controls and both display settings,
startup videos disabled, and capped logs. Each was terminated after localization
inspection and its known disposable partition images were recycled by the runner.
The existing executable's SHA-256 was
`455166dd4c885123e880e77934df9b58b1ff599973b0e3d40455db113ad9898c`;
its source revision was not independently established, so these are localization
observations, not validation of a build of the current branch. No host/guest bot
match or gameplay validation occurred. Personal saves/settings were unused;
the canonical Test Lab fixture was used only as the runner's copy source.

The subsequent prototype build and in-game editor verification are recorded
under issue #25 in `STATUS.md`, including the current executable checksum and
selected menu captures. Neither phase establishes cross-peer AI gameplay.

## Two-peer result after the activity fix

The bounded second run reached one shared, ready System Link lobby and loaded
both peers with identical `2 humans / 6 bots` broadcast, pending and live counts.
After the opening countdown, sampled host AI positions changed across the map.
Guest AI positions stayed `(0, 0, 0)` throughout the remaining gameplay window.
The shared roster and host activity fixes therefore do **not** complete issue
#25. Guest AI state replication must still be implemented; do not describe this
build as working synchronized LAN/Online bot multiplayer. See the follow-up in
`STATUS.md` for build identity, test logs and retained UI evidence.

## Follow-up: invisible actors and combat references

The next two-peer reproduction found allocated models on both peers, but their
entity flags lacked bit 0, unlike human bodies. Host AI positions changed while
guest positions remained zero. Bots also lacked both fields supported by the
original actor-reference encoder: the human-state pointer at entity + `0x150`
and the generic network object ID at + `0x230`.

The current uncommitted adapter enables living host AI bodies after their spawn
state and refreshes spatial membership. It adds actor-reference kind 4 with a
validated bot slot, preserving the original human/object encodings. A separate
20 Hz host state channel carries numeric transforms, actor/lifecycle flags,
current/maximum health and score fields. It never transfers guest pointers or model handles.
Guests refresh their own allocated bodies after applying these snapshots.

The original `sub_003593D0` eligibility predicate rejects bots before the
combat encoder is called. Its wrapper therefore accepts exactly the same
validated AI entities as the bot codec. Merely extending the encoder would
leave human attacks on bots rejected. Subsequent disassembly corrected the
initial health interpretation: live health is **entity + `0x1CC`**, armor is
`0x1D0`, the last damage tick is `0x1D4`, and maximum health is `0x1D8`.
Actor `0xB08`/`0xB0C` are health configuration values and cannot establish damage.

Snapshots use the existing version-framed UDP/Online bridge transport. Reception
checks the configured host, session key, match signature, slot bounds, finite
numeric values and increasing ticks before queuing a state. Extension packets
are consumed before the original game's receive-capacity check and decoder.
Leaving gameplay clears pending states; compatibility identity includes the
changed runtime sources. Native Mac retains its original wrappers.

These are implementation details, not acceptance evidence. Record visibility,
combat, respawn and synchronization outcomes separately in `STATUS.md`.

## Checkpoint: radar, collision, pose and guest body recreation

The radar actor loop also calls the controller-only activity predicate. Its
specific return address (`0x00133BC9`) now accepts allocated AI slots; controller
enumeration remains unchanged. Reviewed two-peer captures show bot radar markers
on both PCs.

Living bot bodies require flags `0x61`: drawing bit 0 alone leaves the original
hit-collision passes disabled. The adapter now restores all three flags after
spawn and replicates them to the guest, while preserving dead/spawn exclusions.
Snapshots carry actual live vitals, actor flags from `0x24`, body angles and the
animation clip/frame channels. Clip changes use the guest's own animation setter;
no model, pose-buffer or other guest pointer is transferred. Missing guest bot
bodies are recreated through the original Arcade constructor when an active,
living host snapshot arrives, without appending duplicate actor-list entries.

An optional `TSFP_BOT_TRACE` file records up to 256 original damage calls to bot
victims, including live health/armor before and after. Isolated input-state files
also accept `VK down sequence dx dy`; motion is consumed once per new sequence.
These diagnostic paths are opt-in and do not change normal input routing.

The latest bounded LAN run records guest-human shots reducing bot health to zero
on both peers, bot-versus-bot damage, and recreated guest bodies after deaths.
Host-human direct-hit attribution, full animation fidelity and guest weapon
visuals remain unverified or unfinished, as do rematches, disconnects and the
actual Online bridge. See the latest checkpoint in `STATUS.md`; this commit is
work in progress, not completion of issue #25.
