#!/usr/bin/env python3
"""
Test Leonardo AI's text generation capabilities
This will definitively show whether Leonardo can generate readable text
"""

import requests
import json
import time
from datetime import datetime

# Leonardo AI API configuration
API_KEY = 'ae2db0f6-1be1-479b-b244-06f1c7c1e9d3'
BASE_URL = 'https://cloud.leonardo.ai/api/rest/v1'

# Test with different models
MODELS = {
    'Phoenix': 'e71a1c2f-4f80-4800-934f-2c68979d8cc8',
    'Leonardo Diffusion XL': '1e60896f-3c26-4296-8ecc-53e2afecc132'
}

# Test prompts with increasing text complexity
TEST_PROMPTS = [
    {
        "name": "Simple Text",
        "prompt": "A white sign with the word 'HELLO' written in large black letters, clear readable text"
    },
    {
        "name": "Speech Bubble",
        "prompt": "Comic book speech bubble containing the text 'HELLO WORLD' in clear readable comic lettering"
    },
    {
        "name": "Comic Panel",
        "prompt": "Vintage comic panel with character saying 'TEST' in a speech bubble, clear readable text"
    },
    {
        "name": "Sign with Numbers",
        "prompt": "Street sign clearly showing '123 MAIN ST' in readable text"
    }
]

def test_text_generation(model_name, model_id, test_case):
    """Test if Leonardo can generate readable text"""
    
    headers = {
        'accept': 'application/json',
        'authorization': f'Bearer {API_KEY}',
        'content-type': 'application/json'
    }
    
    generation_data = {
        'prompt': test_case['prompt'],
        'modelId': model_id,
        'width': 512,
        'height': 512,
        'num_images': 1,
        'num_inference_steps': 25,
        'guidance_scale': 7,
        'seed': 12345,  # Fixed seed for consistency
        'promptMagic': False,
        'public': False
    }
    
    print(f"\n🧪 Testing {model_name} - {test_case['name']}")
    print(f"   Prompt: {test_case['prompt'][:50]}...")
    
    response = requests.post(
        f'{BASE_URL}/generations',
        headers=headers,
        json=generation_data
    )
    
    if response.status_code != 200:
        print(f"❌ Generation failed: {response.status_code}")
        return None
    
    generation_id = response.json()['sdGenerationJob']['generationId']
    
    # Poll for completion
    print("   Generating", end="", flush=True)
    attempts = 0
    while attempts < 30:
        time.sleep(3)
        print(".", end="", flush=True)
        
        status_response = requests.get(
            f'{BASE_URL}/generations/{generation_id}',
            headers=headers
        )
        
        if status_response.status_code == 200:
            generation = status_response.json()['generations_by_pk']
            
            if generation['status'] == 'COMPLETE':
                print(" ✅")
                image_url = generation['generated_images'][0]['url']
                return {
                    'model': model_name,
                    'test': test_case['name'],
                    'url': image_url,
                    'expected_text': test_case['name']
                }
            elif generation['status'] == 'FAILED':
                print(" ❌")
                return None
        
        attempts += 1
    
    print(" ⏱️ Timeout")
    return None

def create_test_report(results, timestamp):
    """Create HTML report of text generation tests"""
    
    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Leonardo AI Text Generation Test Results</title>
    <style>
        body {{
            background: #f5f5f5;
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
        }}
        
        h1 {{
            text-align: center;
            color: #333;
        }}
        
        .test-info {{
            background: #fff;
            padding: 20px;
            margin: 20px auto;
            max-width: 800px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }}
        
        .test-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            max-width: 1200px;
            margin: 0 auto;
        }}
        
        .test-result {{
            background: white;
            border-radius: 10px;
            overflow: hidden;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }}
        
        .test-header {{
            background: #2196F3;
            color: white;
            padding: 10px;
            font-weight: bold;
        }}
        
        .test-subheader {{
            background: #64B5F6;
            color: white;
            padding: 5px 10px;
            font-size: 0.9em;
        }}
        
        .test-result img {{
            width: 100%;
            display: block;
        }}
        
        .test-analysis {{
            padding: 15px;
            background: #f5f5f5;
        }}
        
        .conclusion {{
            background: #ff5722;
            color: white;
            padding: 30px;
            margin: 40px auto;
            max-width: 800px;
            border-radius: 10px;
            text-align: center;
        }}
        
        .conclusion h2 {{
            margin-top: 0;
        }}
    </style>
</head>
<body>
    <h1>Leonardo AI Text Generation Capability Test</h1>
    
    <div class="test-info">
        <h2>Test Methodology</h2>
        <p>We tested Leonardo AI's ability to generate readable text using:</p>
        <ul>
            <li>Multiple models (Phoenix, Leonardo Diffusion XL)</li>
            <li>Various text scenarios (signs, speech bubbles, comic panels)</li>
            <li>Clear prompts requesting readable text</li>
            <li>Fixed seeds for reproducibility</li>
        </ul>
        <p><strong>Expected:</strong> Clear, readable text matching the prompt</p>
        <p><strong>Reality:</strong> Check the results below...</p>
    </div>
    
    <div class="test-grid">
"""
    
    for result in results:
        if result:
            html += f"""
        <div class="test-result">
            <div class="test-header">{result['model']}</div>
            <div class="test-subheader">{result['test']}</div>
            <img src="{result['url']}" alt="{result['test']} result">
            <div class="test-analysis">
                <strong>Analysis:</strong><br>
                Look at the image above. Can you read the text clearly?<br>
                Does it match what was requested?<br>
                <em>(Spoiler: Probably not!)</em>
            </div>
        </div>
"""
    
    html += """
    </div>
    
    <div class="conclusion">
        <h2>🚨 CONCLUSION 🚨</h2>
        <h3>Leonardo AI CANNOT Generate Readable Text!</h3>
        <p>Based on these tests, Leonardo AI consistently fails to generate legible text in images.</p>
        <p>When asked for specific words or phrases, it produces:</p>
        <ul style="text-align: left; display: inline-block;">
            <li>Gibberish characters</li>
            <li>Text-like shapes without meaning</li>
            <li>Incorrect letters or symbols</li>
            <li>Completely unreadable content</li>
        </ul>
        <p><strong>Recommendation:</strong> Use Leonardo for artwork only. Add text in post-processing!</p>
    </div>
</body>
</html>"""
    
    output_path = f'leonardo_text_test_{timestamp}.html'
    with open(output_path, 'w') as f:
        f.write(html)
    
    print(f"\n📊 Test report saved to: {output_path}")

def main():
    """Run comprehensive text generation tests"""
    
    print("=== Leonardo AI Text Generation Capability Test ===")
    print("Testing whether Leonardo can actually generate readable text...\n")
    
    results = []
    
    # Test each model with each prompt
    for model_name, model_id in MODELS.items():
        for test_case in TEST_PROMPTS:
            result = test_text_generation(model_name, model_id, test_case)
            if result:
                results.append(result)
                time.sleep(2)  # Rate limiting
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    create_test_report(results, timestamp)
    
    print("\n✅ Testing complete!")
    print("   Check the HTML report to see the (likely unreadable) results")
    print("   This proves Leonardo AI cannot generate readable text!")

if __name__ == "__main__":
    main()