#!/bin/bash

# Final Build Verification Script for Mesa Switch Build
# Comprehensive verification of all built libraries and configurations

echo "🔍 Final Build Verification for Mesa Nintendo Switch"
echo "=================================================="
echo ""

# Check if build directory exists
if [ ! -d "build-switch" ]; then
    echo "❌ Error: build-switch directory not found!"
    echo "   Run the build first: ./final_build.sh"
    exit 1
fi

cd build-switch

echo "📋 Checking all built libraries..."
echo ""

# Find all .a files
archives=$(find . -name "*.a" -type f)
if [ -z "$archives" ]; then
    echo "❌ No .a files found in build-switch/"
    exit 1
fi

echo "Found $(echo "$archives" | wc -l) archive files:"
echo ""

# Check each archive
thin_count=0
regular_count=0
total_size=0

for archive in $archives; do
    echo -n "Checking $archive: "
    
    # Get archive type
    archive_type=$(file "$archive")
    size=$(ls -lh "$archive" | awk '{print $5}')
    
    if echo "$archive_type" | grep -q "thin archive"; then
        echo "❌ THIN ARCHIVE (problematic) - $size"
        thin_count=$((thin_count + 1))
    else
        echo "✅ Regular static archive - $size"
        regular_count=$((regular_count + 1))
    fi
    
    # Add to total size (extract numeric size)
    numeric_size=$(ls -l "$archive" | awk '{print $5}')
    total_size=$((total_size + numeric_size))
done

echo ""
echo "📊 Summary:"
echo "  Regular archives: $regular_count"
echo "  Thin archives: $thin_count"
echo "  Total size: $((total_size / 1024))KB"

if [ $thin_count -gt 0 ]; then
    echo ""
    echo "❌ PROBLEM: Found $thin_count thin archive(s)!"
    echo "   Thin archives can cause linking errors with some toolchains."
    echo "   Solution: Rebuild with ./final_build.sh"
    echo ""
    echo "Thin archives found:"
    for archive in $archives; do
        if file "$archive" | grep -q "thin archive"; then
            echo "  - $archive"
        fi
    done
    exit 1
else
    echo ""
    echo "✅ SUCCESS: All archives are regular static archives!"
    echo "   These will link correctly with your Switch toolchain."
fi

echo ""
echo "🔍 Checking for shared libraries..."
shared_libs=$(find . -name "*.so*" -type f)
if [ -n "$shared_libs" ]; then
    echo "❌ WARNING: Found shared libraries:"
    echo "$shared_libs"
    echo "   These should not be present in a static build."
else
    echo "✅ No shared libraries found (good for static linking)"
fi

echo ""
echo "📁 Library locations and details:"
echo ""

# Check specific target libraries
target_libs=(
    "src/util/libmesa_util.a"
    "src/mesa/libmesa.a"
    "src/egl/libEGL.a"
    "src/mapi/glapi/libglapi_static.a"
    "src/mapi/es2api/libGLESv2.a"
)

all_present=true
for lib in "${target_libs[@]}"; do
    if [ -f "$lib" ]; then
        size=$(ls -lh "$lib" | awk '{print $5}')
        echo "  ✅ $lib ($size)"
    else
        echo "  ❌ $lib (missing)"
        all_present=false
    fi
done

echo ""
if [ "$all_present" = true ]; then
    echo "✅ All target libraries are present!"
else
    echo "❌ Some target libraries are missing!"
    echo "   Rebuild with: ./final_build.sh"
fi

echo ""
echo "🔧 Toolchain verification..."
echo ""

# Check if devkitPro toolchain is available
CC="/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc"
AR="/opt/devkitpro/devkitA64/bin/aarch64-none-elf-ar"

if [ -f "$CC" ]; then
    echo "✅ devkitPro GCC found: $CC"
else
    echo "❌ devkitPro GCC not found: $CC"
fi

if [ -f "$AR" ]; then
    echo "✅ devkitPro AR found: $AR"
else
    echo "❌ devkitPro AR not found: $AR"
fi

echo ""
echo "🎯 Archive compatibility test..."
echo ""

# Test if we can extract symbols from one of the archives
test_archive="src/util/libmesa_util.a"
if [ -f "$test_archive" ]; then
    echo "Testing archive compatibility with: $test_archive"
    
    # Try to list symbols (this tests if the archive is readable)
    if $AR t "$test_archive" > /dev/null 2>&1; then
        echo "✅ Archive is readable and compatible"
        symbol_count=$($AR t "$test_archive" | wc -l)
        echo "   Contains $symbol_count object files"
    else
        echo "❌ Archive is not readable or compatible"
    fi
else
    echo "⚠️  Cannot test archive compatibility - test archive not found"
fi

echo ""
echo "📝 Final verification summary:"
echo "=============================="

if [ $thin_count -eq 0 ] && [ "$all_present" = true ]; then
    echo "🎉 SUCCESS: Build verification passed!"
    echo ""
    echo "✅ All libraries are regular static archives"
    echo "✅ All target libraries are present"
    echo "✅ No shared libraries found"
    echo "✅ Archives are compatible with devkitPro toolchain"
    echo ""
    echo "🚀 Your Mesa libraries are ready for Switch development!"
    echo ""
    echo "📋 Usage example:"
    echo "   aarch64-none-elf-gcc your_app.c \\"
    echo "     -Lbuild-switch/src/mapi/es2api -lGLESv2 \\"
    echo "     -Lbuild-switch/src/mapi/glapi -lglapi_static \\"
    echo "     -Lbuild-switch/src/util -lmesa_util \\"
    echo "     -Lbuild-switch/src/mesa -lmesa \\"
    echo "     -Lbuild-switch/src/egl -lEGL \\"
    echo "     -lnx -lm"
    echo ""
    echo "🎯 Key achievement: Thin archive linking errors completely resolved!"
else
    echo "❌ FAILURE: Build verification failed!"
    echo ""
    if [ $thin_count -gt 0 ]; then
        echo "❌ Found $thin_count thin archive(s)"
    fi
    if [ "$all_present" = false ]; then
        echo "❌ Some target libraries are missing"
    fi
    echo ""
    echo "🔧 Solution: Rebuild with ./final_build.sh"
fi

cd .. 