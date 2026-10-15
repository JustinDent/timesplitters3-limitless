# Background entry point for Play.cmd; the existing launcher owns game logging.
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Script,
    [switch]$Run
)

$ErrorActionPreference = 'Stop'
try {
    $arguments = @{}
    if ($Run) { $arguments.Action = 'Run' }
    $global:LASTEXITCODE = 0
    & $Script @arguments
    if ($LASTEXITCODE -ne 0) { throw "The game exited with code $LASTEXITCODE." }
}
catch {
    $message = $_.Exception.Message
    $logRoot = Join-Path (Split-Path -Parent $Script) 'Logs'
    try {
        New-Item -ItemType Directory -Force -Path $logRoot | Out-Null
        $logPath = Join-Path $logRoot ('launch-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff') + '.err')
        $_ | Out-String | Set-Content -LiteralPath $logPath -Encoding UTF8
        $message += "`n`nDetails: $logPath"
    }
    catch { $message += "`n`nThe launcher could not write its error log." }
    Add-Type -AssemblyName System.Windows.Forms
    [Windows.Forms.MessageBox]::Show($message, 'TimeSplitters - Launch failed', 'OK', 'Error') | Out-Null
    exit 1
}
