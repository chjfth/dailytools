#!/usr/bin/env python3
#coding: utf-8

# See weird behavior on Python 3.5~3.7...

import os, sys, locale

g_use_ftell = True

def enum_csv_rows_by_filehandle(fh):
	while True:
		
		if g_use_ftell:
			filepos = fh.tell() # tell() is DANGEROUS!!
		else:
			filepos = -1
		
		linetext = fh.readline()
		
		if linetext=='':
			return # meet end of file
		
		yield filepos, linetext

def main(csv_filename):

	fh = open(csv_filename, 'r', encoding='gbk')

	csv_spliter = enum_csv_rows_by_filehandle(fh)
	
	for idx_line, (ofs, row) in enumerate(csv_spliter):
		print("#%d @%d : %s"%(idx_line+1, ofs, row))


if __name__=="__main__":

	global g_use_ftell
	csv_filename = 'err-tell-gbk.csv'
	
	if len(sys.argv)>1:
		print("Using a csv filename %s."%(sys.argv[1]))
		csv_filename = sys.argv[1]
	
	if len(sys.argv)>2 and sys.argv[2]=="0":
		g_use_ftell = False
		
	main(csv_filename)
	
	exit(0)
