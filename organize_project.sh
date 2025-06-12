#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

echo "=== ORGANIZING SKY COMBAT PROJECT ==="
echo "This will clean up and organize everything into a professional structure"
echo

# Create organized structure
echo "Creating organized directory structure..."

# Archive old experiments
mkdir -p archive/experiments
mkdir -p archive/demos
mkdir -p archive/proofs
mkdir -p archive/docs
mkdir -p archive/graphics

# Move experimental files to archive
echo "Archiving experimental files..."
mv sky_combat_*.c archive/experiments/ 2>/dev/null
mv test_*.c archive/experiments/ 2>/dev/null
mv fix_*.c archive/experiments/ 2>/dev/null
mv sky_combat_fun sky_combat_city sky_combat_demo archive/experiments/ 2>/dev/null
mv test_joystick_* archive/experiments/ 2>/dev/null
mv play_fun.sh archive/experiments/ 2>/dev/null

# Move proof-related files
mv prove_*.sh archive/proofs/ 2>/dev/null
mv verify_*.sh archive/proofs/ 2>/dev/null
mv gdb_proofs/ archive/proofs/ 2>/dev/null
mv rhett-proof-system/ archive/proofs/ 2>/dev/null
mv gdb-error-proof/ archive/proofs/ 2>/dev/null

# Move documentation
mv *_COMPLETE.md *_FIXED.md *_INTEGRATION.md archive/docs/ 2>/dev/null
mv *_GUIDE.md *_SUMMARY.md archive/docs/ 2>/dev/null

# Move graphics-related
mv *.html archive/graphics/ 2>/dev/null
mv generate_* archive/graphics/ 2>/dev/null
mv *storyboard* archive/graphics/ 2>/dev/null
mv leonardo_* archive/graphics/ 2>/dev/null
mv comic_* archive/graphics/ 2>/dev/null
mv images/ archive/graphics/ 2>/dev/null

# Move build scripts
mkdir -p archive/build_scripts
mv build_*.sh archive/build_scripts/ 2>/dev/null
mv run*.sh archive/build_scripts/ 2>/dev/null
mv view_*.sh archive/build_scripts/ 2>/dev/null

# Now set up the clean project structure
echo "Setting up clean project structure..."

# Main source structure
mkdir -p src/core
mkdir -p src/models
mkdir -p src/views  
mkdir -p src/controllers
mkdir -p src/systems
mkdir -p src/utils

# Include structure
mkdir -p include/sky_combat/core
mkdir -p include/sky_combat/models
mkdir -p include/sky_combat/views
mkdir -p include/sky_combat/controllers
mkdir -p include/sky_combat/systems
mkdir -p include/sky_combat/utils

# Tests
mkdir -p tests/unit
mkdir -p tests/integration
mkdir -p tests/performance

# Documentation
mkdir -p docs/api
mkdir -p docs/design
mkdir -p docs/guides

# Resources
mkdir -p resources/shaders
mkdir -p resources/textures
mkdir -p resources/models
mkdir -p resources/sounds

# Scripts
mkdir -p scripts/build
mkdir -p scripts/test
mkdir -p scripts/deploy

echo
echo "=== PROJECT STRUCTURE ==="
echo "
sky-combat/
├── CMakeLists.txt          # Main build configuration
├── README.md               # Project overview
├── LICENSE                 # Apache 2.0
├── .gitignore             # Properly configured
│
├── src/                   # Source files
│   ├── core/             # Core game systems
│   ├── models/           # Game models (aircraft, world, etc)
│   ├── views/            # Rendering and UI
│   ├── controllers/      # Input and game logic
│   ├── systems/          # Physics, AI, networking
│   └── utils/            # Utilities and helpers
│
├── include/              # Public headers
│   └── sky_combat/       # Namespaced headers
│
├── tests/                # Test suite
│   ├── unit/            # Unit tests
│   ├── integration/     # Integration tests
│   └── performance/     # Performance tests
│
├── docs/                 # Documentation
│   ├── api/             # API documentation
│   ├── design/          # Design documents
│   └── guides/          # User guides
│
├── resources/           # Game assets
│   ├── shaders/        # GLSL shaders
│   ├── textures/       # Textures
│   ├── models/         # 3D models
│   └── sounds/         # Audio files
│
├── scripts/             # Build and deploy scripts
│   ├── build/          # Build scripts
│   ├── test/           # Test scripts
│   └── deploy/         # Deployment scripts
│
├── build/               # Build output (git ignored)
└── archive/             # Old experiments (git ignored)
"

echo
echo "Files organized! Old experiments moved to archive/"
echo "The main project is now clean and professional."
echo
echo "Next steps:"
echo "1. Run: ./create_main_project.sh"
echo "2. Review the clean structure"
echo "3. Commit the organized project"