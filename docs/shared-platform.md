# Shared PC and native Mac implementation

Ordinary features should be implemented once and built for both hosts. This
requires common behavior plus small adapters for each operating system or GPU
API. Git synchronization, a shared account and a backlog do not translate code.

## Current boundaries

| Area | Common implementation | Host-specific work |
| --- | --- | --- |
| Launcher pages and theme | `Launcher.xaml`, `*Page.cs`, `PageTheme.cs`, `PageTheme.xaml`, `TabsTheme.xaml` | `Ui.cs` maps WPF/Avalonia events and controls; native templates/text rendering, dialogs, process lifetime and file opening |
| Guest execution and gameplay fixes | Pinned runtime, complete patch stack, generated guest code | Build/compiler and process entry |
| Menu, camera and display policy | `pc_display*`, `pc_menu_*`, `pc_cutscene.h`, `pc_ui_layout.h` | OS pointer events, window/resolution application |
| Controls and configuration | `pc_input*`, `pc_bindings.inc`, shared validation | Win32/SDL events and atomic file writes |
| Debug settings | `pc_debug_options.def` generates fields, keys, labels and capability metadata | Win32/Cocoa controls, layout, log-folder opening |
| Menu reconstruction shader | `graphics/menu_sampling_shader.h` and `pc_menu_sampling.h` eligibility | HLSL/GLSL intrinsic/resource syntax and uniform/constant binding |
| Renderer window/events | `RhiHost` callbacks and drawable-size contract | Default Win32 host and SDL/Cocoa host; worker threads use cached physical dimensions |
| Xbox graphics semantics | `d3d8_*` device/resources/shaders/display in Windows and opt-in Mac Vulkan builds | SDL/Vulkan window and CoreText text bitmap; default Mac GL remains a separate fallback |

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

The common frontend now compiles on Mac in the asset-free contract, including
device, resources, state, shader compilation and display policy. `RhiHost` removes
Win32 event pumping and window sizing from that frontend. The current GL renderer
uses the same interface through `renderer_host_sdl.c`; callback registration is
outside the render-thread lifetime and drawable sizes are physical pixels.
The opt-in native Vulkan game now links this frontend; default GL remains available.

The existing Vulkan backend now has an SDL surface adapter and uses the same
loader as SDL. It compiles the PC HLSL through DXC and passes the shared menu
pixel contract, swapchain readback and presentation on this Mac. Portability
extensions are enabled only when advertised. This is the selected integration
route. The opt-in native game connects its Xbox frontend and host services;
normal builds still use GL while wider acceptance remains pending.

The asset-free probe uses `tools/test-rhi-mac.py` and the SDK pinned in
`config/vulkan-sdk.json`. Setup verifies the publisher's archive checksum and
installs only inside ignored `external/vulkan-sdk`; receipts verify the relevant
installed files on later runs. Licenses and version manifests remain with it.
The probe needs macOS 26+ with this SDK; this does not raise the current GL game's
requirement. Run `--setup-sdk` once, then `--driver moltenvk` or
`--driver kosmickrisp`. See [Mac guide](macos.md#common-renderer-development-probe).
The same `tests/rhi_menu_sampling.c` pixel test is registered for Windows D3D11
WARP, without a swapchain there. Remote Windows/CI execution is still pending.

The game can now be built with `tools/build-native-mac.py --renderer vulkan`.
Its SDL adapter creates the window and supplies physical dimensions/events;
CoreText rasterizes the common overlay text. The PC device/resource/shader,
combiner, screen-copy and display code is compiled directly into the Mac game.
The bounded runner selects the pinned GPU environment from `BUILD.json` while
preserving all isolated save/config paths. See the [developer commands](macos.md#opt-in-common-game-renderer).
First-mission evidence is scoped in [STATUS](../STATUS.md#common-xbox-renderer-on-native-mac-2026-10-19).
This is an opt-in migration stage, not a replacement of the validated default.

Interpolation now uses common replay-owned depth/stencil per source subresource,
with fresh history each replay and restoration of guest target bindings. The
shared RHI GPU contract runs on Mac Vulkan and is registered for Windows WARP;
the actual native Xbox device also checks automatic/custom target routing.
The common RHI and GL adapters use one bounded present-timing recorder. It counts
only accepted API presentations, separating guest/interpolated/other kinds and
retaining skipped/failed counts. Failed replay does not advance HLE present totals
or feed stale timing into its scheduler. These are CPU API-call timings, not
scanout measurements.
Next, verify fullscreen and settings application,
and compare targeted scenes before changing defaults.
Keep shared build pins; do not copy another OS's build tree.

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
| `codex/shared-feature-contract` | `780346f` | `1a650b5`: shared-by-default rules and this handoff |
| `codex/shared-renderer-host` | `1a650b5` | `e382e50`: common host interface, SDL adapter and both-host core compile contract |
| `codex/shared-vulkan-mac` | `e382e50` | `4d23a6d`: native common Vulkan backend, pinned SDK and asset-free RHI GPU contract |
| `codex/shared-native-renderer` | `4d23a6d` | `2c63fbf`: common Xbox frontend in an opt-in native game, text/window adapters and shader compatibility fixes |
| `codex/shared-interpolation-depth` | `2c63fbf` | `3e78f7c`: Shared replay depth/stencil isolation, target restoration and GPU/fault-injection contracts |
| `codex/shared-present-metrics` | `3e78f7c` | Shared API-acceptance counts, frame timing and interpolation failure handling |

These are local branches; they have not been merged into main or verified on the
other computer. Measured scope and unavailable checks are recorded in `STATUS.md`.

## Launcher changes that reach both builds

Edit the shared files under `tools/launcher` for ordinary launcher UI work:
`Launcher.xaml` defines Home, and `SettingsPage.cs`, `WhatsNewPage.cs`,
`GameVersionsPage.cs`, and `MaintenancePage.cs` define the other screens and their
interactions. Both builds compile every `*Page.cs` in this directory automatically.
`PageTheme.cs` defines cards and headings; `PageTheme.xaml`/`TabsTheme.xaml` contain
theme values. Mac generates its home/theme syntax at build time, including shared
button states and expander geometry. Do not edit the ignored generated AXAML.

The Mac `NativeNews`, `NativeVersions` and `NativeMaintenance` classes connect
shared pages to the native window's lifetime, clipboard, file dialogs and process
operations; they must not add alternate page layouts. `Ui.cs` contains framework
operations such as input capture, visibility, tooltips and scroll forwarding.
Rich-text rendering and native control templates remain framework adapters.
OS-specific startup/process integrations still need the corresponding host work;
editing a Windows-only adapter cannot automatically implement a Mac OS API.

Use the same shared models for feature rules and the same shared pages for buttons,
labels, spacing and page state. A new host operation belongs behind an adapter.
Build with `tools/build-launcher.ps1 -Test` on Windows and
`tools/build-mac-launcher.py --test` on Mac. Both execute
`tests/launcher_shared_ui.cs` against their actual UI framework. The layout/theme
propagation tests also mutate a disposable source to verify that a single label,
margin, button-padding, corner-radius or disabled-state edit reaches the Mac output.
The release workflow runs both builds before publishing one release's packages.
The Windows two-executable package and existing updater size limits are retained.

API-free Windows/Mac build and regression checks are described in
[launcher compatibility](launcher-compatibility.md). Both host tests remain
required evidence; CI does not generate missing OS adapters or replace visual review.
