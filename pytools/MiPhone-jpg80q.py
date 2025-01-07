#!/usr/bin/env python3

"""[2024-02-01]
XiaoMi Phone M11U saves jpg with 96% quality, which causes the jpg files unnecessarily big.
Re-encoding then to 80% quality will reduce the file size to be 25~30% of the original.
"""

import os, sys, shutil
import re
from fnmatch import fnmatch
from PIL import Image

MARK_PANO = "_PANO"
MARK_SNAP = "_SNAP"
MARK_MVIMG = "_MVIMG" # sth like iPhone Live-photo

pyfilename = os.path.split(sys.argv[0])[1]

def match_startswith(prefix, array):
	# for each ele in array, check whether ele startswith prefix.
	# If yes, return the index; if none found in array, return -1
	for idx, ele in enumerate(array):
		if ele.startswith(prefix):
			return idx
	
	return -1

def copy_screenshots(indir):
	# Copy $indir/Scrn/Screenshot_2023-01-04-18-18-35-559_someappname.jpg
	# to
	# $outdir/IMG_20230104_181835.559_scrn-someappname.jpg

	indir_sub = os.path.join(indir, 'Scrn')
	if os.path.isdir(indir_sub):
		print("=== Scanning Screenshot_xxx.jpg in " + indir_sub + " ...")
	else:
		print("### No Screenshot dir(so skip it): " + indir_sub)
		return

	scrjpgs = os.listdir(indir_sub)
	converged_jpgs = os.listdir(indir)
	converged_stems = [os.path.splitext(filename)[0] for filename in converged_jpgs]

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
		dstfn_stem = f"IMG_{year}{month}{day}_{hour}{minute}{second}.{millisec}"
		dstfn = dstfn_stem + "_scrn.jpg" # example: "IMG_20240712_110504.047_scrn.jpg"

		if match_startswith(dstfn_stem, converged_stems)>=0 :
			# This jpg had been copied since previous run, do not copy again.
			continue

		sjpgpath = os.path.join(indir_sub, scrfn)
		djpgpath = os.path.join(indir, dstfn)

#		print("===", sjpgpath)
#		print("   ", djpgpath)

		print("Screenshot: %s"%(djpgpath))
		shutil.copy(sjpgpath, djpgpath)


def jpgfn_get_newname(jpgfn):

	# If jpgfn should be copied to new-dir, return the new-name it should be.
	# Otherwise, return None.

	m = re.match(r"IMG_([0-9]{8}_[0-9]{6})(.+)\.jpg$", jpgfn)
	#                                     ^ suffix
	if m:
		timestp = m.group(1)
		suffix = m.group(2)
		Suffix = suffix.upper()
		if Suffix==MARK_PANO:
			return None
		elif Suffix==MARK_SNAP:
			return None
		elif Suffix==MARK_MVIMG:
			return None
		elif re.match(r"^_[0-9]+$", suffix):
			# IMG_20240705_080808_1.jpg, IMG_20240705_080808_2.jpg etc
			# This is from burst-shot duplicates; discard it.
			return None
		elif re.match(r"^\.[0-9]{3}_scrn$", suffix):
			# IMG_20240712_110504.047_scrn.jpg etc
			# This is un-renamed(meaning ungiven) screenshot jpg (quality 100).
			return None
		elif re.match(r'_TIMEBURST([0-9]+)', Suffix):
			# IMG_20240621_162431_TIMEBURST1.jpg, IMG_20240621_162431_TIMEBURST2.jpg etc
			return None
		else:
			return jpgfn
	else:
		return None

def rename_panos(indir):
	# For PANO_yyyymmdd_hhmmss...jpg, I will rename it to IMG_yyyymmdd_hhmmss_PANO...jpg
	# so that when the filenames are sorted alphabetically, they natuarally appears timestamp sorted.
	#
	# [2024-07-06] Same process for MVIMG_yyyymmdd_hhmmss...jpg

	files = os.listdir(indir)
	for file in files:
		m = re.match(r"(PANO|MVIMG)_([0-9]{8}_[0-9]{6})(.*)\.jpg$", file)
		#               prefix       timestp            suffix
		if not m:
			continue

		prefix = m.group(1)
		timestp = m.group(2)
		suffix = m.group(3)
		newfile = "IMG_" + timestp + "_" + prefix + suffix + ".jpg"

		oldpath = os.path.join(indir, file)
		newpath = os.path.join(indir, newfile)
		print("Renamed: " + newpath)
		os.rename(oldpath, newpath)

def do_main():
	if len(sys.argv)<3:
		print("Usage:")
		print("    %s <input-dir> <output-dir>"%(pyfilename))
		print("")
		print("Files with the following pattern, will be re-encoded to be 80% jpg quality.")
		print("    IMG_20231107_184622 xxx.jpg")
		print("    IMG_20231107_184622_xxx.jpg")
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

	print("=== Scanning Camera jpg-s in " + indir + " ...")

	rename_panos(indir)

	nskip = 0
	nencode = 0

	for jpgfn in os.listdir(indir):
		jpgfn_new = jpgfn_get_newname(jpgfn)
		if not jpgfn_new:
			continue

		oldjpgpath = os.path.join(indir, jpgfn)
		newjpgpath = os.path.join(outdir, jpgfn_new)

		if os.path.exists(newjpgpath):
			print("Skip existing: %s"%(newjpgpath))
			nskip += 1
			continue

		print("Encoding to: %s"%(newjpgpath))

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
