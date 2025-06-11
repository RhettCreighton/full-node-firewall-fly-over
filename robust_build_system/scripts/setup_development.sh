#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

# Setup script for developers to quickly start using Robust Build System

set -e

echo "Robust Build System - Developer Setup"
echo "===================================="
echo

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
    PACKAGE_MANAGER=""
    if command -v apt &>/dev/null; then
        PACKAGE_MANAGER="apt"
    elif command -v yum &>/dev/null; then
        PACKAGE_MANAGER="yum"
    elif command -v pacman &>/dev/null; then
        PACKAGE_MANAGER="pacman"
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
    PACKAGE_MANAGER="brew"
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

echo "Detected OS: $OS"
echo "Package manager: $PACKAGE_MANAGER"
echo

# Install dependencies based on OS
install_dependencies() {
    case $PACKAGE_MANAGER in
        apt)
            echo "Installing dependencies with apt..."
            sudo apt update
            sudo apt install -y \
                build-essential \
                cmake \
                ninja-build \
                ccache \
                clang \
                clang-tools \
                git
            ;;
        yum)
            echo "Installing dependencies with yum..."
            sudo yum install -y \
                gcc \
                gcc-c++ \
                cmake \
                ninja-build \
                ccache \
                clang \
                git
            ;;
        pacman)
            echo "Installing dependencies with pacman..."
            sudo pacman -S --needed \
                base-devel \
                cmake \
                ninja \
                ccache \
                clang \
                git
            ;;
        brew)
            echo "Installing dependencies with Homebrew..."
            brew install \
                cmake \
                ninja \
                ccache \
                llvm
            ;;
        *)
            echo "Unknown package manager. Please install manually:"
            echo "  - CMake 3.16+"
            echo "  - Ninja build"
            echo "  - ccache"
            echo "  - Clang/LLVM (optional)"
            exit 1
            ;;
    esac
}

# Prompt for installation
read -p "Install system dependencies? [y/N] " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    install_dependencies
fi

# Configure ccache
echo
echo "Configuring ccache..."
ccache --set-config compression=true
ccache --set-config compression_level=1
ccache --set-config max_size=5G
echo "ccache configured with 5GB cache"

# Setup git hooks for projects using Robust Build
echo
echo "Creating git hook template..."
mkdir -p ~/.git-templates/hooks

cat > ~/.git-templates/hooks/pre-commit << 'EOF'
#!/bin/bash
# Pre-commit hook for Robust Build System projects

# Check if this is a Robust Build project
if grep -q "RobustBuildSystem" CMakeLists.txt 2>/dev/null; then
    echo "Running Robust Build verification..."
    
    # Create temporary build directory
    TEMP_BUILD=$(mktemp -d)
    trap "rm -rf $TEMP_BUILD" EXIT
    
    # Try to build with proof verification
    if ! cmake -B "$TEMP_BUILD" -DENABLE_PROOF_SYSTEM=ON -DENABLE_ULTRA_PROOF=ON &>/dev/null; then
        echo "❌ CMake configuration failed"
        exit 1
    fi
    
    if ! cmake --build "$TEMP_BUILD" --target verify_proofs &>/dev/null; then
        echo "❌ Proof verification failed"
        echo "   One or more required functions are not being called"
        exit 1
    fi
    
    echo "✅ Proof verification passed"
fi
EOF

chmod +x ~/.git-templates/hooks/pre-commit

# Create development environment file
cat > ~/.robust_build_env << 'EOF'
# Robust Build System Environment

# Enable ccache
export PATH="/usr/lib/ccache:$PATH"

# Build aliases
alias rb-debug='cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -G Ninja && ninja -C build-debug'
alias rb-release='cmake -B build-release -DCMAKE_BUILD_TYPE=Release -DENABLE_ULTRA_PROOF=ON -G Ninja && ninja -C build-release'
alias rb-clean='rm -rf build*'
alias rb-test='ctest --test-dir build-debug --output-on-failure'

# Quick project setup
rb-init() {
    cat > CMakeLists.txt << 'CMAKE'
cmake_minimum_required(VERSION 3.16)
project(${1:-MyProject} C)

include(RobustBuildSystem)
robust_c99_project()

add_robust_executable(${1:-app}
    REQUIRE_PROOF
    PROOF_FUNCTION "main_init"
    SOURCES src/main.c
)
CMAKE

    mkdir -p src
    cat > src/main.c << 'CSRC'
/* SPDX-FileCopyrightText: 2025 Rhett Creighton
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

void main_init(void) {
    printf("Initialized with Robust Build System!\n");
}

int main(void) {
    main_init();  /* Required by proof system */
    return 0;
}
CSRC

    echo "Created Robust Build project: ${1:-MyProject}"
}
EOF

# Add to shell RC file
SHELL_RC=""
if [[ -f ~/.bashrc ]]; then
    SHELL_RC=~/.bashrc
elif [[ -f ~/.zshrc ]]; then
    SHELL_RC=~/.zshrc
fi

if [[ -n "$SHELL_RC" ]]; then
    if ! grep -q "robust_build_env" "$SHELL_RC"; then
        echo
        echo "Adding Robust Build environment to $SHELL_RC..."
        echo "" >> "$SHELL_RC"
        echo "# Robust Build System" >> "$SHELL_RC"
        echo "[ -f ~/.robust_build_env ] && source ~/.robust_build_env" >> "$SHELL_RC"
    fi
fi

# Create VS Code settings template
echo
echo "Creating VS Code settings template..."
mkdir -p .vscode
cat > .vscode/robust_build_settings.json << 'EOF'
{
    "cmake.generator": "Ninja",
    "cmake.buildDirectory": "${workspaceFolder}/build-${buildType}",
    "cmake.configureSettings": {
        "ENABLE_ULTRA_PROOF": "ON",
        "ENABLE_NATIVE_ARCH": "ON"
    },
    "C_Cpp.default.cStandard": "c99",
    "files.associations": {
        "*.cmake": "cmake",
        "CMakeLists.txt": "cmake"
    }
}
EOF

echo
echo "Setup Complete!"
echo "=============="
echo
echo "✅ Dependencies checked"
echo "✅ ccache configured"
echo "✅ Git hooks created"
echo "✅ Development environment created"
echo
echo "Next steps:"
echo "1. Reload your shell or run: source ~/.robust_build_env"
echo "2. Install Robust Build System: ./install.sh --user"
echo "3. Create a new project: rb-init myproject"
echo "4. Use aliases: rb-debug, rb-release, rb-test"
echo
echo "For VS Code users:"
echo "  Copy .vscode/robust_build_settings.json to your project's .vscode/settings.json"