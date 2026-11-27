param(
    [string]$ArduinoRoot = 'C:/arduino_edgewx',
    [string]$Fqbn = 'esp32:esp32:esp32s3:PSRAM=opi',
    [int]$Jobs = 8
)
$ErrorActionPreference = 'Stop'
$libraryRoot = Split-Path -Parent $PSScriptRoot
$buildRoot = Join-Path $libraryRoot 'tmp/arduino-smoke-build'
$buildLog = Join-Path $libraryRoot 'tmp/arduino-smoke.log'
New-Item -ItemType Directory -Force $buildRoot | Out-Null
$arguments = @(
    '-compile', '-logger=human', "-jobs=$Jobs",
    '-hardware', "$ArduinoRoot/hardware",
    '-hardware', "$ArduinoRoot/portable/packages",
    '-tools', "$ArduinoRoot/tools-builder",
    '-tools', "$ArduinoRoot/hardware/tools/avr",
    '-tools', "$ArduinoRoot/portable/packages",
    '-built-in-libraries', "$ArduinoRoot/libraries",
    '-libraries', (Split-Path -Parent $libraryRoot),
    "-fqbn=$Fqbn", '-ide-version=10819', '-warnings=default',
    '-build-path', $buildRoot,
    "$PSScriptRoot/sketches/compile_smoke/compile_smoke.ino"
)
$ErrorActionPreference = 'Continue' # Windows PowerShell treats native stderr as an error stream.
& "$ArduinoRoot/arduino-builder.exe" @arguments *> $buildLog
$buildExit = $LASTEXITCODE
$ErrorActionPreference = 'Stop'
if ($buildExit -ne 0) {
    Get-Content $buildLog -Tail 60
    throw "Arduino compile/link failed ($buildExit). Log: $buildLog"
}
Get-Content $buildLog -Tail 12
Write-Output "PASS: Arduino compile/link. Artifacts: $buildRoot"
