# Automatic launcher compatibility checks

`Launcher compatibility checks` validates launcher changes on Windows x64 and
native Mac ARM64. It uses standard GitHub-hosted runners, which are free for this
public repository. No OpenAI API key, API credits or third-party AI service is
required. The prior paid review/proposal workflow has been replaced.

## What happens on a PR

1. Every PR runs a small scope check so the final `Launcher compatibility` status
   is always reported. Launcher source, adapters, tests, shared configuration and
   build/workflow changes select both hosts. Removed or renamed launcher files
   remain in scope. Unrelated game/documentation changes skip launcher builds.
2. The shared read-only `launcher-build.yml` workflow builds/tests both launchers,
   runs their common UI tests, verifies all release assets and exercises the Mac
   packaged install/corruption/repair/restart path. Releases reuse this workflow.
3. `Launcher compatibility` passes only after the scope check and all selected
   builds/tests/package checks pass. Failure, cancellation, missing output or an
   unexpectedly skipped build cannot pass. Fork PRs use read-only permissions and
   GitHub's ordinary approval policy; no secrets are needed.
4. Failed checks link to build logs. Fix the shared source or the missing host
   adapter in the same PR, or ask Codex in the existing chat to make the repair.
   CI itself does not generate code, open repair PRs, merge or publish a release.

Common UI/model changes compile into both launchers. OS-specific calls still need
an implementation for the other host or an explicit capability/backlog entry.
For example, the Windows experimental fixed 120/144 FPS targets and monitor
switching remain unavailable on native Mac (MAC-054); its saved values are
preserved and the existing Mac interpolation setting remains usable.

Passing checks establish tested behavior, not complete visual or system-integration
parity. Discord sessions, native dialogs, physical displays, signing/notarization
and clean-machine delivery may need additional host checks. Record gaps in
`MAC_PORT_BACKLOG.md` and measured results in `STATUS.md`.

## Installation and operation

Merge the shared launcher/dual-platform release stack after its checks pass.
Add **Launcher compatibility** from GitHub Actions to the default branch's
required checks after its first successful PR run, preserving existing Windows/Mac
checks and the up-to-date requirement. Keep human merging. The workflow can also
be dispatched manually to validate the selected branch; it never publishes.

No enable variable or API secret is used. Repository setup removed
`OPENAI_API_KEY` and `LAUNCHER_COMPATIBILITY_ENABLED`. The ignored local API key
created during the earlier setup is unused by CI and can be revoked independently.
Do not upload ChatGPT session credentials to GitHub as a substitute.

Standard-runner availability/pricing: [GitHub runner documentation](https://docs.github.com/en/actions/reference/runners/github-hosted-runners).

## Development checks

Run `python -m unittest discover -s tests -p test_launcher_compatibility.py -v`
and `python -m unittest discover -s tests -p test_launcher_release.py -v`.
Both host jobs also run shared UI tests and the host launcher regression suite.
Scope tests exercise real Git rename/deletion diffs and completion tests cover
failure, cancellation, skipped builds and missing scope results.
