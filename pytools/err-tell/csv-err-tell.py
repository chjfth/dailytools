#!/usr/bin/env python3
#coding: utf-8

import os, sys, locale
import csv

def enum_csv_rows_by_filehandle(fh):
	while True:
		filepos = fh.tell() # DANGEROUS?!
		linetext = fh.readline()
		
		if linetext=='':
			return # meet end of file
		
		csv_row = next( csv.reader([linetext]) )
		# -- csv_row is a list of celt.
		yield filepos, csv_row 

def main(csv_filename):

	fh = open(csv_filename, 'r', encoding='gbk')

	fh.seek(0)
	csv_spliter = enum_csv_rows_by_filehandle(fh)
	
	for idx_line, (ofs, row) in enumerate(csv_spliter):
		print("#%d @%d : %s"%(idx_line+1, ofs, row))


if __name__=="__main__":
	
	csv_filename = 'err-tell-gbk.csv'
	
	if len(sys.argv)>1:
		print("Using a csv filename %s."%(sys.argv[1]))
		csv_filename = sys.argv[1]
		
	main(csv_filename)
	
	exit(0)
