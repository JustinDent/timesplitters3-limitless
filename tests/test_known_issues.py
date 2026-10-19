"""Offline known-issues lifecycle and publication tests; no GitHub credentials."""
import contextlib
import importlib.util
import io
from datetime import datetime, timedelta, timezone
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('known_issues', ROOT / 'tools/known-issues.py')
known = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(known)
REPO = 'owner/game'
CONFIG = {'discussion_number': 143, 'title': 'Known issues and workarounds', 'labels': ['bug', 'bugged']}


def issue(number=1, labels=('bug',), **kwargs):
    return dict(number=number, title='Mouse input', state='open',
                labels=[{'name': label} for label in labels], comments=0, **kwargs)


def comment(number=1, body='### Workaround\nUse the keyboard.', association='OWNER', user_type='User'):
    return {'id': number, 'body': body, 'author_association': association,
            'user': {'type': user_type}, 'updated_at': f'2026-10-19T08:00:{number:02d}Z'}


class FakeGitHub:
    def __init__(self):
        self.issues = [issue()]
        self.comments = {}
        self.discussion_comments = []
        self.target = {'id': 'discussion-id', 'title': CONFIG['title'], 'body': 'Previous manual post.',
                       'url': f'https://github.com/{REPO}/discussions/143'}
        self.read_error = self.write_error = False
        self.requests = []

    def request(self, method, path, payload=None):
        self.requests.append((method, path, payload))
        if method == 'GET':
            page = int(path.rsplit('page=', 1)[1])
            records = (self.comments.get(int(path.split('/issues/')[1].split('/')[0]), [])
                       if '/comments?' in path else self.issues)
            return records[(page-1)*100:page*100]
        if payload['query'] == known.QUERY:
            target = self.target.copy() if self.target else None
            if target:
                page = int(payload['variables'].get('after') or 0)
                target['comments'] = {
                    'nodes': self.discussion_comments[page*100:(page+1)*100],
                    'pageInfo': {'hasNextPage': len(self.discussion_comments) > (page+1)*100,
                                 'endCursor': str(page+1)}}
            return ({'errors': [{'message': 'Unavailable'}]} if self.read_error else
                    {'data': {'repository': {'discussion': target}}})
        if self.write_error:
            return {'errors': [{'message': 'Access denied'}]}
        if payload['query'] == known.CREATE:
            assert payload['variables']['id'] == 'discussion-id'
            stamp = datetime(2026, 10, 19, tzinfo=timezone.utc) + timedelta(seconds=len(self.discussion_comments))
            self.discussion_comments.append({'id': 'comment-' + str(len(self.discussion_comments)),
                'createdAt': stamp.isoformat(), 'body': payload['variables']['body'],
                'url': self.target['url'] + '#discussioncomment-123',
                'author': {'login': 'github-actions[bot]', '__typename': 'Bot'}})
        else:
            raise AssertionError('Unexpected mutation; workflow token must never attempt edits.')
        return {'data': {'addDiscussionComment': {'comment': {'url': self.target['url'] + '#discussioncomment-123'}}}}

    @property
    def generated_body(self):
        return max((c for c in self.discussion_comments
                    if c['author']['login'].removesuffix('[bot]') == 'github-actions'),
                   key=lambda c: c['createdAt'])['body']

    def writes(self):
        return [r for r in self.requests if r[2] and r[2].get('query') == known.CREATE]


class KnownIssuesTests(unittest.TestCase):
    def publish(self, api, dry_run=False):
        with contextlib.redirect_stdout(io.StringIO()):
            return known.publish(api, REPO, CONFIG, dry_run)

    def test_open_bugs_aliases_and_case_but_not_prs_closed_or_enhancements(self):
        api = FakeGitHub()
        api.issues = [issue(4, ('BUGGED',)), issue(2), issue(2), issue(3, ('enhancement',)),
                      issue(5, pull_request={}), issue(6)]
        api.issues[-1]['state'] = 'closed'
        self.publish(api)
        body = api.generated_body
        self.assertIn('Open bug reports (2)', body)
        self.assertLess(body.index('[#2'), body.index('[#4'))
        for number in (3, 5, 6):
            self.assertNotIn(f'/issues/{number})', body)

    def test_close_untag_and_reopen_regenerate_same_post(self):
        api = FakeGitHub(); self.publish(api)
        self.assertIn('/issues/1)', api.generated_body)
        api.issues[0]['state'] = 'closed'; self.publish(api)
        self.assertIn('No open issues', api.generated_body)
        api.issues[0]['state'] = 'open'; self.publish(api)
        self.assertIn('/issues/1)', api.generated_body)
        api.issues[0]['labels'] = []; self.publish(api)
        self.assertNotIn('/issues/1)', api.generated_body)
        self.assertTrue(all(r[2]['variables']['id'] == 'discussion-id' for r in api.writes()))

    def test_changed_title_updates_and_identical_runs_do_not_write(self):
        api = FakeGitHub()
        self.assertTrue(self.publish(api))
        self.assertFalse(self.publish(api))
        api.issues[0]['title'] = 'Updated mouse report'
        self.assertTrue(self.publish(api))
        self.assertEqual(len(api.writes()), 2)
        self.assertIn('Updated mouse report', api.generated_body)

    def test_discussion_body_and_other_authors_comments_are_preserved(self):
        api = FakeGitHub()
        human = {'id': 'human', 'body': known.MARKER + '\nMaintainer notes.',
                 'author': {'login': 'owner', '__typename': 'User'}}
        other_bot = {'id': 'other-bot', 'body': known.MARKER + '\nOther bot notes.',
                     'author': {'login': 'another[bot]', '__typename': 'Bot'}}
        api.discussion_comments = [human.copy(), other_bot.copy()]
        self.publish(api)
        self.assertEqual(api.target['body'], 'Previous manual post.')
        self.assertEqual(api.discussion_comments[:2], [human, other_bot])
        self.assertEqual(len(api.discussion_comments), 3)
        api.issues = []; self.publish(api)
        self.assertEqual(len(api.discussion_comments), 4)
        self.assertEqual(api.discussion_comments[:2], [human, other_bot])
        self.assertEqual(api.target['body'], 'Previous manual post.')

    def test_latest_snapshot_on_later_page_is_found_and_unchanged_run_skips(self):
        api = FakeGitHub(); self.publish(api)
        api.discussion_comments[:0] = [{'id': str(n), 'body': 'Conversation.',
            'author': {'login': 'human', '__typename': 'User'}} for n in range(100)]
        self.assertFalse(self.publish(api))
        api.issues = []; self.publish(api)
        self.assertEqual(len(api.discussion_comments), 102)
        self.assertEqual(api.writes()[-1][2]['query'], known.CREATE)
        self.assertIn('No open issues', api.generated_body)

    def test_graphql_bot_login_without_rest_suffix_is_reused(self):
        api = FakeGitHub(); self.publish(api)
        api.discussion_comments[0]['author']['login'] = 'github-actions'
        self.assertFalse(self.publish(api))
        self.assertEqual(len(api.discussion_comments), 1)
        self.assertEqual(api.writes()[-1][2]['query'], known.CREATE)

    def test_newest_snapshot_wins_even_if_comments_arrive_out_of_order(self):
        api = FakeGitHub(); self.publish(api)
        original = api.discussion_comments[0].copy()
        api.issues = []; self.publish(api)
        api.discussion_comments.reverse()
        self.assertFalse(self.publish(api))
        self.assertEqual(len(api.writes()), 2)
        self.assertEqual(api.discussion_comments[-1], original)

    def test_existing_comment_update_rejection_is_reported(self):
        api = FakeGitHub(); self.publish(api)
        api.issues = []; api.write_error = True
        with self.assertRaisesRegex(RuntimeError, 'GraphQL: Access denied'):
            self.publish(api)

    def test_latest_explicit_human_maintainer_workaround_only(self):
        api = FakeGitHub(); api.issues[0]['comments'] = 5
        api.comments[1] = [comment(1), comment(2, '### Workaround\nNew keyboard instructions.'),
                           comment(3, 'Unrelated maintainer conversation.'),
                           comment(4, '### Workaround\nUntrusted advice.', 'CONTRIBUTOR'),
                           comment(5, '### Workaround\nBot advice.', user_type='Bot')]
        self.publish(api)
        self.assertIn('New keyboard instructions', api.generated_body)
        self.assertIn('#issuecomment-2', api.generated_body)
        self.assertNotIn('Untrusted advice', api.generated_body)
        self.assertNotIn('Bot advice', api.generated_body)
        api.comments[1][1]['body'] = '### Workaround\nEdited instructions.'
        self.publish(api)
        self.assertIn('Edited instructions', api.generated_body)

    def test_deleting_last_workaround_restores_no_workaround_text(self):
        api = FakeGitHub(); api.issues[0]['comments'] = 1
        api.comments[1] = [comment()]; self.publish(api)
        api.comments[1] = []; api.issues[0]['comments'] = 0; self.publish(api)
        self.assertIn('No workaround documented yet', api.generated_body)
        self.assertNotIn('Use the keyboard', api.generated_body)

    def test_workaround_from_untrusted_body_is_not_published(self):
        api = FakeGitHub(); api.issues[0]['body'] = '### Workaround\nGuessed fix.'
        self.publish(api)
        self.assertNotIn('Guessed fix', api.generated_body)

    def test_paginated_issues_and_comments_are_all_read(self):
        api = FakeGitHub(); api.issues = [issue(n) for n in range(1, 102)]
        api.issues[0]['comments'] = 101
        api.comments[1] = [comment(n, 'Ordinary chat.') for n in range(1, 101)] + [comment(101)]
        self.publish(api)
        self.assertIn('Open bug reports (101)', api.generated_body)
        self.assertIn('#issuecomment-101', api.generated_body)
        self.assertIn('/issues/101)', api.generated_body)

    def test_failed_pagination_never_replaces_post_with_partial_data(self):
        api = FakeGitHub(); api.issues = [issue(n) for n in range(1, 101)]
        original = api.request
        def request(method, path, payload=None):
            if path.endswith('page=2'):
                raise RuntimeError('Network failed')
            return original(method, path, payload)
        api.request = request
        with self.assertRaisesRegex(RuntimeError, 'Network'):
            self.publish(api)
        self.assertEqual(api.writes(), [])

    def test_titles_and_workarounds_cannot_inject_rows_html_links_or_pings(self):
        api = FakeGitHub(); api.issues[0]['title'] = 'Bad | [link](https://evil.test) @all <img>\n# heading'
        api.issues[0]['comments'] = 1
        api.comments[1] = [comment(body='### Workaround\n@someone | </td> [x](javascript:evil)')]
        self.publish(api)
        body = api.generated_body
        self.assertNotIn('@someone', body)
        self.assertNotIn('<img>', body)
        self.assertIn('\\|', body)
        self.assertIn('\\[link\\]', body)
        self.assertIn('&lt;/td&gt;', body)
        self.assertEqual(body.count('\n| [#'), 1)

    def test_heading_parser_ignores_fences_and_stops_at_next_peer_heading(self):
        self.assertEqual(known.workaround_section('```\n### Workaround\nFake\n```\nText'), '')
        self.assertEqual(known.workaround_section('### Workaround\nUse keyboard.\n### Validation\nNot part.'), 'Use keyboard.')
        self.assertEqual(known.workaround_section('## Workarounds\n```text\nUse keyboard.\n```'), 'Use keyboard.')

    def test_dry_run_never_reads_or_writes_discussions(self):
        api = FakeGitHub(); self.assertFalse(self.publish(api, True))
        self.assertTrue(all(r[0] == 'GET' for r in api.requests))

    def test_wrong_title_missing_discussion_or_read_errors_fail_without_writes(self):
        for failure in ('title', 'missing', 'read'):
            with self.subTest(failure=failure):
                api = FakeGitHub()
                if failure == 'title': api.target['title'] = 'Unrelated post'
                elif failure == 'missing': api.target = None
                else: api.read_error = True
                with self.assertRaises((ValueError, RuntimeError)):
                    self.publish(api)
                self.assertEqual(api.writes(), [])

    def test_write_permission_errors_are_not_reported_as_success(self):
        api = FakeGitHub(); api.write_error = True
        with self.assertRaisesRegex(RuntimeError, 'GraphQL: Access denied'):
            self.publish(api)

    def test_oversized_output_fails_before_discussion_write(self):
        api = FakeGitHub(); api.issues = [issue(n) for n in range(1, 601)]
        for record in api.issues: record['title'] = 'x' * 200
        with self.assertRaisesRegex(ValueError, 'too large'):
            self.publish(api)
        self.assertEqual(api.writes(), [])

    def test_publishing_is_separate_from_pr_tests_and_uses_trusted_checkout(self):
        workflow = (ROOT / '.github/workflows/known-issues.yml').read_text()
        triggers = workflow.split('\non:\n')[1].split('\npermissions:\n')[0]
        self.assertNotIn('pull_request:', triggers)
        self.assertNotIn('pull_request_target:', triggers)
        for event in ('issues:', 'issue_comment:', 'label:', 'workflow_dispatch:', 'workflow_call:', 'schedule:'):
            self.assertIn('  ' + event, triggers)
        self.assertIn("ref: ${{ github.event_name == 'workflow_dispatch' && github.ref || github.event.repository.default_branch }}", workflow)
        self.assertIn('persist-credentials: false', workflow)
        self.assertIn('group: known-issues-discussion', workflow)
        self.assertIn('discussions: write', workflow)
        self.assertNotIn('issues: write', workflow)
        self.assertNotIn('contents: write', workflow)
        self.assertIn("github.repository == 'TheDoffman/timesplitters-future-perfect-pc'", workflow)

    def test_bot_label_changes_explicitly_call_sync_and_pr_check_runs_tests_only(self):
        workflow = (ROOT / '.github/workflows/issue-labels.yml').read_text()
        sync = workflow.split('\n  known-issues:\n')[1]
        self.assertIn('needs: labels', sync)
        self.assertIn('uses: ./.github/workflows/known-issues.yml', sync)
        self.assertIn('discussions: write', sync)
        tests = (ROOT / '.github/workflows/issue-forms.yml').read_text()
        self.assertIn('python -m unittest discover -s tests -p test_known_issues.py -v', tests)
        self.assertNotIn('discussions: write', tests)


if __name__ == '__main__':
    unittest.main()
