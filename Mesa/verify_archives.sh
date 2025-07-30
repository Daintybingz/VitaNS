#!/bin/bash

# Archive Verification Script for Mesa Switch Build
# Checks that all .a files are regular static archives (not thin archives)

echo "🔍 Verifying Mesa archives for Nintendo Switch..."
echo ""

if [ ! -d "build-switch" ]; then
    echo "❌ Error: build-switch directory not found!"
    echo "   Run the build first: ./build_switch.sh"
    exit 1
fi

cd build-switch

echo "📋 Checking all .a files for archive type..."

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

for archive in $archives; do
    echo -n "Checking $archive: "
    
    # Use file command to check archive type
    archive_type=$(file "$archive")
    
    if echo "$archive_type" | grep -q "thin archive"; then
        echo "❌ THIN ARCHIVE (problematic)"
        thin_count=$((thin_count + 1))
    else
        echo "✅ Regular static archive"
        regular_count=$((regular_count + 1))
    fi
done

echo ""
echo "📊 Summary:"
echo "  Regular archives: $regular_count"
echo "  Thin archives: $thin_count"

if [ $thin_count -gt 0 ]; then
    echo ""
    echo "❌ PROBLEM: Found $thin_count thin archive(s)!"
    echo "   Thin archives can cause linking errors with some toolchains."
    echo "   Solution: Rebuild with ARFLAGS='--no-thin'"
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
    echo "   These should link correctly with your Switch toolchain."
fi

echo ""
echo "📋 Archive details:"
for archive in $archives; do
    size=$(ls -lh "$archive" | awk '{print $5}')
    echo "  $archive ($size)"
done

echo ""
echo "🎉 Archive verification completed successfully!" 