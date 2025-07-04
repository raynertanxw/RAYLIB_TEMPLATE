:: TODO: Have a release config of this...

@echo off
setlocal

:: Compiler
set "CC=gcc"
set "LIB_PATH=lib_win"
set "LIBS=-L %LIB_PATH% -lraylib"
set "FRAMEWORKS=-lgdi32 -lwinmm"

:: Source files
set "SRC_DIR=src"
set "SRC=%SRC_DIR%\main.c"
set "RESOURCE_DIR=resources"

:: Output
set "OUT_DIR=bin"
set "OUT=%OUT_DIR%\compiled-raylib-game.exe"

:: Handle command-line arguments
if "%1"=="clean" goto clean
if "%1"=="run" goto run
if "%1"=="run-release" goto run-release

:: Default case: Build the project
call :clean
call :copy_resources
if "%1"=="build-release" goto build-release
call :build
goto :eof

:copy_resources
echo Copying resources...
if exist "%OUT_DIR%\%RESOURCE_DIR%" rd /s /q "%OUT_DIR%\%RESOURCE_DIR%"
xcopy /e /i "%RESOURCE_DIR%" "%OUT_DIR%\%RESOURCE_DIR%"
goto :eof

:build
echo Setting flags debug...
set FLAGS=-DDEBUG=1
goto :do-build

:build-release
echo Setting flags release...
set FLAGS=-DDEBUG=0
goto :do-build

:do-build
echo Building the project...
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

%CC% %FLAGS% %SRC% %LIBS% %FRAMEWORKS% -o %OUT%
if %errorlevel% neq 0 (
    echo Build failed with error level %errorlevel%.
    exit /b %errorlevel%
)
goto :eof

:clean
echo Cleaning up...
if exist "%OUT_DIR%" rd /s /q "%OUT_DIR%"
mkdir "%OUT_DIR%"
goto :eof

:run
call :clean
call :copy_resources
call :build
call :do-run
goto :eof

:run-release
call :clean
call :copy_resources
call :build-release
call :do-run
goto :eof


:do-run
echo Running the executable...
if exist "%OUT%" (
    "%OUT%"
    if %errorlevel% neq 0 (
        echo Execution failed with error level %errorlevel%.
        exit /b %errorlevel%
    )
) else (
    echo Executable not found: %OUT%
    exit /b 1
)

goto :eof
