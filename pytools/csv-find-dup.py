#!/usr/bin/env python3
#coding: utf-8

import os, sys, locale
import argparse
import csv
from enum import Enum,IntEnum # since Python 3.4

#print("sys.getdefaultencoding()=%s"%(sys.getdefaultencoding()))
#print("locale.getpreferredencoding()=%s"%(locale.getpreferredencoding()))

class VerboseLevel(IntEnum):
	vb0 = 0
	vb1 = 1
	vb2 = 2

def count_dups(fh, fields_to_chk, vb):

	assert(type(fields_to_chk)==list and (len(fields_to_chk)==0 or type(fields_to_chk[0])==int))

	fh.seek(0)
	reader = csv.reader(fh)
	ireader = iter(reader)
	
	# read the first line(line #0)
	ar_headers = next(ireader)
	count_fields = len(ar_headers)

	# find longest header chars
	max_header_width = max([len(x) for x in ar_headers])
	max_header_prnwidth = min(max_header_width, 40)

	if len(fields_to_chk)==0:
		fields_to_chk = list(range(0, len(ar_headers)))

	ar_field_stats = [ { header_name : [0] } for header_name in ar_headers ]
		# [0] above means, the header text appears in line #0
	
	for idx_line, row in enumerate(ireader, start=1):
		for i in fields_to_chk:

			field_text = row[i]
			if field_text=="":
				continue
			dict_stat = ar_field_stats[i]
			if field_text in dict_stat:
				dict_stat[field_text].append(idx_line)
			else:
				dict_stat[field_text] = [idx_line]

	ar_dupcount = [ {} for header_name in ar_headers ]
	for idx_field, dict_stat in enumerate(ar_field_stats):
		for key, text2idxlines in dict_stat.items():
			dupcount = len(text2idxlines)
			if dupcount>1 :
				ar_dupcount[idx_field][key] = dupcount

	for idx_field, dict_dupcount in enumerate(ar_dupcount):
		
		if not idx_field in fields_to_chk:
			continue

		headertext = ar_headers[idx_field]

		if vb==VerboseLevel.vb0: 
			# we need to align header text in verbose level 0
			headertext = "%-*.*s"%(max_header_prnwidth, max_header_prnwidth, 
											ar_headers[idx_field])

		dup_groups = len(dict_dupcount)
		if dup_groups==0:
			print("Field %d (%s) no duplicate items."%(idx_field, headertext))
		
		else:
			print("Field %d (%s) has %d group of duplicate items%s"%(
				idx_field, headertext, dup_groups,
				"." if vb==VerboseLevel.vb0 else ":"
				))
		
			if vb>=VerboseLevel.vb1:
				for dup_text, dup_count in dict_dupcount.items():
					print("  (%d*) %s"%(dup_count, dup_text))

					if vb>=VerboseLevel.vb2:
						print_vb2_detail( ar_field_stats[idx_field][dup_text] )

	return 0
	
def print_vb2_detail(ar_idxlines):
	s = ",".join(["%d"%(idx) for idx in ar_idxlines])
	print("    Appears at #lines: %s"%(s))
	

def my_parse_args():
	
	ap = argparse.ArgumentParser(
		add_help=False, # bcz we need to define -h ourselves
		description='Check for text duplicates for all/some CSV columns.'
	)
	
	ap.add_argument('csv_filename', type=str, # nargs='?',
		help='Input a CSV file name to check.'
	)
	
	ap.add_argument('-h', '--header', type=int, action='append',
		help='Tell which CSV fields to check, identified by index(0, 1, 2 etc).\n'
			'If no -h option is provided, scan every field for duplicates, and prints a summary. \n'
			'If one or more -h options are provided, only those headers(fields) are scanned.\n'
	)
	
	ap.add_argument('-v', '--verbose', action='count', default=0,
		help='Enable verbose output.\n'
			'Without -h, \n'
			' * non-verbose mode only list count of duplicate groups.\n'
			' * verbose mode as well list duplicate text from each dup-group.\n'
			'With -h, \n'
			' * non-verbose mode only list duplicate text and dup-count from each dup-group.\n'
			' * verbose mode as well list #line of those duplicates.\n'
	)

	ap.add_argument('-e', '--encoding', type=str, default='',
		help='Assign text encoding of the input csv file. If omitted, system default will be used.\n'
			'Typical encodings: utf8, gbk, big5, utf16le.'
	)

	if len(sys.argv)==1:
		# If no command-line parameter given, display help and quit.
		print("You must provide a csv filename.")
		print("To see full program options, run:\n")
		print("  %s --help"%(os.path.basename(__file__)))
		exit(1)

	if sys.argv[1]=='--help':
		ap.print_help()
		exit(1)

	args = ap.parse_args()

	return args

def main():
	args = my_parse_args()

	csvfile = args.csv_filename
	headers_to_chk = args.header
	text_encoding = locale.getpreferredencoding() if not args.encoding else args.encoding
	vb = VerboseLevel(args.verbose)

	fh = open(csvfile, "r", encoding=text_encoding)
	
	if headers_to_chk==None:
		ret = count_dups(fh, [], vb)
	else:
		ret = count_dups(fh, headers_to_chk, vb)

	if vb==VerboseLevel.vb0:
		print("")
		print("Hint: add option -v or -vv to show more duplicate details.")


if __name__=="__main__":
	ret = main()
	exit(ret)
