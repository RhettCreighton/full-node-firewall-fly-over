# Leonardo AI Text Generation: The Reality

## Executive Summary

**Leonardo AI CANNOT generate readable text in images.** This is a fundamental limitation that applies to ALL Leonardo models, including Phoenix.

## What Went Wrong

I misunderstood user feedback about "Phoenix generates speech bubbles natively" to mean it could render readable text inside those bubbles. This was completely incorrect.

## The Facts

### What Leonardo CAN Do:
- Generate beautiful artwork
- Create comic-style speech bubble SHAPES
- Position bubbles correctly with tails
- Apply comic book styling (halftone, inks, etc.)
- Create signs, banners, and text containers

### What Leonardo CANNOT Do:
- Generate readable text of any kind
- Create legible dialogue in speech bubbles
- Render accurate letters or numbers
- Produce consistent text across images
- Write words that humans can read

### Test Results

When prompted for specific text, Leonardo produces:
- Random gibberish characters
- Distorted letter-like shapes
- Inconsistent pseudo-text
- Never the actual requested words

Example:
- Prompt: "Sign saying HELLO WORLD"
- Result: Sign shape ✓, text like "HΞLLO VVΘRLD" or "H3LL0 W0RLĐ"

## The Correct Workflow

### Step 1: Generate Artwork
```python
prompt = "comic panel with empty speech bubbles, two philosophers"
# Leonardo creates the visual elements
```

### Step 2: Add Text Post-Processing
```python
from PIL import Image, ImageDraw, ImageFont

# Download Leonardo's output
img = Image.open("leonardo_output.jpg")
draw = ImageDraw.Draw(img)

# Add actual readable text
font = ImageFont.truetype("comic_font.ttf", 24)
draw.text((x, y), "Hello World!", font=font, fill="black")
```

## Common Misconceptions

### Myth 1: "Phoenix is trained on comics so it can do text"
**Reality:** Phoenix understands comic AESTHETICS (bubbles, panels, styling) but cannot generate readable text.

### Myth 2: "Just prompt better for text"
**Reality:** No prompt will make Leonardo generate readable text. This is a model limitation.

### Myth 3: "Other Leonardo models can do text"
**Reality:** NO Leonardo model can generate readable text reliably.

## Best Practices

### DO:
- Use Leonardo for artwork generation
- Generate panels with empty text areas
- Add text in post-processing
- Use PIL, ImageMagick, or Photoshop for text
- Plan for a two-step workflow

### DON'T:
- Expect readable text from any prompt
- Waste API calls trying different text prompts
- Promise text generation without post-processing
- Assume "native" means text included

## Alternative Solutions

If you need text in images:
1. **Professional Tools:** Photoshop, GIMP, Affinity
2. **Programmatic:** PIL/Pillow, ImageMagick, Canvas API
3. **Services:** Canva, Adobe Express
4. **Specialized Models:** Models specifically trained for text (research ongoing)

## Conclusion

Leonardo AI is an excellent tool for generating artwork, including comic-style panels with speech bubble shapes. However, it fundamentally cannot generate readable text. Any workflow involving text must include a post-processing step.

This is not a bug or a prompting issue - it's a current limitation of the technology.

## Updated Workflow Example

```python
# 1. Generate with Leonardo (empty bubbles)
response = leonardo_api.generate(
    prompt="comic panel with empty speech bubbles",
    model="phoenix"
)

# 2. Download image
image_url = response['url']
download_image(image_url, "panel.jpg")

# 3. Add text with PIL
img = Image.open("panel.jpg")
draw = ImageDraw.Draw(img)
draw.text((100, 100), "Actual readable text!", font=font)
img.save("panel_with_text.jpg")
```

Remember: Leonardo for art, code for text!