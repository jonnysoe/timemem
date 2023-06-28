@echo off
set INKSCAPE=C:\Program Files\Inkscape\bin\inkscapecom.com
set MAGICK=magick.exe

:: Icon already exist
if exist icon.ico echo Skipping icon.ico generation, delete it to regenerate && exit /b 0

call:checkRequirements
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

:: Image file source
:: https://commons.wikimedia.org/wiki/File:Orologio_viola.svg
if not exist icon.svg curl -L -o icon.svg https://upload.wikimedia.org/wikipedia/commons/e/ec/Orologio_viola.svg
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

call:generateIcon

:: Remove intermediate files
del /f /q 16.png 32.png 48.png 64.png 256.png icon.svg

exit /b %ERRORLEVEL%

:: =====================================================================
:checkRequirements
:: =====================================================================
set MISSING_EXE=
:: inkscape
if not exist "%INKSCAPE%" set MISSING_EXE= inkscape (https://inkscape.org/release/1.2.2/windows)
:: magick
where %MAGICK% > nul 2>&1
if %ERRORLEVEL% neq 0 set MISSING_EXE=%MISSING_EXE% magick (https://imagemagick.org/script/download.php#windows)
:: Display error
if defined MISSING_EXE echo Missing executable(s):%MISSING_EXE% && exit /b 1

exit /b 0

:: =====================================================================
:generateIcon
:: =====================================================================
:: Size guidelines from Microsoft
:: https://learn.microsoft.com/en-us/windows/win32/uxguide/vis-icons?redirectedfrom=MSDN#size-requirements

:: Recomended steps from
:: https://graphicdesign.stackexchange.com/a/77466

:: SVG to multiple PNG
call "%INKSCAPE%" -w 16 -h 16 --export-filename=16.png icon.svg
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

call "%INKSCAPE%" -w 32 -h 32 --export-filename=32.png icon.svg
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

call "%INKSCAPE%" -w 48 -h 48 --export-filename=48.png icon.svg
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

call "%INKSCAPE%" -w 64 -h 64 --export-filename=64.png icon.svg
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

call "%INKSCAPE%" -w 256 -h 256 --export-filename=256.png icon.svg
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

:: Consolidate multiple PNG into ICO
call "%MAGICK%" 16.png 32.png 48.png 64.png 256.png icon.ico

exit /b %ERRORLEVEL%