#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Rhett Creighton
# SPDX-License-Identifier: Apache-2.0

import PIL.Image as Image
import PIL.ImageDraw as ImageDraw
import PIL.ImageFont as ImageFont
import os
import subprocess

def create_panel(width, height, bg_color, content_func):
    """Create a comic panel with the given content"""
    img = Image.new('RGB', (width, height), bg_color)
    draw = ImageDraw.Draw(img)
    content_func(img, draw)
    return img

def draw_code_box(draw, x, y, width, height, code_lines, font_size=14):
    """Draw a code box with syntax highlighting effect"""
    # Background
    draw.rectangle([x, y, x+width, y+height], fill='#1e1e1e', outline='#333', width=2)
    
    # Try to get a monospace font
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", font_size)
    except:
        font = ImageFont.load_default()
    
    line_height = font_size + 4
    current_y = y + 10
    
    for line in code_lines:
        # Simple syntax coloring
        if line.strip().startswith('//') or line.strip().startswith('/*'):
            color = '#608b4e'  # Comment green
        elif 'if' in line or 'for' in line or 'return' in line:
            color = '#c586c0'  # Keyword purple
        elif 'NULL' in line or 'true' in line or 'false' in line:
            color = '#569cd6'  # Constant blue
        else:
            color = '#d4d4d4'  # Default text
            
        draw.text((x + 10, current_y), line, fill=color, font=font)
        current_y += line_height

def draw_speech_bubble(draw, x, y, width, height, text, tail_pos='bottom'):
    """Draw a comic speech bubble"""
    # Main bubble
    draw.rounded_rectangle([x, y, x+width, y+height], radius=15, fill='white', outline='black', width=3)
    
    # Tail
    if tail_pos == 'bottom':
        points = [(x+width//2-10, y+height), (x+width//2+10, y+height), (x+width//2, y+height+20)]
        draw.polygon(points, fill='white', outline='black')
    
    # Text
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16)
    except:
        font = ImageFont.load_default()
    
    # Word wrap
    words = text.split()
    lines = []
    current_line = []
    for word in words:
        current_line.append(word)
        if len(' '.join(current_line)) > 25:
            lines.append(' '.join(current_line[:-1]))
            current_line = [word]
    if current_line:
        lines.append(' '.join(current_line))
    
    y_offset = y + 15
    for line in lines:
        draw.text((x + 10, y_offset), line, fill='black', font=font)
        y_offset += 25

def draw_character(draw, x, y, character_type='developer'):
    """Draw a simple stick figure character"""
    if character_type == 'developer':
        # Head
        draw.ellipse([x-20, y-40, x+20, y], fill='#ffdbac', outline='black', width=2)
        # Body
        draw.line([x, y, x, y+40], fill='black', width=3)
        # Arms
        draw.line([x-20, y+15, x+20, y+15], fill='black', width=3)
        # Legs
        draw.line([x, y+40, x-15, y+60], fill='black', width=3)
        draw.line([x, y+40, x+15, y+60], fill='black', width=3)
        # Glasses
        draw.ellipse([x-15, y-25, x-5, y-15], outline='black', width=2)
        draw.ellipse([x+5, y-25, x+15, y-15], outline='black', width=2)
        draw.line([x-5, y-20, x+5, y-20], fill='black', width=2)
    elif character_type == 'bug':
        # Bug body
        draw.ellipse([x-15, y-10, x+15, y+10], fill='red', outline='black', width=2)
        # Legs
        for i in range(3):
            draw.line([x-15, y-5+i*5, x-25, y-5+i*5], fill='black', width=2)
            draw.line([x+15, y-5+i*5, x+25, y-5+i*5], fill='black', width=2)
        # Antennae
        draw.line([x-5, y-10, x-10, y-20], fill='black', width=2)
        draw.line([x+5, y-10, x+10, y-20], fill='black', width=2)

def panel1(img, draw):
    """Panel 1: The problem"""
    draw.rectangle([0, 0, 600, 50], fill='#2b579a')
    draw.text((20, 15), "Panel 1: The Dreaded Segfault", fill='white', font=ImageFont.load_default())
    
    # Draw worried developer
    draw_character(draw, 100, 200, 'developer')
    draw_speech_bubble(draw, 150, 100, 300, 80, "Oh no! What if someone passes an invalid aircraft ID?", 'bottom')
    
    # Draw menacing bug
    draw_character(draw, 500, 250, 'bug')
    draw.text((450, 280), "SEGFAULT", fill='red', font=ImageFont.load_default())
    
    # Show unsafe code
    code = [
        "// UNSAFE CODE",
        "managed_aircraft_t* get_aircraft(",
        "    aircraft_manager_t* mgr, int id) {",
        "    return &mgr->aircraft[id]; // BOOM!",
        "}"
    ]
    draw_code_box(draw, 50, 320, 500, 120, code)

def panel2(img, draw):
    """Panel 2: The solution approach"""
    draw.rectangle([0, 0, 600, 50], fill='#2b579a')
    draw.text((20, 15), "Panel 2: Truth Bucket to the Rescue!", fill='white', font=ImageFont.load_default())
    
    # Draw confident developer
    draw_character(draw, 100, 200, 'developer')
    draw_speech_bubble(draw, 150, 100, 300, 80, "We'll use formal verification patterns!", 'bottom')
    
    # Draw truth bucket shield
    draw.ellipse([450, 150, 550, 250], fill='gold', outline='black', width=3)
    draw.text((470, 190), "TRUTH", fill='black', font=ImageFont.load_default())
    
    # Show solution preview
    code = [
        "// VERIFIED SAFE",
        "TRUTH_REQUIRE(truth, ",
        "    \"manager is not null\",",
        "    manager != NULL);",
        "TRUTH_REQUIRE(truth,",
        "    \"id is in bounds\",", 
        "    id >= 0 && id < mgr->count);"
    ]
    draw_code_box(draw, 50, 320, 500, 140, code, font_size=12)

def panel3(img, draw):
    """Panel 3: Bounds checking"""
    draw.rectangle([0, 0, 600, 50], fill='#2b579a')
    draw.text((20, 15), "Panel 3: Every Access is Bounds-Checked", fill='white', font=ImageFont.load_default())
    
    # Visual array representation
    array_x, array_y = 50, 150
    for i in range(16):
        color = '#90ee90' if i < 5 else '#f0f0f0'
        draw.rectangle([array_x + i*30, array_y, array_x + i*30 + 25, array_y + 40], 
                      fill=color, outline='black', width=2)
        draw.text((array_x + i*30 + 8, array_y + 10), str(i), fill='black')
    
    draw.text((50, 120), "aircraft[16] - Count: 5", fill='black', font=ImageFont.load_default())
    
    # Show verification
    code = [
        "// Runtime verification",
        "static bool verify_aircraft_count_bounds(",
        "    const truth_bucket_t* bucket,", 
        "    void* context) {",
        "    aircraft_manager_t* mgr = context;",
        "    return mgr != NULL && ",
        "           mgr->aircraft_count >= 0 &&",
        "           mgr->aircraft_count <= MAX_AIRCRAFT;",
        "}"
    ]
    draw_code_box(draw, 50, 250, 500, 180, code, font_size=12)

def panel4(img, draw):
    """Panel 4: Safe wrapper function"""
    draw.rectangle([0, 0, 600, 50], fill='#2b579a')
    draw.text((20, 15), "Panel 4: The Safe Wrapper", fill='white', font=ImageFont.load_default())
    
    # Flow diagram
    draw.rectangle([50, 100, 250, 140], fill='#ffcccc', outline='black', width=2)
    draw.text((80, 110), "Unsafe Request", fill='black')
    
    draw.line([250, 120, 350, 120], fill='black', width=2, arrow='last')
    
    draw.rectangle([350, 100, 550, 140], fill='#ccffcc', outline='black', width=2)
    draw.text((380, 110), "Truth Verification", fill='black')
    
    # Safe wrapper code
    code = [
        "managed_aircraft_t* aircraft_manager_get_safe(",
        "    aircraft_manager_t* manager, int id,",
        "    truth_bucket_t* truth) {",
        "    ",
        "    TRUTH_REQUIRE(truth, \"valid manager\",",
        "                  manager != NULL);",
        "    TRUTH_REQUIRE(truth, \"valid id\",", 
        "                  id >= 0 && id < manager->count);",
        "    ",
        "    return &manager->aircraft[id]; // SAFE!",
        "}"
    ]
    draw_code_box(draw, 50, 200, 500, 220, code, font_size=11)

def panel5(img, draw):
    """Panel 5: F* formal proof"""
    draw.rectangle([0, 0, 600, 50], fill='#2b579a')
    draw.text((20, 15), "Panel 5: Mathematical Proof with F*", fill='white', font=ImageFont.load_default())
    
    # Draw mathematician character
    draw_character(draw, 100, 150, 'developer')
    draw_speech_bubble(draw, 150, 80, 250, 60, "Proven at compile time!", 'bottom')
    
    # F* proof
    code = [
        "// F* Type System Proof",
        "type valid_count = n:nat{n <= 16}",
        "type valid_id (count:valid_count) = ",
        "    n:nat{n < count}",
        "",
        "let get_aircraft (mgr:aircraft_manager)",
        "    (id:valid_id mgr.count) =",
        "    mgr.aircraft.(id) (* Type-safe! *)",
        "",
        "// Compiler rejects invalid access!"
    ]
    draw_code_box(draw, 50, 200, 500, 220, code, font_size=11)

def panel6(img, draw):
    """Panel 6: Victory"""
    draw.rectangle([0, 0, 600, 50], fill='#2b579a')
    draw.text((20, 15), "Panel 6: Segfault Defeated!", fill='white', font=ImageFont.load_default())
    
    # Happy developer
    draw_character(draw, 300, 200, 'developer')
    draw_speech_bubble(draw, 100, 100, 400, 80, "No more segfaults! Every access is verified safe!", 'bottom')
    
    # Dead bug
    draw.ellipse([480, 280, 510, 300], fill='gray', outline='black', width=2)
    draw.text((460, 310), "RIP Segfault", fill='black')
    draw.line([470, 270, 520, 320], fill='red', width=3)
    draw.line([520, 270, 470, 320], fill='red', width=3)
    
    # Summary
    draw.rectangle([50, 350, 550, 430], fill='#e6ffe6', outline='black', width=2)
    draw.text((70, 360), "✓ Null checks on every access", fill='black')
    draw.text((70, 380), "✓ Bounds verified before array access", fill='black')
    draw.text((70, 400), "✓ Type system prevents invalid states", fill='black')

# Generate all panels
panels = []
panel_funcs = [panel1, panel2, panel3, panel4, panel5, panel6]

for i, func in enumerate(panel_funcs):
    panel = create_panel(600, 450, '#f0f0f0', func)
    panels.append(panel)
    panel.save(f'/home/bob/github/canvas/games/sky-combat/panel_{i+1}.png')

# Create comic strip
comic_width = 1200
comic_height = 900
comic = Image.new('RGB', (comic_width, comic_height), 'white')

# Arrange panels in 2x3 grid
for i, panel in enumerate(panels):
    x = (i % 2) * 600
    y = (i // 2) * 450
    comic.paste(panel, (x, y))

# Save comic
comic.save('/home/bob/github/canvas/games/sky-combat/aircraft_safety_comic.png')

# Create HTML viewer
html_content = """<!DOCTYPE html>
<html>
<head>
    <title>Aircraft Manager Safety Comic</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #1e1e1e;
            color: white;
            margin: 0;
            padding: 20px;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        h1 {
            color: #4CAF50;
            margin-bottom: 20px;
        }
        .comic-container {
            background: white;
            border: 5px solid #333;
            box-shadow: 0 0 20px rgba(0,0,0,0.5);
            margin-bottom: 20px;
        }
        .description {
            max-width: 1200px;
            background: #2d2d2d;
            padding: 20px;
            border-radius: 10px;
            line-height: 1.6;
        }
        .code-snippet {
            background: #1e1e1e;
            border: 1px solid #444;
            padding: 15px;
            margin: 10px 0;
            font-family: monospace;
            overflow-x: auto;
        }
        .panel-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            max-width: 1200px;
            margin: 20px 0;
        }
        .panel-view {
            background: white;
            border: 3px solid #333;
            cursor: pointer;
            transition: transform 0.2s;
        }
        .panel-view:hover {
            transform: scale(1.05);
        }
        .panel-view img {
            width: 100%;
            display: block;
        }
    </style>
</head>
<body>
    <h1>🛡️ How Aircraft Manager Prevents Segfaults</h1>
    
    <div class="comic-container">
        <img src="aircraft_safety_comic.png" width="1200" height="900" alt="Aircraft Safety Comic">
    </div>
    
    <div class="description">
        <h2>The Story</h2>
        <p>This comic illustrates how the Sky Combat game's aircraft manager uses formal verification 
        patterns to guarantee memory safety. Instead of hoping that array accesses are valid, we 
        <strong>prove</strong> they are valid using Truth Buckets.</p>
        
        <h2>Key Safety Mechanisms</h2>
        <ol>
            <li><strong>Truth Requirements:</strong> Every function that accesses the aircraft array must 
            first verify preconditions using TRUTH_REQUIRE macros.</li>
            
            <li><strong>Bounds Checking:</strong> The verify_aircraft_count_bounds function ensures the 
            aircraft count never exceeds MAX_MANAGED_AIRCRAFT (16).</li>
            
            <li><strong>Safe Wrappers:</strong> The aircraft_manager_get_safe function wraps unsafe 
            operations with verification checks.</li>
            
            <li><strong>F* Type System:</strong> The formal specification uses dependent types to make 
            invalid array access impossible at the type level.</li>
        </ol>
        
        <h2>Example: Safe Access Pattern</h2>
        <div class="code-snippet">
// Instead of this dangerous code:
managed_aircraft_t* aircraft = &mgr->aircraft[id];  // BOOM if id invalid!

// We use this verified safe pattern:
managed_aircraft_t* aircraft = aircraft_manager_get_safe(mgr, id, truth_bucket);
// Returns NULL if preconditions fail, never segfaults!
        </div>
        
        <h2>Individual Panels</h2>
    </div>
    
    <div class="panel-grid">
        <div class="panel-view" onclick="window.open('panel_1.png')">
            <img src="panel_1.png" alt="Panel 1">
        </div>
        <div class="panel-view" onclick="window.open('panel_2.png')">
            <img src="panel_2.png" alt="Panel 2">
        </div>
        <div class="panel-view" onclick="window.open('panel_3.png')">
            <img src="panel_3.png" alt="Panel 3">
        </div>
        <div class="panel-view" onclick="window.open('panel_4.png')">
            <img src="panel_4.png" alt="Panel 4">
        </div>
        <div class="panel-view" onclick="window.open('panel_5.png')">
            <img src="panel_5.png" alt="Panel 5">
        </div>
        <div class="panel-view" onclick="window.open('panel_6.png')">
            <img src="panel_6.png" alt="Panel 6">
        </div>
    </div>
    
    <div class="description">
        <h2>Learn More</h2>
        <p>Check out the actual implementation in:</p>
        <ul>
            <li><code>src/models/aircraft_manager_truths.c</code> - Truth verification implementation</li>
            <li><code>include/sky_combat/truth_bucket.h</code> - Truth Bucket framework</li>
            <li><code>truth_bucket_system.md</code> - Complete documentation</li>
        </ul>
    </div>
</body>
</html>
"""

with open('/home/bob/github/canvas/games/sky-combat/aircraft_safety_comic.html', 'w') as f:
    f.write(html_content)

print("Comic generated successfully!")
print("Files created:")
print("- aircraft_safety_comic.png (full comic)")
print("- aircraft_safety_comic.html (interactive viewer)")
print("- panel_1.png through panel_6.png (individual panels)")