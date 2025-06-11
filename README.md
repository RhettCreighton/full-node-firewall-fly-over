# Full Node: Firewall Fly-over 🎮✈️🔥

The ultimate multiplayer aerial combat game where you defend the blockchain network from cyber threats!

## 🚀 What's New

- **Infinite Bullets** - Hold R2 for continuous firewall defense streams
- **Pac-Man Ghost AI** - Enemy nodes move with predictable patterns
- **Kentucky Derby Names** - Face legendary adversaries like SECRETARIAT and SEATTLE SLEW
- **Big Explosions** - Massive visual feedback when eliminating threats
- **World Wrapping** - Seamless planet-surface battlefield

## Quick Start

```bash
# Build
./build.sh

# Run
./build/sky_combat_multiplayer_incremental
```

## Controls

### Gamepad (ASTRO C40)
- **Left Stick** - Fly your node
- **Right Stick** - Aim firewall stream
- **R2** - Fire continuous security stream
- **L2** - Missiles
- **UL** - Throttle (30% speed boost)
- **UR** - Turbo boost (mushroom-style burst)
- **L1/R1** - Barrel rolls

### Keyboard
- **WASD/Arrows** - Fly
- **Mouse** - Aim firewall
- **Left Ctrl** - Fire
- **Space** - Boost
- **Q/E** - Barrel rolls

## Multiplayer Features

- 5 simultaneous nodes in the network
- Real-time combat with collision detection
- Respawn system with 3-second delay
- Kill tracking and statistics
- Team support (ready for future updates)

## Truth Bucket System

This game implements a compositional verification approach to prevent crashes:
- Hierarchical truth statements
- Runtime verification
- Formal proof integration points
- See [truth_bucket_system.md](truth_bucket_system.md) for details

## Architecture

Clean MVC architecture with:
- **Models** - Game state, physics, collision detection
- **Views** - Rendering, effects, UI
- **Controllers** - Input handling, game flow

## Building with Safety

```bash
# Build with sanitizers for maximum safety
./build_safe.sh
```

## Dependencies

- raylib 5.x
- CMake 3.10+
- C99 compiler

## License

Apache-2.0

---

*Formerly known as Sky Combat*