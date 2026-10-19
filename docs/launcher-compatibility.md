# Automatic launcher compatibility review

`Launcher compatibility review` reviews launcher changes in both directions:
Windows to native Mac and native Mac to Windows. It runs even when the code builds.
It is a proposal-and-validation workflow; it never merges or publishes a release.
Shared features still belong in shared launcher source. Native OS/framework behavior
belongs in small adapters. AI review can miss a semantic or visual difference, so
passing is evidence of the checks performed, not a guarantee of complete parity.

## What happens on a PR

1. Trusted policy from the default branch checks the full changed-file list,
   including removed/renamed launcher paths. Unrelated PRs receive a successful
   `Launcher compatibility` commit status without using the API.
2. Ready, same-repository launcher PRs from contributors with write access receive
   a bounded Codex review of the exact head and base revisions. Fork PRs require a
   maintainer to copy the change to a repository branch. Drafts wait until ready.
3. The same read-only `launcher-build.yml` workflow used by launcher releases
   builds/tests Windows x64 and Mac ARM64, verifies both packages and runs the Mac
   install/corruption/repair/restart test. A validation source cannot publish.
4. A clean review plus successful builds passes `Launcher compatibility`. A gap,
   failed build, absent key, incomplete review, or malformed output does not pass.
5. An accepted proposal opens `codex/launcher-fix-<PR>-<head>` targeting the original
   feature branch. The original PR stays blocked until its fix is incorporated and
   the new head passes review and both-host validation. No branch is force-pushed.
6. Bot-created PRs do not automatically trigger GitHub's ordinary PR workflows.
   The publisher explicitly dispatches this workflow for the fix PR. That run
   builds/tests both launchers and reviews the fix but cannot open another fix PR.
   Maintainers can make any further corrections. Once the fix is merged into the
   original branch, its normal PR workflows also run on that new head.

An unchanged-head retry reuses an open fix PR. Closed/retargeted repair PRs require
manual follow-up. Changed head/base/target revisions invalidate results. The manual
workflow takes `pull_request` and must run from the default branch. This is useful
for a retry after fixing credentials or an infrastructure failure.

## Installation and activation

This workflow depends on the shared launcher UI and dual-platform release PR stack.
Merge it with that stack before activating it on the default branch. Opening its PR
only tests the orchestration and shared builds; it does not enable the AI service.

Repository administrators complete activation in this order:

1. Configure the securely provisioned dedicated key as the Actions repository
   secret `OPENAI_API_KEY`. Never put it in Git, a PR, or workflow output.
2. Ensure Actions may create pull requests. Jobs request only their required
   permissions; the default workflow permission can stay read-only.
3. Set repository variable `LAUNCHER_COMPATIBILITY_ENABLED` to `true`.
4. Dispatch `Launcher compatibility review` for a ready launcher PR. Confirm a
   real review and both host jobs complete; exercise a controlled adaptation PR.
5. Add **Launcher compatibility** (from GitHub Actions) to main's required status
   checks, retaining existing required checks and requiring branches up to date.
   Do this after the first successful run, not before the workflow is installed.
   Keep human merging enabled and preserve the existing review policy.

Deactivation stops API runs by clearing the enable variable. Relevant PRs fail
closed while disabled; it is not a bypass for the required compatibility status.
An administrator can deliberately revise the branch rule if retiring the service.

## Boundaries and recovery

- Only a source snapshot enters Codex; PR-controlled `.codex`, instructions,
  workflows, symlinks, submodules, private files and Git hooks are not installed as
  agent configuration. The trusted root rules and review prompt are separate.
- Codex runs on a disposable Linux runner with `drop-sudo`, workspace permissions,
  a fresh trusted Codex home, and no repository write token. Its action is the last
  step. The API proxy receives the key; the publishing job never receives it.
- A fresh job parses bounded JSON, checks the current PR revisions again, applies
  the patch in a disposable checkout without executing PR scripts, and checks its
  actual changed paths/modes before creating Git objects and a separate PR.
- Proposals are limited to 64 KiB/25 ordinary launcher-source, adapter, test or
  documentation files. Policy, credentials, workflows and game runtime cannot be
  changed by a proposal. Larger/system-wide changes are reported for manual work.
- The action and CLI/proxy are pinned. Update them intentionally with their policy
  tests. Reviews time out after 20 minutes; only trusted contributors can incur
  API usage. API usage and runner usage follow the account's configured billing.
- A failed review/build can be rerun from Actions. Read the workflow summary for
  findings. Credentials and third-party availability are not mocked into a pass.
- Discord sessions, native dialogs, signing/notarization, clean-machine delivery
  and visual fidelity can need additional host validation. New gaps belong in
  `MAC_PORT_BACKLOG.md` and measured results in `STATUS.md`.

## Development checks

Run `python -m unittest discover -s tests -p test_launcher_compatibility.py -v`
and `python -m unittest discover -s tests -p test_launcher_release.py -v`.
Both host CI jobs run these contracts. They cover scope, trust, stale revisions,
patch limits/paths/modes, retries, recursion prevention, explicit dispatch of bot
repairs, and the requirement for both host builds to pass. Real API/key activation
is a separate acceptance check and is recorded explicitly in STATUS.md.
