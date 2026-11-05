@echo off
setlocal
chcp 65001 >nul
pushd "%~dp0"
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\criar_guia.ps1" -Modo "%~1"
set "GUIA_EXIT=%ERRORLEVEL%"
popd
if not "%GUIA_EXIT%"=="0" echo Falha ao criar o guia. Consulte as mensagens acima.
if /I not "%~1"=="--check" if /I not "%~1"=="--no-pause" pause
exit /b %GUIA_EXIT%
