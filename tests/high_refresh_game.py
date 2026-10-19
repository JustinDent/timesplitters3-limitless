"""Bounded opt-in Windows interpolation test; uses only disposable Test Lab data.

CPU submission times do not prove monitor scanout or perceptual smoothness.
--capture retains at most 24 interpolation BMPs for motion/fidelity review.
"""
import argparse
import ctypes
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import threading
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from testing_profile import XBE_SHA256, prepare_run, recycle_disks, check_fixture


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--fps', type=int, choices=(60, 120, 144), required=True)
    parser.add_argument('--capture', action='store_true')
    parser.add_argument('--capture-from', type=int, default=4500,
                        help='first guest frame to capture (late gameplay by default)')
    parser.add_argument('--stress', action='store_true',
                        help='replay walking, firing and camera turns after the intro')
    parser.add_argument('--legacy-projection-only', action='store_true')
    parser.add_argument('--pause-check', action='store_true')
    parser.add_argument('--profile', action='store_true')
    parser.add_argument('--presentmon', type=Path,
                        help='optional PresentMon console EXE; captures only this test PID')
    parser.add_argument('--gpu-timing', action='store_true', help='opt-in asynchronous D3D11 redraw/present CSV')
    parser.add_argument('--resources', action='store_true', help='sample child private memory and handle count every five seconds')
    parser.add_argument('--seconds', type=int, choices=range(60,601), default=90)
    parser.add_argument('--windowed', action='store_true')
    args = parser.parse_args()
    if os.name != 'nt': parser.error('Windows only')
    assert hashlib.sha256((ROOT / 'private/game/default.xbe').read_bytes()).hexdigest() == XBE_SHA256
    run = ROOT / 'private/test-saves/high-refresh'
    env = prepare_run(ROOT, run)
    tag = str(args.fps) + ('-projection-only' if args.legacy_projection_only else '')
    if args.resources: tag += '-stability'
    (run / 'runtime.ini').write_text(f'frame_target = {args.fps if args.fps > 60 else 0}\nframe_interp = 1\nframe_cap = 60\nfps_overlay = 1\nfullscreen = {int(not args.windowed)}\nvrr = 1\n')
    (run / 'display.ini').write_text('TSFP_DISPLAY_V1 1280 720\n')
    sequence = [(5000, 69), (10000, 69), (14000, 40), (18000, 69),
                (22000, 69), (26000, 69), (30000, 69), (34000, 69), (45000, 27)]
    if args.pause_check: sequence += [(80000, 27), (83000, 27)]
    events = [(at, f'key {key} 1') for at, key in sequence]
    events += [(at+180, f'key {key} 0') for at, key in sequence]
    # Slow camera pan in active gameplay; no desktop input is synthesized.
    events += [(at, 'mouse 2 0') for at in range(65000, 76000, 100)]
    if args.stress:
        events += [(at, 'mouse 4 0') for at in range(85000, 115000, 50)]
        for start, stop, key in [(85000, 90000, 87), (92000, 97000, 68),
                                  (99000, 104000, 83), (106000, 111000, 65)]:
            events += [(start, f'key {key} 1'), (stop, f'key {key} 0')]
        for at in range(87000, 112000, 700):
            events += [(at, 'key 1 1'), (at+180, 'key 1 0')]
    (run / 'input.txt').write_text('\n'.join(f'{at} {event}' for at,event in sorted(events))+'\n')
    env.update(TSFP_PC_INPUT_REPLAY=str(run / 'input.txt'), RECOMP_WINDOW_BACKGROUND='1' if args.windowed else '0',
               RECOMP_PC_MOUSE='1', RECOMP_INTERP_TRACE=str(run / (tag+'.csv')),
               RECOMP_INTERP_DEBUG='1', RECOMP_FPS='5', TSFP_PC_INPUT_STATE=str(run / 'state.txt'))
    (run / 'state.txt').write_text('0 0\n')
    (run / 'discord.ini').write_text('TSFP_DISCORD_V1 0 0\n')
    env['TSFP_DISCORD_CONFIG'] = str(run / 'discord.ini')
    if args.gpu_timing: env['TSFP_GPU_TRACE']=str(run/(tag+'-gpu.csv'))
    if args.profile: env.update(RECOMP_WAIT_LOG='1',RECOMP_WAIT_PROFILE='1',
        RECOMP_SAMPLE='200',RECOMP_SAMPLE_REPORT='10',RECOMP_SAMPLE_DEPTH='8',
        RECOMP_SAMPLE_DUMP=str(run/(tag+'-sample.txt')))
    if args.legacy_projection_only: env['RECOMP_INTERP_AFFINE'] = 'none'
    if args.capture:
        env.update(RECOMP_INTERP_DUMP=str(run / tag), RECOMP_INTERP_DUMP_FROM=str(args.capture_from))
    lock = run / 'run.lock'
    with lock.open('x') as marker: marker.write(str(os.getpid()))
    process = None
    profiler = None
    threads = []
    def drain(pipe, path):
        with path.open('wb') as output:
            remaining = 4 * 1024 * 1024
            while chunk := pipe.read(4096):
                output.write(chunk[:remaining]); remaining = max(0, remaining-len(chunk))
    try:
        process = subprocess.Popen([str(ROOT / 'build/windows/Release/futureperfect_recomp.exe')],
            cwd=ROOT, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            creationflags=subprocess.CREATE_NO_WINDOW)
        print(f'Isolated {args.fps} FPS test PID {process.pid}, limit {args.seconds} seconds', flush=True)
        if args.presentmon:
            profiler=subprocess.Popen([str(args.presentmon.resolve()), '--process_id', str(process.pid),
                '--session_name', 'TSFP-'+str(process.pid), '--output_file', str(run/(tag+'-presentmon.csv')),
                '--no_console_stats', '--no_track_input', '--qpc_time_ms', '--timed', str(args.seconds),
                '--terminate_after_timed', '--terminate_on_proc_exit'],
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, creationflags=subprocess.CREATE_NO_WINDOW)
            thread=threading.Thread(target=drain,args=(profiler.stdout,run/(tag+'-presentmon.log')),daemon=True)
            thread.start();threads.append(thread)
        for pipe, suffix in [(process.stdout, '.out'), (process.stderr, '.err')]:
            thread = threading.Thread(target=drain, args=(pipe, run / (tag+suffix)), daemon=True)
            thread.start(); threads.append(thread)
        # Observe only the documented scene/flags in our own disposable child.
        kernel = ctypes.WinDLL('kernel32', use_last_error=True)
        kernel.OpenProcess.restype = ctypes.c_void_p
        kernel.ReadProcessMemory.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p]
        kernel.CloseHandle.argtypes = [ctypes.c_void_p]
        class MemoryCounters(ctypes.Structure):
            _fields_=[('cb',ctypes.c_ulong),('PageFaultCount',ctypes.c_ulong)]+[
                (name,ctypes.c_size_t) for name in ('PeakWorkingSetSize','WorkingSetSize',
                'QuotaPeakPagedPoolUsage','QuotaPagedPoolUsage','QuotaPeakNonPagedPoolUsage',
                'QuotaNonPagedPoolUsage','PagefileUsage','PeakPagefileUsage','PrivateUsage')]
        kernel.K32GetProcessMemoryInfo.argtypes=[ctypes.c_void_p,ctypes.POINTER(MemoryCounters),ctypes.c_ulong]
        kernel.GetProcessHandleCount.argtypes=[ctypes.c_void_p,ctypes.POINTER(ctypes.c_ulong)]
        handle = kernel.OpenProcess(0x410, False, process.pid)
        assert handle, 'Cannot inspect owned test child'
        def word(address):
            value = ctypes.c_uint32()
            assert kernel.ReadProcessMemory(handle, address+0x10000, ctypes.byref(value), 4, None)
            return value.value
        started=time.monotonic(); skipped=False; states=[];resources=[];next_resource=0
        try:
            while time.monotonic()-started < args.seconds:
                assert process.poll() is None, f'Game exited early: {process.returncode}'
                if profiler and profiler.poll() is not None:
                    assert profiler.returncode==0, 'PresentMon failed; see its bounded log'
                trace=run/(tag+'-presentmon.csv')
                assert not args.presentmon or not trace.exists() or trace.stat().st_size<16*1024*1024, 'PresentMon CSV exceeded 16 MiB'
                elapsed=time.monotonic()-started
                if args.resources and elapsed>=next_resource:
                    memory=MemoryCounters();memory.cb=ctypes.sizeof(memory);count=ctypes.c_ulong()
                    assert kernel.K32GetProcessMemoryInfo(handle,ctypes.byref(memory),memory.cb)
                    assert kernel.GetProcessHandleCount(handle,ctypes.byref(count))
                    resources.append(dict(seconds=round(elapsed,2),private_bytes=memory.PrivateUsage,
                        working_set_bytes=memory.WorkingSetSize,handles=count.value))
                    next_resource=elapsed+5
                if elapsed > 50:
                    scene, flags=word(0x79094C),word(0x7DE458)
                    if flags & 0x20000000 and not skipped:
                        (run / 'state.txt').write_text('27 1\n');time.sleep(.2)
                        (run / 'state.txt').write_text('0 0\n');skipped=True
                    pause_pointer=word(0x761C48)
                    paused=word(pause_pointer)&1 if pause_pointer else 0
                    states.append((round(elapsed,2),scene,flags,paused))
                time.sleep(.5)
        finally:
            kernel.CloseHandle(handle)
        import json
        (run / (tag+'-states.json')).write_text(json.dumps(states))
        if args.resources:(run/(tag+'-resources.json')).write_text(json.dumps(resources,indent=2))
    finally:
        if process is not None and process.poll() is None:
            process.terminate(); process.wait(timeout=10)
        if profiler is not None and profiler.poll() is None:
            try: profiler.wait(timeout=10)
            except subprocess.TimeoutExpired: profiler.terminate();profiler.wait(timeout=10)
        for thread in threads: thread.join(10)
        lock.unlink(); recycle_disks(ROOT, run)
    check_fixture(ROOT)
    if args.gpu_timing:
        import csv
        trace=run/(tag+'-gpu.csv')
        assert trace.exists(), 'D3D11 timing probe did not produce a CSV'
        with trace.open() as stream: samples=[r for r in csv.DictReader(stream) if None not in r.values()]
        assert any(r['gpu_valid']=='1' for r in samples), 'No valid GPU timestamp samples'
        print('GPU timing:', trace, 'complete samples:', len(samples),flush=True)
    if args.presentmon:
        trace=run/(tag+'-presentmon.csv')
        print('PresentMon:', str(trace) if trace.exists() and trace.stat().st_size else 'No frame events captured; GPU/display timing unavailable from ETW.',flush=True)
    log = (run / (tag+'.err')).read_text(errors='replace')
    assert '[CRASH]' not in log
    assert any(scene==2 and not flags & 0x20000000 for _,scene,flags,_ in states), 'Did not reach Story gameplay'
    if args.pause_check:
        assert any(80.5<t<82.5 and pause for t,_,_,pause in states), 'Pause not observed'
        assert any(t>84 and not pause for t,_,_,pause in states), 'Resume not observed'
    print('\n'.join(line for line in log.splitlines() if '[INTERP]' in line or '[FPS]' in line or '[PC-DISPLAY] Camera' in line))
    print('Evidence:', run / (tag+'.csv'))


if __name__ == '__main__': main()
