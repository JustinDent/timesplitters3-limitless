"""Exercise the actual guest Settings menu using recorded game input.

One reusable scratch directory; never touches personal saves/settings.
Captures only three rolling native frames (up to ~100 MiB at 4K).
"""
import os
import argparse
from pathlib import Path
import subprocess
import tempfile
import shutil
import time
from PIL import Image

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root',type=Path,default=ROOT,help='Checkout containing the verified private/game')
    parser.add_argument('--exe',type=Path,default=None)
    args=parser.parse_args(); root=args.root.resolve()
    import hashlib
    assert hashlib.sha256((root/'private/game/default.xbe').read_bytes()).hexdigest()=='3cfd001a84fc3e08175d6c4b2e42ebf49577a089b5d0bd87ac724f61a41816bc'
    folder=root/'private/display-validation'
    frames=folder/'frames'; frames.mkdir(parents=True,exist_ok=True)
    # Fresh disposable save tree avoids existing profiles changing menu navigation.
    saves=Path(tempfile.mkdtemp(prefix='menu-saves-',dir=folder)).resolve()
    assert saves.parent==folder.resolve() and not saves.is_symlink()
    config=folder/'pc-display.ini'
    # Explicit baseline makes reruns deterministic without changing user files.
    config.write_text('TSFP_DISPLAY_V1 0 0\n')
    events=[]
    def key(at,vk): events.extend([(at,f'{at} key {vk} 1'),(at+180,f'{at+180} key {vk} 0')])
    for at in [15000,18000,23000,27000]: key(at,13)
    for at,vk in [(33000,38),(34000,38),(35000,13),(43000,40),(44000,40),(45000,13),
                  (48000,39),(49000,37),(50000,39),(50500,39),(51000,40),(51500,40),(52000,13), # right, left to Original, two right; apply 720x480
                  (55000,38),(56000,13), # keep
                  (58000,37),(59000,37),(60000,40),(61000,40),(62000,13), # preview Original
                  # let timeout revert
                  (80000,40),(80500,40),(81000,40),(81500,40),(83000,13)]: key(at,vk)
    replay=folder/'input.txt'; replay.write_text('\n'.join(t for _,t in sorted(events))+'\n')
    env={k:v for k,v in os.environ.items() if not k.startswith(('RECOMP_','TSFP_'))}
    controls=folder/'pc-controls.json';controls.write_bytes((root/'tests/fixtures/pc-controls.json').read_bytes())
    runtime=folder/'runtime.conf';runtime.write_text('render_scale = 1\nfullscreen = 0\n')
    env.update(RECOMP_GAME_DIR=str(root/'private/game'),RECOMP_SAVE_DIR=str(saves),
        RECOMP_INPUT_CONFIG=str(controls), RECOMP_DISPLAY_CONFIG=str(runtime), TSFP_DISPLAY_CONFIG=str(config),
        RECOMP_TRACE_BUDGET='0', RECOMP_PC_MOUSE='1',RECOMP_WINDOW_BACKGROUND='1',
        TSFP_PC_INPUT_REPLAY=str(replay),RECOMP_HLE_D3D8_DUMP=str(frames/'frame'),
        RECOMP_HLE_D3D8_DUMP_EVERY='60',RECOMP_HLE_D3D8_DUMP_KEEP_LAST='3')
    exe=(args.exe or root/'build/windows/Release/futureperfect_recomp.exe').resolve()
    shots=set()
    with (folder/'game.out').open('wb') as out,(folder/'game.err').open('wb') as err:
        p=subprocess.Popen([str(exe)],cwd=exe.parent,env=env,stdout=out,stderr=err)
        start=time.monotonic()
        try:
            while time.monotonic()-start<86:
                assert p.poll() is None,'Game exited'
                files=sorted(frames.glob('*.bmp'),key=lambda x:x.stat().st_mtime_ns)
                # These are this test's own disposable frames; preserve only the newest three.
                for f in files[:-3]: f.unlink()
                elapsed=time.monotonic()-start
                for at,label in [(49.5,'original-picker'),(51,'picker'),(54,'confirmation'),(57,'saved'),(79,'reverted')]:
                    if elapsed>=at and label not in shots and files:
                        try:
                            with Image.open(files[-1]) as im: im.save(folder/f'{label}.png')
                            shots.add(label)
                        except OSError: pass # file is still being written
                time.sleep(.1)
        finally:
            if p.poll() is None: p.terminate()
            p.wait(timeout=5)
            # Only this run's newly-created fixture; never the reusable/user saves.
            assert saves.parent==folder.resolve() and saves.name.startswith('menu-saves-')
            assert not saves.is_symlink() and not any(p.is_symlink() for p in saves.rglob('*'))
            shutil.rmtree(saves)
    log=(folder/'game.err').read_text(errors='replace')
    assert '[CRASH]' not in log and '[ICALL] unresolved' not in log
    for expected in ['added to main Settings','Opened,','Native scene target','[PC-DISPLAY] Saved','[PC-DISPLAY-MENU] Reverted','Returned to original menu']:
        assert expected in log,expected+'\n'+log[-8000:]
    assert '[PC-DISPLAY] Saved 720x480' in log
    assert '[PC-DISPLAY] Applied 0x0' in log
    assert config.read_text()=='TSFP_DISPLAY_V1 720 480\n'
    print('PASS: actual guest menu, native scene resize, confirmation/save, timeout rollback, return')

if __name__=='__main__': main()
