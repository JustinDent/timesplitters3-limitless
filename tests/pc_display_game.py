"""Exercise the actual guest Settings menu using recorded game input.

One reusable scratch directory; never touches personal saves/settings.
Captures only three rolling native frames (up to ~100 MiB at 4K).
"""
import os
from pathlib import Path
import subprocess
import time
from PIL import Image
import pc_settings_game as ui

ROOT=Path(__file__).resolve().parents[1]

def main():
    folder=ROOT/'private/display-validation'
    frames=folder/'frames'; frames.mkdir(parents=True,exist_ok=True)
    config=folder/'pc-display.ini'
    # Explicit baseline makes reruns deterministic without changing user files.
    config.write_text('TSFP_DISPLAY_V1 0 0\n')
    events=[]
    def key(at,vk): events.extend([(at,f'{at} key {vk} 1'),(at+180,f'{at+180} key {vk} 0')])
    for at in [15000,18000,23000,27000]: key(at,13)
    for at,vk in [(33000,38),(34000,38),(35000,13),(43000,40),(44000,40),(45000,13),
                  (48000,13),(50000,40),(51000,40),(52000,13), # apply native
                  (55000,38),(56000,13), # keep
                  (58000,13),(60000,40),(61000,40),(62000,13), # apply next
                  # let timeout revert
                  (80000,40),(81000,40),(82000,40),(83000,13)]: key(at,vk)
    replay=folder/'input.txt'; replay.write_text('\n'.join(t for _,t in sorted(events))+'\n')
    env={k:v for k,v in os.environ.items() if not k.startswith(('RECOMP_','TSFP_'))}
    env.update(RECOMP_GAME_DIR=str(ROOT/'private/game'),RECOMP_SAVE_DIR=str(folder/'saves'),
        RECOMP_INPUT_CONFIG=str(ROOT/'tests/fixtures/pc-controls.json'), TSFP_DISPLAY_CONFIG=str(config),
        RECOMP_TRACE_BUDGET='0', RECOMP_PC_MOUSE='1',RECOMP_WINDOW_BACKGROUND='1',
        TSFP_PC_INPUT_REPLAY=str(replay),RECOMP_HLE_D3D8_DUMP=str(frames/'frame'),
        RECOMP_HLE_D3D8_DUMP_EVERY='60',RECOMP_HLE_D3D8_DUMP_KEEP_LAST='24')
    exe=ROOT/'build/windows/Release/futureperfect_recomp.exe'
    shots=set()
    with (folder/'game.out').open('wb') as out,(folder/'game.err').open('wb') as err:
        p=subprocess.Popen([str(exe)],cwd=exe.parent,env=env,stdout=out,stderr=err)
        start=time.monotonic()
        try:
            game=ui.wait_window(p.pid,'TimeSplitters FP'); ui.user.ShowWindow(game,0)
            while time.monotonic()-start<86:
                assert p.poll() is None,'Game exited'
                files=sorted(frames.glob('*.bmp'),key=lambda x:x.stat().st_mtime_ns)
                # These are this test's own disposable frames; preserve only the newest three.
                for f in files[:-3]: f.unlink()
                elapsed=time.monotonic()-start
                for at,label in [(51,'picker'),(55.5,'confirmation'),(80,'reverted')]:
                    if elapsed>=at and label not in shots and files:
                        try:
                            with Image.open(files[-1]) as im: im.save(folder/f'{label}.png')
                            shots.add(label)
                        except OSError: pass # file is still being written
                time.sleep(.1)
        finally:
            if p.poll() is None: p.terminate()
            p.wait(timeout=5)
    log=(folder/'game.err').read_text(errors='replace')
    assert '[CRASH]' not in log and '[ICALL] unresolved' not in log
    for expected in ['added to main Settings','Opened,','Native scene target','[PC-DISPLAY] Saved','[PC-DISPLAY-MENU] Reverted','Returned to original menu']:
        assert expected in log,expected+'\n'+log[-8000:]
    print('PASS: actual guest menu, native scene resize, confirmation/save, timeout rollback, return')

if __name__=='__main__': main()
