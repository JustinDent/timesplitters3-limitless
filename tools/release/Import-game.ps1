[CmdletBinding()]
param([string]$Iso, [string]$Folder, [switch]$ValidateOnly)
$ErrorActionPreference = 'Stop'
try {
    if ($Iso -and $Folder) { throw 'Choose either an ISO or an extracted folder.' }
    Add-Type -Path (Join-Path $PSScriptRoot 'ImportGame.cs')
    if (!$Iso -and !$Folder) {
        Add-Type -AssemblyName System.Windows.Forms
        $choice = [Windows.Forms.MessageBox]::Show(
            "Import your US Xbox game once. After import, the ISO is no longer needed.`n`nYes: choose an ISO`nNo: choose an extracted game folder",
            'TimeSplitters - First-time setup', 'YesNoCancel', 'Information')
        if ($choice -eq 'Cancel') { exit 2 }
        if ($choice -eq 'Yes') {
            $picker = New-Object Windows.Forms.OpenFileDialog
            $picker.Title = 'Select your TimeSplitters: Future Perfect US Xbox ISO'
            $picker.Filter = 'Xbox disc image (*.iso)|*.iso'
            try { if ($picker.ShowDialog() -ne 'OK') { exit 2 }; $Iso = $picker.FileName } finally { $picker.Dispose() }
        } else {
            $picker = New-Object Windows.Forms.FolderBrowserDialog
            $picker.Description = 'Select the extracted Xbox folder containing default.xbe, pak, and xmv.'
            try { if ($picker.ShowDialog() -ne 'OK') { exit 2 }; $Folder = $picker.SelectedPath } finally { $picker.Dispose() }
        }
    }
    if ($ValidateOnly) {
        if (!$Iso) { throw 'ValidateOnly requires -Iso.' }
        $count = [TsfpImport]::ValidateIso((Resolve-Path -LiteralPath $Iso).Path)
        Write-Output "Supported Xbox ISO: $count files."
    } else {
        $destination = Join-Path $PSScriptRoot 'Data'
        if ($Iso) { [TsfpImport]::ImportIso((Resolve-Path -LiteralPath $Iso).Path, $destination) }
        else { [TsfpImport]::ImportFolder((Resolve-Path -LiteralPath $Folder).Path, $destination) }
        Write-Output 'Game data imported. The original ISO or folder is no longer needed to play.'
    }
} catch { Write-Error $_ -ErrorAction Continue; exit 1 }
