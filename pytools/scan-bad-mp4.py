#!/usr/bin/env python3
#coding: utf-8

import os, sys, time

thispyname = os.path.basename(__file__)

d_subinfo = {}

def is_valid_mp4(mp4file):
#	good_header1 = b'\x00\x00\x00\x1C' + b'ftypmp42'
#	good_header2 = b'\x00\x00\x00\x14' + b'ftypisom'
	with open(mp4file, "rb") as f:
		headbytes = f.read(12)
		if headbytes[4:8]==b'ftyp':
			subinfo = headbytes[8:12]
			if subinfo in d_subinfo:
				d_subinfo[subinfo] += 1
			else:
				d_subinfo[subinfo] = 1
			return True
		else:
			return False

def main(scandir):

	count = 0
	badcount = 0
	for root, dirs, files in os.walk(scandir):
		for file in files:
			if not file.lower().endswith(".mp4"):
				continue

			count += 1
			mp4file = os.path.join(root, file)
			isgood = is_valid_mp4(mp4file)

			if isgood:
				print("\rScaned mp4 files %d ...     "%(count), end='')
			else:
				badcount += 1
				print("\r[Bad+%d] %s"%(badcount, mp4file))

	print("")
	print("Good mp4 files count: %d"%(count-badcount))
	for subinfo, subcount in d_subinfo.items():
		print("  [%s] %d"%( subinfo.decode('ascii'), subcount ))



if __name__=="__main__":

	nargs = len(sys.argv)
	if nargs==1:
		print("This program scans a directory for .mp4 files, to see whether they have correct headers.")
		print("Usage:")
		print("    %s <dir-to-scan>"%(thispyname))
		sys.exit(1)

	scandir = sys.argv[1]
	if not os.path.isdir(scandir):
		print("Not a directory: %s"%(scandir))
		sys.exit(1)

	ret = main(scandir)
	exit(ret)

