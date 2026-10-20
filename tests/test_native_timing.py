"""Compile and execute the native update model and the actual guest hook."""
import os
from pathlib import Path
import subprocess
import sys

import pytest
from native_compiler import native_compiler

ROOT = Path(__file__).resolve().parents[1]


@pytest.mark.parametrize('source', ['native_timing.c', 'native_timing_guest.c'])
def test_native_timing(source, tmp_path):
    compiler, environment = native_compiler(tmp_path)
    runtime = ROOT / 'external/ts3FPrecomp/src'
    includes = [runtime, runtime / 'config']
    output = tmp_path / ('native-timing.exe' if os.name == 'nt' else 'native-timing')
    if os.name == 'nt':
        command = [compiler, '/nologo', '/std:c11', '/W4',
                   *['/I' + str(path) for path in includes], str(ROOT / 'tests' / source),
                   '/Fe:' + str(output), '/Fo:' + str(tmp_path / 'timing.obj')]
    else:
        command = [compiler, '-std=c11', '-Wall', '-Wextra',
                   *['-I' + str(path) for path in includes], str(ROOT / 'tests' / source),
                   '-lm', '-o', str(output)]
    subprocess.run(command, env=environment, cwd=tmp_path, check=True)
    subprocess.run([str(output)], env={k:v for k,v in os.environ.items()
                   if k != 'TSFP_NATIVE_TRACE'}, check=True, timeout=10)


@pytest.mark.parametrize('encoding,helper', [
    ('f30f2a0544794e00', 'RECOMP_I32_TO_F32'),
    ('f20f2a0544794e00', 'RECOMP_I32_TO_F64'),
    ('db0544794e00', 'RECOMP_I32_TO_F64'),
])
def test_generated_numeric_conversion(encoding, helper):
    sys.path.insert(0, str(ROOT / 'external/ts3FPrecomp'))
    from tools.recomp.disasm import Disassembler
    from tools.recomp.lifter import Lifter
    raw = bytes.fromhex(encoding)
    instructions = Disassembler().disassemble_function(raw, 0x1000, 0x1000 + len(raw))
    code = '\n'.join(Lifter().lift_instruction(instructions[0]))
    assert helper + '(0x4E7944,' in code
    assert 'MEM32(0x4E7944)' in code


def test_native_update_statistics():
    sys.path.insert(0,str(ROOT/'tools'))
    from native_update_stats import summarize
    rows=[dict(seconds=i/144,scene=2,flags=0,game_delta=60/144,raw_delta=60/144)
          for i in range(145)]
    result=summarize(rows,0)
    assert result['mean_update_fps']==144
    assert result['game_seconds_per_wall_second']==1
    assert result['frame_ms']['p99']==6.9444
    rows[72]['game_delta']=0
    assert summarize(rows,0)['intervals']==142
    rows.append(dict(seconds='1.1',scene=None,flags=None,game_delta=None,raw_delta=None))
    assert summarize(rows,0)['intervals']==142


def test_gameplay_checks_reject_frozen_opening():
    from native_refresh_game import gameplay_checks
    frozen=[dict(seconds=t,position=[1,2,3],
                 hand=dict(clip=836,time=[1.14885,1.14885])) for t in (86,90,100,110)]
    assert not any(gameplay_checks(frozen).values())
    assert not any(gameplay_checks([]).values())
    moving=[dict(seconds=t,position=[t-86,2,3],equipped=[0xffffffff,14],
                 ammo=[dict(type=1,reserve=100,loaded=120-t)],
                 hand=dict(clip=clip,time=[.2,.19]))
            for t,clip in [(86,832),(90,834),(100,838),(110,836)]]
    assert all(gameplay_checks(moving).values())
    for sample in moving:sample['equipped']=[0xffffffff,0]
    assert not gameplay_checks(moving)['opening_firearm_equipped']
    # A changing camera/actor does not establish advancing hand animation.
    for sample in moving:sample['hand']=None
    assert gameplay_checks(moving)==dict(replayed_movement=True,
        hand_animation_advanced=False,hand_animation_changed=False,
        opening_firearm_equipped=False,ammunition_consumed=True)
    for sample in moving:
        sample['ammo']=[dict(type=1,reserve=110-sample['seconds'],
                            loaded=sample['seconds'])]
    assert not gameplay_checks(moving)['ammunition_consumed']  # reload only


def test_opening_handoff_rejects_unarmed_natural_intro():
    from native_refresh_game import opening_firearm_check
    assert opening_firearm_check([])=={}
    samples=[dict(seconds=157.6,equipped=[0xffffffff,0xffffffff]),
             dict(seconds=180.6,equipped=[0xffffffff,37])]
    assert opening_firearm_check(samples)=={}
    samples.append(dict(seconds=299.7,equipped=[0xffffffff,37]))
    assert opening_firearm_check(samples)==dict(opening_firearm_equipped_at_end=False)
    samples[-1]['equipped'][1]=14
    assert opening_firearm_check(samples)==dict(opening_firearm_equipped_at_end=True)
    from native_refresh_game import gameplay_checks
    late=[dict(seconds=t,position=[t,0,0],equipped=[0xffffffff,weapon],
               hand=dict(clip=t,time=[.2,.1]))
          for t,weapon in [(50,37),(86,37),(95,14),(115,14)]]
    checks=gameplay_checks(late)
    checks.update(opening_firearm_check(late))
    assert checks['opening_firearm_equipped_at_end']
    assert not checks['opening_firearm_equipped']
    assert not all(checks.values())


def test_native_presents_require_real_guest_frames():
    from native_refresh_game import native_present_checks
    assert not any(native_present_checks('').values())
    valid='[PRESENT] backend=d3d11 total=3000 guest=3000 interpolated=0 other=0 skipped=0 failed=0'
    assert all(native_present_checks(valid).values())
    mixed=valid.replace('guest=3000','guest=2999').replace('interpolated=0','interpolated=1')
    assert not native_present_checks(valid+'\n'+mixed)['interpolation_disabled']
    assert not native_present_checks(valid.replace('failed=0','failed=1'))['present_failures_absent']
    assert not native_present_checks(valid.replace('3000','0'))['guest_presents_observed']

@pytest.mark.parametrize('harness_name,ranges', [
    ('native_script_events.c', [(0x228870,0x228b35),(0x218910,0x218aa6),
                               (0x61570,0x6160f),(0x585e0,0x5861b),(0x59cf0,0x59d14)]),
    ('native_npc_dispatch.c', [(0x15eae0,0x15f0ca)]),
    ('native_player_dispatch.c', [(0xd5eb0,0xd683d)]),
])
def test_original_fractional_dispatch(tmp_path, harness_name, ranges):
    import hashlib
    from test_recomp_float_joins import compile_and_run
    from tools.recomp import config
    from tools.recomp.translator import FunctionTranslator
    xbe=ROOT/'private/game/default.xbe'
    if not xbe.is_file():
        pytest.skip('Supported US XBE required for script-event integration')
    image=xbe.read_bytes()
    assert hashlib.sha256(image).hexdigest()=='3cfd001a84fc3e08175d6c4b2e42ebf49577a089b5d0bd87ac724f61a41816bc'
    previous=dict(vars(config))
    try:
        config.configure_from_xbe(str(xbe))
        functions={start:dict(end=end,name=f'sub_{start:08X}') for start,end in ranges}
        translator=FunctionTranslator(image,functions)
        code='\n'.join(translator.translate_function(start,info)
                       for start,info in functions.items())
    finally:
        vars(config).update(previous)
    harness=(ROOT/'tests'/harness_name).read_text()
    policy=f'#define RECOMP_NUMERIC_POLICY_HEADER "{(ROOT / "src/pc_native_numeric.h").as_posix()}"'
    compile_and_run(tmp_path,harness.replace('/* NUMERIC_POLICY */',policy)
                    .replace('/* GENERATED_ROUTINES */',code))
