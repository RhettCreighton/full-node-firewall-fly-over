#!/usr/bin/env python3
"""
Generate 'The Eternal Core' comic - REALISTIC implementation
Based on actual Leonardo AI capabilities research
"""

import os
import requests
import json
import time
from datetime import datetime
from PIL import Image, ImageDraw, ImageFont
import io

# Leonardo AI API configuration
API_KEY = 'ae2db0f6-1be1-479b-b244-06f1c7c1e9d3'
BASE_URL = 'https://cloud.leonardo.ai/api/rest/v1'

# Phoenix model - good for comic style, but NO text generation
PHOENIX_MODEL_ID = 'e71a1c2f-4f80-4800-934f-2c68979d8cc8'

# Comic scenes WITHOUT text in prompts (Leonardo can't generate readable text)
COMIC_SCENES = [
    {
        "title": "The Array Bounds Garden",
        "prompt": """Vintage comic book panel, ancient Greek garden with numbered stone tablets 0 through 15,
        two philosophers in robes - SOCRATES (older with long beard) and PLATO (younger), 
        Socrates pointing at tablets questioningly, Plato holding scroll confidently,
        empty speech bubbles above their heads, bold black inks, CMYK flats, halftone dots, 
        clean Dan DeCarlo line style, vintage comic book aesthetic""",
        "concept": "Array bounds checking",
        "dialogue": {
            "socrates": "What if I try to access array[16]?",
            "plato": "if (index >= 16) return -1;\nBounds check prevents crash!"
        }
    },
    {
        "title": "The NULL Pointer Bridge", 
        "prompt": """Vintage comic book panel, broken stone bridge over dark chasm,
        SOCRATES pointing at gap with concern, PLATO holding torch confidently,
        empty speech bubbles above their heads, bold black inks, CMYK flats, 
        halftone dots, clean Dan DeCarlo line style, dramatic shadows""",
        "concept": "NULL pointer safety",
        "dialogue": {
            "socrates": "This pointer is NULL!",
            "plato": "if (!ptr) return;\nNever dereference NULL!"
        }
    },
    {
        "title": "The Division Theater",
        "prompt": """Vintage comic book panel, empty Greek amphitheater,
        PLATO holding bread questioningly, SOCRATES gesturing wisely,
        mathematical symbols floating, empty speech bubbles, bold black inks, 
        CMYK flats, halftone dots, clean Dan DeCarlo line style""",
        "concept": "Division by zero protection",
        "dialogue": {
            "plato": "Divide by zero?",
            "socrates": "if (denom < 0.0001) return SAFE;\nHandle the paradox!"
        }
    },
    {
        "title": "The Resource Walls",
        "prompt": """Vintage comic book panel, Acropolis walls with 16 numbered guard posts (0-15),
        SOCRATES questioning, PLATO at gate explaining, empty speech bubbles,
        bold black inks, CMYK flats, halftone dots, clean Dan DeCarlo line style""",
        "concept": "Resource limits",
        "dialogue": {
            "socrates": "What if aircraft #17 arrives?",
            "plato": "if (count >= MAX) return -1;\nThe walls enforce limits!"
        }
    },
    {
        "title": "The Oracle's Truth",
        "prompt": """Vintage comic book splash panel, Temple of Delphi interior, mystical smoke,
        ORACLE PRIESTESS in center, SOCRATES and PLATO on sides, empty speech bubbles,
        'KNOW THYSELF' carved in stone, bold black inks, CMYK flats, halftone dots,
        clean Dan DeCarlo line style, dramatic composition""",
        "concept": "Complete verification",
        "dialogue": {
            "oracle": "Know thy code's limits!",
            "socrates": "The Truth Buckets verify all!",
            "plato": "No coredump shall ever occur!"
        }
    }
]

def generate_comic_panel(scene_data):
    """Generate a comic panel WITHOUT text (Leonardo can't do readable text)"""
    
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
        'guidance_scale': 7,
        'seed': int(time.time()),
        'promptMagic': False,  # We want exact prompt
        'public': False
    }
    
    print(f"\n🎨 Generating artwork: {scene_data['title']}")
    print(f"   Note: Text will be added in post-processing")
    
    response = requests.post(
        f'{BASE_URL}/generations',
        headers=headers,
        json=generation_data
    )
    
    if response.status_code != 200:
        print(f"❌ Generation failed: {response.status_code}")
        print(f"   Error: {response.text}")
        return None
    
    generation_id = response.json()['sdGenerationJob']['generationId']
    print(f"   Generation ID: {generation_id}")
    
    # Poll for completion
    print("   Generating comic panel artwork", end="", flush=True)
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
            print(" ✅ Complete!")
            image_url = generation['generated_images'][0]['url']
            print(f"   Artwork URL: {image_url}")
            return {
                'url': image_url,
                'id': generation['id'],
                'scene': scene_data
            }
        elif generation['status'] == 'FAILED':
            print(" ❌ Failed!")
            return None
        
        attempts += 1
    
    print(" ⏱️ Timeout!")
    return None

def add_text_to_panel(image_url, dialogue):
    """Add text to speech bubbles using PIL (post-processing)"""
    
    # Download the image
    response = requests.get(image_url)
    img = Image.open(io.BytesIO(response.content))
    
    # Create drawing context
    draw = ImageDraw.Draw(img)
    
    # Try to use a comic-style font if available
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf", 24)
    except:
        font = ImageFont.load_default()
    
    # Note: In a real implementation, we would:
    # 1. Detect speech bubble locations using CV or manual coordinates
    # 2. Add text properly positioned within bubbles
    # 3. Use proper comic fonts
    # For now, we'll just note that this is where text would be added
    
    # Save the image with text
    output_path = f"docs/comic/panel_with_text_{int(time.time())}.png"
    img.save(output_path)
    
    return output_path

def create_realistic_viewer(results, timestamp):
    """Create viewer explaining the realistic approach"""
    
    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>The Eternal Core - Realistic Leonardo AI Approach</title>
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
            letter-spacing: 5px;
        }}
        
        .reality-check {{
            background: #e74c3c;
            color: white;
            padding: 20px;
            margin: 20px auto;
            max-width: 800px;
            border-radius: 10px;
            font-size: 1.2em;
        }}
        
        .solution {{
            background: #27ae60;
            color: white;
            padding: 20px;
            margin: 20px auto;
            max-width: 800px;
            border-radius: 10px;
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
            overflow: hidden;
        }}
        
        .panel-header {{
            background: #e74c3c;
            color: white;
            padding: 15px;
            font-weight: bold;
            font-size: 1.2em;
            text-align: center;
        }}
        
        .comic-panel img {{
            width: 100%;
            display: block;
        }}
        
        .dialogue-overlay {{
            background: #f39c12;
            color: #000;
            padding: 15px;
            font-family: 'Courier New', monospace;
        }}
        
        .dialogue-overlay h4 {{
            margin: 0 0 10px 0;
        }}
        
        .dialogue {{
            margin: 5px 0;
            padding: 5px;
            background: white;
            border-radius: 5px;
        }}
        
        .implementation {{
            max-width: 1000px;
            margin: 60px auto;
            background: #2c3e50;
            color: white;
            padding: 40px;
            border-radius: 10px;
        }}
        
        .code {{
            background: #1a1a1a;
            color: #27ae60;
            padding: 20px;
            margin: 15px 0;
            border-radius: 5px;
            font-family: 'Courier New', monospace;
            overflow-x: auto;
        }}
    </style>
</head>
<body>
    <div class="header">
        <h1>THE ETERNAL CORE</h1>
        <p style="color: #bdc3c7; font-size: 1.2em;">Realistic Leonardo AI Implementation</p>
    </div>
    
    <div class="reality-check">
        <h2>⚠️ Research Findings</h2>
        <p>After extensive research, we found that Leonardo AI (including Phoenix model) 
        <strong>CANNOT generate readable text in images</strong>. When prompted for text,
        it produces gibberish or incorrect characters.</p>
    </div>
    
    <div class="solution">
        <h2>✅ The Solution: Two-Step Process</h2>
        <ol>
            <li><strong>Step 1:</strong> Generate comic artwork with empty speech bubbles using Leonardo AI</li>
            <li><strong>Step 2:</strong> Add text to speech bubbles using post-processing (PIL, ImageMagick, etc.)</li>
        </ol>
    </div>
    
    <div class="comic-grid">
"""
    
    for result in results:
        if result:
            scene = result['scene']
            html += f"""
        <div class="comic-panel">
            <div class="panel-header">{scene['title']}</div>
            <img src="{result['url']}" alt="{scene['title']} - artwork only">
            <div class="dialogue-overlay">
                <h4>Dialogue to be added:</h4>
"""
            for character, text in scene['dialogue'].items():
                html += f"""
                <div class="dialogue">
                    <strong>{character.upper()}:</strong> {text.replace('\n', '<br>')}
                </div>
"""
            html += f"""
            </div>
        </div>
"""
    
    html += """
    </div>
    
    <div class="implementation">
        <h2>Implementation Details</h2>
        
        <h3>What Leonardo AI CAN do:</h3>
        <ul>
            <li>Generate beautiful comic-style artwork</li>
            <li>Create empty speech bubbles</li>
            <li>Maintain consistent art style</li>
            <li>Generate vintage comic aesthetics</li>
        </ul>
        
        <h3>What Leonardo AI CANNOT do:</h3>
        <ul>
            <li>Generate readable text within images</li>
            <li>Create legible speech bubble content</li>
            <li>Render accurate letters or words</li>
        </ul>
        
        <h3>Recommended Workflow:</h3>
        <div class="code">
# 1. Generate artwork with empty bubbles
response = leonardo_ai.generate(
    prompt="comic panel with empty speech bubbles..."
)

# 2. Download generated image
image = download_image(response.url)

# 3. Add text using PIL or similar
add_comic_text(image, dialogue_data)

# 4. Save final comic with text
save_comic(image, "final_comic.png")
        </div>
        
        <h3>Alternative Approaches:</h3>
        <ul>
            <li><strong>ControlNet Method:</strong> Use edge detection with pre-made text (0.71-0.9 weight)</li>
            <li><strong>External Tools:</strong> Use Canva or similar for adding speech bubbles</li>
            <li><strong>Manual Editing:</strong> Add text in Photoshop/GIMP</li>
        </ul>
    </div>
</body>
</html>"""
    
    output_path = f'docs/comic/eternal_core_realistic_{timestamp}.html'
    os.makedirs('docs/comic', exist_ok=True)
    with open(output_path, 'w') as f:
        f.write(html)
    
    print(f"\n📚 Realistic implementation guide saved to: {output_path}")
    
    # Save implementation notes
    with open(f'docs/comic/leonardo_ai_reality_{timestamp}.json', 'w') as f:
        json.dump({
            'timestamp': timestamp,
            'finding': 'Leonardo AI cannot generate readable text',
            'solution': 'Two-step process: artwork generation + text overlay',
            'alternatives': [
                'ControlNet with edge detection',
                'External tools like Canva',
                'Manual post-processing'
            ]
        }, f, indent=2)

def main():
    """Generate comic with realistic understanding of Leonardo AI's capabilities"""
    
    print("=== The Eternal Core - Realistic Leonardo AI Implementation ===")
    print("Based on research: Leonardo AI CANNOT generate readable text\n")
    
    results = []
    for scene in COMIC_SCENES:
        result = generate_comic_panel(scene)
        results.append(result)
        if result:
            time.sleep(3)  # Rate limiting
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    create_realistic_viewer(results, timestamp)
    
    print("\n✅ Realistic implementation complete!")
    print("   Generated artwork with empty speech bubbles")
    print("   Text would need to be added in post-processing")
    print("\nKey Learning: Leonardo AI generates art, not text!")

if __name__ == "__main__":
    main()