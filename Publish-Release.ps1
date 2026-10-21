#requires -Version 5.1
<#
.SYNOPSIS
Build, validate, package and publish the VERSION on a clean, up-to-date main.
.DESCRIPTION
Requires the complete bundled repository textures. Run git lfs pull first.
Missing, incomplete or unhydrated textures stop preparation and publication.
.EXAMPLE
.\Publish-Release.ps1 -PrepareOnly
.EXAMPLE
.\Publish-Release.ps1
#>
[CmdletBinding()]
param([switch]$PrepareOnly, [switch]$Draft)
$ErrorActionPreference = 'Stop'
$python = Join-Path $PSScriptRoot '.venv\Scripts\python.exe'
if (!(Test-Path -LiteralPath $python)) { throw 'Run setup.ps1 first.' }
$releaseArguments = @((Join-Path $PSScriptRoot 'tools\publish-release.py'))
if ($PrepareOnly) { $releaseArguments += '--prepare-only' }
if ($Draft) { $releaseArguments += '--draft' }
$previousPreference = $ErrorActionPreference
try {
    $ErrorActionPreference = 'Continue'
    & $python @releaseArguments
    $releaseExitCode = $LASTEXITCODE
}
finally { $ErrorActionPreference = $previousPreference }
if ($releaseExitCode -ne 0) { throw "Release stopped (exit $releaseExitCode). See the error above; no existing release was overwritten." }
