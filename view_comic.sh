#!/bin/bash
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

echo "Aircraft Manager Safety Comic - Viewer Options"
echo "=============================================="
echo
echo "1. Interactive Terminal Viewer (recommended)"
echo "2. HTML Viewer (open in browser)"
echo "3. ASCII Text Viewer"
echo "4. SVG Image Viewer"
echo
echo -n "Choose option [1-4]: "
read choice

case $choice in
    1)
        echo "Launching interactive terminal viewer..."
        echo "Controls: Arrow keys to navigate, 'a' for all panels, 'q' to quit"
        sleep 2
        ./comic_viewer
        ;;
    2)
        echo "Opening HTML viewer in default browser..."
        xdg-open aircraft_safety_comic.html 2>/dev/null || \
        open aircraft_safety_comic.html 2>/dev/null || \
        echo "Please open aircraft_safety_comic.html in your browser"
        ;;
    3)
        echo "Showing ASCII comic (press q to quit)..."
        less aircraft_safety_comic.txt
        ;;
    4)
        echo "Opening SVG image..."
        xdg-open aircraft_safety_comic.svg 2>/dev/null || \
        open aircraft_safety_comic.svg 2>/dev/null || \
        echo "Please open aircraft_safety_comic.svg in an image viewer"
        ;;
    *)
        echo "Invalid choice"
        exit 1
        ;;
esac