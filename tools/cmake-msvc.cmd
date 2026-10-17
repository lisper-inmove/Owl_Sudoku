@echo off
REM Runs CMake with the MSVC environment (vcvars64) loaded.
REM Usage: tools\cmake-msvc.cmd --preset ninja
REM        tools\cmake-msvc.cmd --build --preset debug

setlocal

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
	echo [cmake-msvc] vswhere.exe was not found. Is Visual Studio installed? 1>&2
	exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%i"
if not defined VSROOT (
	echo [cmake-msvc] Visual Studio with the C++ toolset was not found. 1>&2
	exit /b 1
)

call "%VSROOT%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1

cmake %*
exit /b %errorlevel%
