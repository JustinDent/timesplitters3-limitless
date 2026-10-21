# Evidence-loop validation (2026-10-20 EDT)

Host: Windows x64. Focused branch: `feat/sol-evidence-retrieval`, depending on
unmerged repository-context commit `2e06a0a` (itself based on `main` `c932bee`).
No game source, runtime checkout, patch, personal settings or save was changed.
The existing untracked #105 patch and `.pytest-work/` were preserved.

## Checks

- **77** repository context / CI scope / Mac backlog tests passed.
- **22** Node context/evidence contracts passed: Git-indexed source, exact
  identifiers, public defaults versus personal settings, source/commit changes,
  pinned runtime, redaction, junctions/hardlinks, patch ordering, byte limits,
  duplicate requests, exact-one budgets, cancellation and native restrictions.
- **110** installed local-router regressions passed, including every-call
  accounting, task guards, context preflight/compaction behavior and preserving
  source provenance inside the total 20-KB outgoing packet bound.

The checks exposed a documentation hazard: the supervisor's existing literal
routing detector treated a descriptive opt-out phrase in root AGENTS as a
blanket restriction. The explanatory wording was corrected; explicit user and
environment opt-outs were not weakened. Actual-repository preflight/advisory
regressions then passed.

## Real model exchange

A disposable Git fixture contained an incorrect numeric function and a contract
with a randomly chosen multiplier. Neither source body was in the initial task.
No response or routing decision was injected:

1. Actual `gpt-6.1-sol` requested both tracked file excerpts with `need_evidence`.
2. The local broker returned them with revision/hash/line provenance.
3. A second actual Sol response gave source-grounded correction advice.
4. Actual `qwen3.8:27b` verified the advice against files, made the local change
   and ran the independent verifier successfully.

Result: **43 seconds, two actual hosted calls, one retrieval round, five local
commands, two independent local checks, zero hosted tools and zero GitHub writes**.
The outgoing advisory contexts were 8,840 and 10,104 bytes. Retrieved evidence
was 1,171 bytes. The verifier, contract and normal hosted Codex configuration
were preserved. Later hardening of source freshness, exact identifier matching
and public metadata selection was covered by the contract/regression tests.

Separate read-only broker checks on the real repo located Swap gate-arm sites
in 177 ms and kernel release references in 89 ms. Those are local lookup timings,
not a benchmark of fixing #105 or a guarantee of future model performance.

Machine receipts: router `validation/sol-evidence/native-result.json` and
`install.json`; original installed-router backup:
`panel-backups/sol-evidence-20261021T030750595Z`. The repository stores portable
broker/loop source; the machine router/supervisor integration is installed locally.
macOS execution and remote CI remain unrun from this Windows host.

## Activation

Reload LOCAL SMART before its next task to load the new supervisor modules. If
the backend survives a reload, fully close those windows and reopen using the
usual Start shortcut. No UI restart was forced. Normal hosted sessions keep
their existing behavior. Other branches/devices require the dependent context
commits and their own router integration; synchronization is unverified.

The unfinished #105 patch SHA-256 remains
`be5f4ee917aff73b84a059470a745d175209adf0694d1437f0ade268fb495757`.
This validates knowledge retrieval/advisory continuation, not that game fix.
