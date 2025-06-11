# Gun Tuning Solution - Right Stick Bends Bullet Stream

## Problem
The user wanted:
"i want the right joystick to bend the bullet stream the direction I push"

While also maintaining that bullets shoot straight initially.

## Solution
The right stick now BENDS the bullet stream after firing:

### How It Works
1. **Bullets shoot straight initially** - All bullets use forward vector at spawn
2. **Right stick bends the stream** - Applies acceleration to curve bullets in flight
3. **Visual feedback** - Crosshair shows current bend direction

### Right Stick Controls
- **X-Axis**: Bends bullets left/right
- **Y-Axis**: Bends bullets up/down (inverted for intuitive control)
- **Center**: No bend, bullets fly straight

### Key Implementation Details

1. **Bullet structure updated:**
   ```c
   typedef struct bullet_s {
       Vector3 position;
       Vector3 velocity;
       Vector3 bend_force;  // Bending force applied by right stick
       // ...
   }
   ```

2. **Weapon system parameters:**
   ```c
   float bullet_bend_x;     // -1.0 to 1.0, bends left/right
   float bullet_bend_y;     // -1.0 to 1.0, bends up/down
   float bend_strength;     // How much bullets curve (default: 30.0f)
   ```

3. **Bullet physics:**
   ```c
   // Apply bending force to velocity (curves the bullet path)
   current->velocity = Vector3Add(current->velocity, 
       Vector3Scale(current->bend_force, dt));
   ```

4. **Visual feedback:**
   - "BULLET BEND" indicator on HUD
   - Crosshair visualization shows bend direction
   - Orange dot indicates current stick position
   - X/Y values displayed numerically

## Result
Players can now dynamically curve their bullet streams around obstacles or to hit enemies behind cover. The bullets still shoot straight forward initially (satisfying the "GUNS SHOULD GO STRAIGHT" requirement) but bend in flight based on right stick input.