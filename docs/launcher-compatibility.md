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

## Subscription-based cloud repairs

Codex cloud uses ChatGPT sign-in and the plan's usage allowance. It does not
require an OpenAI API key, but its allowance is limited. Do not enable paid
overages or an API fallback for this repository's compatibility automation.

The repository's ChatGPT Codex Connector installation is restricted to this
repository. Code Review is configured for team PRs on every push; additional
credit use for reviews is off. A private cloud environment named
`timesplitters-future-perfect-pc` is published with shared-source/host-adapter
instructions and no API secrets. Its initial Linux/Python 3.12.14 setup at
`c238ab32229fb5da6b3e618112d704f9dc6c8082` passed the 7 compatibility,
16 release-policy and 3 shared-UI Python tests. These are portable contracts,
not the Windows/Mac C# UI or native package tests.

Automatic reviews, cloud coding and automatic repair dispatch are separate
capabilities. The current web event-task dry run can read this repository and
filter PR activity, but cannot directly select or launch the published coding
environment. No event repair task is enabled yet. On 2026-10-19, the isolated
[PR #192 dispatch test](https://github.com/TheDoffman/timesplitters-future-perfect-pc/pull/192#issuecomment-6280238841)
reached the GitHub bot, which required a separate integration environment.
Its setup link redirected to the ChatGPT home page in both available browsers.
The published current-generation cloud environment does not satisfy that
legacy integration requirement. Do not enable a watcher that repeatedly posts
repair requests until a successful cloud task and branch update verify the path.

The published environment remains usable for manually started cloud coding
tasks. A trusted local Codex worker signed in with ChatGPT is a separate possible
fallback, requiring the host computer to be awake. Neither an unattended cloud
repair watcher nor a local background worker is installed by this documentation.

A repair watcher must restrict work to open, non-draft, same-repository PRs by
the maintainer targeting `main`. It must wait for current-head CI, use evidence
from failed checks or actionable compatibility findings, and deduplicate work
by PR/head revision. Repairs start from the exact PR head, preserve its edits,
and use a reviewable branch. Allow one repair cycle plus one follow-up correction
per original head; stop for concurrent changes, missing access, exhausted plan
usage or unresolved failures. Quiet runs need no status comments.

Cloud repairs must obey `AGENTS.md`, preserve settings and pins, keep common
policy shared and update adapters/backlog evidence. Do not weaken checks,
force-push, merge, publish releases, alter protections or transfer private game
data. Inspect both hosts' required checks on the resulting revision before
reporting a repair successful. Visual and gameplay limits remain explicit.

See OpenAI's [GitHub integration](https://learn.chatgpt.com/docs/third-party/github),
[cloud environments](https://learn.chatgpt.com/docs/environments/cloud-environments)
and [scheduled tasks](https://learn.chatgpt.com/docs/automations) documentation.

## Development checks

Run `python -m unittest discover -s tests -p test_launcher_compatibility.py -v`
and `python -m unittest discover -s tests -p test_launcher_release.py -v`.
Both host jobs also run shared UI tests and the host launcher regression suite.
Scope tests exercise real Git rename/deletion diffs and completion tests cover
failure, cancellation, skipped builds and missing scope results.
