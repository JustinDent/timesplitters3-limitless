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

No new PC features have been registered yet. This list starts with the new
workflow; it is not an audit of existing platform differences. See `STATUS.md`
for previously recorded Mac limitations and validation coverage.

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
