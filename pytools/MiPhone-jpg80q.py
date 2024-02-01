#!/usr/bin/env python3

"""[2024-02-01]
XiaoMi Phone M11U saves jpg with 96% quality, which causes the jpg files unnecessarily big.
Re-encoding then to 80% quality will reduce the file size to be 25~30% of the original.
"""

import os, sys
import re
from PIL import Image

pyfilename = os.path.split(sys.argv[0])[1]

def jpgfn_need_process(jpgfn):
	m = re.match(r"IMG_[0-9]{8}_[0-9]{6}(.+).jpg", jpgfn)
	if m:
		suffix = m.group(1)
		if re.match(r"_[0-9]+", suffix):
			# This is from burst-shot duplicates; discard it.
			return False
		else:
			return True
	else:
		return False

if __name__=='__main__':
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

	nskip = 0
	nencode = 0

	for jpgfn in os.listdir(indir):
		if not jpgfn_need_process(jpgfn):
			continue

		oldjpgpath = os.path.join(indir, jpgfn)
		newjpgpath = os.path.join(outdir, jpgfn)

		if os.path.exists(newjpgpath):
			print("Skip existing %s"%(newjpgpath))
			nskip += 1
			continue

		print("Encoding %s"%(newjpgpath))

		with Image.open(oldjpgpath) as img:
			img.save(newjpgpath, quality=80)

		nencode += 1

	print("Done, skip %d + re-encode %d, total %d"%(nskip, nencode, nskip+nencode))
