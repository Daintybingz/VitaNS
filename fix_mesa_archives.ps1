# PowerShell script to fix Mesa archives for Switch
# This script rebuilds the Mesa static libraries using the correct ar tool

Write-Host "Fixing Mesa archives for Switch..." -ForegroundColor Green

# Set up environment variables
$env:DEVKITPRO = "C:\devkitPro"
$env:PATH = "$env:DEVKITPRO\devkitA64\bin;$env:PATH"

# Go to Mesa directory
Set-Location "Mesa"

# Clean previous build
if (Test-Path "build-switch") {
    Remove-Item -Recurse -Force "build-switch"
}

# Configure with options to create regular static archives
Write-Host "Configuring Mesa build..." -ForegroundColor Yellow
meson setup build-switch --cross-file switch.meson --wipe -Dshared-glapi=false -Dstatic-libglapi=true

# Build
Write-Host "Building Mesa..." -ForegroundColor Yellow
ninja -C build-switch

# Copy libraries to VitaNS
Write-Host "Copying libraries to VitaNS..." -ForegroundColor Yellow
$outputDir = "Vita3K-Switch/external/custom-mesa/lib"

# Ensure output directory exists
if (!(Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force
}

# Copy all .a files
Get-ChildItem -Recurse -Filter "*.a" build-switch | ForEach-Object {
    Copy-Item $_.FullName $outputDir -Force
    Write-Host "Copied $($_.Name)" -ForegroundColor Green
}

Write-Host "Mesa archives fixed successfully!" -ForegroundColor Green 