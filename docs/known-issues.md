# Automatic known issues and workarounds

GitHub Actions maintains the existing pinned
[Known issues and workarounds Discussion](https://github.com/TheDoffman/timesplitters-future-perfect-pc/discussions/143).
The first run replaces its manually maintained snapshot with the live list.
Its title, pinned placement, category and locked state stay unchanged.

Open issues labeled **bug** or **bugged** appear, with links to the full reports.
The existing bug form and labeler use **bug**; no new label is needed.
Closing, deleting, transferring out or removing the bug label removes the entry.
Reopening a still-labeled bug brings it back. A closed issue is not a promise
that its fix is already in a published release.

To document a workaround, a repository owner, member or collaborator adds a
comment on the issue:

```markdown
### Workaround
Use the keyboard to select the option while mouse support is being fixed.
```

The most recently updated qualifying human maintainer comment wins. Edits update
the post; deleting it falls back to the next qualifying comment, or displays
"No workaround documented yet." To withdraw older advice, edit the latest note
to explicitly say no workaround is currently available. Ordinary conversation,
reporter guesses, bot comments and headings inside code fences are excluded.
The post links to the source note; long notes are shortened to 600 characters.

The generated post refreshes on issue, label and issue-comment changes. The
automatic labeler also calls the sync after finishing, because changes made with
`GITHUB_TOKEN` do not trigger another issue workflow. A daily reconciliation and
**Actions → Known issues Discussion → Run workflow** cover missed events and
bot-created changes. Identical snapshots do not write or bump the Discussion.
Concurrent runs serialize and always read the current issue list, not old event
payloads. A failed or oversized read leaves the previous post in place.

The entire Discussion body is generated: keep workaround notes on their issues.
The target number, exact title and accepted labels live in
`.github/known-issues.json`. A missing target or changed title fails visibly
instead of creating duplicates or editing another post. Update the configuration
if the target is intentionally changed. Forks do not publish to this post.

Automation uses trusted default-branch code with `contents: read`, `issues: read`
and `discussions: write`; no personal token or secret is needed. It never checks
out or executes issue text. Tests run inside the existing **Issue form tests**
check, and the publishing workflow does not run on pull requests.

Read-only local preview (public repository reads only; no token needed):

```sh
python tools/known-issues.py --repository TheDoffman/timesplitters-future-perfect-pc --dry-run
python -m unittest discover -s tests -p test_known_issues.py -v
```
