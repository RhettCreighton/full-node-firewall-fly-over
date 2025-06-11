#!/usr/bin/env python3
"""
Add beautiful speech bubbles to The Eternal Core comic images
"""

import os
import requests
from PIL import Image, ImageDraw, ImageFont
import textwrap
import io
import json
from datetime import datetime

# Comic dialogue for each scene
SCENE_DIALOGUES = {
    "Academy Garden": [
        ("SOCRATES", "Tell me Plato, what happens if I try to water garden plot number 16?"),
        ("PLATO", "Observe Master! The guards prevent it: if (index >= 16) return -1;"),
        ("SOCRATES", "Ah! So boundaries are enforced by logic, not mere hope!"),
        ("PLATO", "Exactly! Just as these gardens have exactly 16 plots, numbered 0 through 15.")
    ],
    "Null Bridge": [
        ("SOCRATES", "What if this bridge to knowledge is broken - what if it is NULL?"),
        ("PLATO", "Then we must not cross! if (!pointer) return; - We verify existence first!"),
        ("SOCRATES", "So wisdom is checking the bridge before stepping upon it?"),
        ("PLATO", "Yes Master! The void below represents the chaos of dereferenced NULL.")
    ],
    "Oracle": [
        ("SOCRATES", "The Oracle speaks in riddles. What does 'Know Thy Code' mean?"),
        ("PLATO", "Through Truth Buckets and formal proofs, we achieve absolute certainty!"),
        ("SOCRATES", "Then this Full Node game has what we philosophers seek - certain knowledge!"),
        ("BOTH", "No coredump shall ever occur! The proof is complete!")
    ]
}

def create_speech_bubble(draw, x, y, width, height, tail_side='left'):
    """Create a beautiful speech bubble with tail"""
    # Main bubble
    radius = 20
    
    # Draw rounded rectangle
    draw.rounded_rectangle(
        [(x, y), (x + width, y + height)],
        radius=radius,
        fill='white',
        outline='black',
        width=3
    )
    
    # Add tail
    tail_width = 30
    tail_height = 20
    
    if tail_side == 'left':
        tail_points = [
            (x + 50, y + height),
            (x + 50 + tail_width, y + height),
            (x + 30, y + height + tail_height)
        ]
    else:
        tail_points = [
            (x + width - 50 - tail_width, y + height),
            (x + width - 50, y + height),
            (x + width - 30, y + height + tail_height)
        ]
    
    # Draw tail
    draw.polygon(tail_points, fill='white', outline='black', width=3)
    
    # Cover the line where tail meets bubble
    if tail_side == 'left':
        draw.rectangle(
            [(x + 48, y + height - 3), (x + 83, y + height + 3)],
            fill='white',
            outline=None
        )
    else:
        draw.rectangle(
            [(x + width - 83, y + height - 3), (x + width - 48, y + height + 3)],
            fill='white',
            outline=None
        )

def add_dialogue_to_image(image_url, dialogue_list, output_path, scene_title):
    """Add beautiful speech bubbles with dialogue to an image"""
    
    print(f"Adding dialogue to {scene_title}...")
    
    # Download and open image
    response = requests.get(image_url)
    img = Image.open(io.BytesIO(response.content))
    draw = ImageDraw.Draw(img)
    
    # Try to load nice fonts
    try:
        # Font paths for different systems
        font_paths = [
            "/usr/share/fonts/truetype/liberation/LiberationSerif-Bold.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf",
            "/System/Library/Fonts/Times.ttc",
            "C:\\Windows\\Fonts\\times.ttf"
        ]
        
        title_font = None
        for path in font_paths:
            if os.path.exists(path):
                title_font = ImageFont.truetype(path, 28)
                break
        
        if not title_font:
            title_font = ImageFont.load_default()
            
        name_font = ImageFont.truetype(font_paths[0] if os.path.exists(font_paths[0]) else font_paths[1], 22)
        text_font = ImageFont.truetype(font_paths[0].replace('-Bold', '') if os.path.exists(font_paths[0].replace('-Bold', '')) else font_paths[1].replace('-Bold', ''), 18)
    except:
        # Fallback to default
        title_font = ImageFont.load_default()
        name_font = title_font
        text_font = title_font
    
    # Add scene title banner
    banner_height = 60
    draw.rectangle([(0, 0), (img.width, banner_height)], fill='#8B4513', outline=None)
    draw.text((img.width//2, banner_height//2), scene_title, fill='white', font=title_font, anchor='mm')
    
    # Calculate bubble positions
    bubble_width = 380
    bubble_padding = 15
    line_spacing = 22
    y_start = 80
    y_spacing = 20
    
    current_y = y_start
    
    for i, (speaker, text) in enumerate(dialogue_list):
        # Calculate bubble height based on text
        wrapped_text = textwrap.wrap(text, width=35)
        text_height = len(wrapped_text) * line_spacing
        bubble_height = text_height + 45  # Extra space for speaker name
        
        # Alternate sides - Socrates left, Plato right
        if speaker == "SOCRATES" or (speaker == "BOTH" and i % 2 == 0):
            x = 30
            tail_side = 'left'
        else:
            x = img.width - bubble_width - 30
            tail_side = 'right'
        
        # Create beautiful speech bubble
        create_speech_bubble(draw, x, current_y, bubble_width, bubble_height, tail_side)
        
        # Add speaker name with philosophical styling
        name_color = '#8B4513' if speaker == "SOCRATES" else '#4169E1'
        if speaker == "BOTH":
            name_color = '#800080'
            
        draw.text(
            (x + bubble_padding, current_y + bubble_padding),
            speaker + ":",
            fill=name_color,
            font=name_font
        )
        
        # Add wrapped text
        text_y = current_y + bubble_padding + 30
        for line in wrapped_text:
            draw.text(
                (x + bubble_padding, text_y),
                line,
                fill='black',
                font=text_font
            )
            text_y += line_spacing
        
        # Move to next position
        current_y += bubble_height + y_spacing
    
    # Add philosophical footer
    footer_height = 50
    footer_y = img.height - footer_height
    draw.rectangle([(0, footer_y), (img.width, img.height)], fill='#F5F5DC', outline=None)
    draw.text(
        (img.width//2, footer_y + footer_height//2),
        '"The unexamined code is not worth running" - Socrates',
        fill='#8B4513',
        font=text_font,
        anchor='mm'
    )
    
    # Save the enhanced image
    img.save(output_path, quality=95)
    print(f"✅ Saved: {output_path}")

def enhance_comic():
    """Enhance the generated comic with beautiful speech bubbles"""
    
    # Load the latest generation record
    records = sorted([f for f in os.listdir('docs/comic') if f.startswith('generation_record_')])
    if not records:
        print("❌ No generation records found. Run generate_comic_demo.py first!")
        return
        
    latest_record = records[-1]
    print(f"Using generation record: {latest_record}")
    
    with open(f'docs/comic/{latest_record}', 'r') as f:
        data = json.load(f)
    
    # Create enhanced directory
    os.makedirs('docs/comic/enhanced', exist_ok=True)
    
    # Process each scene
    enhanced_images = []
    
    for scene in data['scenes']:
        # Match dialogue to scene
        if "Academy Garden" in scene['title']:
            dialogue = SCENE_DIALOGUES["Academy Garden"]
            scene_key = "academy"
        elif "Null Bridge" in scene['title']:
            dialogue = SCENE_DIALOGUES["Null Bridge"]
            scene_key = "bridge"
        elif "Oracle" in scene['title']:
            dialogue = SCENE_DIALOGUES["Oracle"]
            scene_key = "oracle"
        else:
            continue
            
        output_path = f"docs/comic/enhanced/{scene_key}_enhanced.png"
        add_dialogue_to_image(
            scene['url'],
            dialogue,
            output_path,
            scene['title']
        )
        
        enhanced_images.append({
            'title': scene['title'],
            'path': output_path,
            'dialogue': dialogue
        })
    
    # Create enhanced HTML viewer
    create_enhanced_viewer(enhanced_images, data['timestamp'])

def create_enhanced_viewer(images, original_timestamp):
    """Create a beautiful HTML viewer for the enhanced comic"""
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    
    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>The Eternal Core - Enhanced Comic</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Cinzel:wght@600&family=Crimson+Text:ital@0;1&display=swap');
        
        body {{
            font-family: 'Crimson Text', Georgia, serif;
            background: linear-gradient(135deg, #f5f5dc 0%, #e8dcc0 100%);
            margin: 0;
            padding: 0;
            line-height: 1.6;
        }}
        
        .header {{
            text-align: center;
            padding: 40px 20px;
            background: linear-gradient(to bottom, rgba(139,69,19,0.1), transparent);
        }}
        
        h1 {{
            font-family: 'Cinzel', serif;
            color: #8B4513;
            font-size: 4em;
            margin: 0;
            text-shadow: 3px 3px 6px rgba(0,0,0,0.2);
            letter-spacing: 2px;
        }}
        
        .subtitle {{
            font-size: 1.5em;
            color: #696969;
            font-style: italic;
            margin-top: 10px;
        }}
        
        .intro {{
            max-width: 800px;
            margin: 40px auto;
            background: white;
            padding: 40px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.1);
            border-left: 5px solid #8B4513;
        }}
        
        .panel {{
            max-width: 1200px;
            margin: 40px auto;
            background: white;
            border-radius: 15px;
            overflow: hidden;
            box-shadow: 0 15px 40px rgba(0,0,0,0.2);
            border: 3px solid #8B4513;
        }}
        
        .panel img {{
            width: 100%;
            display: block;
        }}
        
        .philosophy-quote {{
            background: #f5f5dc;
            padding: 30px;
            text-align: center;
            font-size: 1.3em;
            font-style: italic;
            color: #8B4513;
            border-top: 2px solid #8B4513;
        }}
        
        .proof-summary {{
            max-width: 900px;
            margin: 60px auto;
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 30px;
            padding: 0 20px;
        }}
        
        .proof-item {{
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 5px 20px rgba(0,0,0,0.1);
            text-align: center;
            border-top: 4px solid #8B4513;
        }}
        
        .proof-item h3 {{
            font-family: 'Cinzel', serif;
            color: #8B4513;
            margin-top: 0;
        }}
        
        .proof-item code {{
            display: block;
            background: #f5f5dc;
            padding: 10px;
            border-radius: 5px;
            font-size: 0.9em;
            margin-top: 15px;
        }}
        
        .conclusion {{
            background: linear-gradient(135deg, #8B4513, #A0522D);
            color: white;
            padding: 60px 20px;
            text-align: center;
            margin-top: 60px;
        }}
        
        .conclusion h2 {{
            font-family: 'Cinzel', serif;
            font-size: 2.5em;
            margin-bottom: 20px;
        }}
        
        .scroll-indicator {{
            text-align: center;
            padding: 20px;
            animation: bounce 2s infinite;
        }}
        
        @keyframes bounce {{
            0%, 20%, 50%, 80%, 100% {{
                transform: translateY(0);
            }}
            40% {{
                transform: translateY(-10px);
            }}
            60% {{
                transform: translateY(-5px);
            }}
        }}
    </style>
</head>
<body>
    <div class="header">
        <h1>The Eternal Core</h1>
        <div class="subtitle">A Philosophical Proof Through Ancient Wisdom</div>
    </div>
    
    <div class="intro">
        <h2>The Quest for Certainty</h2>
        <p>Join Socrates and Plato on a philosophical journey through ancient Greece, 
        where timeless wisdom meets modern software verification. Through dialogue and 
        demonstration, they shall prove that <strong>Full Node: Firewall Fly-over</strong> 
        will never experience a coredump.</p>
        
        <p><em>"For what is a coredump but the mortal end of a program that has transgressed 
        the sacred boundaries of memory?"</em> - Plato</p>
    </div>
    
    <div class="scroll-indicator">↓ Scroll to begin the journey ↓</div>
"""
    
    # Add each enhanced panel
    for i, img_data in enumerate(images):
        html += f"""
    <div class="panel">
        <img src="{img_data['path'].replace('docs/comic/', '')}" alt="{img_data['title']}">
        <div class="philosophy-quote">
            Scene {i+1}: Where ancient wisdom illuminates modern truth
        </div>
    </div>
"""
    
    # Add proof summary
    html += """
    <div class="proof-summary">
        <div class="proof-item">
            <h3>Array Bounds</h3>
            <p>Like the Academy's numbered gardens, every access is checked.</p>
            <code>if (index >= MAX) return -1;</code>
        </div>
        
        <div class="proof-item">
            <h3>Null Pointers</h3>
            <p>Like testing a bridge before crossing, we verify existence.</p>
            <code>if (!ptr) return;</code>
        </div>
        
        <div class="proof-item">
            <h3>Division Safety</h3>
            <p>Like sharing among none, we handle the paradox.</p>
            <code>if (magnitude < ε) return UNIT;</code>
        </div>
    </div>
    
    <div class="conclusion">
        <h2>The Proof is Complete</h2>
        <p>Through the marriage of ancient philosophy and modern verification,<br>
        we have achieved what Socrates sought: <strong>Absolute Certainty</strong></p>
        
        <p style="font-size: 1.5em; margin-top: 30px;">
            "The unexamined code is not worth running!"
        </p>
        
        <p style="margin-top: 40px; opacity: 0.8;">
            Full Node: Firewall Fly-over ∴ No Coredump ✓ Q.E.D.
        </p>
    </div>
</body>
</html>"""
    
    # Save enhanced viewer
    output_path = f'docs/comic/eternal_core_enhanced_{timestamp}.html'
    with open(output_path, 'w') as f:
        f.write(html)
    
    print(f"\n📖 Enhanced comic saved to: {output_path}")
    print(f"🎨 Individual panels saved in: docs/comic/enhanced/")
    
    # Also create a simple viewer
    create_simple_viewer(images)

def create_simple_viewer(images):
    """Create a simple image-only viewer"""
    
    html = """<!DOCTYPE html>
<html>
<head>
    <title>The Eternal Core - Speech Bubble Version</title>
    <style>
        body {
            background: #2c2c2c;
            margin: 0;
            padding: 20px;
            text-align: center;
        }
        .panel {
            max-width: 1024px;
            margin: 20px auto;
            box-shadow: 0 10px 30px rgba(0,0,0,0.5);
        }
        img {
            width: 100%;
            display: block;
        }
        h1 {
            color: white;
            font-family: Georgia, serif;
        }
    </style>
</head>
<body>
    <h1>The Eternal Core - With Speech Bubbles</h1>
"""
    
    for img in images:
        html += f'<div class="panel"><img src="{img["path"].replace("docs/comic/", "")}"></div>\n'
    
    html += """
</body>
</html>"""
    
    with open('docs/comic/speech_bubble_version.html', 'w') as f:
        f.write(html)
    
    print("📚 Simple viewer saved to: docs/comic/speech_bubble_version.html")

if __name__ == "__main__":
    print("=== The Eternal Core - Speech Bubble Enhancer ===")
    print("Adding philosophical dialogue to the generated scenes...\n")
    
    enhance_comic()
    
    print("\n✅ Enhancement complete!")
    print("   The philosophers now speak their wisdom!")
    print("\n🎭 To view the enhanced comic:")
    print("   firefox docs/comic/speech_bubble_version.html")