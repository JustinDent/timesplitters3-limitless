"""One-time validation of bot-owned Discussion publishing."""
import importlib.util
import os
from pathlib import Path

spec = importlib.util.spec_from_file_location('known_issues', Path(__file__).with_name('known-issues.py'))
known = importlib.util.module_from_spec(spec)
spec.loader.exec_module(known)
repo = os.environ['GITHUB_REPOSITORY']
owner, name = repo.split('/')
api = known.GitHub(os.environ['GITHUB_TOKEN'])
result = api.request('POST', 'graphql', {'query': '''query($owner: String!, $name: String!) {
  viewer { login }
  repository(owner: $owner, name: $name) {
    id
    discussionCategories(first: 100) { nodes { id name } }
    discussions(first: 100) {
      nodes { id number title body url author { login } }
      pageInfo { hasNextPage }
    }
  }
}''', 'variables': {'owner': owner, 'name': name}})
if result.get('errors'):
    raise RuntimeError(result['errors'])
data = result['data']; repository = data['repository']
if repository['discussions']['pageInfo']['hasNextPage']:
    raise RuntimeError('Refusing to create with an incomplete Discussion list.')
print('Authenticated as ' + data['viewer']['login'])
target = next((d for d in repository['discussions']['nodes']
               if (d['author'] or {}).get('login') == data['viewer']['login']
               and d['body'].startswith(known.MARKER)), None)
body = known.render(api, repo, {'bug', 'bugged'})
if target is None:
    category = next(c['id'] for c in repository['discussionCategories']['nodes'] if c['name'] == 'General')
    result = api.request('POST', 'graphql', {'query': '''mutation($repo: ID!, $category: ID!, $body: String!) {
      createDiscussion(input: {repositoryId: $repo, categoryId: $category,
        title: "Known issues and workarounds", body: $body}) {
        discussion { id number url }
      }
    }''', 'variables': {'repo': repository['id'], 'category': category, 'body': body}})
    if result.get('errors'):
        raise RuntimeError(result['errors'])
    target = result['data']['createDiscussion']['discussion']
    print('Created bot-owned Discussion: ' + target['url'])
result = api.request('POST', 'graphql', {'query': known.UPDATE,
    'variables': {'id': target['id'], 'body': body + '\n<!-- bot-update-verified -->\n'}})
if result.get('errors'):
    raise RuntimeError(result['errors'])
print('Bot-owned update verified: ' + target['url'])
