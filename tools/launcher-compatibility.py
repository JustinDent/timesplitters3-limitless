"""Trusted orchestration for launcher parity review; never execute PR code with writes."""
import argparse
import base64
import json
import os
from pathlib import Path, PurePosixPath
import re
import subprocess
import tempfile
import urllib.error
import urllib.parse
import urllib.request

REPOSITORY = 'TheDoffman/timesplitters-future-perfect-pc'
CONTEXT = 'Launcher compatibility'
FIX_PREFIX = 'codex/launcher-fix-'
MAX_PATCH = 64 * 1024
MAX_REPORT = 96 * 1024
POLICY_FILES = {
    '.github/workflows/launcher-compatibility.yml',
    '.github/workflows/launcher-build.yml', '.github/workflows/launcher-release.yml',
    '.github/prompts/launcher-compatibility.md',
    '.github/prompts/launcher-compatibility.schema.json',
    'tools/launcher-compatibility.py', 'tests/test_launcher_compatibility.py',
}
ADAPTERS = {
    'tools/build-launcher.ps1', 'tools/build-mac-launcher.py',
    'tools/test-mac-launcher.py', 'tools/play-native-mac.py', 'tools/playable-launch.ps1',
    'tools/release/ImportGame.cs', 'tools/build-game-icon.ps1',
}
DOCS = {'STATUS.md', 'MAC_PORT_BACKLOG.md', 'docs/launcher.md',
        'docs/windows-launcher.md', 'docs/shared-platform.md', 'docs/macos.md'}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def source_path(path):
    parts = PurePosixPath(path).parts
    if not re.fullmatch(r'[A-Za-z0-9_./-]+', path) or any(p in ('.', '..') or p.startswith('.') for p in parts):
        return False
    if path in POLICY_FILES:
        return False
    return (path in ADAPTERS or path in DOCS or
            (path.startswith('tools/launcher/') and Path(path).suffix in ('.cs', '.xaml', '.axaml', '.py', '.csproj', '.json', '.manifest')) or
            (path.startswith('tests/launcher_') and path.endswith('.cs')) or
            (path.startswith('tests/test_launcher_') and path.endswith('.py')))


def relevant(path):
    return (source_path(path) or path in POLICY_FILES or path in {
        'AGENTS.md', 'tools/launcher-release.py', 'tools/fetch-launcher-art.ps1',
        'config/mac-launcher-sdk.json', 'config/launcher-artwork.json',
        'config/pc-controls.default.json', 'config/pc-discord.default.ini',
        'src/pc_display_modes.inc', 'tests/test_native_mac_launcher.py',
    }) and path not in DOCS


def sha(value):
    require(isinstance(value, str) and re.fullmatch('[0-9a-f]{40}', value), 'Expected an exact commit SHA.')
    return value


class GitHub:
    def __init__(self, token, repository=REPOSITORY):
        require(repository == REPOSITORY, 'Unexpected repository.')
        self.token = token
        self.root = 'https://api.github.com/repos/' + repository

    def call(self, method, route, data=None):
        body = json.dumps(data).encode() if data is not None else None
        request = urllib.request.Request(self.root + route, data=body, method=method, headers={
            'Authorization': 'Bearer ' + self.token, 'Accept': 'application/vnd.github+json',
            'X-GitHub-Api-Version': '2022-11-28', 'Content-Type': 'application/json',
        })
        try:
            with urllib.request.urlopen(request, timeout=45) as response:
                raw = response.read()
                return json.loads(raw) if raw else None
        except urllib.error.HTTPError as error:
            # Do not echo request headers or API bodies into public logs.
            raise RuntimeError(f'GitHub {method} {route.split("?")[0]} returned {error.code}.') from None

    def files(self, number):
        files = []
        for page in range(1, 32):
            batch = self.call('GET', f'/pulls/{number}/files?per_page=100&page={page}')
            files.extend(batch)
            if len(batch) < 100:
                return files
        raise ValueError('PR exceeds the review file limit; manual review required.')

    def status(self, head, state, message):
        self.call('POST', '/statuses/' + sha(head), {
            'state': state, 'context': CONTEXT, 'description': message[:140],
            'target_url': f'https://github.com/{REPOSITORY}/actions/runs/{os.environ.get("GITHUB_RUN_ID", "0")}',
        })


def pr_number(event):
    value = event.get('number') or event.get('inputs', {}).get('pull_request')
    require(str(value).isdigit() and int(value) > 0, 'A pull request number is required.')
    return int(value)


def validate_pr(pr):
    require(pr['state'] == 'open', 'Pull request is no longer open.')
    require(pr['base']['repo']['full_name'] == REPOSITORY, 'Unexpected base repository.')
    require(pr['head'].get('repo') and pr['head']['repo']['full_name'] == REPOSITORY,
            'Fork PR: a maintainer must copy the change to a repository branch before AI review.')
    sha(pr['head']['sha'])
    sha(pr['base']['sha'])


def prepare(api, event, enabled, key_present):
    number = pr_number(event)
    pr = api.call('GET', f'/pulls/{number}')
    head = sha(pr['head']['sha'])
    result = {'run': False, 'number': number, 'head': head, 'base': sha(pr['base']['sha']),
              'base_ref': pr['base']['ref'], 'head_ref': pr['head']['ref'], 'allow_fix': True}
    api.status(head, 'pending', 'Checking launcher compatibility scope.')
    files = api.files(number)
    paths = [f[k] for f in files for k in ('filename', 'previous_filename') if k in f]
    require(len(files) == pr['changed_files'], 'Incomplete changed-file list; review cannot be skipped.')
    if not any(relevant(p) for p in paths):
        api.status(head, 'success', 'No launcher implementation changes.')
        return result
    api.status(head, 'pending', 'Reviewing Windows and native Mac compatibility.')
    try:
        validate_pr(pr)
        require(not pr['draft'], 'Mark the PR ready for review to start compatibility review.')
        require(enabled, 'Enable LAUNCHER_COMPATIBILITY_ENABLED after installing the API secret.')
        require(key_present, 'OPENAI_API_KEY is missing; compatibility review has not run.')
        author = pr['user']['login']
        bot_fix = author == 'github-actions[bot]' and pr['head']['ref'].startswith(FIX_PREFIX)
        if not bot_fix:
            permission = api.call('GET', '/collaborators/' + urllib.parse.quote(author, safe='') + '/permission')
            require(permission['permission'] in ('admin', 'maintain', 'write'),
                    'Automatic review requires a trusted repository contributor.')
        result['allow_fix'] = not pr['head']['ref'].startswith(FIX_PREFIX)
        result['run'] = True
    except ValueError as error:
        api.status(head, 'failure', str(error))
        result['reason'] = str(error)
    return result


def git(root, *args, data=None):
    return subprocess.check_output(['git', '-c', 'core.hooksPath=/dev/null',
                                    '-c', 'core.autocrlf=false', *args], cwd=root, input=data)


def snapshot(root, destination, head, base, policy):
    """Copy regular source blobs, excluding PR-controlled configs/instructions/hooks."""
    sha(head); sha(base)
    destination.mkdir(parents=True, exist_ok=False)
    total = 0
    for entry in git(root, 'ls-tree', '-rz', head).split(b'\0'):
        if not entry:
            continue
        metadata, raw_path = entry.split(b'\t', 1)
        mode, kind, blob = metadata.decode().split()
        path = raw_path.decode('utf-8')
        if not (source_path(path) or path in {'config/mac-launcher-sdk.json',
                'config/pc-controls.default.json', 'config/pc-discord.default.ini',
                'src/pc_display_modes.inc', 'requirements.txt'}):
            continue
        require(kind == 'blob' and mode in ('100644', '100755'), 'Non-regular source: ' + path)
        contents = git(root, 'cat-file', 'blob', blob)
        total += len(contents)
        require(len(contents) <= 2*1024**2 and total <= 20*1024**2, 'Source snapshot too large.')
        target = destination / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(contents)
        target.chmod(0o755 if mode == '100755' else 0o644)
    git(destination, 'init', '-q')
    git(destination, 'add', '.')
    git(destination, '-c', 'user.name=Compatibility snapshot', '-c', 'user.email=snapshot@localhost',
        'commit', '-qm', 'Exact PR source snapshot')
    diff = git(root, 'diff', '--no-ext-diff', '--no-textconv', '--binary', base + '...' + head)
    require(len(diff) <= 2*1024**2, 'PR diff too large for automatic review.')
    (destination.parent / 'change.diff').write_bytes(diff)
    (destination.parent / 'trusted-AGENTS.md').write_bytes((policy / 'AGENTS.md').read_bytes())


def parse_report(raw, allow_fix):
    require(len(raw.encode()) <= MAX_REPORT, 'Review output is too large.')
    report = json.loads(raw)
    require(isinstance(report, dict) and set(report) == {'verdict', 'summary', 'findings', 'patch'}, 'Invalid review fields.')
    require(report['verdict'] in ('pass', 'changes_needed', 'blocked'), 'Invalid review verdict.')
    require(isinstance(report['summary'], str) and 0 < len(report['summary']) <= 8000, 'Invalid review summary.')
    require(isinstance(report['findings'], list) and len(report['findings']) <= 30 and
            all(isinstance(s, str) and len(s) <= 4000 for s in report['findings']), 'Invalid findings.')
    patch = report['patch']
    require(isinstance(patch, str) and len(patch.encode()) <= MAX_PATCH, 'Patch exceeds 64 KiB.')
    require((report['verdict'] == 'changes_needed') == bool(patch), 'Verdict and patch disagree.')
    require(report['verdict'] != 'pass' or not report['findings'], 'A passing review cannot have unresolved findings.')
    require(allow_fix or not patch, 'Bot repair PRs cannot create recursive repair PRs.')
    return report


def apply_patch(root, patch):
    """Validate the actual resulting Git tree, not paths claimed by the model."""
    require(0 < len(patch.encode()) <= MAX_PATCH, 'Invalid patch size.')
    git(root, 'apply', '--index', '--whitespace=error', '-', data=patch.encode())
    records = git(root, 'diff', '--cached', '--raw', '--no-abbrev', '--no-renames', '-z').split(b'\0')
    changes = []
    for index in range(0, len(records)-1, 2):
        fields = records[index].decode().split()
        old_mode, new_mode = fields[0][1:], fields[1]
        path = records[index+1].decode('utf-8')
        require(source_path(path), 'Patch targets protected or unrelated path: ' + path)
        require(new_mode in ('000000', '100644', '100755') and old_mode in ('000000', '100644', '100755'),
                'Symlinks and submodules are not allowed.')
        require(not (old_mode == '100644' and new_mode == '100755'), 'Unexpected executable mode change.')
        blob = fields[3]
        contents = None if new_mode == '000000' else git(root, 'cat-file', 'blob', blob)
        require(contents is None or b'\0' not in contents, 'Binary launcher patches require manual review.')
        changes.append((path, new_mode, contents))
    require(0 < len(changes) <= 25, 'Patch must change between 1 and 25 source files.')
    return changes


def current_pr(api, context):
    pr = api.call('GET', f'/pulls/{context["number"]}')
    validate_pr(pr)
    require(pr['head']['sha'] == context['head'] and pr['base']['sha'] == context['base'] and
            pr['head']['ref'] == context['head_ref'] and pr['base']['ref'] == context['base_ref'],
            'PR changed during review; rerun against its current head and base.')
    return pr


def create_fix(api, context, report, root):
    changes = apply_patch(root, report['patch'])
    current_pr(api, context)
    branch = FIX_PREFIX + str(context['number']) + '-' + context['head'][:12]
    # Never reset or force-push an existing fix branch. A retry reuses its PR.
    existing = api.call('GET', '/pulls?state=all&head=' + urllib.parse.quote('TheDoffman:' + branch, safe='') + '&per_page=100')
    if existing:
        require(len(existing) == 1 and existing[0]['state'] == 'open' and existing[0]['base']['ref'] == context['head_ref'],
                'Existing repair was closed or retargeted; manual follow-up required.')
        return existing[0]
    tree = []
    for path, mode, contents in changes:
        blob = None
        if contents is not None:
            blob = api.call('POST', '/git/blobs', {'encoding': 'base64', 'content': base64.b64encode(contents).decode()})['sha']
        tree.append({'path': path, 'mode': '100644' if mode == '000000' else mode, 'type': 'blob', 'sha': blob})
    parent = api.call('GET', '/git/commits/' + context['head'])
    tree_sha = api.call('POST', '/git/trees', {'base_tree': parent['tree']['sha'], 'tree': tree})['sha']
    commit = api.call('POST', '/git/commits', {'message': f'Adapt launcher compatibility for PR #{context["number"]}',
                                           'tree': tree_sha, 'parents': [context['head']]})['sha']
    # A previous interrupted attempt may already have created this exact branch.
    refs = api.call('GET', '/git/matching-refs/heads/' + urllib.parse.quote(branch, safe='/'))
    exact = [r for r in refs if r['ref'] == 'refs/heads/' + branch]
    if exact:
        existing_commit = api.call('GET', '/git/commits/' + exact[0]['object']['sha'])
        require(existing_commit['tree']['sha'] == tree_sha and
                [p['sha'] for p in existing_commit['parents']] == [context['head']],
                'Existing fix branch differs; it will not be overwritten.')
    else:
        current_pr(api, context)
        api.call('POST', '/git/refs', {'ref': 'refs/heads/' + branch, 'sha': commit})
    body = (f'## Change\n\nProposed launcher compatibility fix for #{context["number"]}. '
            'Targets its source branch; merging remains a maintainer decision.\n\n'
            '## Cause and fix\n\n' + report['summary'] + '\n\n'
            '## Validation\n\nGenerated from exact source `' + context['head'] + '`; Windows and Mac '
            'builds, shared UI regressions and package repair tests are dispatched separately. '
            'Check their results before merging. AI review does not certify visual or OS integration parity.\n\n'
            '## Shared implementation and platform validation\n\n'
            'Both directions are reviewed using shared launcher policy and thin host adapters. '
            'See MAC-021 in MAC_PORT_BACKLOG.md for existing delivery gaps.\n')
    return api.call('POST', '/pulls', {'title': f'Launcher compatibility follow-up for #{context["number"]}',
                                     'head': branch, 'base': context['head_ref'], 'body': body})


def finalize(api, context, raw, review_result, build_result, root):
    current_pr(api, context)
    if review_result != 'success':
        api.status(context['head'], 'failure', 'Compatibility review failed or timed out; inspect the workflow run.')
        return
    report = parse_report(raw, context['allow_fix'])
    if report['patch']:
        fix = create_fix(api, context, report, root)
        default = api.call('GET', '')['default_branch']
        # GITHUB_TOKEN-created PRs do not trigger normal PR workflows. Dispatch
        # explicitly; that review-only child run cannot produce another repair.
        api.call('POST', '/actions/workflows/launcher-compatibility.yml/dispatches',
                 {'ref': default, 'inputs': {'pull_request': str(fix['number'])}})
        api.status(context['head'], 'failure', f'Compatibility fix proposed in PR #{fix["number"]}; merge it into this branch and rerun.')
        return
    if report['verdict'] == 'blocked':
        api.status(context['head'], 'failure', 'Unresolved compatibility gap; see the review report in the workflow summary.')
        return
    api.status(context['head'], 'success' if build_result == 'success' else 'failure',
               'Review and both native launcher builds passed.' if build_result == 'success'
               else 'Compatibility review passed, but both-host validation did not complete successfully.')


def output(values):
    with open(os.environ['GITHUB_OUTPUT'], 'a', encoding='utf-8') as stream:
        for key, value in values.items():
            if key == 'context':
                value = json.dumps(value, separators=(',', ':'))
            elif isinstance(value, bool):
                value = str(value).lower()
            require('\n' not in str(value), 'Unsafe workflow output.')
            stream.write(f'{key}={value}\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('command', choices=('prepare', 'snapshot', 'finalize', 'fail'))
    args = parser.parse_args()
    policy = Path(__file__).resolve().parents[1]
    context = json.loads(os.environ.get('REVIEW_CONTEXT', '{}'))
    if args.command == 'snapshot':
        work = Path('review-input').resolve()
        snapshot(Path('candidate').resolve(), work / 'source', context['head'], context['base'], policy)
        (work / 'review-context.json').write_text(json.dumps(context), encoding='utf-8')
        return
    api = GitHub(os.environ['GH_TOKEN'], os.environ['GITHUB_REPOSITORY'])
    if args.command == 'prepare':
        event = json.loads(Path(os.environ['GITHUB_EVENT_PATH']).read_text(encoding='utf-8'))
        context = prepare(api, event, os.environ.get('COMPATIBILITY_ENABLED') == 'true',
                          os.environ.get('HAS_OPENAI_KEY') == 'true')
        output({'run': context['run'], 'head': context['head'], 'context': context})
        return
    if args.command == 'fail':
        api.status(context['head'], 'failure', 'Compatibility automation failed; inspect the workflow and rerun.')
        return
    raw = os.environ.get('REVIEW_JSON', '')
    try:
        # This checkout has no persisted credentials; no PR script is executed.
        with tempfile.TemporaryDirectory(prefix='launcher-review-') as temporary:
            root = Path(temporary)
            git(root, 'init', '-q')
            git(root, 'fetch', '--no-tags', '--depth=1', 'https://github.com/' + REPOSITORY + '.git', sha(context['head']))
            git(root, 'checkout', '--detach', '-q', 'FETCH_HEAD')
            finalize(api, context, raw, os.environ['REVIEW_RESULT'], os.environ['BUILD_RESULT'], root)
        if raw:
            report = parse_report(raw, context['allow_fix'])
            # JSON is data, not shell code; bound it and escape HTML in the summary.
            import html
            with open(os.environ['GITHUB_STEP_SUMMARY'], 'a', encoding='utf-8') as summary:
                summary.write('<pre>' + html.escape(json.dumps({k: v for k, v in report.items() if k != 'patch'}, indent=2)) + '</pre>\n')
    except Exception:
        api.status(context['head'], 'failure', 'Compatibility automation failed; inspect the workflow and rerun.')
        raise


if __name__ == '__main__':
    main()
