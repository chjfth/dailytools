#!/usr/bin/env python3
#coding: utf-8

import re, os, sys

def unquote_html_entity(s):
	s = s.replace('&nbsp;', ' ')
	s = s.replace('&lt', '<')
	s = s.replace('&gt', '>')
	return s


if len(sys.argv)==1:
	print("Need a .vtt filename as input.")
	exit(1)

infile = sys.argv[1]
outfile = os.path.splitext(infile)[0]+".srt"

count = 0
with open(infile, "r", encoding='utf8') as infh, \
		open(outfile, "w", encoding='utf8') as outfh:

	while True:
		line = infh.readline()
		
		if not line:
			break
		
		m = re.match(r'([0-9\:]+)\.([0-9]{3}) --> ([0-9\:]+)\.([0-9]{3})', line)
		
		if m:
			# Got a time-range line
			count += 1
			srtsect = "%d\n"%(count)
			srtsect += "{},{} --> {},{}\n".format(m.group(1), m.group(2), m.group(3), m.group(4))
			outfh.write(srtsect)
			continue
		
		if count==0:
			# ignore all vtt text before first subtitle line
			continue

		# This is subtitle line.
		# Replace each unquote html entity reference
		line = unquote_html_entity(line)
		outfh.write(line)
	

print("Done: %s"%(outfile))
