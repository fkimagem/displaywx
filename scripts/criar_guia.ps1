param([string]$Modo = '')
$ErrorActionPreference = 'Stop'
try {
    $raizGuia = Split-Path -Parent $PSScriptRoot
    Set-Location -LiteralPath $raizGuia
    $codexGuia = Get-Command codex.exe -ErrorAction SilentlyContinue
    if (-not $codexGuia) { $codexGuia = Get-Command codex -ErrorAction SilentlyContinue }
    if ($codexGuia) { $executavelGuia = $codexGuia.Source }
    else {
        $candidatosGuia = Get-ChildItem -Path "$env:LOCALAPPDATA\OpenAI\Codex\bin\*\codex.exe" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending
        $executavelGuia = $candidatosGuia | Select-Object -First 1 -ExpandProperty FullName
    }
    if (-not $executavelGuia) { throw 'Codex CLI nao encontrado. Instale o Codex e execute codex login antes de tentar novamente.' }
    foreach ($arquivoGuia in @('docs\GUIA_DISPLAYWX.md', 'docs\GUIA_DMA.md', 'scripts\gerar_guia.py', 'scripts\gerar_guia_local.ps1', '.cursor\agents\criar-guia-pdf-displaywx.md')) {
        if (-not (Test-Path -LiteralPath $arquivoGuia)) { throw "Arquivo obrigatorio ausente: $arquivoGuia" }
    }
    if ($Modo -eq '--check') {
        Write-Host "Codex encontrado: $executavelGuia"
        Write-Host 'Fontes e agente encontrados. Verificacao local concluida.'
        exit 0
    }
    if ($Modo -and $Modo -ne '--no-pause') { throw 'Opcao desconhecida. Use --check ou --no-pause.' }
    New-Item -ItemType Directory -Force -Path 'output\pdf' | Out-Null
    $inicioGuia = [DateTime]::UtcNow
    $promptGuia = Get-Content -LiteralPath '.cursor\agents\criar-guia-pdf-displaywx.md' -Raw -Encoding UTF8
    Write-Host 'Codex: lendo o Markdown atual e recriando o manual...'
    $promptGuia | & $executavelGuia -a never exec --sandbox workspace-write --cd $raizGuia --color never --output-last-message 'output\pdf\ultima_execucao_codex.txt' -
    if ($LASTEXITCODE -ne 0) { throw "Codex terminou com codigo $LASTEXITCODE. Verifique login, conexao e as mensagens acima." }
    $pdfGuia = Get-Item -LiteralPath 'output\pdf\Guia_DisplayWX.pdf' -ErrorAction SilentlyContinue
    if (-not $pdfGuia -or $pdfGuia.Length -lt 1000 -or $pdfGuia.LastWriteTimeUtc -lt $inicioGuia) {
        throw 'O Codex nao produziu um PDF atualizado. Consulte output\pdf\ultima_execucao_codex.txt.'
    }
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'gerar_guia_local.ps1') -Validate
    if ($LASTEXITCODE -ne 0) { throw 'A verificacao final do PDF e dos Markdown falhou.' }
    Write-Host "PDF atualizado: $($pdfGuia.FullName)" -ForegroundColor Green
    exit 0
} catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
