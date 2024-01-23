import os, sys
from PIL import Image, ImageDraw

exename = os.path.split(sys.argv[0])[1]

pngname = "white-lump.png"

nargs = len(sys.argv)

if nargs<4:
    print("Create a white bar image, gradient lump top-middle-bottom.");
    print("Usage: ");
    print("    %s <width> <top> <middle> [bottom]"%(exename));
    print("Example:")
    print("    %s 500  30 20 30");
    print("        This will create a 500x80 %s"%(pngname));
    exit(1)

width = int(sys.argv[1])
tops = int(sys.argv[2])
middles = int(sys.argv[3])
bottoms = int(sys.argv[4] if nargs>4 else tops)

height = tops + middles + bottoms

# Create a new image(100x50) with white background
image = Image.new('RGBA', (width, height), (255, 255, 255, 255))

# Create a drawing object
draw = ImageDraw.Draw(image)

thresA = 55
thresB = 255-thresA

# Draw gradient transparency from top to bottom
for y in range(tops):
    alpha = thresA + int((y / tops) * thresB)
    draw.line((0, y, width, y), fill=(255, 255, 255, alpha))

for y in range(middles):
    basey = tops
    draw.line((0, basey+y, width, basey+y), fill=(255, 255, 255, 255))

for y in range(bottoms):
    basey = tops + middles
    alpha = thresA + int((1 - y / bottoms) * thresB)
    draw.line((0, basey+y, width, basey+y), fill=(255, 255, 255, alpha))

# Save the image as a PNG file
image.save(pngname, 'PNG')
