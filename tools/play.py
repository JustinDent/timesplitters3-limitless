"""Run the windowed game with diagnostics written directly to local files."""

import subprocess
import sys
from datetime import datetime
from pathlib import Path


def main():
    executable = Path(sys.argv[1]).resolve()
    log_directory = Path(sys.argv[2]).resolve()
    log_directory.mkdir(parents=True, exist_ok=True)
    tag = datetime.now().strftime("play-%Y%m%d-%H%M%S")
    with (log_directory / f"{tag}.out").open("wb") as output, (
        log_directory / f"{tag}.err"
    ).open("wb") as errors:
        # Native file handles avoid console buffering and PowerShell's text
        # redirection. This is also how the bounded gameplay runner launches.
        result = subprocess.run(
            [str(executable)], cwd=executable.parent, stdout=output, stderr=errors
        )
    return result.returncode


if __name__ == "__main__":
    sys.exit(main())
