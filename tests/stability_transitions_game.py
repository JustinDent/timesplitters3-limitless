"""Bounded, operator-steered integration session on an isolated Test Lab copy.

Append JSON lines to commands.jsonl: keys (virtual-key numbers), label, gap,
window (toggle/focus), restart, or stop. Inputs go to the game's replay hook;
window messages target only the child. Captures are renderer output, not scanout.
The entire session is limited to ten minutes, including process restarts.
"""
import argparse
import ctypes as c
import hashlib
import json
import os
import re
from pathlib import Path
import subprocess
import sys
import threading
import time
from PIL import Image
import pc_settings_game as ui
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from testing_profile import prepare_run,recycle_disks,check_fixture,XBE_SHA256,signature,BODY_SIZE

def weapon_set_metrics(run):
    """Check settled five-second guest-swap windows, excluding transitions.

    Captures still need review: counters alone cannot identify a menu screen.
    """
    report=json.loads((run/'report.json').read_text())
    samples=[(float(t),float(fps)) for t,fps in re.findall(
        r'\[FPS\] t=\s*([\d.]+)s\s+([\d.]+) fps',(run/'run0.err').read_text(errors='replace'))]
    result={}
    for item in report:
        label=item.get('command',{}).get('label')
        if label not in ('before-weapons','weapons','weapons-selection','after-weapons'):continue
        end=item['seconds']; wait=item['command'].get('wait',2.5)
        # A window ends at t and spans the previous five seconds. Leave two
        # seconds after input for assets/transitions before starting a window.
        values=[fps for t,fps in samples if end-wait+7<=t<=end]
        result[label]=values
    (run/'weapon-set-metrics.json').write_text(json.dumps(result,indent=2)+'\n')
    assert len(result)==4 and all(len(v)>=1 and min(v)>=50 for v in result.values()),result
    return result

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--run-name',default='stability-transitions')
    parser.add_argument('--scenario',choices=['weapon-set'])
    args=parser.parse_args()
    assert args.run_name.replace('-','').isalnum() and len(args.run_name)<48
    assert os.name=='nt'
    assert hashlib.sha256((ROOT/'private/game/default.xbe').read_bytes()).hexdigest()==XBE_SHA256
    run=ROOT/'private/test-saves'/args.run_name;env=prepare_run(ROOT,run)
    (run/'runtime.ini').write_text('frame_target = 144\nframe_cap = 60\nframe_interp = 1\nfullscreen = 0\nvrr = 1\n')
    (run/'display.ini').write_text('TSFP_DISPLAY_V1 1280 720\n')
    (run/'discord.ini').write_text('TSFP_DISCORD_V1 0 0\n')
    commands=run/'commands.jsonl';commands.write_text('')
    if args.scenario=='weapon-set':
        route=[dict(label='main-menu',wait=24),
            dict(keys=[40,69,69,69,69,38,38],gap=1.3,label='before-weapons',wait=12),
            dict(keys=[69],label='weapons',wait=20),
            dict(keys=[40,40,40],label='weapons-selection',wait=10),
            dict(keys=[70],label='after-weapons',wait=15),dict(stop=True)]
        commands.write_text(''.join(json.dumps(step)+'\n' for step in route))
    state=run/'input-state.txt';state.write_text('0 0\n')
    initial=[(5000,69),(10000,69),(14000,40),(18000,69)]
    replay=run/'input.txt';replay.write_text('\n'.join(f'{at+d} key {k} {v}' for at,k in initial for d,v in [(0,1),(180,0)])+'\n')
    env.update(TSFP_PC_INPUT_REPLAY=str(replay),TSFP_PC_INPUT_STATE=str(state),
        RECOMP_WINDOW_BACKGROUND='1',RECOMP_PC_MOUSE='1',TSFP_DISCORD_CONFIG=str(run/'discord.ini'),
        RECOMP_HLE_D3D8_DUMP=str(run/'frame'),RECOMP_HLE_D3D8_DUMP_EVERY='120',
        RECOMP_HLE_D3D8_DUMP_KEEP_LAST='3',RECOMP_FPS='5',RECOMP_INTERP_DEBUG='1')
    def profiles():
        result={}
        for profile in (run/'saves/UserData/45410066').glob('*/tsftrpft'):
            raw=profile.read_bytes()
            assert len(raw)==BODY_SIZE+20 and signature(raw[:BODY_SIZE])==raw[BODY_SIZE:]
            result[profile.parent.name]=hashlib.sha256(raw).hexdigest()
        assert result
        return result
    original=profiles();report=[];samples=[];threads=[];process=None;handle=None
    kernel=c.WinDLL('kernel32',use_last_error=True)
    kernel.OpenProcess.restype=c.c_void_p
    kernel.ReadProcessMemory.argtypes=[c.c_void_p,c.c_void_p,c.c_void_p,c.c_size_t,c.c_void_p]
    kernel.CloseHandle.argtypes=[c.c_void_p]
    user=ui.user;user.GetForegroundWindow.restype=c.c_void_p
    user.SetForegroundWindow.argtypes=[c.c_void_p]
    def word(address):
        value=c.c_uint32()
        if not kernel.ReadProcessMemory(handle,address+0x10000,c.byref(value),4,None):return None
        return value.value
    def snapshot():
        pointer=word(0x761C48)
        return dict(pid=process.pid,scene=word(0x79094C),flags=word(0x7DE458),
            paused=word(pointer)&1 if pointer else 0,mission=word(0x78AD48))
    def drain(pipe,path):
        with path.open('wb') as out:
            left=4*1024*1024
            while chunk:=pipe.read(4096):out.write(chunk[:left]);left=max(0,left-len(chunk))
    def launch(index):
        nonlocal process,handle,window
        state.write_text('0 0\n')
        process=subprocess.Popen([str(ROOT/'build/windows/Release/futureperfect_recomp.exe')],cwd=ROOT,env=env,
            stdout=subprocess.PIPE,stderr=subprocess.PIPE,creationflags=subprocess.CREATE_NO_WINDOW)
        handle=kernel.OpenProcess(0x410,False,process.pid);assert handle
        for pipe,ext in [(process.stdout,'out'),(process.stderr,'err')]:
            t=threading.Thread(target=drain,args=(pipe,run/f'run{index}.{ext}'),daemon=True);t.start();threads.append(t)
        window=ui.wait_window(process.pid,'TimeSplitters FP')
        print('Child',index,process.pid,flush=True)
    def close():
        nonlocal handle
        if process and process.poll() is None:
            user.PostMessageW(window,0x10,0,0)
            try:process.wait(10)
            except subprocess.TimeoutExpired:process.terminate();process.wait(10)
        if handle:kernel.CloseHandle(handle);handle=None
        for t in threads:t.join(5)
        threads.clear()
    def capture(label):
        assert label.replace('-','').isalnum() and len(label)<48
        assert (run/(label+'.png')).exists() or len(list(run.glob('*.png')))<48
        for _ in range(20):
            frames=sorted(run.glob('frame*.bmp'),key=lambda p:p.stat().st_mtime_ns)
            if frames:
                try:
                    with Image.open(frames[-1]) as im:im.save(run/(label+'.png'))
                    return
                except OSError:pass
            time.sleep(.1)
        raise AssertionError('No renderer capture')
    window=None;index=0;seen=0;start=time.monotonic();next_sample=0
    lock=run/'run.lock'
    with lock.open('x') as f:f.write(str(os.getpid()))
    try:
        launch(index)
        while time.monotonic()-start<600:
            assert process.poll() is None,'Unexpected game exit'
            elapsed=time.monotonic()-start
            if elapsed>=next_sample:
                item=dict(seconds=round(elapsed,2),**snapshot());samples.append(item)
                (run/'current.json').write_text(json.dumps(item));next_sample=elapsed+1
            lines=commands.read_text().splitlines();assert commands.stat().st_size<65536
            if seen<len(lines):
                command=json.loads(lines[seen]);seen+=1
                assert len(command.get('keys',[]))<=16 and 0.1<=command.get('gap',.7)<=4
                assert 0<=command.get('wait',2.5)<=30
                budget=len(command.get('keys',[]))*(.18+command.get('gap',.7))+command.get('wait',2.5)
                if elapsed+budget+(22 if command.get('restart') else 0)>600:break
                if command.get('stop'):break
                if command.get('restart'):
                    before=profiles();close()
                    report.append(dict(restart_saved_profiles=profiles(),before_exit_profiles=before,exit_code=process.returncode))
                    assert process.returncode==0,'Game did not exit cleanly'
                    index+=1;assert index<=3;launch(index)
                for key in command.get('keys',[]):
                    state.write_text(f'{key} 1\n');time.sleep(.18);state.write_text('0 0\n');time.sleep(command.get('gap',.7))
                if command.get('window')=='toggle':user.PostMessageW(window,0x104,13,1<<29)
                if command.get('window')=='focus':
                    user.ShowWindow(window,6);time.sleep(1)
                    lost=user.GetForegroundWindow()!=window
                    user.ShowWindow(window,9);user.SetForegroundWindow(window);time.sleep(1)
                    report.append(dict(focus_lost=lost,focus_restored=user.GetForegroundWindow()==window))
                time.sleep(command.get('wait',2.5))
                item=dict(seconds=round(time.monotonic()-start,2),command=command,**snapshot(),fullscreen=not bool(user.GetWindowLongW(window,-16)&0x00C00000))
                if 'label' in command:capture(command['label'])
                report.append(item);(run/'report.json').write_text(json.dumps(report,indent=2))
                print(json.dumps(item),flush=True)
            time.sleep(.1)
    finally:
        close();lock.unlink()
        (run/'states.json').write_text(json.dumps(samples))
        (run/'report.json').write_text(json.dumps(report,indent=2))
        recycle_disks(ROOT,run);check_fixture(ROOT)
        saved=profiles()
        (run/'saved-profiles.json').write_text(json.dumps(saved,indent=2))
        print('Profiles changed:',saved!=original,'Signatures valid; fixture preserved',flush=True)
        for frame in run.glob('frame*.bmp'):
            assert not frame.is_symlink() and frame.resolve().parent==run.resolve();frame.unlink()
    for log in run.glob('run*.err'):
        text=log.read_text(errors='replace')
        assert '[CRASH]' not in text and '[ICALL] unresolved' not in text,log
    assert process.returncode==0,'Final game exit was not clean'
    if args.scenario=='weapon-set':print('Weapon-set settled FPS:',weapon_set_metrics(run),flush=True)
    print('PASS: bounded session completed; inspect report and captures for scenario coverage',flush=True)

if __name__=='__main__':main()
