[CmdletBinding()]
param(
    [ValidateSet('Status', 'Setup', 'Extract', 'Test', 'Generate', 'Build', 'Run', 'SmokeTest')]
    [string]$Action = 'Status',
    [string]$Iso,
    [string]$Python,
    [string]$InputSequence,
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo')]
    [string]$Configuration = 'Release',
    [int]$Seconds = 30
)

$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
$toolRoot = Join-Path $projectRoot 'external\ts3FPrecomp'
$privateRoot = Join-Path $projectRoot 'private'
$gameRoot = Join-Path $privateRoot 'game'
$logRoot = Join-Path $projectRoot 'logs'
$envPython = Join-Path $projectRoot '.venv\Scripts\python.exe'
$cmake = Join-Path $projectRoot '.venv\Scripts\cmake.exe'
$buildRoot = Join-Path $projectRoot 'build\windows'
$exe = Join-Path $buildRoot "$Configuration\futureperfect_recomp.exe"
$revision = 'd90c92988a9f99d0580d2ca3f7efd6ec3a7b7845'
$expectedHash = '3CFD001A84FC3E08175D6C4B2E42EBF49577A089B5D0BD87AC724F61A41816BC'

function Invoke-Checked {
    param([string]$Program, [string[]]$Arguments)
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed (exit $LASTEXITCODE)." }
}

function Assert-Game {
    $xbe = Join-Path $gameRoot 'default.xbe'
    if (!(Test-Path -LiteralPath $xbe)) { throw 'Run Extract with the path to your Xbox ISO first.' }
    if ((Get-FileHash -LiteralPath $xbe -Algorithm SHA256).Hash -ne $expectedHash) {
        throw 'This executable does not match the supported US Xbox build. Inspect it before regenerating.'
    }
}

function Apply-LocalPatches {
    foreach ($patch in Get-ChildItem -LiteralPath (Join-Path $projectRoot 'patches') -Filter '*.patch') {
        $previousErrorPreference = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            & git -C $toolRoot apply --reverse --check --quiet $patch.FullName 2>$null
            $alreadyApplied = $LASTEXITCODE -eq 0
        }
        finally { $ErrorActionPreference = $previousErrorPreference }
        if ($alreadyApplied) { continue }
        Invoke-Checked 'git' @('-C', $toolRoot, 'apply', '--check', $patch.FullName)
        Invoke-Checked 'git' @('-C', $toolRoot, 'apply', $patch.FullName)
    }
}

New-Item -ItemType Directory -Force -Path $privateRoot, $logRoot | Out-Null
$controlsPath = Join-Path $projectRoot 'config\pc-controls.json'
if (!(Test-Path -LiteralPath $controlsPath)) {
    Copy-Item -LiteralPath (Join-Path $projectRoot 'config\pc-controls.default.json') -Destination $controlsPath
}
Push-Location $projectRoot
try {
    if ($Action -eq 'Setup') {
        if (!(Test-Path -LiteralPath $envPython)) {
            if (!$Python) { throw 'Supply -Python with the full path to Python 3.12 or newer.' }
            Invoke-Checked $Python @('-m', 'venv', (Join-Path $projectRoot '.venv'))
        }
        Invoke-Checked $envPython @('-m', 'pip', 'install', '-r', (Join-Path $projectRoot 'requirements.txt'))
        if (!(Test-Path -LiteralPath $toolRoot)) {
            Invoke-Checked 'git' @('clone', 'https://github.com/nemonicicon/ts3FPrecomp.git', $toolRoot)
            Invoke-Checked 'git' @('-C', $toolRoot, 'checkout', '--detach', $revision)
        }
        $actualRevision = & git -C $toolRoot rev-parse HEAD
        if ($LASTEXITCODE -ne 0 -or $actualRevision -ne $revision) { throw 'Tool checkout differs from the pinned revision.' }
        Apply-LocalPatches
        Invoke-Checked 'git' @('-C', $toolRoot, 'submodule', 'update', '--init', '--depth', '1',
            'third_party/SDL', 'third_party/XbSymbolDatabase')
        $db = Join-Path $toolRoot 'third_party\XbSymbolDatabase'
        Invoke-Checked $cmake @('-S', $db, '-B', "$db\build", '-G', 'Visual Studio 17 2022', '-A', 'x64',
            '-DXBSDB_BUILD_UNITTEST=OFF')
        Invoke-Checked $cmake @('--build', "$db\build", '--config', 'Release', '--target', 'XbSymbolDatabaseCLI', '--parallel', '4')
        & (Join-Path $projectRoot 'tools\fetch-video.ps1')
    }
    elseif ($Action -eq 'Status') {
        Write-Output "Tool checkout: $toolRoot"
        Write-Output "Python ready: $(Test-Path -LiteralPath $envPython)"
        Write-Output "Game extracted: $(Test-Path -LiteralPath (Join-Path $gameRoot 'default.xbe'))"
        Write-Output "Windows build: $(Test-Path -LiteralPath $exe)"
        if (Test-Path -LiteralPath (Join-Path $gameRoot 'default.xbe')) {
            Assert-Game
            Write-Output 'Executable hash matches the US Xbox build (45410066).'
        }
    }
    else {
        if (!(Test-Path -LiteralPath $envPython)) { throw 'Run Setup first.' }
        Push-Location $toolRoot
        try {
            switch ($Action) {
                'Extract' {
                    if (!$Iso) { throw 'Supply -Iso with the full path to your Xbox ISO.' }
                    $isoPath = (Resolve-Path -LiteralPath $Iso).Path
                    Invoke-Checked $envPython @('-m', 'tools.xiso', 'unpack', $isoPath, '-o', $gameRoot)
                    Assert-Game
                    @{ iso = $isoPath; xbe_sha256 = $expectedHash } | ConvertTo-Json |
                        Set-Content -LiteralPath (Join-Path $privateRoot 'disc.json') -Encoding UTF8
                }
                'Test' {
                    Apply-LocalPatches
                    Invoke-Checked $envPython @('-m', 'pytest', 'tools', '-q', '--tb=short')
                    Invoke-Checked $envPython @('-m', 'tools.conformance')
                }
                'Generate' {
                    Assert-Game
                    $xbe = Join-Path $gameRoot 'default.xbe'
                    Invoke-Checked $envPython @('-m', 'tools.xdk_symbols', $xbe)
                    Invoke-Checked $envPython @('scripts/recompile.py', $xbe,
                        '--work-dir', (Join-Path $privateRoot 'pipeline'), '--project', 'titles/futureperfect',
                        '--game-name', 'TimeSplitters: Future Perfect', '--trace-all-entries')
                }
                'Build' {
                    Assert-Game
                    Apply-LocalPatches
                    if (!(Test-Path -LiteralPath 'titles/futureperfect/src/recomp/gen/recomp_funcs.h')) {
                        throw 'Run Generate first.'
                    }
                    Invoke-Checked $cmake @('-S', 'titles/futureperfect', '-B', $buildRoot,
                        '-G', 'Visual Studio 17 2022', '-A', 'x64',
                        "-DTSFP_LOCAL_PATH_TEST_SOURCE=$projectRoot/tests/save_paths.c",
                        "-DTSFP_PC_CONTROLS_DIR=$projectRoot/src")
                    Invoke-Checked $cmake @('--build', $buildRoot, '--config', $Configuration,
                        '--target', 'futureperfect_recomp', 'tsfp_save_paths_test', 'tsfp_pc_controls_test', 'tsfp_pc_settings_test', 'tsfp_pc_debug_test', 'tsfp_pc_prompts_test', '--parallel', '4')
                    $testRoot = Join-Path $projectRoot 'build\path-tests'
                    $testConfig = Join-Path $projectRoot 'tests\fixtures\pc-controls.json'
                    New-Item -ItemType Directory -Force -Path $testRoot | Out-Null
                    Invoke-Checked (Join-Path $buildRoot "$Configuration\tsfp_pc_prompts_test.exe") @($testConfig)
                    Invoke-Checked (Join-Path $buildRoot "$Configuration\tsfp_save_paths_test.exe") @($testRoot)
                    Invoke-Checked (Join-Path $buildRoot "$Configuration\tsfp_pc_controls_test.exe") @(
                        $testConfig, $testRoot)
                    Invoke-Checked (Join-Path $buildRoot "$Configuration\tsfp_pc_settings_test.exe") @(
                        $testConfig, $testRoot)
                    Invoke-Checked (Join-Path $buildRoot "$Configuration\tsfp_pc_debug_test.exe") @(
                        $testConfig, $testRoot)
                    # Only successful tests reach cleanup; retain failures for diagnosis.
                    $cleanupPath = (Resolve-Path -LiteralPath $testRoot).ProviderPath
                    $expectedTestRoot = Join-Path ([IO.Path]::GetFullPath($projectRoot)) 'build\path-tests'
                    if ($cleanupPath -ne $expectedTestRoot -or
                        ((Get-Item -LiteralPath $cleanupPath).Attributes -band [IO.FileAttributes]::ReparsePoint)) {
                        throw "Refusing to clean an unexpected test directory: $cleanupPath"
                    }
                    Remove-Item -LiteralPath $cleanupPath -Recurse -Force
                    Write-Output 'Build tests passed; temporary build/path-tests data removed.'
                }
                { $_ -in 'Run', 'SmokeTest' } {
                    Assert-Game
                    if (!(Test-Path -LiteralPath $exe)) { throw 'Run Build first.' }
                    $previousGameDir = $env:RECOMP_GAME_DIR
                    $previousSaveDir = $env:RECOMP_SAVE_DIR
                    $previousDump = $env:RECOMP_HLE_D3D8_DUMP
                    $previousDumpEvery = $env:RECOMP_HLE_D3D8_DUMP_EVERY
                    $previousDumpKeep = $env:RECOMP_HLE_D3D8_DUMP_KEEP_LAST
                    $previousSequence = $env:RECOMP_INPUT_SEQ
                    $previousBackground = $env:RECOMP_WINDOW_BACKGROUND
                    $pcEnvironment = @{}
                    foreach ($name in @('RECOMP_INPUT_CONFIG', 'RECOMP_PC_MOUSE', 'RECOMP_MOUSE_SENSITIVITY', 'RECOMP_MOUSE_INVERT_Y', 'RECOMP_TRACE_BUDGET')) {
                        $pcEnvironment[$name] = [Environment]::GetEnvironmentVariable($name)
                    }
                    try {
                        $env:RECOMP_GAME_DIR = $gameRoot
                        $controlsPath = Join-Path $projectRoot 'config\pc-controls.json'
                        $controls = Get-Content -LiteralPath $controlsPath -Raw | ConvertFrom-Json
                        $mouseSensitivity = [double]$controls.mouse.sensitivity
                        if ([double]::IsNaN($mouseSensitivity) -or [double]::IsInfinity($mouseSensitivity) -or
                            $mouseSensitivity -lt 0.001 -or $mouseSensitivity -gt 10) {
                            throw 'Mouse sensitivity must be between 0.001 and 10 degrees per raw count.'
                        }
                        $env:RECOMP_INPUT_CONFIG = $controlsPath
                        $env:RECOMP_PC_MOUSE = if ($controls.mouse.enabled) { '1' } else { '0' }
                        $env:RECOMP_MOUSE_SENSITIVITY = $mouseSensitivity.ToString('R', [Globalization.CultureInfo]::InvariantCulture)
                        $env:RECOMP_MOUSE_INVERT_Y = if ($controls.mouse.invert_y) { '1' } else { '0' }
                        if ($Action -eq 'SmokeTest') {
                            $env:RECOMP_WINDOW_BACKGROUND = '1'
                            $tag = Get-Date -Format 'yyyyMMdd-HHmmss'
                            $env:RECOMP_SAVE_DIR = Join-Path $privateRoot "test-saves\$tag"
                            $captureRoot = Join-Path $privateRoot "captures\$tag"
                            New-Item -ItemType Directory -Force -Path $captureRoot | Out-Null
                            $env:RECOMP_HLE_D3D8_DUMP = Join-Path $captureRoot 'frame'
                            $env:RECOMP_HLE_D3D8_DUMP_EVERY = '300'
                            $env:RECOMP_HLE_D3D8_DUMP_KEEP_LAST = '8'
                            if ($InputSequence) { $env:RECOMP_INPUT_SEQ = $InputSequence }
                            @{
                                seconds = $Seconds
                                executable = $exe
                                xbe_sha256 = $expectedHash
                                input_sequence = $env:RECOMP_INPUT_SEQ
                                pc_input_replay = $env:TSFP_PC_INPUT_REPLAY
                                save_directory = $env:RECOMP_SAVE_DIR
                            } | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $captureRoot 'run.json') -Encoding UTF8
                            Invoke-Checked $envPython @('scripts/run_and_report.py', $exe,
                                '--seconds', "$Seconds", '--out-dir', $logRoot, '--tag', $tag, '--profile', '100000')
                        }
                        else {
                            $env:RECOMP_SAVE_DIR = Join-Path $privateRoot 'saves'
                            # Generated function tracing is for diagnosis; its default
                            # 400,000 lines can stall an interactive launch on output.
                            $env:RECOMP_TRACE_BUDGET = '0'
                            Push-Location (Split-Path -Parent $exe)
                            try {
                                Invoke-Checked $envPython @((Join-Path $projectRoot 'tools\play.py'), $exe, $logRoot)
                            }
                            finally { Pop-Location }
                        }
                    }
                    finally {
                        $env:RECOMP_GAME_DIR = $previousGameDir
                        $env:RECOMP_SAVE_DIR = $previousSaveDir
                        $env:RECOMP_HLE_D3D8_DUMP = $previousDump
                        $env:RECOMP_HLE_D3D8_DUMP_EVERY = $previousDumpEvery
                        $env:RECOMP_HLE_D3D8_DUMP_KEEP_LAST = $previousDumpKeep
                        $env:RECOMP_INPUT_SEQ = $previousSequence
                        $env:RECOMP_WINDOW_BACKGROUND = $previousBackground
                        foreach ($name in $pcEnvironment.Keys) { [Environment]::SetEnvironmentVariable($name, $pcEnvironment[$name]) }
                    }
                }
            }
        }
        finally { Pop-Location }
    }
}
finally { Pop-Location }
