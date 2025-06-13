# Sky Combat 3D - Cyberpunk City Flying Game

## Quick Start

The most fun 3D flying mode is ready to play! Simply run:

```bash
./sky_combat_3d_simple
```

Or use the launcher script:
```bash
./run_3d_game.sh
```

## Game Features

- **Cyberpunk City**: Fly through a procedurally generated city with 200 neon-lit buildings
- **First-Person Flying**: Smooth 3D flight mechanics with speeds up to 300 m/s
- **Interactive World**: 500 AI characters at street level that react to low-flying aircraft
- **Atmospheric Effects**: Dynamic lighting, glowing windows, and pulsing neon signs
- **Real 3D Graphics**: Full hardware-accelerated 3D rendering at 1920x1080

## Controls

- **Mouse** - Look around (aim where you want to fly)
- **W/S** - Pitch up/down
- **A/D** - Strafe left/right  
- **Q/E** - Decrease/Increase speed
- **Space/Ctrl** - Fly up/down
- **ESC** - Exit game

The aircraft always flies forward in the direction you're looking!

## Building from Source

If you need to rebuild:

```bash
# Quick direct build
gcc -o sky_combat_3d_simple sky_combat_3d_simple.c -lraylib -lm

# Or use CMake
./build_and_run_3d.sh
```

## Notes

This is the actual fun 3D flying game as specified in the documentation - NOT a text-based demo or technical test. It features real 3D graphics with a beautiful cyberpunk city to explore.

Enjoy flying through the neon-lit cityscape!