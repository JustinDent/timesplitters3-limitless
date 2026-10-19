# Windows game crash reports

The Windows game starts `tsfp_crash_helper.exe` from beside its executable.
Unhandled exceptions on the main thread or a worker thread notify this helper
through inherited, restricted handles and a fixed shared-memory packet. The
crashing thread copies its Windows context and thread-local Xbox registers; it
does not allocate, format output, resolve symbols or acquire game locks. The
helper writes the dump while the process is still alive. The game waits at most
12 seconds for capture, then exits with the original exception.

Xbox MMIO traps and memory watchpoints retain their existing vectored handler.
Handled SEH exceptions do not create reports. The former first-chance crash
diagnostic block is bypassed in the PC build so unsafe instruction/stack reads
and symbol lookup cannot interfere before final unhandled capture.

Packaged launches write `Logs/Crashes/crash-<UTC>-<PID>-<helper PID>/` containing:

- `report.txt`: readable exception/address summary and capture result.
- `report.json`: process/thread, executable SHA-256, native RIP/RSP and available
  Xbox registers. `valid=false` means guest memory was not initialized.
- `crash.dmp`: native exception/context, thread stacks, modules and a custom
  Xbox-register stream (`LastReservedStream + 1`). Full guest RAM is excluded.
- `game-log-tail.txt`: at most the last 64 KiB of `RECOMP_LOG_FILE`, when available.

`TSFP_CRASH_DIR` overrides the destination. Without it, direct execution uses
`Logs/Crashes` beside the executable. Paths through junctions/symlinks are refused.
Each dump is capped at 16 MiB through the DbgHelp I/O callback. Capture keeps five
recent report directories during ordinary serial sessions (under 81 MiB plus
small summaries). Active/linked directories and unknown files are not removed;
those can prevent cleanup. Concurrent sessions may temporarily exceed five.
Incomplete dumps are removed and a failure summary is retained when possible.
An unavailable helper disables capture with a runtime diagnostic; the game can
still launch. Unwritable storage can prevent a report entirely.

After a newly recorded crash, the Windows launcher restores its window and opens
Repair / Verify > Diagnostics. **Open latest game report** opens the folder;
**Export game report...** saves only the four known report files to a new local
ZIP. No automatic upload occurs. Dumps and log excerpts can contain personal
information; review them before sharing. The ordinary copyable diagnostics
include only a validated exception code, report ID, capture status and build hash.

## Freeze detection

The same helper watches game-frame progress independently of the game threads.
After guest execution starts, it allows **three minutes before the first frame**.
After frame progress has begun, **60 seconds without another frame boundary**
triggers a `suspected_hang` report. The diagnostic commands below use a one-second
threshold to keep tests short; ordinary launches always use the production grace
periods. The legacy `RECOMP_WATCHDOG_SECS` timer remains a separate, opt-in
developer tool that exits the process; it is not enabled by this feature.

The watchdog suspends observation while the game is minimized or in the
background, during modal settings/debug dialogs, and during Windows menu/resize
loops. Returning to normal observation grants the full active grace period.
A helper polling gap over five seconds resets grace to avoid immediately
reporting after machine sleep/resume or system-wide scheduling delays.
The ordinary in-game pause menu continues to render and therefore makes frame
progress. A background timer or responsive window cannot manufacture progress.

There is **one report per continuous stall**, including when capture fails.
Actual new frame progress rearms the watchdog for a later incident. A hang report
does not terminate the game. Recovery remains possible, and a subsequent fatal
exception still produces its own crash report. Capture uses the existing 16 MiB
dump cap, 64 KiB log tail and five-report retention budget.

Hang reports use the same `crash-*` incident folders and `crash.dmp` filename for
compatibility, with `kind=suspected_hang` and `exception_code=null`. Their dumps
contain native thread stacks and contexts **without an exception stream**. Xbox
registers are explicitly marked as a cached last-frame snapshot, not the frozen
thread's live registers. The reported thread identifies the last frame producer
or, before any frame, the guest entry thread.

The launcher checks for completed reports while the game runs, restores its
window, opens Diagnostics and permits local open/export without closing the
hung game. Release updates and repair remain blocked during play. Nothing is
automatically uploaded, and the launcher does not kill the game.

This detects stalled frame progress rather than proving a deadlock. An unusually
long foreground load can produce a suspected-hang report; a logic/audio freeze
that keeps producing frames can escape detection. Hangs before guest execution
is armed, while intentionally suppressed, or after shutdown begins are outside
this initial detector's scope. Inspect the thread dump to diagnose the cause.

## Reproduce and verify

Use Windows PowerShell and the supported build workflow:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File tsfp.ps1 Build
.\.venv\Scripts\python.exe tools/test-windows-crash.py
.\.venv\Scripts\python.exe tools/test-windows-hang.py
powershell.exe -NoProfile -ExecutionPolicy Bypass -File tools/build-launcher.ps1 -Test
```

The crash test executes the real game binary with exact diagnostic commands
before any game data, settings or saves are opened. It checks normal exit,
handled exception, fatal main-thread exception and fatal worker-thread exception.
The latter two must produce a dump whose saved exception resolves to
`tsfp_crash_test_site`, using a PDB GUID/age matching the dump. A wrong executable
or unexpected function must fail verification. Reports and `validation.json`
stay in `build/windows/crash-tests`. The command can be run repeatedly, with
report rotation; it never crashes an already-running interactive game.
It also checks five-report rotation, a blocked report destination, a missing
helper and preservation of unrelated files. `launcher-tests.exe --crashes`
checks report discovery, locking, export limits, atomic failure cleanup and UI
actions. For the real launcher/native-game handoff, run:

```powershell
& build/launcher/launcher-tests.exe --crash-integration `
    "$PWD/build/windows/Release/futureperfect_recomp.exe"
```

Run this integration check after the native crash tests have finished: report
export deliberately refuses while any game process is running. The integration
uses a disposable installation with only the executable, helper and test settings.

The hang suite checks responsive, transient, inhibited, unarmed and startup-grace
cases; a sustained freeze that remains running; recovery; a later second stall;
and a subsequent crash. Its offline verifier resolves the frozen test thread to
`tsfp_hang_test_site` with exact PDB GUID/age and confirms there is no exception
stream. The native `tsfp_hang_policy_test` checks deterministic policy times,
including background/modal suppression and machine sleep/resume. To verify the
launcher detects and exports a hang before the process recovers and exits:

```powershell
& build/launcher/launcher-tests.exe --hang-integration `
    "$PWD/build/windows/Release/futureperfect_recomp.exe"
```

Manual offline verification, including for a translated `sub_XXXXXXXX` function:

```powershell
& build/windows/Release/tsfp_crash_helper.exe --resolve C:\reports\crash.dmp `
    C:\symbols\futureperfect_recomp.exe C:\symbols expected_function_name
```

No symbol server or permissive mismatched-symbol mode is enabled. A missing or
mismatched PDB fails the check. Release builds retain public generated function
symbols; the deliberately faulting source is additionally compiled with `/Zi`.

Each successful Windows build replaces one local
`build/windows/crash-symbols.zip` containing its executable, PDB, hashes and source
revision/working-tree status. Packaging emits a versioned `*-symbols.zip` beside
the player package, outside it. Preserve this bundle for each published build;
a later rebuild's PDB is not interchangeable. This task does not publish releases.

## Scope

This captures ordinary unhandled Windows exceptions. Forced termination, power
loss, fail-fast exits that bypass exception filters, an attached debugger, or
another library replacing the unhandled filter can prevent capture. There is no
unfinished-session inference yet. Freeze detection has the limits described
above. Native stacks may expose
translated function names, but this is not a reconstructed Xbox call stack.
The initial version uses the bounded existing log tail, not a new gameplay
breadcrumb instrumentation system. Mac capture/report UI is queued separately.
