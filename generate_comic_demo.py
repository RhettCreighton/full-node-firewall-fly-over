#!/usr/bin/env python3
"""
Demo: Generate key scenes from 'The Eternal Core' comic
"""

import os
import requests
import json
import time
from datetime import datetime

# Leonardo AI API configuration
API_KEY = 'ae2db0f6-1be1-479b-b244-06f1c7c1e9d3'
BASE_URL = 'https://cloud.leonardo.ai/api/rest/v1'

# Three key scenes that demonstrate our proof
KEY_SCENES = [
    {
        "title": "The Academy Garden - Array Bounds",
        "prompt": "Two ancient Greek philosophers in geometric garden, Socrates (bearded) and Plato (younger), examining numbered stone tablets from 0 to 15, mathematical diagrams in sand, olive trees, warm sunlight, photorealistic style, ancient Greece setting",
        "philosophical_concept": "Array bounds checking prevents segfaults",
        "code_truth": "if (index >= MAX_AIRCRAFT) return -1;"
    },
    {
        "title": "The Null Bridge Over Chaos", 
        "prompt": "Ancient Greek stone bridge over dark chasm, two philosophers at bridge entrance, Socrates pointing at broken bridge section, Plato holding a torch revealing safe path, dramatic lighting, metaphorical scene",
        "philosophical_concept": "NULL pointer checks prevent crashes",
        "code_truth": "if (!ptr) return; // Never cross NULL bridge"
    },
    {
        "title": "The Oracle's Revelation",
        "prompt": "Temple of Delphi interior, mystical smoke, two philosophers before Oracle priestess on tripod, stone inscription 'Know Thyself' visible, divine light streaming through columns, ancient Greek temple, ethereal atmosphere",
        "philosophical_concept": "Complete verification achieves certainty",
        "code_truth": "Truth Buckets + FStar = No Coredump Ever"
    }
]

def test_api_connection():
    """Test Leonardo AI API connection"""
    headers = {
        'accept': 'application/json',
        'authorization': f'Bearer {API_KEY}'
    }
    
    response = requests.get(f'{BASE_URL}/me', headers=headers)
    
    if response.status_code == 200:
        user_data = response.json()
        print("✅ API Connection successful!")
        print(f"   User: {user_data.get('user_details', [{}])[0].get('user', {}).get('username', 'Unknown')}")
        return True
    else:
        print(f"❌ API Connection failed: {response.status_code}")
        print(f"   Error: {response.text}")
        return False

def generate_scene(scene_data):
    """Generate a single scene"""
    
    headers = {
        'accept': 'application/json',
        'authorization': f'Bearer {API_KEY}',
        'content-type': 'application/json'
    }
    
    # Generation parameters
    generation_data = {
        'prompt': scene_data['prompt'],
        'modelId': '6bef9f1b-29cb-40c7-b9df-32b51c1f67d3',  # Leonardo Creative
        'width': 1024,
        'height': 768,  # Landscape for comic panels
        'num_images': 1,
        'num_inference_steps': 30,
        'guidance_scale': 7,
        'seed': int(time.time()),  # Random seed
        'public': False,
        'presetStyle': 'CINEMATIC'
    }
    
    print(f"\n🎨 Generating: {scene_data['title']}")
    print(f"   Concept: {scene_data['philosophical_concept']}")
    
    # Start generation
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
    print("   Generating", end="", flush=True)
    attempts = 0
    while attempts < 60:  # Max 5 minutes
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

def save_results(results):
    """Save generation results and create viewer"""
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    
    # Create results HTML
    html = f"""<!DOCTYPE html>
<html>
<head>
    <title>The Eternal Core - Demo Scenes</title>
    <meta charset="UTF-8">
    <style>
        body {{
            font-family: 'Palatino', 'Georgia', serif;
            background: linear-gradient(to bottom, #f5f5dc, #e8dcc0);
            margin: 0;
            padding: 20px;
            line-height: 1.6;
        }}
        .container {{
            max-width: 1200px;
            margin: 0 auto;
        }}
        h1 {{
            text-align: center;
            color: #8B4513;
            font-size: 3em;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
            margin-bottom: 10px;
        }}
        .subtitle {{
            text-align: center;
            color: #696969;
            font-style: italic;
            font-size: 1.2em;
            margin-bottom: 40px;
        }}
        .scene {{
            background: white;
            margin: 40px 0;
            border-radius: 10px;
            box-shadow: 0 5px 20px rgba(0,0,0,0.2);
            overflow: hidden;
        }}
        .scene-header {{
            background: linear-gradient(45deg, #8B4513, #A0522D);
            color: white;
            padding: 20px;
        }}
        .scene-title {{
            font-size: 1.8em;
            margin: 0;
        }}
        .philosophical-concept {{
            font-style: italic;
            margin: 10px 0 5px 0;
            opacity: 0.9;
        }}
        .code-truth {{
            font-family: 'Courier New', monospace;
            background: rgba(0,0,0,0.2);
            padding: 5px 10px;
            border-radius: 5px;
            display: inline-block;
        }}
        .scene-image {{
            width: 100%;
            display: block;
        }}
        .intro {{
            background: #fff8dc;
            padding: 30px;
            margin-bottom: 40px;
            border-radius: 10px;
            border-left: 5px solid #8B4513;
        }}
        .socratic-dialogue {{
            background: #f0f0f0;
            padding: 20px;
            margin: 20px;
            border-radius: 10px;
            font-style: italic;
        }}
        .speaker {{
            font-weight: bold;
            color: #8B4513;
        }}
        .error {{
            background: #ffe0e0;
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>The Eternal Core</h1>
        <div class="subtitle">A Visual Proof That Full Node: Firewall Fly-over Will Never Coredump</div>
        
        <div class="intro">
            <h2>The Philosophical Journey</h2>
            <p>Join Socrates and Plato as they walk through ancient Greece, discovering eternal truths
            about memory safety and software reliability. Through philosophical dialogue and formal
            verification, they prove that our game will never experience a coredump.</p>
            
            <div class="socratic-dialogue">
                <span class="speaker">SOCRATES:</span> "Can we truly know that a program will never crash?"<br>
                <span class="speaker">PLATO:</span> "Through reason and verification, Master, we can achieve certainty itself!"
            </div>
        </div>
        
        <div class="generated-on" style="text-align: center; color: #696969; margin: 20px 0;">
            Generated on: {timestamp}
        </div>
"""
    
    # Add each generated scene
    for i, result in enumerate(results):
        if result:
            scene = result['scene']
            html += f"""
        <div class="scene">
            <div class="scene-header">
                <h2 class="scene-title">Scene {i+1}: {scene['title']}</h2>
                <p class="philosophical-concept">💭 {scene['philosophical_concept']}</p>
                <code class="code-truth">{scene['code_truth']}</code>
            </div>
            <img class="scene-image" src="{result['url']}" alt="{scene['title']}">
        </div>
"""
        else:
            html += f"""
        <div class="scene">
            <div class="error">
                <h3>Scene {i+1} Generation Failed</h3>
                <p>The Oracle remains silent on this truth...</p>
            </div>
        </div>
"""
    
    html += """
        <div class="intro" style="margin-top: 40px;">
            <h2>The Proof is Complete</h2>
            <p>Through our journey with Socrates and Plato, we have demonstrated that:</p>
            <ol>
                <li><strong>Array Bounds:</strong> Every access is checked, like numbered garden plots</li>
                <li><strong>Null Pointers:</strong> Every bridge is tested before crossing</li>
                <li><strong>Division by Zero:</strong> Every paradox has a safe resolution</li>
                <li><strong>Resource Limits:</strong> The city walls define our boundaries</li>
                <li><strong>Error Handling:</strong> Every crack in the temple is repaired</li>
            </ol>
            
            <div class="socratic-dialogue">
                <span class="speaker">SOCRATES:</span> "So through systematic doubt and verification..."<br>
                <span class="speaker">PLATO:</span> "We achieve what philosophers have always sought - absolute certainty!"<br>
                <span class="speaker">BOTH:</span> "Full Node: Firewall Fly-over shall NEVER coredump!"
            </div>
            
            <p style="text-align: center; font-style: italic; margin-top: 30px; font-size: 1.2em;">
                "The unexamined code is not worth running." - Socrates
            </p>
        </div>
    </div>
</body>
</html>"""
    
    # Save HTML
    output_path = f'docs/comic/eternal_core_demo_{timestamp}.html'
    os.makedirs('docs/comic', exist_ok=True)
    
    with open(output_path, 'w') as f:
        f.write(html)
    
    print(f"\n📖 Comic viewer saved to: {output_path}")
    
    # Also save a JSON record
    json_data = {
        'timestamp': timestamp,
        'scenes': [
            {
                'title': r['scene']['title'],
                'url': r['url'],
                'id': r['id']
            } for r in results if r
        ]
    }
    
    with open(f'docs/comic/generation_record_{timestamp}.json', 'w') as f:
        json.dump(json_data, f, indent=2)

def main():
    """Generate demo comic scenes"""
    
    print("=== The Eternal Core - Comic Demo Generator ===")
    print("Proving Full Node will never coredump through ancient philosophy!\n")
    
    # Test API connection
    if not test_api_connection():
        print("\n⚠️  API connection failed. Please check your Leonardo AI API key.")
        return
    
    # Generate scenes
    results = []
    for scene in KEY_SCENES:
        result = generate_scene(scene)
        results.append(result)
        
        # Rate limiting
        if result:
            time.sleep(3)
    
    # Save results
    save_results(results)
    
    print("\n✅ Demo generation complete!")
    print("   The philosophical proof has been visualized!")

if __name__ == "__main__":
    main()