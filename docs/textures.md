# Texture overhaul and local preview

[Back to the README](../README.md)

The texture overhaul is in development: a modern reinterpretation of materials
and surfaces, using up to 4096×4096 for major surfaces and smaller sizes where
appropriate. It is not a completed full-game remaster, and the public download
does not include the private development artwork or original game assets.

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
