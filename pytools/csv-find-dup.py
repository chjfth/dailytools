#!/usr/bin/env python3
#coding: utf-8

import os, sys, locale
import argparse
import csv

#print("sys.getdefaultencoding()=%s"%(sys.getdefaultencoding()))
#print("locale.getpreferredencoding()=%s"%(locale.getpreferredencoding()))

def count_dups(fh, verbose):
	fh.seek(0)
	reader = csv.reader(fh)
	ireader = iter(reader)
	
	ar_headers = next(ireader)
	count_fields = len(ar_headers)

	# find longest header chars
	max_header_width = max([len(x) for x in ar_headers])
	max_header_prnwidth = min(max_header_width, 40)

	ar_field_stats = [ { header_name : 1 } for header_name in ar_headers ]
	
	for row in ireader:
		for i, field_text in enumerate(row):
			if field_text=="":
				continue
			dict_stat = ar_field_stats[i]
			if field_text in dict_stat:
				dict_stat[field_text] += 1
			else:
				dict_stat[field_text] = 1

	ar_dupcount = [ {} for header_name in ar_headers ]
	for idx_field, dict_stat in enumerate(ar_field_stats):
		
		for key, count in dict_stat.items():
			if count>1 :
				ar_dupcount[idx_field][key] = count

	for idx_field, dict_dupcount in enumerate(ar_dupcount):
		
		fixed_width_fieldtext = "%-*.*s"%(max_header_prnwidth, max_header_prnwidth, 
									ar_headers[idx_field])

		dup_groups = len(dict_dupcount)
		if dup_groups==0:
			print("Field %d (%s) no duplicate items."%(idx_field, fixed_width_fieldtext))
		else:
			print("Field %d (%s) has %d group of duplicate items%s"%(
				idx_field, fixed_width_fieldtext, dup_groups,
				"." if not verbose else ":"
				))
			if verbose:
				for dup_text, dup_count in dict_dupcount.items():
					print("  (%d*) %s"%(dup_count, dup_text))

	return 0
	

def my_parse_args():
	
	ap = argparse.ArgumentParser(
		add_help=False, # bcz we need to define -h ourselves
		description='Check for text duplicates for all/some CSV columns.'
	)
	
	ap.add_argument('csv_filename', type=str, # nargs='?',
		help='Input a CSV file name to check.'
	)
	
	ap.add_argument('-h', '--header', type=str, action='append',
		help='Tell which CSV fields to check. A CSV field is identified by its header value, that is, the field text from its first line.\n'
			'If no -h option is provided, scan every field for duplicates, and prints a summary. \n'
			'If one or more -h options are provided, only those headers(fields) are scanned.\n'
	)
	
	ap.add_argument('-v', '--verbose', action='store_true',
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

if __name__=="__main__":
	args = my_parse_args()

	csvfile = args.csv_filename
	headers_to_chk = args.header
	text_encoding = locale.getpreferredencoding() if not args.encoding else args.encoding
	verbose = 0 if not args.verbose else 1

	fh = open(csvfile, "r", encoding=text_encoding)
	
	if headers_to_chk==None:
		ret = count_dups(fh, verbose)
	else:
		ret = 4

	exit(ret)

