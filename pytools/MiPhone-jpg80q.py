#!/usr/bin/env python3

"""[2024-02-01]
XiaoMi Phone M11U saves jpg with 96% quality, which causes the jpg files unnecessarily big.
Re-encoding then to 80% quality will reduce the file size to be 25~30% of the original.
"""

import os, sys, shutil
import re
from PIL import Image

pyfilename = os.path.split(sys.argv[0])[1]

def copy_screenshots(indir):
	# Copy $indir/Scrn/Screenshot_2023-01-04-18-18-35-559_someappname.jpg
	# to
	# $outdir/IMG_20230104_181835.559_scrn-someappname.jpg

	indir_sub = os.path.join(indir, 'Scrn')
	if os.path.isdir(indir_sub):
		print("=== Scanning Screenshot_xxx.jpg in " + indir_sub + "...")
	else:
		print("### No Screenshot dir: " + indir_sub)
		return

	scrjpgs = os.listdir(indir_sub)

	for scrfn in scrjpgs:

		m = re.match(
			r"Screenshot_([0-9]{4})-([0-9]{2})-([0-9]{2})-([0-9]{2})-([0-9]{2})-([0-9]{2})-([0-9]{3})_(.+).jpg",
			scrfn
		)
		if not m:
			continue

		year = m.group(1)
		month = m.group(2)
		day = m.group(3)
		hour = m.group(4)
		minute = m.group(5)
		second = m.group(6)
		millisec = m.group(7)
		dstfn = f"IMG_{year}{month}{day}_{hour}{minute}{second}_{millisec}.jpg"

		sjpgpath = os.path.join(indir_sub, scrfn)
		djpgpath = os.path.join(indir, dstfn)

#		print("===", sjpgpath)
#		print("   ", djpgpath)

		if os.path.exists(djpgpath):
			continue

		print("Screenshot: %s"%(djpgpath))
		shutil.copy(sjpgpath, djpgpath)


def jpgfn_get_newname(jpgfn):

	# Note: For PANO_yyyymmdd_hhmmss...jpg, I will rename it to IMG_yyyymmdd_hhmmss_pano...jpg
	# so that when the filenames are sorted alphabetically, they natuarally appears timestamp sorted.

	m = re.match(r"(IMG|PANO)(_[0-9]{8}_[0-9]{6})(.+).jpg", jpgfn)
	if m:
		prefix = m.group(1)
		midpart = m.group(2)
		suffix = m.group(3)
		if re.match(r"^_[0-9]+$", suffix):
			# This is from burst-shot duplicates; discard it.
			return None
		else:
			if(prefix=="IMG"):
				return jpgfn
			else:
				return "IMG" + midpart + "_pano" + suffix + ".jpg"
	else:
		return None

def do_main():
	if len(sys.argv)<3:
		print("Usage:")
		print("    %s <input-dir> <output-dir>"%(pyfilename))
		print("")
		print("Files with the following pattern, will be re-encoded to be 80% jpg quality.")
		print("    IMG_20231107_184622 xxx.jpg")
		print("    IMG_20231107_184622_xxx.jpg")
		print("    PANO_20231107_184622 xxx.jpg")
		print("    PANO_20231107_184622_xxx.jpg")
		print("")
		print("Files like IMG_20231107_184622.jpg will be left as is.")
		exit(1)

	indir = sys.argv[1]
	outdir = sys.argv[2]

	if not os.path.isdir(indir):
		print("Input dir not exist: %s"%(indir))

	if not os.path.exists(outdir):
		os.mkdir(outdir)

	copy_screenshots(indir)

	nskip = 0
	nencode = 0

	for jpgfn in os.listdir(indir):
		jpgfn_new = jpgfn_get_newname(jpgfn)
		if not jpgfn_new:
			continue

		oldjpgpath = os.path.join(indir, jpgfn)
		newjpgpath = os.path.join(outdir, jpgfn_new)

		if os.path.exists(newjpgpath):
			print("Skip existing %s"%(newjpgpath))
			nskip += 1
			continue

		print("Encoding to %s"%(newjpgpath))

		with Image.open(oldjpgpath) as img:
			exif_info = img.info.get('exif')
			if exif_info:
				img.save(newjpgpath,
				         quality=80,
				         exif=exif_info
				         )
			else:
				img.save(newjpgpath, quality=80)

		nencode += 1

		oldsize = os.path.getsize(oldjpgpath)
		newsize = os.path.getsize(newjpgpath)
		print("    %.1f MB -> %.1f MB (reduced to %d%%)"%(
			oldsize/(1024*1024),
			newsize/(1024*1024) ,
			newsize*100/oldsize
		))

	print("Done, skip %d + re-encode %d, total %d"%(nskip, nencode, nskip+nencode))

if __name__=='__main__':
	do_main()
