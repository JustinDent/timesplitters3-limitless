# PC features awaiting Mac adaptation

This is the handoff list for Windows PC features to bring to native Mac later.
Add or update an entry when implementing a new PC feature, before reporting the
PC work complete. Also record behavioral fixes that need Mac adaptation. Mac
work is deferred until requested; it does not block completion of the PC feature.

Keep one stable ID per feature (`MAC-001`, `MAC-002`, and so on). Update existing
entries rather than duplicating them, and retain completed entries as history.
Include commit/PR references when available; label local changes uncommitted
until those references exist. Keep paths relative to the repository.

## Queue

### MAC-001 — System Link and Online multiplayer

- **Status:** Queued
- **PC behavior:** System Link uses the native UDP/XNet bridge; Online offers
  host/join discovery and routes compatible peers through the directory bridge.
- **PC source:** Main through `16fcea5` (PRs #4 and #5);
  `src/pc_network.c`, `src/pc_network_guest.inc`, `src/pc_online.c`,
  `src/pc_display_menu.c`, `patches/zz-pc-network.patch`,
  `patches/zzz-pc-online.patch`, and `tools/online*.py`.
- **PC validation:** Main records local two-instance lobby/match loading in
  `docs/multiplayer.md`; cross-internet routing/full-match validation is pending.
- **Existing Mac support:** The shared menu/display and original guest XNet
  paths are retained. Windows host UI/transport calls are guarded during the
  main/mac-port conflict resolution; no native Mac multiplayer adapter is linked.
- **Mac adaptation needed:** Native socket/identity transport, directory host/join
  UI, launch/cleanup integration, and platform-appropriate dependencies.
- **Mac acceptance checks:** Build and launch on Apple Silicon; verify existing
  display/settings menus, then test isolated two-instance discovery, joining,
  gameplay, shutdown and user-setting preservation before cross-network tests.
- **Mac implementation and evidence:** Deferred. Conflict resolution preserves
  the Mac build boundary; host-independent dispatch and Windows checks are
  recorded in `STATUS.md`. Native Mac execution remains unverified for this merge.

This is a handoff for the incoming network feature, not a complete audit of all
platform differences. See `STATUS.md` for prior native validation and limitations.

## Status meanings

- **Queued:** PC feature recorded; Mac adaptation is pending.
- **In progress:** Mac implementation is underway.
- **Awaiting validation:** Implementation is present but relevant Mac checks remain.
- **Verified:** Mac acceptance checks passed, with evidence linked below.
- **Not applicable:** No corresponding Mac capability is needed; explain why.

If shared code already supplies the feature, record that fact and use awaiting
validation until Mac checks pass. Do not infer Mac support from Windows results.

## Entry template

Copy this template into the queue for each new entry. Replace all placeholders.

### MAC-NNN — Feature name

- **Status:** Queued
- **PC behavior:** What the user can do and how it should behave.
- **PC source:** Branch, commit/PR (or uncommitted), and relevant source/patch paths.
- **PC validation:** Checks performed and any remaining Windows gaps.
- **Existing Mac support:** Shared behavior already available, or unknown.
- **Mac adaptation needed:** Adapter, build, UI, rendering or configuration work;
  include dependencies and settings compatibility where relevant.
- **Mac acceptance checks:** Concrete behavior to verify with isolated tests.
- **Mac implementation and evidence:** Commit/files, host checks and a link to
  the relevant `STATUS.md` validation record; pending until performed.
