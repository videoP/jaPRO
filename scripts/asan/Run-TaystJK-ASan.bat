@echo off
setlocal EnableExtensions

cd /d "%~dp0"

if not exist "taystjk.x86_64.exe" (
	echo ERROR: taystjk.x86_64.exe was not found next to this launcher.
	pause
	exit /b 1
)

if not exist "clang_rt.asan_dynamic-x86_64.dll" (
	echo ERROR: The Microsoft AddressSanitizer runtime DLL is missing.
	pause
	exit /b 1
)

set "REPORT_DIR=%~dp0asan-reports"
if not exist "%REPORT_DIR%" mkdir "%REPORT_DIR%"
if not exist "%REPORT_DIR%" (
	echo ERROR: Could not create the report directory:
	echo %REPORT_DIR%
	pause
	exit /b 1
)

for /f %%I in ('powershell.exe -NoLogo -NoProfile -Command "Get-Date -Format yyyyMMdd-HHmmss"') do set "TIMESTAMP=%%I"
if not defined TIMESTAMP set "TIMESTAMP=%RANDOM%"

set "REPORT_NAME=TaystJK-ASan-%TIMESTAMP%"
set "ASAN_SAVE_DUMPS=%REPORT_DIR%\%REPORT_NAME%.dmp"
set "ASAN_LOG=%REPORT_DIR%\%REPORT_NAME%.log"
set "TAYSTJK_ASAN_OPTIONS=alloc_dealloc_mismatch=1:malloc_context_size=30:print_cmdline=1"
if defined ASAN_OPTIONS (
	set "ASAN_OPTIONS=%TAYSTJK_ASAN_OPTIONS%:%ASAN_OPTIONS%"
) else (
	set "ASAN_OPTIONS=%TAYSTJK_ASAN_OPTIONS%"
)

echo Running TaystJK with Microsoft AddressSanitizer enabled.
echo Reproduce the crash, then send the generated report files to the developer.
echo.
echo Log:  %ASAN_LOG%
echo Dump: %ASAN_SAVE_DUMPS%
echo.

"%~dp0taystjk.x86_64.exe" ^
	+set developer 1 ^
	+set logfile 2 ^
	+set r_verbose 1 ^
	+set fs_debug 1 ^
	%* > "%ASAN_LOG%" 2>&1
set "EXIT_CODE=%ERRORLEVEL%"

echo.
echo TaystJK exited with code %EXIT_CODE%.
echo Log: %ASAN_LOG%
if exist "%ASAN_SAVE_DUMPS%" (
	echo AddressSanitizer dump: %ASAN_SAVE_DUMPS%
) else (
	echo No AddressSanitizer dump was produced.
)
echo.
echo Please send the log, any dump file, and reproduction steps to the developer.
pause

exit /b %EXIT_CODE%
