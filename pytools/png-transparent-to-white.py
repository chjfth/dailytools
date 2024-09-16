#!/usr/bin/env python3
#coding: utf-8

import os, sys
from PIL import Image


def gen_mask_png(infile):
	
	stem, ext = os.path.splitext(infile)
	outfile = stem + ".mask" + ext
	
	# Load the image
	image = Image.open(infile).convert("RGBA")
	pixels = image.load()

	# Loop through the pixels in the image
	for x in range(image.width):
	    for y in range(image.height):
	        # Get the current pixel (R, G, B, A)
	        r, g, b, a = pixels[x, y]
	       
	        if a==0:
	            # Change this pixel to white, so that it can be used
	            # as a masking image in SDWebUI [Inpaint upload].
	            pixels[x, y] = (255, 255, 255, 255)
	        else:
	            pixels[x, y] = (0, 0, 0, 255)

	# Save the modified image
	image.save(outfile)
	print("Success. Outputfile:")
	print(outfile)

if __name__=='__main__':
	if len(sys.argv)==1:
		print("Need a 32-bit png file as input.")
		print("This program changes [all transparent pixels into pure-white] and "
			"[all non-transparent pixels into pure-black], then outputs an black-and-white new png, "
			"so that the new png can be used as a mask file for Stable Diffusion WebUI.")
		sys.exit(1)
	
	gen_mask_png(sys.argv[1])
	