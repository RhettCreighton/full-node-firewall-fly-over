# Sky Combat - Incremental Multiplayer Implementation Plan

## Principle: Start with the working original, add multiplayer features one by one

## Phase 1: Copy Original as Base ✓
- [x] Start with exact copy of sky_combat_ultimate.c
- [x] Rename to sky_combat_multiplayer_incremental.c
- [x] Verify it still works identically

## Phase 2: Replace Single Aircraft with Aircraft Manager
- [ ] Add aircraft_manager to game struct
- [ ] Create manager with 1 aircraft (player)
- [ ] Replace all `game->aircraft` references with `aircraft_manager_get(mgr, 0)`
- [ ] Test: Game should work exactly the same

## Phase 3: Add AI Aircraft (No Combat Yet)
- [ ] Add 4 AI aircraft to manager
- [ ] Give them team colors (2 red, 2 blue)
- [ ] Make them fly around using AI pilot
- [ ] Test: Player controls unchanged, AI visible

## Phase 4: Enable Aircraft Combat
- [ ] Enable weapon collision detection for all aircraft
- [ ] Add damage system from aircraft_manager
- [ ] Add respawn system
- [ ] Test: Can shoot AI, they respawn

## Phase 5: Add Team System
- [ ] Set player to team 0
- [ ] AI targets based on teams
- [ ] Friendly fire disabled
- [ ] Test: Teams fight each other

## Phase 6: Add Match Rules
- [ ] Add match_state system
- [ ] Track team scores
- [ ] Time limits
- [ ] Test: Score tracking works

## Phase 7: Add Powerups
- [ ] Add powerup_manager
- [ ] Spawn points around map
- [ ] Collection effects
- [ ] Test: Powerups work for all aircraft

## Phase 8: Preserve All Original Features
- [ ] Overdrive system still works
- [ ] Boss spawns after waves (single player mode)
- [ ] All weapons work
- [ ] All visual effects work
- [ ] Controls identical
- [ ] Camera identical
- [ ] UI adapted for multiplayer

## Key Files to Create

### sky_combat_multiplayer_incremental.c
```c
// Start as exact copy of sky_combat_ultimate.c
// Modify incrementally per phases above
```

## Testing After Each Phase

1. **Controls Test**
   - Left stick movement
   - Right stick camera
   - UR boost
   - UL throttle
   - All weapon types
   
2. **Visual Test**
   - All effects working
   - Speed lines
   - Screen shake/flash
   - Particles
   
3. **Gameplay Test**
   - Overdrive charges/activates
   - Combos work
   - Rank progression
   - Wave system (if applicable)

## Regression Prevention

### DO NOT CHANGE:
- Input system behavior
- Aircraft physics constants
- Camera behavior
- Effect timings
- Control mappings

### CAREFULLY PRESERVE:
- All visual effects
- All UI elements (adapt for multiplayer)
- All sound cues (when added)
- Game feel and timing

### ADAPT THOUGHTFULLY:
- HUD for multiple players
- Score display for teams
- Enemy waves (optional in multiplayer)
- Boss battles (single player mode only?)

## Implementation Order

1. Copy original file
2. Add aircraft_manager (Phase 2)
3. Test thoroughly
4. Add AI aircraft (Phase 3)
5. Test thoroughly
6. Continue phase by phase
7. Never skip testing between phases

This incremental approach ensures we never lose the excellent game feel of the original while carefully adding multiplayer features.