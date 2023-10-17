@rem Copyright 2021 The Dawn & Tint Authors
@rem
@rem Redistribution and use in source and binary forms, with or without
@rem modification, are permitted provided that the following conditions are met:
@rem
@rem 1. Redistributions of source code must retain the above copyright notice, this
@rem    list of conditions and the following disclaimer.
@rem
@rem 2. Redistributions in binary form must reproduce the above copyright notice,
@rem    this list of conditions and the following disclaimer in the documentation
@rem    and/or other materials provided with the distribution.
@rem
@rem 3. Neither the name of the copyright holder nor the names of its
@rem    contributors may be used to endorse or promote products derived from
@rem    this software without specific prior written permission.
@rem
@rem THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
@rem AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
@rem IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
@rem DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
@rem FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
@rem DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
@rem SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
@rem CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
@rem OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
@rem OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

goto :main

:task_begin
set TASK_NAME=%~1
echo %TASK_NAME% starting at %Time%
exit /b 0

:print_last_task_duration
if not "%TASK_NAME%" == "" (
    echo %TASK_NAME% completed at %Time%
)
exit /b 0

:status
echo.
echo.
call :print_last_task_duration
echo.
echo *****************************************************************
echo %~1
echo *****************************************************************
echo.
call :task_begin "%~1"
exit /b 0

:main

set ORIGINAL_SRC_DIR= %~dp0\..\..\..
set TEMP_DIR=%TEMP%\dawn-temp
set SRC_DIR=%TEMP_DIR%\dawn-src
set BUILD_DIR=%TEMP_DIR%\dawn-build

cd /d %ORIGINAL_SRC_DIR%
if not exist ".git\" (
    echo "ORIGINAL_SRC_DIR should point to project root: %ORIGINAL_SRC_DIR%"
    goto :error
)

if exist %TEMP_DIR% (
    call :status "Deleting %TEMP_DIR%"
    del /q/f/s %TEMP_DIR% > NUL || goto :error
    rmdir /q/s %TEMP_DIR% > NUL || goto :error
)
mkdir %TEMP_DIR% || goto :error

call :status "Fetching and installing DXC"
@echo on
set DXC_RELEASE="https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2207/dxc_2022_07_18.zip"
curl -k -L %DXC_RELEASE% --output "%TEMP_DIR%\dxc_release.zip" || goto :error
powershell.exe -Command "Expand-Archive -LiteralPath '%TEMP_DIR%\dxc_release.zip' -DestinationPath '%TEMP_DIR%\dxc'" || goto :error
set DXC_PATH=%TEMP_DIR%\dxc\bin\x64

call :status "Fetching and installing Windows SDK for d3dcompiler DLL"
@echo on
set WINSDK_DLL_INSTALLER=https://go.microsoft.com/fwlink/?linkid=2164145
set WINSDK_VERSION=10.0.20348.0
curl -k -L %WINSDK_DLL_INSTALLER% --output "%TEMP_DIR%\winsdksetup.exe" || goto :error
start "download" /wait "%TEMP_DIR%\winsdksetup.exe" /quiet /norestart /ceip off /features OptionId.DesktopCPPx64 /layout "%TEMP_DIR%\winsdkinstall" || goto :error
start "install" /wait "%TEMP_DIR%\winsdkinstall\Installers\Windows SDK for Windows Store Apps Tools-x86_en-us.msi" || goto :error
set D3DCOMPILER_PATH=C:\Program Files (x86)\Windows Kits\10\bin\%WINSDK_VERSION%\x64
@echo off

call :status "Installing depot_tools"
@echo on
pushd %TEMP_DIR%
rem For Windows, we must download and extract a bundle.
rem See https://chromium.googlesource.com/chromium/src/+/HEAD/docs/windows_build_instructions.md#install
powershell -Command "(New-Object Net.WebClient).DownloadFile('https://storage.googleapis.com/chrome-infra/depot_tools.zip', 'depot_tools.zip')" || goto :error
powershell -Command "Expand-Archive -Force 'depot_tools.zip' 'depot_tools'" || goto :error
rem Run gclient once to install deps
set PATH=%TEMP_DIR%\depot_tools;%PATH%
set DEPOT_TOOLS_UPDATE=1
set DEPOT_TOOLS_WIN_TOOLCHAIN=0
call gclient || goto :error
@echo off
popd

call :status "Cloning to clean source directory"
@echo on
mkdir %SRC_DIR% || goto :error
cd /d %SRC_DIR% || goto :error
call git clone %ORIGINAL_SRC_DIR% . || goto :error
@echo off

call :status "Fetching dependencies"
@echo on
copy scripts\standalone.gclient .gclient || goto :error
call gclient sync || goto :error
@echo off

call :status "Adding the Ninja from DEPS to the PATH"
@echo on
set PATH=%SRC_DIR%\third_party\ninja;%PATH%
@echo off

call :status "Configuring build system"
@echo on
mkdir %BUILD_DIR%
cd /d %BUILD_DIR%
set COMMON_CMAKE_FLAGS=             ^
    -DTINT_BUILD_DOCS=O             ^
    -DTINT_BUILD_BENCHMARKS=1       ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DTINT_BUILD_BENCHMARKS=1       ^
    -DTINT_BUILD_SPV_READER=1       ^
    -DTINT_BUILD_WGSL_READER=1      ^
    -DTINT_BUILD_GLSL_WRITER=1      ^
    -DTINT_BUILD_HLSL_WRITER=1      ^
    -DTINT_BUILD_MSL_WRITER=1       ^
    -DTINT_BUILD_SPV_WRITER=1       ^
    -DTINT_BUILD_WGSL_WRITER=1      ^
    -DTINT_RANDOMIZE_HASHES=1

@echo off

call :status "Building dawn"
@echo on
rem Disable msbuild "Intermediate or Output directory cannot reside in Temporary directory"
set IgnoreWarnIntDirInTempDetected=true
rem Add Python3 to path as this Kokoro image only has Python2 in it
set PATH=C:\Python37;%PATH%
rem To use ninja with CMake requires VC env vars
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
@echo on
rem Note that we need to specify the C and C++ compiler only because Cygwin is in PATH and CMake finds GCC and picks that over MSVC
cmake %SRC_DIR% -G "Ninja" -DCMAKE_C_COMPILER="cl.exe" -DCMAKE_CXX_COMPILER="cl.exe" %COMMON_CMAKE_FLAGS% || goto :error
cmake --build . || goto :error
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" /clean_env
@echo off

call :status "Running tint_unittests"
@echo on
tint_unittests.exe || goto :error
@echo off

call :status "Testing test/tint/test-all.sh"
@echo on
cd /d %SRC_DIR% || goto :error
rem Run tests with DXC, FXC and Metal validation
set OLD_PATH=%PATH%
set PATH=C:\Program Files\Metal Developer Tools\macos\bin;%PATH%
if "%BUILD_TYPE%" == "Debug" (
    rem TODO(crbug.com/2034): Add back glsl once we fix the ~7x slowdown in Windows Debug builds
    set TEST_ALL_FORMATS=wgsl,spvasm,msl,hlsl
) else (
    set TEST_ALL_FORMATS=wgsl,spvasm,msl,hlsl,glsl
)
call git bash -- ./test/tint/test-all.sh %BUILD_DIR%/tint.exe --verbose --format %TEST_ALL_FORMATS% || goto :error
set PATH=%OLD_PATH%
@echo off

call :status "Done"
exit /b 0

:error
echo BUILD FAILED! errorlevel: %errorlevel%
exit /b %errorlevel%
