"""Compile and execute the native update model and the actual guest hook."""
import os
from pathlib import Path
import subprocess
import sys

import pytest
from native_compiler import native_compiler

ROOT = Path(__file__).resolve().parents[1]

class ObjectRockingQuantization(AssertionError):
    """Measured repeated live transforms; fixture/build errors remain failures."""


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
    assert result['raw_seconds_per_wall_second']==1
    assert result['frame_ms']['p99']==6.9444
    slow=[dict(row,game_delta=row['game_delta']/4) for row in rows]
    assert summarize(slow,0)['game_seconds_per_wall_second']==.25
    assert summarize(slow,0)['raw_seconds_per_wall_second']==1
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


def test_capture_cleanup_preserves_other_evidence(tmp_path):
    from native_refresh_game import recycle_captures
    for i in range(24):
        (tmp_path/f'native-144-frame{i:03}.bmp').write_bytes(b'frame')
    other=tmp_path/'reference-60-frame000.bmp'
    other.write_bytes(b'reference')
    recycle_captures(tmp_path,'native-144')
    assert [p.name for p in sorted(tmp_path.glob('native-144-*'))]==[
        f'native-144-frame{i:03}.bmp' for i in (21,22,23)]
    assert other.read_bytes()==b'reference'
    for i in (720,1440,2160):
        (tmp_path/f'native-144-frame_s{i:08}.bmp').write_bytes(b'rolling')
    recycle_captures(tmp_path,'native-144')
    assert [p.name for p in sorted(tmp_path.glob('native-144-*'))]==[
        f'native-144-frame_s{i:08}.bmp' for i in (720,1440,2160)]
    with pytest.raises(ValueError):
        recycle_captures(tmp_path,'../reference-60')
    target=tmp_path/'outside.bmp'
    target.write_bytes(b'keep')
    link=tmp_path/'native-144-frame000.bmp'
    try:
        link.symlink_to(target)
    except OSError:
        return  # Link creation may require developer mode on Windows.
    with pytest.raises(ValueError):
        recycle_captures(tmp_path,'native-144')
    assert target.read_bytes()==b'keep'


@pytest.mark.parametrize('harness_name,ranges', [
    ('native_script_events.c', [(0x228870,0x228b35),(0x218910,0x218aa6),
                               (0x61570,0x6160f),(0x585e0,0x5861b),(0x59cf0,0x59d14)]),
    ('native_npc_dispatch.c', [(0x15eae0,0x15f0ca)]),
    ('native_player_dispatch.c', [(0xd5eb0,0xd683d)]),
    ('native_camera_motion.c', [(0xc73c0,0xc760e),(0xc7610,0xc77bc)]),
    ('native_camera_effects.c', [(0x151e80,0x151f1c),(0x1533e0,0x153431),
                               (0x153440,0x153490),(0xc79c0,0xc8114),
                               (0xcea8d,0xceaa6)]),
    ('native_camera_track.c', [(0xcd691,0xcdd3e)]),
    ('native_menu_motion.c', [(0x2dab30,0x2dab77),(0x2daf90,0x2db005),
                            (0x2da0e0,0x2da440)]),
    ('native_entity_velocity.c', [(0xa7360,0xa7666),(0xa7870,0xa79a2)]),
    ('native_weapon_sound.c', [(0x45390,0x453d0),(0x21780,0x217a2),
                              (0x21fb0,0x2201f),(0x1b6ff0,0x1b7258)]),
    ('native_weapon_cooldown.c', [(0x1c320c,0x1c3274)]),
    ('native_weapon_fire.c', [(0x45390,0x453d0),(0x21780,0x217a2),
                             (0x21fb0,0x2201f),(0x1b6ff0,0x1b7258),
                             (0x1b5660,0x1b567b),(0x1bc890,0x1bc988),
                             (0x244f30,0x24598e)]),
    ('native_npc_weapon_fire.c', [(0x45390,0x453d0),(0x21780,0x217a2),
                                 (0x21fb0,0x2201f),(0x1b6ff0,0x1b7258),
                                 (0x1b5660,0x1b567b),(0x1bb5c0,0x1bc64f),
                                 (0x174e0,0x17510),(0x17510,0x17531),
                                 (0x1b6790,0x1b67d5),(0x1b66e0,0x1b673f),
                                 (0x1b8530,0x1b8688),(0x11f780,0x11f7df),
                                 (0x45cf0,0x45d5e),(0xa5a60,0xa5aa9),
                                 (0x238510,0x23854c),(0x2382b0,0x23833b),
                                 (0x3533f0,0x3533f6),(0x358560,0x358590),
                                 (0x35de60,0x35de66)]),
    ('native_object_bounce.c', [(0xa84f0,0xa8a97),
                              (0x151650,0x15178f),(0x153550,0x1535a1),
                              (0x153650,0x1536a0),(0x153270,0x1533e0),
                              (0x45390,0x453d0),(0x21910,0x21967)]),
    ('native_mothership.c', [(0x217050,0x21862f),(0x2157b0,0x215982),
                           (0x1515e0,0x15163b),(0x151e80,0x151f1c),
                           (0x152090,0x15211d)]),
    ('native_ship_debris.c', [(0x2168c0,0x216ca5)]),
    pytest.param('native_object_rocking.c', [(0xa84f0,0xa8a97),
                                            (0x151650,0x15178f),(0x153550,0x1535a1),
                                            (0x153650,0x1536a0),(0x153270,0x1533e0)],
                 marks=pytest.mark.xfail(strict=True, raises=ObjectRockingQuantization,
                     reason='Object rocking remains quantized to carried whole ticks')),
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
    constants=''
    try:
        config.configure_from_xbe(str(xbe))
        if harness_name in ('native_mothership.c','native_ship_debris.c'):
            import re, struct
            # Original scalar constants referenced by the complete routines.
            # Include switch-table data; production generation reads this XBE too.
            from capstone import Cs, CS_ARCH_X86, CS_MODE_32
            addresses=set(range(0x218630,0x218648,4) if harness_name=='native_mothership.c'
                          else range(0x507d88,0x507db8,4))
            decoder=Cs(CS_ARCH_X86,CS_MODE_32)
            for start,end in ranges:
                offset=config.va_to_file_offset(start)
                for instruction in decoder.disasm(image[offset:offset+end-start],start):
                    for literal in re.findall(r'0x[0-9a-f]+',instruction.op_str):
                        address=int(literal,16)
                        if 0x470000<=address<0x4e0000: addresses.add(address)
            constants='\n'.join(
                f'MEM32(0x{address:x})=0x{struct.unpack_from("<I",image,config.va_to_file_offset(address))[0]:08x};'
                for address in sorted(addresses))
        if harness_name=='native_object_bounce.c':
            # The CRT register-argument adapter enters the asin kernel:
            # x / sqrt((1+x)*(1-x)), evaluated with FPATAN. Only this
            # mathematical service is replaced by host asin in the fixture.
            for address,expected in [
                (0x3c9df8,'83ec0cdd1424e87d300000e80d00000083c40cc3'),
                (0x3c9e29,'3d0000f03f732bd9e8d8c1d9e8d8e2dec9d9fad9f3'),
            ]:
                offset=config.va_to_file_offset(address)
                assert image[offset:offset+len(bytes.fromhex(expected))].hex()==expected
        if harness_name in ('native_object_rocking.c','native_object_bounce.c'):
            import struct
            constants='\n'.join(
                f'MEM32(0x{address:x})=0x{struct.unpack_from("<I",image,config.va_to_file_offset(address))[0]:08x};'
                for address in (0x475c78,0x475cc4,0x475cc8,0x475cd4,0x475cdc,
                                0x475ce8,0x475dbc,0x4783dc,0x47d3a4,0x47d564,
                                0x47d568,0x47d56c,0x47d570,0x47d574,0x47d578,
                                0x475d28))
        if harness_name=='native_weapon_sound.c':
            import struct
            constants='\n'.join(
                f'MEM32(0x{address:x})=0x{struct.unpack_from("<I",image,config.va_to_file_offset(address))[0]:08x};'
                for address in (0x475c78,0x475d28,0x475ccc,0x4785d0,0x475cd4,
                                0x47d1f8,0x478674,0x47d01c,0x47864c))
        if harness_name=='native_weapon_fire.c':
            import struct
            constants='\n'.join(
                f'MEM32(0x{address:x})=0x{struct.unpack_from("<I",image,config.va_to_file_offset(address))[0]:08x};'
                for address in (0x475c78,0x475ce8,0x475ccc,0x4785d0,0x475cd4,
                                0x47666c,0x47cfd8,0x475d10,0x475d28,
                                0x47d1f8,0x478674,0x47d01c,0x47864c))
        if harness_name=='native_npc_weapon_fire.c':
            import struct
            constants='\n'.join(
                f'MEM32(0x{address:x})=0x{struct.unpack_from("<I",image,config.va_to_file_offset(address))[0]:08x};'
                for address in (0x475c78,0x475ccc,0x475cd0,0x475cd4,0x475cdc,
                                0x475ce8,0x475d04,0x475d18,0x475d1c,0x475d24,
                                0x475db4,0x4765f4,0x4785d0,0x47d55c,0x47d670,
                                0x47e2b4,0x47e2b8,0x47e2bc,0x475d28,
                                0x47d1f8,0x478674,0x47d01c,0x47864c,0x475cc8,0x47d6a4))
        if harness_name=='native_camera_track.c':
            import struct
            # Execute this complete switch branch. Redirect only its three
            # exits to a terminal RET, rather than the enclosing camera tail.
            for site in (0xcd816,0xcd9d7,0xcdd38):
                offset=config.va_to_file_offset(site)
                assert image[offset]==0xe9
                assert site+5+struct.unpack_from('<i',image,offset+1)[0]==0xce18c
                image=image[:offset+1]+struct.pack('<i',0xcdd3d-site-5)+image[offset+5:]
            end=config.va_to_file_offset(0xcdd3d)
            assert image[end:end+5].hex()=='a17c0c7b00'
            image=image[:end]+b'\xc3'+image[end+1:]
        if harness_name=='native_camera_effects.c':
            # Isolate the original caller's load/test/branch/call/join. The
            # appended RET replaces only the first instruction after that
            # slice; all gate/call bytes and the complete callee stay original.
            start=config.va_to_file_offset(0xcea8d)
            end=config.va_to_file_offset(0xceaa5)
            assert image[start:end].hex()=='8b0d44794e0085c97e0e8d742414e8208fffffa1c00c7b00'
            image=image[:end]+b'\xc3'+image[end+1:]
        if harness_name=='native_weapon_cooldown.c':
            # Execute the complete deferred-action cooldown branch. All its
            # exits join here; replace only the following instruction by RET.
            end=config.va_to_file_offset(0x1c3273)
            assert image[end:end+6].hex()=='8b0d7c0c7b00'
            image=image[:end]+b'\xc3'+image[end+1:]
        functions={start:dict(end=end,name=f'sub_{start:08X}') for start,end in ranges}
        if harness_name=='native_weapon_fire.c':
            functions[0x244f30]['name']='sub_00244F30_gen'
        if harness_name=='native_npc_weapon_fire.c':
            functions[0x1bb5c0]['name']='sub_001BB5C0_gen'
        translator=FunctionTranslator(image,functions)
        code='\n'.join(f'void {info["name"]}(void);' for info in functions.values())+'\n'
        code+='\n'.join(translator.translate_function(start,info)
                       for start,info in functions.items())
    finally:
        vars(config).update(previous)
    harness=(ROOT/'tests'/harness_name).read_text()
    policy=f'#define RECOMP_NUMERIC_POLICY_HEADER "{(ROOT / "src/pc_native_numeric.h").as_posix()}"'
    output=compile_and_run(tmp_path,harness.replace('/* NUMERIC_POLICY */',policy)
                           .replace('/* FIRE_POLICY */',
                                    f'#include "{(ROOT / "src/pc_native_weapon_guest.h").as_posix()}"')
                           .replace('/* ORIGINAL_CONSTANTS */',constants)
                           .replace('/* GENERATED_ROUTINES */',code))
    if harness_name in ('native_mothership.c','native_ship_debris.c'):
        print(output)
    if harness_name=='native_object_rocking.c':
        rows=[dict(field.split('=',1) for field in line.split()[1:])
              for line in output.splitlines() if line.startswith('rocking_audit ')]
        assert len(rows)==6,output
        if any(int(r['frozen']) for r in rows if r['fps']!='60'):
            raise ObjectRockingQuantization(output)
    if harness_name=='native_npc_weapon_fire.c':
        rows=[dict(field.split('=',1) for field in line.split()[1:])
              for line in output.splitlines() if line.startswith('npc_fire_audit ')]
        assert len(rows)==6,output
        reference=next(r for r in rows if r['fps']=='60' and r['scoped']=='0')
        failures=[r for r in rows if r['scoped']=='1' and
                  (r['shots']!=reference['shots'] or r['sounds']!=reference['sounds'] or
                   abs(float(r['timer'])-float(reference['timer']))>.00001)]
        if failures:
            raise AssertionError(output)
    if harness_name=='native_weapon_fire.c':
        rows=[dict(field.split('=',1) for field in line.split()[1:])
              for line in output.splitlines() if line.startswith('fire_audit ')]
        assert len(rows)==12,output
        failures=[]
        for row in rows:
            if row['native']=='0' or row['open_gate']=='1':
                continue
            reference=next(r for r in rows if r['native']=='0' and
                           r['open_gate']=='0' and r['interval']==row['interval'])
            if (row['shots']!=reference['shots'] or
                abs(float(row['timer'])-float(reference['timer']))>.00001):
                failures.append(f"{row['fps']} FPS / {row['interval']} s interval: "
                                f"shots {row['shots']} vs {reference['shots']}, "
                                f"timer {row['timer']} vs {reference['timer']}")
        if failures:
            raise AssertionError('\n'.join(failures)+'\n'+output)
