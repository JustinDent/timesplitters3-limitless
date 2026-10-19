# CI and branch protection

Every pull request runs these checks, without file-path or target-branch filters:

- **Issue form tests**
- **Issue resolution tests**
- **Patch transitions (windows-latest)**
- **Patch transitions (macos-latest)**

The issue-test workflows contain only their tests. Event automation runs in
separate workflows, so open PRs do not list skipped `labels`, `versions` or
`comment` jobs:

- **Automatic issue labels** runs when issues are opened, edited or reopened.
- **Issue form versions** validates and proposes reference updates after release
  changes, relevant pushes to `main`, or a manual dispatch.
- **Issue resolution notes** comments on linked issues only after a PR into
  `main` is merged, using trusted default-branch code.

Required job names and their GitHub Actions source stay the same; no branch
ruleset changes are needed when moving the automation.

The runtime jobs always test suite selection. They run the affected patch,
native, release and launcher tests when relevant files change. A documentation
edit therefore still gets completed checks without a full game build. No
private game files or personal saves are needed in CI.

The `Base` branch ruleset targets `main` with active enforcement. It requires a
pull request, all four checks from GitHub Actions, and an up-to-date branch.
Deletion and force pushes remain blocked. There are no bypass actors or required
reviewer approvals, so a solo maintainer can merge once checks pass.

If `main` advances, use **Update branch** or merge `main` into the PR branch and
wait for fresh checks. Repository-level **Allow auto-merge** makes the option
available; it does not automatically select every PR for merging. Choose
**Enable auto-merge** on the individual PRs you want to merge after checks pass.

Published version references are updated through a PR on
`automation/issue-form-versions`, rather than a direct push to `main`. Updating
that branch preserves its previous tip and the current default branch as
ancestors; it never force-pushes or replaces unrelated edits. The workflow needs
`contents: write`, `pull-requests: write` and the repository's **Allow GitHub
Actions to create and approve pull requests** setting. It does not approve PRs.

GitHub may require a maintainer to select **Approve workflows to run** on a
token-created PR before its checks start. After approval, the same required
checks and up-to-date rule apply. Enable auto-merge on the version PR if desired.
No personal access token, new secret or branch-protection bypass is needed.

If renaming a required job or changing its matrix, update the ruleset's check
names as part of the same rollout so PRs are not left waiting for a missing check.
