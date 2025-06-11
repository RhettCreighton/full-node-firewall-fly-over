#!/usr/bin/env python3
"""
Generate 'The Eternal Core' comic using Leonardo AI API
"""

import os
import requests
import json
import time
from PIL import Image, ImageDraw, ImageFont
import textwrap
import io

# Leonardo AI API configuration
API_KEY = os.environ.get('LEONARDO_API_KEY', 'ae2db0f6-1be1-479b-b244-06f1c7c1e9d3')
BASE_URL = 'https://cloud.leonardo.ai/api/rest/v1'

# Comic scenes with prompts and dialogue
SCENES = [
    {
        "title": "Scene 1: The Parthenon Steps at Dawn",
        "prompt": "Two ancient Greek philosophers on the marble steps of the Parthenon at sunrise, golden light on columns, one older bearded man in simple robe (Socrates), one younger clean-shaven man in fine robe (Plato), discussing, photorealistic, cinematic lighting, ancient Greece",
        "dialogue": [
            ("SOCRATES", "Tell me Plato, what is this 'coredump' the young programmers fear?"),
            ("PLATO", "Master, it's when a program dies violently, attempting to access forbidden memory."),
        ]
    },
    {
        "title": "Scene 2: The Academy Gardens",
        "prompt": "Two Greek philosophers in beautiful geometric garden with olive trees, mathematical patterns drawn in sand, numbered garden plots from 0 to 15, Plato's Academy, ancient Greece, detailed, sunlight through trees",
        "dialogue": [
            ("PLATO", "Each plot is numbered, 0 through 15."),
            ("SOCRATES", "What if I try to water plot 16?"),
            ("PLATO", "The guards prevent it! if (index >= 16) return -1;"),
        ]
    },
    {
        "title": "Scene 3: The Agora Marketplace",
        "prompt": "Bustling ancient Greek marketplace (agora) with merchants, some stalls empty, others full of goods, two philosophers examining stalls, classical architecture, busy crowd, ancient Athens",
        "dialogue": [
            ("SOCRATES", "What if I buy figs from an empty stall?"),
            ("PLATO", "You check first! if (!merchant) return;"),
        ]
    },
    {
        "title": "Scene 4: Theater of Dionysus",
        "prompt": "Ancient Greek amphitheater with stone seats, two philosophers on stage, one holding bread, empty audience seats, dramatic lighting, Theater of Dionysus Athens",
        "dialogue": [
            ("PLATO", "If I share bread among zero spectators..."),
            ("SOCRATES", "A paradox!"),
            ("PLATO", "We handle it: if (magnitude < 0.0001) return UNIT;"),
        ]
    },
    {
        "title": "Scene 5: The Acropolis Walls",
        "prompt": "Fortified walls of the Acropolis with exactly 16 guard posts, two philosophers pointing at guard positions, ancient Greek military architecture, strategic defensive positions",
        "dialogue": [
            ("SOCRATES", "What if a 17th guard arrives?"),
            ("PLATO", "He's turned away! if (count >= MAX) return -1;"),
        ]
    },
    {
        "title": "Scene 6: Temple of Hephaestus",
        "prompt": "Well-preserved ancient Greek temple with architects examining and repairing small cracks, two philosophers observing, detailed Doric columns, craftsmanship visible",
        "dialogue": [
            ("PLATO", "See how architects handle each crack?"),
            ("SOCRATES", "They don't let the temple fall!"),
        ]
    },
    {
        "title": "Scene 7: Piraeus Port",
        "prompt": "Ancient Greek port with numbered ships arriving in sequence extending to horizon, two philosophers watching from dock, sailing vessels, busy harbor, Mediterranean sea",
        "dialogue": [
            ("SOCRATES", "How can we be certain for ALL time?"),
            ("PLATO", "By induction - if ship N is safe, ship N+1 is safe!"),
        ]
    },
    {
        "title": "Scene 8: Oracle at Delphi",
        "prompt": "Sacred Oracle temple at Delphi with mystical atmosphere, vapors rising, 'Know Thyself' inscription visible, two philosophers in reverent poses, ancient Greek religious site",
        "dialogue": [
            ("SOCRATES", "The Oracle speaks: Know thy code's limits!"),
            ("PLATO", "We have achieved certainty!"),
        ]
    },
    {
        "title": "Scene 9: Return to Athens",
        "prompt": "Two philosophers walking back to Athens at sunset, city glowing in distance, sense of completion and wisdom, ancient Greek landscape, golden hour lighting",
        "dialogue": [
            ("SOCRATES", "The unexamined code is not worth running!"),
            ("PLATO", "We transformed failure into impossibility!"),
        ]
    }
]

def generate_image(prompt, width=1024, height=1024):
    """Generate an image using Leonardo AI API"""
    
    headers = {
        'accept': 'application/json',
        'authorization': f'Bearer {API_KEY}',
        'content-type': 'application/json'
    }
    
    # Create generation
    generation_data = {
        'prompt': prompt,
        'modelId': '6bef9f1b-29cb-40c7-b9df-32b51c1f67d3',  # Leonardo Creative
        'width': width,
        'height': height,
        'num_images': 1,
        'num_inference_steps': 30,
        'guidance_scale': 7,
        'init_strength': 0.5,
        'presetStyle': 'DYNAMIC',
        'public': False
    }
    
    print(f"Generating image: {prompt[:50]}...")
    
    response = requests.post(
        f'{BASE_URL}/generations',
        headers=headers,
        json=generation_data
    )
    
    if response.status_code != 200:
        print(f"Error: {response.status_code} - {response.text}")
        return None
        
    generation_id = response.json()['sdGenerationJob']['generationId']
    
    # Poll for completion
    while True:
        time.sleep(5)
        status_response = requests.get(
            f'{BASE_URL}/generations/{generation_id}',
            headers=headers
        )
        
        if status_response.status_code != 200:
            print(f"Status error: {status_response.text}")
            return None
            
        generation = status_response.json()['generations_by_pk']
        
        if generation['status'] == 'COMPLETE':
            image_url = generation['generated_images'][0]['url']
            return image_url
        elif generation['status'] == 'FAILED':
            print("Generation failed!")
            return None

def add_speech_bubbles(image_path, dialogue, output_path):
    """Add speech bubbles to an image"""
    
    # Open image
    if image_path.startswith('http'):
        response = requests.get(image_path)
        img = Image.open(io.BytesIO(response.content))
    else:
        img = Image.open(image_path)
    
    draw = ImageDraw.Draw(img)
    
    # Try to use a good font, fallback to default
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24)
        small_font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20)
    except:
        font = ImageFont.load_default()
        small_font = font
    
    # Add speech bubbles
    bubble_height = 80
    y_offset = 50
    
    for i, (speaker, text) in enumerate(dialogue):
        # Position bubbles
        if i % 2 == 0:  # Left side for Socrates
            x = 50
        else:  # Right side for Plato
            x = img.width - 400
            
        y = y_offset + (i * (bubble_height + 20))
        
        # Draw bubble background
        bubble_width = 350
        draw.rounded_rectangle(
            [(x, y), (x + bubble_width, y + bubble_height)],
            radius=20,
            fill='white',
            outline='black',
            width=3
        )
        
        # Add speaker name
        draw.text((x + 10, y + 5), speaker, fill='black', font=font)
        
        # Wrap and add text
        wrapped_text = textwrap.wrap(text, width=30)
        text_y = y + 30
        for line in wrapped_text:
            draw.text((x + 10, text_y), line, fill='black', font=small_font)
            text_y += 22
    
    # Save with bubbles
    img.save(output_path)
    print(f"Saved: {output_path}")

def generate_comic():
    """Generate the complete comic"""
    
    os.makedirs('docs/comic/panels', exist_ok=True)
    
    # Generate each scene
    for i, scene in enumerate(SCENES):
        print(f"\nGenerating {scene['title']}...")
        
        # Generate image
        image_url = generate_image(scene['prompt'])
        
        if image_url:
            # Add speech bubbles and save
            output_path = f"docs/comic/panels/scene_{i+1:02d}.png"
            add_speech_bubbles(image_url, scene['dialogue'], output_path)
        else:
            print(f"Failed to generate scene {i+1}")
            
        # Rate limiting
        time.sleep(2)
    
    print("\nComic generation complete!")
    
    # Create HTML viewer
    create_comic_viewer()

def create_comic_viewer():
    """Create an HTML page to view the comic"""
    
    html = """<!DOCTYPE html>
<html>
<head>
    <title>The Eternal Core - A Philosophical Comic</title>
    <style>
        body {
            font-family: Georgia, serif;
            background: #f5f5dc;
            margin: 0;
            padding: 20px;
        }
        h1 {
            text-align: center;
            color: #8B4513;
            font-size: 3em;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        h2 {
            text-align: center;
            color: #696969;
            font-style: italic;
        }
        .comic-panel {
            max-width: 1024px;
            margin: 40px auto;
            box-shadow: 0 0 20px rgba(0,0,0,0.3);
            border: 5px solid #8B4513;
        }
        .comic-panel img {
            width: 100%;
            display: block;
        }
        .scene-title {
            background: #8B4513;
            color: white;
            padding: 10px;
            text-align: center;
            font-size: 1.5em;
        }
        .philosophy-note {
            background: #fff8dc;
            padding: 20px;
            margin: 20px auto;
            max-width: 800px;
            border-left: 5px solid #8B4513;
            font-style: italic;
        }
    </style>
</head>
<body>
    <h1>The Eternal Core</h1>
    <h2>A Philosophical Proof That Full Node: Firewall Fly-over Will Never Coredump</h2>
    
    <div class="philosophy-note">
        "The unexamined code is not worth running" - Socrates
    </div>
"""
    
    for i, scene in enumerate(SCENES):
        html += f"""
    <div class="comic-panel">
        <div class="scene-title">{scene['title']}</div>
        <img src="panels/scene_{i+1:02d}.png" alt="{scene['title']}">
    </div>
"""
    
    html += """
    <div class="philosophy-note">
        Through systematic verification and philosophical inquiry, we have proven that
        Full Node: Firewall Fly-over shall never experience a coredump. Q.E.D.
    </div>
</body>
</html>"""
    
    with open('docs/comic/the_eternal_core.html', 'w') as f:
        f.write(html)
    
    print("Created comic viewer: docs/comic/the_eternal_core.html")

if __name__ == "__main__":
    print("=== The Eternal Core - Comic Generator ===")
    print(f"Using Leonardo AI API Key: {API_KEY[:10]}...")
    generate_comic()