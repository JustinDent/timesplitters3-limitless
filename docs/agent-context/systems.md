# Subsystem entry points

<!-- packet:start -->
Root tracked policy: `src/pc_input*`, `pc_controls*`, `pc_bindings.inc` for input;
`pc_display*`, `pc_settings*`, `pc_refresh*` for display/configuration;
`pc_menu_*`, `pc_ui_layout.h`, `pc_cutscene.h`, `pc_camera.h` for menus/camera;
`pc_debug_options.def` for the shared debug fields/keys/labels/capabilities;
`src/graphics/` for common shader/math policy; `src/native/` for host adapters.
Patches connect these files to upstream; the generated guest files are not the
authoritative place to implement portable behavior.

Textures: root `src/texture_pack*`, `texture_mode*`, runtime `src/d3d/` and HLE
texture hooks; offline tooling `tools/texture-*`. Sanitized replacements now live
in `assets/textures` (DDS via Git LFS), audited/bundled by `tools/build_textures.py`;
private source art and original assets stay local. Networking: `src/pc_network*`,
`pc_online*`, runtime HLE/kernel patches and `tools/online.py`. Crash capture:
`src/pc_crash*`, `pc_hang_policy.h`, runtime crash hooks and launcher diagnostics.
Audio/movie timing is runtime `src/hle/hle_dsound*`, `hle_xmv*`, not launcher code.
Find the connecting patch by symbol/path using `tools/repo-context.py find`.
Consult `docs/shared-platform.md` for shared versus adapter responsibilities.
<!-- packet:end -->

| Area | Start here (tracked root unless noted) | Useful checks/docs |
| --- | --- | --- |
| Bindings, input prompts | `src/pc_controls.c`, `pc_input.c`, `pc_bindings.inc`, `pc_prompts.c` | `tests/pc_controls.c`, `tests/pc_prompts.c`, `docs/windows-settings.md` |
| Mouse/menu handling | `src/pc_menu_mouse_guest.inc`, `pc_menu_choices.inc`, `pc_menu_sliders.inc`, `pc_menu_team.inc` | `tests/test_menu_mouse.py`, `test_terminal_mouse.py`, `test_ui_layout.py` |
| Auto-look / camera / cutscene | `src/pc_auto_look_guest.inc`, `pc_camera.h`, `pc_cutscene.h`, `pc_ui_layout.h` | `tests/test_auto_look.py`, `test_cutscene_letterbox.py`, `test_camera_feed.py` |
| Display model and native UI | `src/pc_display.c`, `pc_display_*.inc`, `pc_settings.c`, `src/native/` | `tests/test_pc_display_modes.py`, `test_debug_settings_model.py` |
| GPU clear / screen copies / sampling | `src/pc_clear_rhi.inc`, `src/graphics/`, runtime `src/d3d/` | `tests/test_rhi_menu_sampling.py`, `tests/rhi/`, `docs/shared-platform.md` |
| Texture loading and formats | `src/texture_pack.c`, `texture_pack_format.h`, `texture_pack_io.h`, `texture_mode.c` | `tests/test_texture_runtime.py`, `tests/texture_pack_tools.py` |
| Texture production and audit | `tools/texture-worklist.py`, `tools/texture-hybrid-plan.py` | Root AGENTS texture rules, `docs/textures.md` |
| System Link / online / bots | `src/pc_network.c`, `pc_network_guest.inc`, `pc_network_bots*`, `pc_online.c`, `tools/online.py` | `tests/test_network_guest_dispatch.py`, `test_online.py`, `test_network_identity.py` |
| Discord presence | `src/pc_discord*` plus runtime integration patches | `tests/test_discord_presence.py`, `test_discord_macos.py` |
| Saves and profiles | Runtime save-routing patch; `tools/testing_profile.py`, `tools/test-native-mac.py` | `docs/testing-profile.md`, `tests/test_testing_profile.py` |
| Crash/hang diagnostics | `src/pc_crash.c`, `pc_hang_policy.h`, `tools/crash-symbols.py` | `tests/test_crash_capture.py`, `test_native_crash.py`, `test_crash_symbols.py` |
| Guest lift/runtime correctness | Runtime `tools/recomp/`, manual hooks, `tools/recomp-generation.py` | `tests/test_recomp_float_joins.py`, `test_recomp_vehicle_joins.py` |
| Audio/movie startup | Runtime `src/hle/` sound and XMV files | `tests/test_audio_timing.py`, `test_xmv_stop.py` |

Names in this table are entry points, not a complete call graph. Read the
applicable adapter plus one relevant test. Tests may encode historical migration
or compatibility behavior, so confirm current defaults in root instructions and
effective source. Preserve unknown/unsupported saved settings across hosts.

## State and data boundaries

`config/*.default.*` is tracked default configuration; `config/pc-controls.json`
and `config/pc-display.ini` are personal. `private/game` is the single extracted
game-data copy; `private/saves` is personal; `private/test-fixtures/unlocked` is
the canonical local fixture and must remain unchanged. Test runners copy it into
bounded disposable test roots. `private/texture-remaster/pack` is the one active
6-GiB-capped art-production pack; source art and original game assets stay ignored.
Sanitized compiled DDS/JSON replacements are tracked in `assets/textures` and
bundled with game releases; `tools/texture-repository.py` stages them with LFS,
`tools/build_textures.py` audits them. Leave pointers unhydrated for unrelated
source work. Packaging must reject missing LFS content, not silently omit it.

For installed layouts, use the launcher platform adapter instead of assuming
checkout names. Never include private data, generated guest code, credentials,
environment dumps or personal configuration in durable context. Store paths,
symbols, semantics and sanitized measured conclusions instead.

## Prompt text layout read scopes

`pc_prompt_guest.inc` reads through `pc_prompts_read_region`; the bounded
thread-local `pc_text_read_scope.h` cache uses existing OS VM query adapters.
It is valid only during synchronous text layout, with no mapping/protection
changes or event dispatch. Nested scopes retain intervals; the next outer scope
clears them. Scratch writes still use a fresh protection query. Scope wrapping,
label construction/drawing and nested prompt measurement through the shared
hooks in `pc_tutorial_ui.inc`, `pc_display_ui.inc` and `pc_prompt_guest.inc`.
`pc_display_menu.c` includes `pc_prompts.h` before the .inc callers to avoid
implicit declarations. Do not extend caches across menu ticks or frames.
Tests cover region boundaries/expiry and actual prompt-hook scratch ABI/fallbacks.
Issue #105 also requires the runtime late-frame pacing patch; caching alone
measured 30 FPS. See STATUS.md for measured acceptance and Mac limits.
