#!/usr/bin/env python3
#coding: utf-8

import re, os, sys

if len(sys.argv)==1:
	print("Need an .vtt filename as input.")
	exit(1)

infile = sys.argv[1]
outfile = os.path.splitext(infile)[0]+".srt"

count = 0
with open(infile, "r", encoding='utf8') as infh, \
		open(outfile, "w", encoding='utf8') as outfh:
	while True:
		line = infh.readline()
		if line=='':
			break
		m = re.match(r'([0-9\:]+)\.([0-9]{3}) --> ([0-9\:]+)\.([0-9]{3})', line)
		if not m:
			continue
		
		subtitle_text = infh.readline()
		
		#print("%s"%(m.group(0)))

		count += 1
		srtsect = "%d\n"%(count)
		srtsect += "{},{} --> {},{}\n".format(m.group(1), m.group(2), m.group(3), m.group(4))
		srtsect += subtitle_text+"\n"
		
		outfh.write(srtsect)

print("Done: %s"%(outfile))
