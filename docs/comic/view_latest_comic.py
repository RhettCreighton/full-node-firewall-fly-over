#!/usr/bin/env python3
"""
Create viewer for the latest generated comic images
"""

import json
from datetime import datetime

# The successfully generated images
COMIC_IMAGES = [
    {
        "title": "The Academy Garden - Array Bounds",
        "url": "https://cdn.leonardo.ai/users/f26f4645-9d1f-4608-8d43-255fb78008b6/generations/ac34d2d6-040b-4c68-8cdd-f36b662c462c/Leonardo_Anime_XL_Comic_book_page_two_ancient_Greek_philosophe_0.jpg",
        "concept": "Array bounds checking prevents segfaults",
        "dialogue": "CODE: if (index >= 16) return -1;<br>MEANING: We check every array access!"
    },
    {
        "title": "The Null Bridge Over Chaos",
        "url": "https://cdn.leonardo.ai/users/f26f4645-9d1f-4608-8d43-255fb78008b6/generations/431e2f03-fcd2-43f3-b7ab-2898c78d9c50/Leonardo_Anime_XL_Comic_book_panel_ancient_Greek_stone_bridge_0.jpg",
        "concept": "NULL pointer checks prevent crashes",
        "dialogue": "CODE: if (!ptr) return;<br>MEANING: Never use a pointer without verifying it exists!"
    },
    {
        "title": "The Oracle's Revelation",
        "url": "https://cdn.leonardo.ai/users/f26f4645-9d1f-4608-8d43-255fb78008b6/generations/619298da-820f-4ef1-83b5-a5cc35260f3f/Leonardo_Anime_XL_Comic_book_splash_panel_Temple_of_Delphi_int_0.jpg",
        "concept": "Complete verification achieves certainty",
        "dialogue": "CODE: assert(verify_all_truths());<br>MEANING: We prove mathematically that no crash is possible!"
    },
    {
        "title": "The Division Paradox",
        "url": "https://cdn.leonardo.ai/users/f26f4645-9d1f-4608-8d43-255fb78008b6/generations/b8a0ed4e-e21b-4746-b035-3f15814f89df/Leonardo_Anime_XL_Comic_book_panel_Greek_theater_PLATO_dividin_0.jpg",
        "concept": "Division by zero handling",
        "dialogue": "CODE: if (denominator < 0.0001) return SAFE_VALUE;<br>MEANING: We detect and handle impossible math!"
    },
    {
        "title": "The Walled City",
        "url": "https://cdn.leonardo.ai/users/f26f4645-9d1f-4608-8d43-255fb78008b6/generations/c439823f-f5fb-41c1-b874-af3421109e45/Leonardo_Anime_XL_Comic_book_panel_Acropolis_walls_with_exactl_0.jpg",
        "concept": "Resource limits enforced",
        "dialogue": "CODE: if (count >= MAX_AIRCRAFT) return -1;<br>MEANING: We enforce hard limits - no overflow possible!"
    }
]

def create_comic_viewer():
    """Create an awesome comic book viewer"""
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    
    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>The Eternal Core - Code & Philosophy Comic</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Bangers&family=Comic+Neue:wght@400;700&display=swap');
        
        body {{
            background: #2c2c2c;
            background-image: 
                repeating-linear-gradient(
                    45deg,
                    transparent,
                    transparent 10px,
                    rgba(255,255,255,.02) 10px,
                    rgba(255,255,255,.02) 20px
                );
            margin: 0;
            padding: 20px;
            font-family: 'Comic Neue', cursive;
        }}
        
        .comic-header {{
            text-align: center;
            margin-bottom: 40px;
            background: linear-gradient(135deg, #e74c3c, #f39c12);
            border: 5px solid #000;
            padding: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.5);
            transform: perspective(500px) rotateX(5deg);
        }}
        
        h1 {{
            font-family: 'Bangers', cursive;
            font-size: 5em;
            color: #fff;
            text-shadow: 5px 5px 0 #000, 10px 10px 20px rgba(0,0,0,0.5);
            margin: 0;
            letter-spacing: 5px;
            animation: pulse 2s infinite;
        }}
        
        @keyframes pulse {{
            0%, 100% {{ transform: scale(1); }}
            50% {{ transform: scale(1.05); }}
        }}
        
        .subtitle {{
            font-size: 1.5em;
            color: #fff;
            text-shadow: 2px 2px 0 #000;
            margin-top: 10px;
        }}
        
        .comic-panel {{
            max-width: 1200px;
            margin: 40px auto;
            background: #fff;
            border: 5px solid #000;
            padding: 20px;
            box-shadow: 0 20px 40px rgba(0,0,0,0.8);
            position: relative;
            overflow: hidden;
        }}
        
        .comic-panel:nth-child(odd) {{
            transform: rotate(-1deg);
        }}
        
        .comic-panel:nth-child(even) {{
            transform: rotate(1deg);
        }}
        
        .panel-header {{
            background: #000;
            color: #fff;
            padding: 15px;
            margin: -20px -20px 20px -20px;
            font-family: 'Bangers', cursive;
            font-size: 1.8em;
            letter-spacing: 2px;
        }}
        
        .comic-panel img {{
            width: 100%;
            display: block;
            border: 3px solid #000;
        }}
        
        .dialogue-overlay {{
            background: #ffeb3b;
            border: 3px solid #000;
            padding: 20px;
            margin: 20px 0;
            position: relative;
            font-size: 1.2em;
            font-weight: bold;
        }}
        
        .dialogue-overlay::before {{
            content: "💭";
            position: absolute;
            top: -15px;
            left: 20px;
            font-size: 30px;
        }}
        
        .code-part {{
            background: #2c3e50;
            color: #2ecc71;
            padding: 10px;
            border-radius: 5px;
            font-family: 'Courier New', monospace;
            margin: 10px 0;
            display: inline-block;
        }}
        
        .meaning-part {{
            color: #e74c3c;
            font-size: 1.1em;
            margin-top: 10px;
        }}
        
        .concept-badge {{
            position: absolute;
            top: 20px;
            right: -30px;
            background: #e74c3c;
            color: #fff;
            padding: 10px 50px;
            transform: rotate(45deg);
            font-family: 'Bangers', cursive;
            font-size: 1.2em;
            box-shadow: 0 5px 10px rgba(0,0,0,0.3);
        }}
        
        .proof-summary {{
            max-width: 1000px;
            margin: 60px auto;
            background: #fff;
            border: 5px solid #000;
            padding: 40px;
            box-shadow: 0 20px 40px rgba(0,0,0,0.8);
        }}
        
        .proof-summary h2 {{
            font-family: 'Bangers', cursive;
            font-size: 3em;
            text-align: center;
            color: #e74c3c;
            text-shadow: 3px 3px 0 #000;
        }}
        
        .philosophy-quote {{
            text-align: center;
            font-size: 2em;
            font-style: italic;
            color: #2c3e50;
            margin: 40px 0;
            padding: 20px;
            border-left: 5px solid #e74c3c;
            border-right: 5px solid #e74c3c;
        }}
        
        .footer {{
            text-align: center;
            padding: 40px;
            color: #fff;
            font-size: 1.2em;
        }}
    </style>
</head>
<body>
    <div class="comic-header">
        <h1>THE ETERNAL CORE</h1>
        <div class="subtitle">Where Ancient Philosophy Meets Modern Code</div>
    </div>
"""
    
    # Add each comic panel
    for i, panel in enumerate(COMIC_IMAGES):
        html += f"""
    <div class="comic-panel">
        <div class="panel-header">{panel['title']}</div>
        <div class="concept-badge">PROVEN!</div>
        <img src="{panel['url']}" alt="{panel['title']}">
        <div class="dialogue-overlay">
            <div>🏛️ <strong>Ancient Wisdom + Modern Code:</strong></div>
            <div class="code-part">{panel['dialogue'].split('<br>')[0]}</div>
            <div class="meaning-part">{panel['dialogue'].split('<br>')[1]}</div>
        </div>
    </div>
"""
    
    # Add proof summary
    html += """
    <div class="proof-summary">
        <h2>THE PHILOSOPHICAL PROOF IS COMPLETE!</h2>
        
        <p style="font-size: 1.3em; line-height: 1.8;">
            Through their journey across Ancient Greece, Socrates and Plato have demonstrated
            that <strong>Full Node: Firewall Fly-over</strong> will never coredump by proving:
        </p>
        
        <ul style="font-size: 1.2em; line-height: 2;">
            <li>✅ <strong>Array Bounds:</strong> Every access is guarded like Plato's garden</li>
            <li>✅ <strong>NULL Pointers:</strong> Every bridge is tested before crossing</li>
            <li>✅ <strong>Division by Zero:</strong> Every paradox has a safe resolution</li>
            <li>✅ <strong>Resource Limits:</strong> The city walls enforce absolute boundaries</li>
            <li>✅ <strong>Truth Buckets:</strong> Mathematical certainty through verification</li>
        </ul>
        
        <div class="philosophy-quote">
            "The unexamined code is not worth running!"<br>
            - Socrates
        </div>
        
        <p style="text-align: center; font-size: 1.5em;">
            <strong>∴ No Coredump Shall Ever Occur!</strong> Q.E.D.
        </p>
    </div>
    
    <div class="footer">
        <p>🎨 Art by Leonardo AI • 💭 Philosophy by Ancient Greece • 🔍 Verification by Truth Buckets</p>
        <p>Generated: {timestamp}</p>
    </div>
</body>
</html>"""
    
    # Save the viewer
    output_path = f'docs/comic/eternal_core_with_code_{timestamp}.html'
    with open(output_path, 'w') as f:
        f.write(html)
    
    print(f"\n📚 Comic viewer created: {output_path}")
    print("\n🎨 View your philosophical code comic:")
    print(f"   firefox {output_path}")
    
    # Also save the image URLs for reference
    with open(f'docs/comic/latest_generation_{timestamp}.json', 'w') as f:
        json.dump(COMIC_IMAGES, f, indent=2)

if __name__ == "__main__":
    create_comic_viewer()