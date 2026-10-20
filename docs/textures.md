# Texture overhaul and local preview

[Back to the README](../README.md)

The texture overhaul is in development: a modern reinterpretation of materials
and surfaces, using up to 4096×4096 for major surfaces and smaller sizes where
appropriate. It is not a completed full-game remaster, and the public download
does not include the private development artwork or original game assets.

See the [texture alpha overview](texture-alpha.md) for the current source scope,
local pack snapshot and remaining validation gates.

## Preview an installed local pack

From a Windows source checkout with a built runtime and an existing pack,
double-click **Play-Textures.cmd**. It loads replacements from
`private/texture-remaster/pack`; it does not download or generate a pack.
On the issue #74 source branch, F10 or **PC Settings > 4K Textures**
switches between cached originals and replacements live. This is separate from
render resolution. Restart after changing pack files. Ordinary **Play.cmd** starts
in Classic mode with the local pack available for switching.

For a native Mac launch:

```sh
./Play-Native-Mac.command --texture-pack private/texture-remaster/pack
```

The current shared loader accepts full-mip DXT1/DXT3/DXT5 and RGBA DDS
replacements with compatible alpha.
Missing, invalid or over-budget replacements fall back to original textures.
The active pack has a **6 GiB disk budget**, including metadata, and the loader
limits resident replacement textures to **512 MiB**. Source artwork and temporary
review files are separate from those limits.

## Progress and quality

Coverage depends on the locally installed pack and its matching source revision.
Local development batches are not a promise that a GitHub release contains them.
The original game remains the fallback wherever a replacement is unfinished.

Run this from the repository root to audit the local DDS files and refresh the
coverage and size plan:

```powershell
.\.venv\Scripts\python.exe tools/texture-worklist.py
```

Keep installed replacements, unfinished artwork and unapproved drafts distinct.
Projected pack sizes describe a plan, not completed coverage. Prompts and actual
source dimensions belong beside the artwork under
`private/texture-remaster/authored`; a DDS resampled to 4K is not native 4K artwork.

Offline checks cover hashes, dimensions, mip chains and storage. They do not
prove UV alignment, tiling, lighting or full-level consistency. Native Mac
replacement support has synthetic GPU coverage; visual validation of the real
pack remains pending. See [measured progress](../STATUS.md) for recorded checks.

Retain one active pack and one game extraction. Keep temporary texture outputs
below 256 MiB, and preserve selected source art, review evidence, saves and
settings. See [project storage rules](../AGENTS.md#project-storage-and-texture-work).

The worklist reports temporary logical and allocated bytes by directory. It
counts comparison reports and any new output subdirectory conservatively;
only `authored/`, the active `pack/`, and root-level catalog/provenance files
are excluded. At or above 256 MiB it writes its diagnostic report and exits
with status 2. Restore headroom before further production. A cleanup must
preserve selected visual evidence and verify any compressed or reproducible
replacement before recycling the redundant files.

## Hybrid artwork production

Issue #74 uses individual redesigns for prominent assets and faithful enhancement
for simpler materials. Text, UI, effects, reflections and character identity need
specialist review. This changes production priorities, not the audited scope.

Refresh the local audit and production queue from the repository root:

```powershell
.\.venv\Scripts\python.exe tools/texture-worklist.py
.\.venv\Scripts\python.exe tools/texture-hybrid-plan.py --batch-size 8
```

The planner writes ignored `private/texture-remaster/hybrid-plan.json`. It groups
visually classified environmental candidates by material and occurrence count,
with separate queues for individual work, specialist review, held corrections and
unclassified originals. Occurrence count is a prioritization hint, not proof of
visual prominence. The planner does not generate, approve or install artwork.

Inspect original pixels and alpha before authoring a ready batch. Use built-in
image generation with one request per asset and shared family constraints. Keep
secondary materials modest in size; reserve up to 4K for prominent surfaces.
Compile accepted candidates, review actual DDS contact sheets and repetition,
and run the pack audit. Defer failed drafts to focused correction batches instead
of repeatedly blocking the production queue. Retain exact prompts, source sizes,
resampling and review decisions locally.

The compiler requires an exact source/output aspect ratio by default. For a
reviewed opaque color source whose generated dimensions differ by at most one
pixel along the long axis, explicitly pass `--allow-one-pixel-aspect-rounding`.
Both source axes must be at least 512 pixels. The option records the source
rounding and LANCZOS resampling in metadata; it rejects larger differences,
non-opaque images, channel conversions and original-alpha overrides. It does
not crop or pad the image, and does not repair displaced UV details. Review
the actual encoded result before installation.

Keep one active pack under 6 GiB, including metadata, and temporary review/export
outputs under 256 MiB. Consolidated playtesting remains deferred until at least
90% of audited in-scope textures are installed and all categories are reviewed.
Neither batch routing nor successful DDS encoding proves visual completion.

## Preserve original alpha while reworking surface color

Some textures store cutouts or material data in alpha. When that channel should
remain unchanged, the compiler can take it from a verified original PNG:

```powershell
$alpha = 'private/texture-remaster/originals/TEXTURE_KEY.png'
$hash = (Get-FileHash -LiteralPath $alpha -Algorithm SHA256).Hash.ToLowerInvariant()
.\.venv\Scripts\python.exe tools/texture-pack.py private/texture-remaster/authored/ART.png --key TEXTURE_KEY --width 1024 --height 512 --format DXT5 --alpha-source $alpha --alpha-source-sha256 $hash
```

Use the actual catalog key, matching aspect ratio and reviewed output dimensions.
Verify the original export against its catalog payload before using it; the PNG
hash detects later changes but does not prove its game identity. This option
replaces authored alpha during format conversion, independently of color
resampling. It uses bilinear alpha resampling, records the original PNG hash,
dimensions and range, and requires color channels with DXT5 or RGBA8. Defaults
are unchanged. Do not apply it to an intentional new cutout shape or alpha-only
artwork. Review both channels and their alignment after compilation.

For a color-only cutout rework, an opaque color plate can avoid revealing bad
RGB hidden beneath generated transparency. Author natural material colors just
beyond the original cutout edges, keeping visible features registered, and use
the verified original mask for the final texture. This padding is source artwork;
the compiled texture still has the original transparency. Retain the source,
prompt and mask provenance. Do not fill missing color automatically or use the
mask to conceal a changed silhouette, atlas layout or gameplay marking.

Inspect the compiled result over a checkerboard and compare the full-size edges,
not only the transparent source preview. Very low-opacity source pixels can have
strong red/yellow RGB that becomes visible under a different mask. Batch 122
found that opacity-aware resizing alone did not remove this defect. A reviewed
opaque color plate resolved one wood cutout; this does not establish suitability
for other materials or approve their UV alignment. See the batch 122 entry in
`STATUS.md` for measured alpha error and the pending scene checks.

DXT5 alpha is lossy. Compare decoded alpha with the resampled original; use
RGBA8 at a suitable size when compression damages an important mask. Preserving
original alpha does not prove new color details align with it or establish how
the channel is used in a scene. Runtime validation remains part of the final gate.
