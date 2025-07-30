@echo off
echo Rebuilding Mesa with regular static archives...

cd Mesa

REM Clean previous build
if exist build-switch rmdir /s /q build-switch

REM Configure with options to create regular static archives
echo Configuring Mesa build...
meson setup build-switch --cross-file switch.meson --wipe -Dshared-glapi=false -Dstatic-libglapi=true -Ddefault-library=static

REM Build
echo Building Mesa...
ninja -C build-switch

REM Copy libraries to VitaNS
echo Copying libraries to VitaNS...
set OUTPUT_DIR=..\Vita3K-Switch\external\custom-mesa\lib

REM Ensure output directory exists
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

REM Copy all .a files
for /r build-switch %%f in (*.a) do (
    copy "%%f" "%OUTPUT_DIR%"
    echo Copied %%~nxf
)

echo Mesa rebuild completed successfully! 