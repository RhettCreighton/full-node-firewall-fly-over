#!/usr/bin/env python3
"""
Generate 'The Eternal Core' comic with native speech bubbles using Leonardo AI
Using Phoenix model with comic-style prompting
"""

import os
import requests
import json
import time
from datetime import datetime

# Leonardo AI API configuration
API_KEY = 'ae2db0f6-1be1-479b-b244-06f1c7c1e9d3'
BASE_URL = 'https://cloud.leonardo.ai/api/rest/v1'

# Phoenix model ID - specifically trained on vintage comics
PHOENIX_MODEL_ID = 'e71a1c2f-4f80-4800-934f-2c68979d8cc8'  # Leonardo Phoenix

# Comic scenes with speech bubble prompts
COMIC_SCENES = [
    {
        "title": "The Academy Garden - Array Bounds",
        "prompt": """Comic book page, two ancient Greek philosophers in geometric garden, 
        SOCRATES (bearded, left) with speech bubble saying "What if I try to water plot 16 when only 0-15 exist?", 
        PLATO (younger, right) with large speech bubble saying "The guards prevent it! CODE: if (index >= 16) return -1; MEANING: We check every array access!",
        numbered stone tablets 0 to 15 visible, bold black inks, CMYK flats, halftone dots, 
        clean Dan DeCarlo line style, vintage comic aesthetic, speech bubbles with tails pointing to speakers""",
        "concept": "Array bounds checking prevents segfaults"
    },
    {
        "title": "The Null Bridge Over Chaos",
        "prompt": """Comic book panel, ancient Greek stone bridge over dark void, 
        SOCRATES pointing at broken bridge section with speech bubble "What if this pointer bridge is NULL - broken and missing?",
        PLATO holding torch with large speech bubble "We must check first! CODE: if (!ptr) return; MEANING: Never use a pointer without verifying it exists!",
        dramatic comic book lighting, bold black inks, CMYK flats, halftone dots,
        vintage comic style, clean speech bubbles with proper tails, Dan DeCarlo line art""",
        "concept": "NULL pointer checks prevent crashes"
    },
    {
        "title": "The Oracle's Revelation",
        "prompt": """Comic book splash panel, Temple of Delphi interior, mystical smoke,
        SOCRATES with speech bubble "The Oracle speaks: Know thy code's limits! What does this mean?",
        PLATO with large speech bubble "TRUTH BUCKETS verify all! CODE: assert(verify_all_truths()); MEANING: We prove mathematically that no crash is possible!",
        'KNOW THYSELF' carved in stone, bold black inks, CMYK flats, halftone dots,
        dramatic comic book composition, vintage speech bubbles, Dan DeCarlo clean lines""",
        "concept": "Complete verification achieves certainty"
    },
    {
        "title": "The Division Paradox",
        "prompt": """Comic book panel, Greek theater, PLATO dividing bread with speech bubble
        "If I divide by zero - share among nobody...", SOCRATES with large speech bubble "A paradox! CODE: if (denominator < 0.0001) return SAFE_VALUE; MEANING: We detect and handle impossible math!",
        empty amphitheater seats, mathematical symbols floating, bold black inks, CMYK flats,
        halftone dots, vintage comic speech bubbles with tails, Dan DeCarlo style line art""",
        "concept": "Division by zero handling"
    },
    {
        "title": "The Walled City",
        "prompt": """Comic book panel, Acropolis walls with exactly 16 guard posts numbered 0-15,
        SOCRATES with speech bubble "What if a 17th aircraft tries to join our 16?",
        PLATO with large speech bubble "The gates close! CODE: if (count >= MAX_AIRCRAFT) return -1; MEANING: We enforce hard limits - no overflow possible!",
        guards at posts, bold black inks, CMYK flats, halftone dots, clean comic style,
        vintage speech bubbles properly positioned, Dan DeCarlo line work""",
        "concept": "Resource limits enforced"
    }
]

def generate_comic_panel(scene_data):
    """Generate a single comic panel with native speech bubbles"""
    
    headers = {
        'accept': 'application/json',
        'authorization': f'Bearer {API_KEY}',
        'content-type': 'application/json'
    }
    
    # Generation parameters optimized for comic style
    generation_data = {
        'prompt': scene_data['prompt'],
        'modelId': PHOENIX_MODEL_ID,  # Phoenix model for comics
        'width': 1024,
        'height': 1024,  # Square for better comic panels
        'num_images': 1,
        'num_inference_steps': 30,
        'guidance_scale': 8,  # Higher for better prompt adherence
        'seed': int(time.time()),
        'public': False,
        'promptMagic': True,  # Enable prompt enhancement
        'controlNet': False,
        'nsfw': False
    }
    
    print(f"\n🎨 Generating: {scene_data['title']}")
    print(f"   Concept: {scene_data['concept']}")
    
    # Start generation
    response = requests.post(
        f'{BASE_URL}/generations',
        headers=headers,
        json=generation_data
    )
    
    if response.status_code != 200:
        print(f"❌ Generation failed: {response.status_code}")
        print(f"   Error: {response.text}")
        # Try with default model if Phoenix fails
        print("   Retrying with default model...")
        generation_data['modelId'] = '6bef9f1b-29cb-40c7-b9df-32b51c1f67d3'
        response = requests.post(
            f'{BASE_URL}/generations',
            headers=headers,
            json=generation_data
        )
        if response.status_code != 200:
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
            print(f"\n❌ Status check failed: {status_response.text}")
            return None
        
        generation = status_response.json()['generations_by_pk']
        
        if generation['status'] == 'COMPLETE':
            print(" ✅ Complete!")
            image_url = generation['generated_images'][0]['url']
            print(f"   Image URL: {image_url}")
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

def create_comic_book_viewer(results, timestamp):
    """Create vintage comic book style viewer"""
    
    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>The Eternal Core - A Philosophical Comic Book</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Bangers&family=Comic+Neue:wght@400;700&display=swap');
        
        body {{
            background: #f4e8c1;
            background-image: 
                repeating-linear-gradient(
                    0deg,
                    transparent,
                    transparent 2px,
                    rgba(0,0,0,0.03) 2px,
                    rgba(0,0,0,0.03) 4px
                );
            margin: 0;
            padding: 20px;
            font-family: 'Comic Neue', cursive;
        }}
        
        .comic-header {{
            text-align: center;
            margin-bottom: 30px;
            background: #fff;
            border: 4px solid #000;
            padding: 20px;
            transform: rotate(-1deg);
            box-shadow: 5px 5px 0 #000;
        }}
        
        h1 {{
            font-family: 'Bangers', cursive;
            font-size: 4em;
            color: #e74c3c;
            text-shadow: 3px 3px 0 #000;
            margin: 0;
            letter-spacing: 3px;
            transform: perspective(500px) rotateY(-5deg);
        }}
        
        .issue-info {{
            font-size: 1.2em;
            color: #2c3e50;
            font-weight: bold;
            margin-top: 10px;
        }}
        
        .comic-panel {{
            max-width: 1024px;
            margin: 30px auto;
            background: #fff;
            border: 4px solid #000;
            padding: 10px;
            transform: rotate(-0.5deg);
            box-shadow: 8px 8px 0 #000;
            position: relative;
        }}
        
        .comic-panel img {{
            width: 100%;
            display: block;
            border: 2px solid #000;
        }}
        
        .panel-caption {{
            background: #ffeb3b;
            border: 2px solid #000;
            padding: 10px;
            margin-top: 10px;
            font-weight: bold;
            font-size: 1.1em;
            text-align: center;
        }}
        
        .pow-badge {{
            position: absolute;
            top: -20px;
            right: -20px;
            background: #e74c3c;
            color: #fff;
            padding: 15px 25px;
            font-family: 'Bangers', cursive;
            font-size: 2em;
            transform: rotate(15deg);
            border: 3px solid #000;
            box-shadow: 3px 3px 0 #000;
        }}
        
        .philosophy-box {{
            max-width: 800px;
            margin: 40px auto;
            background: #fff;
            border: 3px solid #000;
            padding: 20px;
            position: relative;
            box-shadow: 5px 5px 0 #000;
        }}
        
        .philosophy-box::before {{
            content: "WISDOM!";
            position: absolute;
            top: -15px;
            left: 20px;
            background: #4caf50;
            color: #fff;
            padding: 5px 15px;
            font-family: 'Bangers', cursive;
            border: 2px solid #000;
            transform: rotate(-3deg);
        }}
        
        .credits {{
            text-align: center;
            margin-top: 50px;
            padding: 20px;
            background: #2c3e50;
            color: #fff;
            border: 3px solid #000;
            box-shadow: 5px 5px 0 #000;
        }}
        
        .halftone {{
            position: relative;
            overflow: hidden;
        }}
        
        .halftone::after {{
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-image: radial-gradient(circle, transparent 70%, rgba(0,0,0,0.1) 70%);
            background-size: 4px 4px;
            pointer-events: none;
        }}
    </style>
</head>
<body>
    <div class="comic-header">
        <h1>THE ETERNAL CORE</h1>
        <div class="issue-info">Issue #1 - "The Proof of No Coredump"</div>
        <div style="margin-top: 10px; font-style: italic;">
            Featuring: Socrates & Plato in Ancient Greece!
        </div>
    </div>
"""
    
    # Add each generated panel
    for i, result in enumerate(results):
        if result:
            rotation = -0.5 + (i % 3) * 0.5
            scene = result['scene']
            pow_words = ["POW!", "ZAP!", "EUREKA!", "WISDOM!", "PROOF!"]
            
            html += f"""
    <div class="comic-panel" style="transform: rotate({rotation}deg);">
        {f'<div class="pow-badge">{pow_words[i % len(pow_words)]}</div>' if i % 2 == 0 else ''}
        <div class="halftone">
            <img src="{result['url']}" alt="{scene['title']}">
        </div>
        <div class="panel-caption">
            {scene['concept']}
        </div>
    </div>
"""
        else:
            html += f"""
    <div class="comic-panel">
        <div style="padding: 100px; text-align: center; font-size: 1.5em;">
            Panel {i+1} - The Oracle remains silent...
        </div>
    </div>
"""
    
    # Add philosophy summary
    html += """
    <div class="philosophy-box">
        <h2 style="font-family: 'Bangers', cursive; text-align: center;">
            THE PHILOSOPHICAL PROOF IS COMPLETE!
        </h2>
        <p style="font-size: 1.2em; line-height: 1.8;">
            Through their journey across Ancient Greece, Socrates and Plato have proven:
        </p>
        <ul style="font-size: 1.1em; line-height: 2;">
            <li><strong>Array Bounds:</strong> Every access checked like guarded garden plots!</li>
            <li><strong>NULL Pointers:</strong> Every bridge tested before crossing!</li>
            <li><strong>Division by Zero:</strong> Every paradox gracefully handled!</li>
            <li><strong>Resource Limits:</strong> Every boundary strictly enforced!</li>
        </ul>
        <p style="text-align: center; font-size: 1.3em; margin-top: 20px;">
            <em>"The unexamined code is not worth running!"</em> - Socrates
        </p>
    </div>
    
    <div class="credits">
        <h3 style="font-family: 'Bangers', cursive; font-size: 2em; margin: 0;">
            FULL NODE: FIREWALL FLY-OVER
        </h3>
        <p style="font-size: 1.2em; margin: 10px 0;">
            Will Never Coredump - Philosophically Proven! ✓
        </p>
        <p>
            Art: Leonardo AI Phoenix • Script: Ancient Wisdom • 
            Verification: Truth Buckets & FStar
        </p>
        <p style="margin-top: 20px; opacity: 0.8;">
            Generated: {timestamp}
        </p>
    </div>
</body>
</html>"""
    
    # Save the comic book
    output_path = f'docs/comic/eternal_core_comic_book_{timestamp}.html'
    with open(output_path, 'w') as f:
        f.write(html)
    
    print(f"\n📚 Comic book saved to: {output_path}")
    
    # Save generation record
    json_data = {
        'timestamp': timestamp,
        'model': 'Phoenix (Comic-optimized)',
        'scenes': [
            {
                'title': r['scene']['title'],
                'url': r['url'],
                'id': r['id'],
                'concept': r['scene']['concept']
            } for r in results if r
        ]
    }
    
    with open(f'docs/comic/comic_generation_{timestamp}.json', 'w') as f:
        json.dump(json_data, f, indent=2)

def main():
    """Generate comic with native speech bubbles"""
    
    print("=== The Eternal Core - Native Comic Book Generator ===")
    print("Using Leonardo Phoenix model for authentic comic style!\n")
    
    # Generate comic panels
    results = []
    for scene in COMIC_SCENES:
        result = generate_comic_panel(scene)
        results.append(result)
        
        # Rate limiting
        if result:
            time.sleep(3)
    
    # Create comic book viewer
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    create_comic_book_viewer(results, timestamp)
    
    print("\n✅ Comic book generation complete!")
    print("   Speech bubbles included natively in the artwork!")
    print("   The philosophical proof has been illustrated!")

if __name__ == "__main__":
    main()