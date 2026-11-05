param([switch]$Validate)
$ErrorActionPreference = 'Stop'
$raizGuia = Split-Path -Parent $PSScriptRoot
$candidatosPythonGuia = @(
    (Join-Path $raizGuia '.venv-guia\Scripts\python.exe'),
    "$env:USERPROFILE\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe"
)
$pythonSistemaGuia = Get-Command python.exe -ErrorAction SilentlyContinue
if ($pythonSistemaGuia) { $candidatosPythonGuia += $pythonSistemaGuia.Source }
foreach ($pythonGuia in $candidatosPythonGuia) {
    if (-not (Test-Path -LiteralPath $pythonGuia)) { continue }
    & $pythonGuia -c 'import reportlab, pypdf, PIL' 2>$null
    if ($LASTEXITCODE -ne 0) { continue }
    $argumentosGuia = @((Join-Path $PSScriptRoot 'gerar_guia.py'))
    if ($Validate) { $argumentosGuia += '--validate' }
    & $pythonGuia @argumentosGuia
    exit $LASTEXITCODE
}
Write-Host 'Python com reportlab, pypdf e Pillow nao encontrado. Use o runtime do Codex ou instale scripts\requirements-guia.txt em .venv-guia.' -ForegroundColor Red
exit 1
