#requires -Version 5.1
<#
.SYNOPSIS
Prepares and builds a fresh TimeSplitters: Future Perfect PC checkout.
.DESCRIPTION
Requires Git, Python 3.12+, and Visual Studio 2022 with the C++ workload and
Windows SDK already installed. Runs Setup, Extract, Test, Generate, and Build
through tsfp.ps1, stopping at the first failure. Downloads require internet access.
.EXAMPLE
.\setup.ps1 -Python 'C:\Python312\python.exe' -Iso 'D:\Games\Future Perfect.iso'
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$Python,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$Iso,

    [ValidateSet('Debug', 'Release', 'RelWithDebInfo')]
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'

# Resolve inputs before tsfp.ps1 changes working directories.
if (!(Test-Path -LiteralPath $Python -PathType Leaf)) {
    throw "Python executable not found: $Python. Supply -Python with a path to Python 3.12 or newer."
}
if (!(Test-Path -LiteralPath $Iso -PathType Leaf)) {
    throw "Game ISO not found: $Iso. Supply -Iso with a path to your US Xbox ISO."
}
$pythonPath = (Resolve-Path -LiteralPath $Python).ProviderPath
$isoPath = (Resolve-Path -LiteralPath $Iso).ProviderPath
if (!(Get-Command git -CommandType Application -ErrorAction SilentlyContinue)) {
    throw 'Git was not found on PATH. Install Git and reopen PowerShell before running setup.'
}
& $pythonPath -c 'import sys; sys.exit(0 if sys.version_info >= (3, 12) else 1)'
if ($LASTEXITCODE -ne 0) {
    throw 'The supplied Python must run successfully and be version 3.12 or newer.'
}

$driver = Join-Path $PSScriptRoot 'tsfp.ps1'
$steps = @('Setup', 'Extract', 'Test', 'Generate', 'Build')
for ($index = 0; $index -lt $steps.Count; $index++) {
    $step = $steps[$index]
    Write-Host "[$($index + 1)/$($steps.Count)] $step"
    $arguments = @{ Action = $step; Configuration = $Configuration }
    if ($step -eq 'Setup') { $arguments.Python = $pythonPath }
    if ($step -eq 'Extract') { $arguments.Iso = $isoPath }
    try {
        & $driver @arguments
    }
    catch {
        throw "Fresh-checkout setup failed during ${step}: $($_.Exception.Message)"
    }
}

Write-Host "Setup complete. Executable: $(Join-Path $PSScriptRoot "build\windows\$Configuration\futureperfect_recomp.exe")"
if ($Configuration -eq 'Release') {
    Write-Host 'Launch with Play.cmd, or run .\tsfp.ps1 Run from the repository.'
}
else {
    Write-Host "Launch from the repository with .\tsfp.ps1 Run -Configuration $Configuration"
}
