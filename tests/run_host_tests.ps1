param([string]$CompilerDirectory = 'C:/Program Files/LLVM/bin')
$ErrorActionPreference = 'Stop'
$testRoot = Split-Path -Parent $PSScriptRoot
$testBuild = Join-Path $testRoot 'tmp/correcoes-validation'
New-Item -ItemType Directory -Force $testBuild | Out-Null
function Invoke-Checked([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed ($LASTEXITCODE)" }
}
$cc = Join-Path $CompilerDirectory 'clang.exe'
$cxx = Join-Path $CompilerDirectory 'clang++.exe'
foreach ($unit in @('dwx_safe', 'textmetrics')) {
    Invoke-Checked $cc @('-std=c17', '-Wall', '-Wextra', '-Werror', '-c', "$testRoot/src/extras/$unit.c", '-o', "$testBuild/$unit.obj")
}
Invoke-Checked $cxx @('-std=c++20', '-Wall', '-Wextra', '-Werror', "$PSScriptRoot/safe_algorithms_test.cpp", "$testBuild/dwx_safe.obj", "$testBuild/textmetrics.obj", '-o', "$testBuild/safe_algorithms_test.exe")
Invoke-Checked "$testBuild/safe_algorithms_test.exe" @()
Invoke-Checked $cxx @('-std=c++20', '-Wall', '-Wextra', '-Werror', "$PSScriptRoot/sd_contract_test.cpp", '-o', "$testBuild/sd_contract_test.exe")
Invoke-Checked "$testBuild/sd_contract_test.exe" @()
