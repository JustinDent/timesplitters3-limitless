# Shared PC and native Mac implementation

Ordinary features should be implemented once and built for both hosts. This
requires common behavior plus small adapters for each operating system or GPU
API. Git synchronization, a shared account and a backlog do not translate code.

## Current boundaries

| Area | Common implementation | Host-specific work |
| --- | --- | --- |
| Guest execution and gameplay fixes | Pinned runtime, complete patch stack, generated guest code | Build/compiler and process entry |
| Menu, camera and display policy | `pc_display*`, `pc_menu_*`, `pc_cutscene.h`, `pc_ui_layout.h` | OS pointer events, window/resolution application |
| Controls and configuration | `pc_input*`, `pc_bindings.inc`, shared validation | Win32/SDL events and atomic file writes |
| Debug settings | `pc_debug_options.def` generates fields, keys, labels and capability metadata | Win32/Cocoa controls, layout, log-folder opening |
| Menu reconstruction shader | `graphics/menu_sampling_shader.h` and `pc_menu_sampling.h` eligibility | HLSL/GLSL intrinsic/resource syntax and uniform/constant binding |
| Xbox graphics semantics | Shared HLE, format helpers and vertex decoder/interpreter | Still substantially duplicated between Windows `d3d8_*` and native `renderer_gl.c` |

The shared debug definition automatically exposes a new boolean setting in both
dialogs and persistence, unless its required capability is unavailable. Runtime
behavior for that setting must still be implemented. Mac currently lacks the
network-latency provider, so its dialog preserves that saved value without
offering an unsupported control. Existing keys and defaults remain compatible.

The menu shader has one algorithmic body. Changing its reconstruction or glyph
coverage rules changes both backends. Its language adapters must contain only
syntax/resource operations, not a second set of coefficients or feature rules.
Windows keeps the existing RHI renderer; Mac currently binds the same shader
logic through its GL adapter. This is an incremental step, not full RHI parity.

## Adding a feature

1. Put platform-independent state, validation and behavior in the common model
   or patch stack. Extend an existing shared entry point before creating a new
   host-specific implementation. Keep generated game C reproducible.
2. Extend adapters only for OS/GPU operations. Use capability checks for missing
   providers. An operating-system guard around portable logic requires a reason
   and an explicit adaptation/validation entry.
3. Add behavioral checks using the same inputs/expectations on both hosts.
   Register them in `.github/workflows/runtime-patches.yml`. Its suite selector
   includes source/test changes; PR checks always report even for unrelated
   paths. New tests do not run merely because a file has been created.
4. Run available native checks and record other-host checks as pending. Keep
   shared-source, compile, GPU, window/input and gameplay evidence distinct.
5. Update `MAC_PORT_BACKLOG.md` for remaining adapters/validation and `STATUS.md`
   for measured results. Record a platform exception rather than silently
   presenting partial functionality as complete cross-platform support.

Source-only CI needs no game assets. Menu sampling runs the same pixel contract
on Windows D3D11 WARP and macOS offscreen OpenGL. Debug model checks include adding
a definition-only option to a disposable fixture; native UI tests additionally
check generated labels, capability visibility and save/cancel/preservation.
The Mac menu translation-unit check catches missing internal callbacks that a
mocked policy test can miss. A configured workflow is not evidence that its
remote Windows job has run successfully.

Use the supported native builder and bounded isolated runner for local game
checks. Keep the 60 Hz guest clock and up-to-120 interpolated host presentation
target; preserve both display configs, controls, personal saves and the canonical
Test Lab fixture. See root `AGENTS.md` for commands and cleanup requirements.

## Remaining renderer consolidation

The Windows frontend already targets the pinned runtime's `rhi.h`, which accepts
HLSL shader source. The Mac GL renderer still implements much of the Xbox-facing
device/resource/shader behavior separately. Sharing a small shader library does
not make every future Windows renderer feature work on Mac automatically.

The next renderer milestone is to route native Mac through that common frontend
and graphics interface, retaining the validated GL path while the new adapter is
tested. Evaluate the existing RHI backends and shader compilation boundary before
choosing a Mac backend or adding dependencies. Any dependency change needs shared
pins, provenance, disk-use review and reproducible host setup. Do not silently
upgrade one device or replace a working renderer before its checks pass.

Then migrate remaining shader/resource behavior incrementally, using shared
asset-free contracts plus bounded scene checks. Cube/volume textures, movie
planes, framebuffer effects and advanced combiners need explicit acceptance;
first-menu success cannot certify them. Khallos sky fidelity, complete campaign
coverage, real texture-pack scenes and stable 120 host presents remain separate
validation gaps. Networking and launcher distribution also remain genuine host
integration work, tracked in the backlog.

## Local implementation milestones

| Focused branch | Dependency | Result |
| --- | --- | --- |
| `codex/shared-platform-baseline` | main `5131d29` plus native stack `f562e14` | `bee4ed9`: merged native build/regression baseline and backlog reconciliation |
| `codex/shared-menu-rendering` | `bee4ed9` | `dde016d`: one menu shader algorithm and GPU contract for both APIs |
| `codex/shared-debug-settings` | `dde016d` | `780346f`: one debug setting definition list, UI/persistence consumers and capability checks |
| `codex/shared-feature-contract` | `780346f` | Shared-by-default rules and this handoff |

These are local branches; they have not been merged into main or verified on the
other computer. Measured scope and unavailable checks are recorded in `STATUS.md`.
