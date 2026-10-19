"""Bounded actual guest Display/Smoothness menu test using the isolated Test Lab.

No desktop input is synthesized. Three rotating 720p BMPs and selected PNGs;
runtime/PC display, controls, Discord, save data and logs are isolated.
"""
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import threading
import time
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from testing_profile import prepare_run,recycle_disks,check_fixture,XBE_SHA256

def main():
    assert os.name=='nt'
    assert hashlib.sha256((ROOT/'private/game/default.xbe').read_bytes()).hexdigest()==XBE_SHA256
    run=ROOT/'private/test-saves/motion-menu'
    env=prepare_run(ROOT,run)
    runtime=run/'runtime.ini';runtime.write_text('# retain this comment\nfuture_option = keep\nframe_cap = 60\nframe_interp = 1\nframe_target = 0\nfullscreen = 0\nvrr = 1\n')
    display=run/'display.ini';display.write_text('TSFP_DISPLAY_V1 1280 720\n')
    original_display=display.read_bytes()
    (run/'discord.ini').write_text('TSFP_DISCORD_V1 0 0\n')
    sequence=[(5000,69),(10000,69),(14000,40),(18000,69),
              (21500,38),(22500,38),(23500,38),(24500,69),
              (27000,40),(28000,40),(29000,40),(30000,69)]
    sequence += [(at,40) for at in (32000,33000,34000,35000,36000)]
    sequence += [(37000,69),(40000,39),(41000,40),(42000,39),(43000,40),(44000,69),
                 (48000,8)]
    sequence += [(at,40) for at in (50000,51000,52000,53000,54000)]
    sequence += [(55000,69),(60000,39),(61000,8)]
    events=[]
    for at,key in sequence:events.extend([(at,f'key {key} 1'),(at+180,f'key {key} 0')])
    (run/'input.txt').write_text('\n'.join(f'{at} {event}' for at,event in sorted(events))+'\n')
    env.update(TSFP_PC_INPUT_REPLAY=str(run/'input.txt'),RECOMP_WINDOW_BACKGROUND='1',RECOMP_PC_MOUSE='1',
        TSFP_DISCORD_CONFIG=str(run/'discord.ini'),RECOMP_HLE_D3D8_DUMP=str(run/'frame'),
        RECOMP_HLE_D3D8_DUMP_EVERY='60',RECOMP_HLE_D3D8_DUMP_FROM='1500',RECOMP_HLE_D3D8_DUMP_KEEP_LAST='3')
    lock=run/'run.lock'
    with lock.open('x') as f:f.write(str(os.getpid()))
    process=None;threads=[];shots=set();saved=None
    def drain(pipe,path):
        with path.open('wb') as output:
            left=4*1024*1024
            while chunk:=pipe.read(4096):output.write(chunk[:left]);left=max(0,left-len(chunk))
    try:
        process=subprocess.Popen([str(ROOT/'build/windows/Release/futureperfect_recomp.exe')],cwd=ROOT,env=env,
            stdout=subprocess.PIPE,stderr=subprocess.PIPE,creationflags=subprocess.CREATE_NO_WINDOW)
        print('Isolated in-game settings PID',process.pid,flush=True)
        for pipe,suffix in [(process.stdout,'out'),(process.stderr,'err')]:
            t=threading.Thread(target=drain,args=(pipe,run/('menu.'+suffix)),daemon=True);t.start();threads.append(t)
        start=time.monotonic()
        while time.monotonic()-start<64:
            assert process.poll() is None,'Game exited early'
            elapsed=time.monotonic()-start
            if elapsed>=47 and saved is None:saved=runtime.read_bytes()
            for at,label in [(26,'settings'),(32,'display'),(39,'smoothness'),(47,'saved'),(59,'reopened')]:
                if elapsed>=at and label not in shots:
                    frames=sorted(run.glob('frame*.bmp'),key=lambda p:p.stat().st_mtime_ns)
                    if frames:
                        try:
                            with Image.open(frames[-1]) as im:im.save(run/(label+'.png'))
                            shots.add(label)
                        except OSError:pass
            time.sleep(.1)
    finally:
        if process is not None and process.poll() is None:process.terminate();process.wait(timeout=10)
        for t in threads:t.join(10)
        lock.unlink();recycle_disks(ROOT,run)
    check_fixture(ROOT)
    log=(run/'menu.err').read_text(errors='replace')
    assert '[CRASH]' not in log and '[ICALL] unresolved' not in log
    assert 'Selected page 0 row 6' in log and 'Selected page 3 row 2' in log,log[-10000:]
    assert saved and runtime.read_bytes()==saved,'Back changed saved preferences'
    text=saved.decode();assert 'frame_target = 120' in text and 'refresh_rate = 120' in text,text
    assert '# retain this comment' in text and 'future_option = keep' in text
    assert display.read_bytes()==original_display
    assert len(shots)==5
    for frame in run.glob('frame*.bmp'):
        assert not frame.is_symlink() and frame.resolve().parent==run.resolve()
        frame.unlink()
    print('PASS: guest Display/Smoothness menu, saved 120 FPS/Hz, reopened selection, Back and unrelated settings preservation')

if __name__=='__main__':main()
