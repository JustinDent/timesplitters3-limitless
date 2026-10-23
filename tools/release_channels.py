"""Shared game release identity for packaging and publishing (no network access)."""
import re

VERSION = re.compile(r'v?(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)(?:-([a-z][a-z0-9-]{0,39})\.([1-9][0-9]*))?\Z')
MINIMUM_LAUNCHER = '1.3.0'


def identity(version):
    match = VERSION.fullmatch(version)
    if not match:
        raise ValueError('Use X.Y.Z for Stable or X.Y.Z-channel.N for a named preview (N starts at 1).')
    channel = match[4] or 'stable'
    if match[4] == 'stable':
        raise ValueError('Stable versions must not have a prerelease suffix.')
    # Historical alpha/beta tags belong to the general Preview channel.
    if channel in ('alpha', 'beta', 'rc'):
        channel = 'preview'
    return dict(schema=1, channel=channel,
                name='Stable' if channel == 'stable' else 'Preview' if channel == 'preview' else channel.replace('-', ' ').title()+' Preview',
                minimum_launcher=MINIMUM_LAUNCHER)
