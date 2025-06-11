#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

cd "$(dirname "$0")"

echo "=== SKY COMBAT: 5 PLAYER BATTLE ==="
echo
echo "You are the GREEN aircraft fighting 4 AI enemies!"
echo
echo "Controls:"
echo "  WASD/Arrows - Fly"
echo "  Space - Fire" 
echo "  Shift - Boost"
echo "  1-5 - Switch camera view"
echo
echo "Starting battle..."
echo

./build/sky_combat_5player