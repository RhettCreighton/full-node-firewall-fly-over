#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Package the Robust Build System for distribution

set -e

VERSION="1.0.0"
PACKAGE_NAME="robust-build-system"
PACKAGE_DIR="${PACKAGE_NAME}-${VERSION}"

echo "Packaging Robust Build System v${VERSION}..."

# Clean previous packages
rm -rf "$PACKAGE_DIR" "${PACKAGE_NAME}"*.tar.gz "${PACKAGE_NAME}"*.zip

# Create package directory
mkdir -p "$PACKAGE_DIR"

# Copy essential files
cp -r cmake "$PACKAGE_DIR/"
cp -r scripts "$PACKAGE_DIR/" 2>/dev/null || mkdir -p "$PACKAGE_DIR/scripts"
cp install.sh "$PACKAGE_DIR/"
cp OPTIMIZATION_GUIDE.md "$PACKAGE_DIR/"
cp OPTIMIZATION_SUMMARY.md "$PACKAGE_DIR/"
cp README.md "$PACKAGE_DIR/" 2>/dev/null || echo "# Robust Build System" > "$PACKAGE_DIR/README.md"

# Create minimal example
mkdir -p "$PACKAGE_DIR/example/src"
cp -r example/* "$PACKAGE_DIR/example/" 2>/dev/null || true
cp -r optimized_example/* "$PACKAGE_DIR/example/" 2>/dev/null || true

# Create tarball
tar -czf "${PACKAGE_NAME}-${VERSION}.tar.gz" "$PACKAGE_DIR"

# Create zip for Windows users
zip -qr "${PACKAGE_NAME}-${VERSION}.zip" "$PACKAGE_DIR"

# Create checksums
sha256sum "${PACKAGE_NAME}-${VERSION}.tar.gz" > "${PACKAGE_NAME}-${VERSION}.tar.gz.sha256"
sha256sum "${PACKAGE_NAME}-${VERSION}.zip" > "${PACKAGE_NAME}-${VERSION}.zip.sha256"

# Clean up
rm -rf "$PACKAGE_DIR"

echo "✓ Created ${PACKAGE_NAME}-${VERSION}.tar.gz"
echo "✓ Created ${PACKAGE_NAME}-${VERSION}.zip"
echo "✓ Created checksum files"

# Display usage instructions
cat << EOF

To install from package:
    tar -xzf ${PACKAGE_NAME}-${VERSION}.tar.gz
    cd ${PACKAGE_NAME}-${VERSION}
    ./install.sh --user    # or --system for system-wide
EOF