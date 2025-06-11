#!/usr/bin/env python3
"""
Generate 'The Eternal Core' comic - THE CORRECT WAY
Leonardo AI generates artwork, we add text afterwards
"""

import os
import requests
import json
import time
from datetime import datetime
from PIL import Image, ImageDraw, ImageFont
import textwrap
import urllib.request

# Leonardo AI API configuration
API_KEY = 'ae2db0f6-1be1-479b-b244-06f1c7c1e9d3'
BASE_URL = 'https://cloud.leonardo.ai/api/rest/v1'

# Phoenix model - generates comic ARTWORK (not text!)
PHOENIX_MODEL_ID = 'e71a1c2f-4f80-4800-934f-2c68979d8cc8'

# Comic scenes with empty speech bubbles
COMIC_SCENES = [
    {
        "title": "The Array Bounds Garden",
        "prompt": """Vintage comic book panel, ancient Greek garden with numbered stone tablets 0 through 15,
        two Greek philosophers (one older bearded SOCRATES, one younger PLATO) in discussion,
        two empty speech bubbles, one pointing to each philosopher,
        bold black inks, CMYK flats, halftone dots, clean Dan DeCarlo line style""",
        "dialogue": [
            {"speaker": "SOCRATES", "text": "What if I try to\naccess array[16]?"},
            {"speaker": "PLATO", "text": "if (index >= 16)\n  return -1;\nBounds check\nprevents crash!"}
        ]
    },
    {
        "title": "The NULL Pointer Bridge",
        "prompt": """Vintage comic book panel, broken stone bridge over dark chasm,
        SOCRATES pointing at gap, PLATO holding torch, two empty speech bubbles,
        bold black inks, CMYK flats, halftone dots, dramatic shadows""",
        "dialogue": [
            {"speaker": "SOCRATES", "text": "This pointer\nis NULL!"},
            {"speaker": "PLATO", "text": "if (!ptr) return;\nNever dereference\nNULL!"}
        ]
    },
    {
        "title": "The Division Theater",
        "prompt": """Vintage comic book panel, Greek amphitheater,
        PLATO holding bread, SOCRATES gesturing, two empty speech bubbles,
        mathematical symbols floating, bold black inks, halftone dots""",
        "dialogue": [
            {"speaker": "PLATO", "text": "Divide by zero?"},
            {"speaker": "SOCRATES", "text": "if (denom < 0.0001)\n  return SAFE;\nHandle the\nparadox!"}
        ]
    },
    {
        "title": "The Resource Walls",
        "prompt": """Vintage comic book panel, Acropolis walls with 16 guard posts,
        SOCRATES at wall, PLATO at gate, two empty speech bubbles,
        bold black inks, CMYK flats, clean comic style""",
        "dialogue": [
            {"speaker": "SOCRATES", "text": "What if aircraft\n#17 arrives?"},
            {"speaker": "PLATO", "text": "if (count >= MAX)\n  return -1;\nThe walls\nenforce limits!"}
        ]
    },
    {
        "title": "The Oracle's Truth",
        "prompt": """Vintage comic book splash panel, Temple of Delphi interior,
        Oracle priestess with two philosophers, three empty speech bubbles,
        mystical smoke, 'KNOW THYSELF' carved, bold inks, dramatic composition""",
        "dialogue": [
            {"speaker": "ORACLE", "text": "Know thy\ncode's limits!"},
            {"speaker": "SOCRATES", "text": "The Truth Buckets\nverify all!"},
            {"speaker": "PLATO", "text": "No coredump\nshall ever occur!"}
        ]
    }
]

def generate_artwork(scene_data):
    """Generate comic artwork with empty speech bubbles"""
    
    headers = {
        'accept': 'application/json',
        'authorization': f'Bearer {API_KEY}',
        'content-type': 'application/json'
    }
    
    generation_data = {
        'prompt': scene_data['prompt'],
        'modelId': PHOENIX_MODEL_ID,
        'width': 1024,
        'height': 1024,
        'num_images': 1,
        'num_inference_steps': 30,
        'guidance_scale': 8,
        'seed': int(time.time()),
        'promptMagic': False,
        'public': False
    }
    
    print(f"\n🎨 Generating artwork: {scene_data['title']}")
    print(f"   Creating empty speech bubbles for post-processing...")
    
    response = requests.post(
        f'{BASE_URL}/generations',
        headers=headers,
        json=generation_data
    )
    
    if response.status_code != 200:
        print(f"❌ Generation failed: {response.status_code}")
        return None
    
    generation_id = response.json()['sdGenerationJob']['generationId']
    print(f"   Generation ID: {generation_id}")
    
    # Poll for completion
    print("   Generating", end="", flush=True)
    attempts = 0
    while attempts < 60:
        time.sleep(5)
        print(".", end="", flush=True)
        
        status_response = requests.get(
            f'{BASE_URL}/generations/{generation_id}',
            headers=headers
        )
        
        if status_response.status_code != 200:
            print(f"\n❌ Status check failed")
            return None
        
        generation = status_response.json()['generations_by_pk']
        
        if generation['status'] == 'COMPLETE':
            print(" ✅")
            image_url = generation['generated_images'][0]['url']
            return image_url
        elif generation['status'] == 'FAILED':
            print(" ❌")
            return None
        
        attempts += 1
    
    print(" ⏱️ Timeout!")
    return None

def add_text_to_comic(image_url, dialogue, output_path):
    """Download image and add text to speech bubbles"""
    
    # Download image
    print("   Downloading artwork...")
    urllib.request.urlretrieve(image_url, output_path)
    
    # Open with PIL
    img = Image.open(output_path)
    draw = ImageDraw.Draw(img)
    
    # Try to use a comic font if available
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf", 24)
        small_font = ImageFont.truetype("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 18)
    except:
        font = ImageFont.load_default()
        small_font = font
    
    print("   Adding dialogue text...")
    
    # Position text based on typical comic layout
    # This is approximate - in production you'd use bubble detection
    positions = [
        (200, 150),   # Top left bubble
        (600, 150),   # Top right bubble
        (400, 800)    # Bottom center (for Oracle panel)
    ]
    
    for i, dialogue_item in enumerate(dialogue):
        if i < len(positions):
            x, y = positions[i]
            
            # Add speaker name
            draw.text((x, y - 30), dialogue_item['speaker'], 
                     font=small_font, fill='black')
            
            # Add dialogue
            lines = dialogue_item['text'].split('\n')
            for j, line in enumerate(lines):
                draw.text((x, y + j * 28), line, 
                         font=font, fill='black')
    
    # Save with text
    img.save(output_path)
    print("   ✅ Text added!")

def create_viewer(results, timestamp):
    """Create HTML viewer for the comic"""
    
    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>The Eternal Core - Proper Implementation</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Bangers&display=swap');
        
        body {{
            background: #1a1a1a;
            margin: 0;
            padding: 20px;
            font-family: Arial, sans-serif;
        }}
        
        .header {{
            text-align: center;
            color: white;
            margin-bottom: 40px;
        }}
        
        h1 {{
            font-family: 'Bangers', cursive;
            font-size: 4em;
            color: #ffeb3b;
            text-shadow: 3px 3px 0 #e74c3c, 6px 6px 0 #000;
            margin: 0;
        }}
        
        .workflow {{
            background: #2c3e50;
            color: white;
            padding: 30px;
            margin: 20px auto;
            max-width: 800px;
            border-radius: 10px;
        }}
        
        .step {{
            background: #34495e;
            padding: 15px;
            margin: 10px 0;
            border-radius: 5px;
            border-left: 5px solid #e74c3c;
        }}
        
        .comic-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(500px, 1fr));
            gap: 30px;
            max-width: 1400px;
            margin: 0 auto;
        }}
        
        .comic-panel {{
            background: white;
            border: 5px solid #000;
            box-shadow: 0 10px 30px rgba(255,255,255,0.1);
        }}
        
        .panel-header {{
            background: #e74c3c;
            color: white;
            padding: 15px;
            font-weight: bold;
            text-align: center;
        }}
        
        .comic-panel img {{
            width: 100%;
            display: block;
        }}
    </style>
</head>
<body>
    <div class="header">
        <h1>THE ETERNAL CORE</h1>
        <p style="color: #bdc3c7;">Generated Correctly with Post-Processing</p>
    </div>
    
    <div class="workflow">
        <h2>The Correct Workflow</h2>
        <div class="step">
            <strong>Step 1:</strong> Leonardo AI generates artwork with empty speech bubbles
        </div>
        <div class="step">
            <strong>Step 2:</strong> PIL adds readable text to the bubbles
        </div>
        <div class="step">
            <strong>Result:</strong> Beautiful comics with actual readable dialogue!
        </div>
    </div>
    
    <div class="comic-grid">
"""
    
    for result in results:
        if result:
            html += f"""
        <div class="comic-panel">
            <div class="panel-header">{result['title']}</div>
            <img src="{result['filename']}" alt="{result['title']}">
        </div>
"""
    
    html += """
    </div>
</body>
</html>"""
    
    output_path = f'docs/comic/eternal_core_proper_{timestamp}.html'
    with open(output_path, 'w') as f:
        f.write(html)
    
    print(f"\n📚 Comic viewer saved to: {output_path}")

def main():
    """Generate comic the PROPER way"""
    
    print("=== The Eternal Core - Proper Comic Generator ===")
    print("Step 1: Leonardo generates artwork")
    print("Step 2: We add text with PIL\n")
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    results = []
    
    # Ensure output directory exists
    os.makedirs('docs/comic/panels', exist_ok=True)
    
    for i, scene in enumerate(COMIC_SCENES):
        image_url = generate_artwork(scene)
        
        if image_url:
            output_file = f"docs/comic/panels/panel_{i+1}_{timestamp}.jpg"
            add_text_to_comic(image_url, scene['dialogue'], output_file)
            
            results.append({
                'title': scene['title'],
                'filename': output_file,
                'url': image_url
            })
            
            time.sleep(3)  # Rate limiting
    
    create_viewer(results, timestamp)
    
    print("\n✅ Comic generation complete!")
    print("   This is the CORRECT way - artwork from AI, text from code!")

if __name__ == "__main__":
    main()