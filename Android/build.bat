@echo off
setlocal

if "%ANDROID_HOME%"=="" (
    echo ANDROID_HOME is not set
    exit /b 1
)

set NDK_PATH=%ANDROID_HOME%\ndk
for /d %%d in ("%NDK_PATH%\*") do set NDK_PATH=%%d

if "%NDK_PATH%"=="" (
    echo NDK not found
    exit /b 1
)

set BUILD_DIR=%CD%\build
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

"%NDK_PATH%\ndk-build.cmd" ^
  NDK_PROJECT_PATH=. ^
  APP_BUILD_SCRIPT=Android.mk ^
  NDK_APPLICATION_MK=Application.mk ^
  NDK_OUT=%BUILD_DIR%

echo Build complete
pause
