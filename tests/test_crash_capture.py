"""Exercise the real Windows crash adapter/helper without proprietary game data."""
import json
from pathlib import Path
import subprocess
import sys

import pytest
from native_compiler import native_compiler

ROOT = Path(__file__).resolve().parents[1]


@pytest.mark.skipif(sys.platform != 'win32', reason='Windows crash adapter')
def test_simultaneous_unhandled_exceptions_wait_for_dump(tmp_path):
    compiler, environment = native_compiler(tmp_path)
    driver = tmp_path / 'driver.c'
    driver.write_text('#include "pc_crash.h"\n'
                      'int main(void) { if (!pc_crash_init(0)) return 4; '
                      'return pc_crash_test_command("--tsfp-crash-test=concurrent"); }\n')
    for sources, name, libraries in [
            ([ROOT/'tools/crash_helper.c'], 'tsfp_crash_helper.exe', ['dbghelp.lib', 'bcrypt.lib', 'user32.lib']),
            ([driver, ROOT/'src/pc_crash.c'], 'driver.exe', ['user32.lib'])]:
        subprocess.run([compiler, '/nologo', '/std:c11', '/O2', '/MD', '/Zi',
                        '/I'+str(ROOT/'src'), *map(str, sources), '/Fe:'+str(tmp_path/name),
                        '/link', '/DEBUG', *libraries], check=True, cwd=tmp_path, env=environment)
    environment = {key: value for key, value in environment.items()
                   if not key.startswith(('TSFP_', 'RECOMP_'))}
    for iteration in range(3):
        area = tmp_path / str(iteration)
        environment['TSFP_CRASH_DIR'] = str(area)
        crashed = subprocess.run([str(tmp_path/'driver.exe')], cwd=tmp_path,
                                 env=environment, capture_output=True, timeout=20)
        assert crashed.returncode == 0xC0000005
        reports = list(area.glob('crash-*/report.json'))
        assert len(reports) == 1, 'Concurrent faults must leave one completed report'
        report = json.loads(reports[0].read_text())
        assert report['dump_saved'], report
        assert report['exception_code'] == '0xC0000005'
        resolved = subprocess.run([str(tmp_path/'tsfp_crash_helper.exe'), '--resolve',
                                   str(reports[0].parent/'crash.dmp'), str(tmp_path/'driver.exe'),
                                   str(tmp_path), 'tsfp_crash_test_site'], capture_output=True,
                                  text=True, timeout=20)
        assert resolved.returncode == 0, resolved.stdout + resolved.stderr
