@rem Copyright 2021 The Tint Authors.
@rem
@rem Licensed under the Apache License, Version 2.0 (the "License");
@rem you may not use this file except in compliance with the License.
@rem You may obtain a copy of the License at
@rem
@rem     http://www.apache.org/licenses/LICENSE-2.0
@rem
@rem Unless required by applicable law or agreed to in writing, software
@rem distributed under the License is distributed on an "AS IS" BASIS,
@rem WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@rem See the License for the specific language governing permissions and
@rem limitations under the License.

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

set ORIGINAL_SRC_DIR= %~dp0\..\..
set TEMP_DIR=%TEMP%\tint-temp
set SRC_DIR="%TEMP_DIR%\tint-src"
set BUILD_DIR="%TEMP_DIR%\tint-build"

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

call :status "Fetching DXC"
@echo on
set DXC_LATEST_ARTIFACT="https://ci.appveyor.com/api/projects/dnovillo/directxshadercompiler/artifacts/build%%2FRelease%%2Fdxc-artifacts.zip?branch=master&pr=false&job=image%%3A%%20Visual%%20Studio%%202019"
curl -L %DXC_LATEST_ARTIFACT% --output "%TEMP_DIR%\dxc.zip" || goto :error
@echo off

call :status "Unpacking DXC"
@echo on
powershell.exe -Command "Expand-Archive -LiteralPath '%TEMP_DIR%\dxc.zip' -DestinationPath '%TEMP_DIR%\dxc'" || goto :error
set PATH=%TEMP_DIR%\dxc\bin;%PATH%
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
copy standalone.gclient .gclient || goto :error
call gclient sync || goto :error
@echo off

call :status "Configuring build system"
@echo on
mkdir %BUILD_DIR%
cd /d %BUILD_DIR%
set COMMON_CMAKE_FLAGS=-DTINT_BUILD_DOCS=O -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
@echo off

call :status "Building tint"
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

call :status "Testing test/test-all.sh"
@echo on
cd /d %SRC_DIR% || goto :error
set PATH=C:\Program Files\Metal Developer Tools\macos\bin;%PATH%
where metal.exe
git bash -- ./test/test-all.sh ../tint-build/tint.exe --verbose
@echo off

call :status "Done"
exit /b 0

:error
echo BUILD FAILED! errorlevel: %errorlevel%
exit /b %errorlevel%
