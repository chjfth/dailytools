#!/usr/bin/env python3

"""[2025-08-13] Re-enocde Mi8 phone IMG_2025xxxx_yyyyyy.jpg photos to 80% quality to cur-dir.
"""

import os, sys, shutil
from PIL import Image

pyfilename = os.path.split(sys.argv[0])[1]
output_quality = 80

def do_main():
	if len(sys.argv)<4:
		print("Usage:")
		print("    %s <jpg_input_dir> <start_page> <end_page>"%(pyfilename))
		exit(1)

	indir = sys.argv[1]
	start_page = int(sys.argv[2])
	end_page = int(sys.argv[3])

	if not os.path.isdir(indir):
		print("Input dir not exist: %s"%(indir))

	nowpage = start_page

	jpgfn_list = os.listdir(indir)
	jpgfn_list.sort()
	
	for jpgfn in jpgfn_list:
		
		if not jpgfn.lower().endswith('.jpg'):
			continue
		
		jpgfp = os.path.join(indir, jpgfn)
		
		print(f"{jpgfn} => {nowpage:03d}")
		
		with Image.open(jpgfp) as img:
			newjpgpath = f"page{nowpage:03d}.jpg"

			exif_info = img.info.get('exif')
			if exif_info:
				img.save(newjpgpath,
				         quality=output_quality,
				         exif=exif_info
				         )
			else:
				img.save(newjpgpath, quality=80)

		nowpage +=1

	if nowpage != end_page+1:
		print(f"[ERROR] Input page range does NOT match jpg files in {indir}.")
		exit(1)

if __name__=='__main__':
	do_main()

""" Then use ImageMagick command line to produc PDF.

	convert *.jpg  -units PixelsPerInch  -density 360   CombinedImages.pdf

"""