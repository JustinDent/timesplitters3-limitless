# Runtime and patch navigation

<!-- packet:start -->
Runtime layers: generated Xbox guest calls title hooks in
`external/ts3FPrecomp/titles/futureperfect/src/recomp_manual.c`, HLE in
`external/ts3FPrecomp/src/hle/`, kernel/NV2A services in `src/kernel/`, then Xbox
graphics frontend in `src/d3d/d3d8_*.c` and RHI backends. Those short `src/...`
paths are RUNTIME-relative, distinct from tracked root `src/pc_*.c`.
`hle_d3d8.c` owns Swap/Present integration; `kernel_bridge.c` and
`xbox_memory_layout.c` contain vblank/flip-gate plumbing. Start FPS investigation
there only when timing/trace evidence implicates it. Visible menu names may
not exist as strings in generated guest code; find a measured caller/address.

`tools/apply-patches.py:patch_records` defines effective order (not just filename
sort). The loader stages an unwind of the EXACT previous recorded patch bytes,
then applies the entire current top-level stack before updating runtime files.
Find its ledger with `git -C external/ts3FPrecomp rev-parse --absolute-git-dir`:
`tsfp-patches.json` lives THERE, not in root `.git` or the build folder.
A conflict "during unwind" means current runtime/recorded stack diverge; it
does not establish a bad new patch. Preserve edits and investigate the named
hunk; never delete the ledger, reset the runtime or remove dependency patches.
<!-- packet:end -->

## Source ownership and execution

| Layer | Actual location | Change/reproduce through |
| --- | --- | --- |
| Host policy, settings, input, shared helpers | Root `src/pc_*`, `src/graphics/`, `src/native/` | Tracked source; patched upstream CMake includes it |
| Runtime integration and title hooks | `external/ts3FPrecomp/titles/futureperfect/src/recomp_manual.c`, title `CMakeLists.txt` | A separate tracked patch |
| HLE graphics, audio, input, movies | Runtime `src/hle/hle_d3d8*.c`, `hle_dsound*.c`, `hle_input.c`, `hle_xmv*.c` | Tracked patch |
| Xbox kernel/thread/memory/NV2A | Runtime `src/kernel/`, `src/nv2a/` | Tracked patch; distinguish Win32 and native adapter paths |
| Xbox GPU frontend / host backend | Runtime `src/d3d/d3d8_*.c`, `rhi*.c` | Tracked patch plus shared root shader policy |
| Generated guest functions | Runtime `titles/futureperfect/src/recomp/gen/` | Generator/manual hooks; NEVER direct edits |
| Lifter and symbol data | Runtime `tools/recomp/`, `scripts/recompile.py`, `config/extra_symbols/` | Reproducible generator patch, regenerate |
| Build products | `build/windows/`, `build/macos/` | Supported builder; not an assumed location for guest C |

Generated functions often have address-based names. Do not infer a menu handler
from a plausible label. Search only the evidenced address/symbol, inspect bounded
lines and trace the actual caller. `recomp_manual.c` wraps selected functions;
generated declarations are in `recomp_funcs.h`. Use
`python tools/recomp-generation.py external/ts3FPrecomp` to check the fingerprint.
New HLE declarations/manual wrapper names can require generation even when a
small function-body change only needs compilation. Follow the builder's check.

## Patch lifecycle

Top-level `patches/*.patch` is the active stack. The legacy subdirectory contains
migration inputs only. The loader moves networking ahead of native boot, build
identity after Discord, and the presentation integration series to the end;
consult the generated ordered index instead of guessing from the number of z's.
The `-fixed-60-fps.patch` is the tail of that series. Earlier 144-FPS,
presentation-timing, recovery and shared-presentation patches remain structural
dependencies even though higher-FPS runtime behavior is disabled/removed.

Create a new patch against the **correct preceding effective stack**, validate
hunk counts/context with the complete loader and transition tests. A dry-run of
one patch against pristine upstream does not validate integration. Adding a new
patch whose assumptions depend on the fixed-60 tail may need an explicit loader
ordering change and tests; a longer filename alone is insufficient.

On a conflict, record upstream HEAD, dirty runtime paths, ledger location,
reported phase/patch/hunk and proposed recovery. The loader uses a small temporary
source tree, not a duplicate game extraction. It preserves unrelated changes and
checks for concurrent modifications. Escalate a specific provenance/recovery
question when needed; repeated application with unchanged bytes cannot fix a
conflict. Do not alter MSVC flags to compensate for an unexplained patch failure.

## Renderer and timing boundaries

Common Xbox frontend: `d3d8_device.c`, `d3d8_resources.c`, `d3d8_states.c`,
`d3d8_shaders.c`, `d3d8_screencopy.c`, `d3d8_display.c`; backend selection through
`rhi.c`, `rhi_d3d11.c`, `rhi_vulkan.c`. Legacy Mac GL remains separate. Shared
algorithms live in root `src/graphics/` and settings in `src/pc_debug_options.def`.
See [shared-platform](../shared-platform.md) for adapter and GPU contract details.

Original rendering defaults to 60 and the console interrupt clock remains 60 Hz.
At `c932bee`, launcher opt-in native 120/144 beta settings require a compatible
game build; that selector does not prove the current runtime supports them.
Historical interpolation names and tests do not restore the retired engine. API presentation
counts are not GPU completion or display scanout. To investigate slowdowns,
compare stable/slow/stable timing, distinguish CPU work, GPU work, synchronization
and intentional waits, then run one discriminating intervention. Flip-gate
involvement in #105 remains a hypothesis until a reproduced measurement proves it.
