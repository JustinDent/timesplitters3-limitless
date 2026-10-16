"""Paths shared by source builds and the self-contained portable companion."""
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
PORTABLE = TOOLS.name == 'Online' and TOOLS.parent.name == 'Support'
ROOT = TOOLS.parents[1] if PORTABLE else TOOLS.parent
HANDSHAKES = ROOT / ('Logs/Online' if PORTABLE else 'private/online')
SETTINGS = ROOT / ('Settings/pc-online.json' if PORTABLE else 'config/pc-online.json')
MANIFEST = TOOLS / 'network-identity.json' if PORTABLE else ROOT / 'build/windows/generated/network-identity.json'


def directory_folder(root):
    root = Path(root)
    return root / ('Settings/OnlineDirectory' if (root/'Support/Online/online.py').is_file()
                   else 'private/online-directory')
