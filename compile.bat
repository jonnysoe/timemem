@echo off

:: Compile script to use MSVC and Ninja
call :vsCheck
if %ERRORLEVEL% equ 0 call :vsBuild %*

exit /b %ERRORLEVEL%

:: =====================================================================
:vsCheck
:: =====================================================================
set MISSING_EXE=
call :vsInit
if %ERRORLEVEL% neq 0 set MISSING_EXE= MSVC (https://visualstudio.microsoft.com/visual-cpp-build-tools/)
where cmake > nul
if %ERRORLEVEL% neq 0 set MISSING_EXE= cmake (https://github.com/Kitware/CMake/releases/latest)
:: Display error
if defined MISSING_EXE echo Missing executable(s):%MISSING_EXE% && exit /b 1

exit /b 0

:: =====================================================================
:vsInit
:: =====================================================================
:: Need to setup MSVC environment with vcvarsall.bat,
:: because it is not set by default to cater multiple versions
:: if there are other compilers in PATH variable,
:: Ninja will use the first known compiler it encounters
if defined VSCMD_VER exit /b 0

set VCVARSALLBAT=

:: Try %ProgramFiles(x86)%, the free BuildTools default location
:: set will overwrite the value in every loop so this will always be the latest vcvarsall.bat
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio" for /f "tokens=* USEBACKQ" %%A in (`dir /s /b "%ProgramFiles(x86)%\Microsoft Visual Studio\" ^| findstr "vcvarsall.bat"`) do set VCVARSALLBAT=%%A
if defined VCVARSALLBAT call "%VCVARSALLBAT%" %PROCESSOR_ARCHITECTURE%
:: take any arbitrary variable it sets,
:: because vcvarsall.bat doesn't return error code
if defined VSCMD_VER exit /b 0

:: Retry with native %ProgramFiles%
if exist "%ProgramFiles%\Microsoft Visual Studio" for /f "tokens=* USEBACKQ" %%A in (`dir /s /b "%ProgramFiles%\Microsoft Visual Studio\" ^| findstr "vcvarsall.bat"`) do set VCVARSALLBAT=%%A
if defined VCVARSALLBAT call "%VCVARSALLBAT%" %PROCESSOR_ARCHITECTURE%
if defined VSCMD_VER exit /b 0
exit /b 1

:: =====================================================================
:ninjaCheck
:: =====================================================================
where ninja > nul
if %ERRORLEVEL% neq 0 echo Optional: ninja (https://github.com/ninja-build/ninja/releases/latest) && exit /b 1

exit /b 0

:: =====================================================================
:vsBuild
:: =====================================================================
set USER_ARGS=%*
set NINJA_GENERATOR=
set CONFIGURE=

:: Optionally use Ninja if found, else leave it to use the latest Visual Studio generator set by vcvarsall.bat
:: This project is very small, but Ninja can still shave a few seconds of build time
call :ninjaCheck
if %ERRORLEVEL% equ 0 set NINJA_GENERATOR=-G Ninja

:: Remove Ninja if user specified a generator
echo %USER_ARGS% | findstr c:"-G" > nul
if %ERRORLEVEL% equ 0 set NINJA_GENERATOR=

:: Reconfigure CMake Decision
:: - Never configured
if not exist build\CMakeCache.txt set CONFIGURE=1
:: - If it was previously configured to Debug, then reconfigure as well
if not defined CONFIGURE findstr CMAKE_BUILD_TYPE: build\CMakeCache.txt | findstr Debug > nul
if %ERRORLEVEL% equ 0 set CONFIGURE=1
type nul > nul
:: - Previously wasn't configured with Ninja
if defined NINJA_GENERATOR if not exist build\build.ninja set CONFIGURE=1

:: Delete build directory
if defined CONFIGURE rmdir /s /q build > nul 2>&1

:: Configure CMake
if not exist build cmake -S . -B build %NINJA_GENERATOR% %USER_ARGS%
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

:: Build
cmake --build build

exit /b %ERRORLEVEL%