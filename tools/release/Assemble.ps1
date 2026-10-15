[CmdletBinding()]
param([string]$OutputDirectory = $PSScriptRoot)
$ErrorActionPreference = 'Stop'
try {
    $manifest = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'parts.json') -Raw | ConvertFrom-Json
    $outputRoot = (Resolve-Path -LiteralPath $OutputDirectory).Path
    if ([IO.Path]::GetFileName($manifest.archive) -ne $manifest.archive) { throw 'Invalid archive filename.' }
    $destination = Join-Path $outputRoot $manifest.archive
    if (Test-Path -LiteralPath $destination) {
        if ((Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash -ne $manifest.sha256) {
            throw 'A different ZIP already exists. Move it elsewhere before assembling this download.'
        }
        Write-Output "The complete ZIP is already verified: $destination"
        exit 0
    }
    foreach ($part in $manifest.parts) {
        if ([IO.Path]::GetFileName($part.name) -ne $part.name) { throw 'Invalid part filename.' }
        $path = Join-Path $PSScriptRoot $part.name
        if (!(Test-Path -LiteralPath $path)) { throw "Missing $($part.name). Download every release asset into this folder first." }
        Write-Output "Checking $($part.name)..."
        if ((Get-Item -LiteralPath $path).Length -ne $part.bytes -or
            (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ne $part.sha256) {
            throw "$($part.name) is incomplete or corrupt. Download that part again."
        }
    }
    $drive = New-Object IO.DriveInfo ([IO.Path]::GetPathRoot($outputRoot))
    if ($drive.AvailableFreeSpace -lt ($manifest.bytes + 67108864)) { throw 'At least 3 GB free is needed to assemble the ZIP.' }
    $temporary = $destination + '.assembling-' + [Guid]::NewGuid().ToString('N')
    $output = [IO.File]::Open($temporary, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write)
    try {
        foreach ($part in $manifest.parts) {
            Write-Output "Joining $($part.name)..."
            $inputFile = [IO.File]::OpenRead((Join-Path $PSScriptRoot $part.name))
            try { $inputFile.CopyTo($output, 1048576) } finally { $inputFile.Dispose() }
        }
    } finally { $output.Dispose() }
    if ((Get-Item -LiteralPath $temporary).Length -ne $manifest.bytes -or
        (Get-FileHash -LiteralPath $temporary -Algorithm SHA256).Hash -ne $manifest.sha256) {
        throw "Combined ZIP failed verification. Temporary output retained at: $temporary"
    }
    [IO.File]::Move($temporary, $destination)
    Write-Output "Verified complete ZIP: $destination"
    Write-Output 'Right-click the ZIP, choose Extract All, then open the extracted folder and double-click Play.cmd.'
    Write-Output 'No ISO or game import is needed. Keep all files in the extracted game folder together.'
} catch { Write-Error $_ -ErrorAction Continue; exit 1 }
