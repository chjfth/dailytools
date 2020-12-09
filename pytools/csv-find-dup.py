#!/usr/bin/env python3
#coding: utf-8

import os, sys, locale
import argparse
import csv
import bisect
from enum import Enum,IntEnum # since Python 3.4

#print("sys.getdefaultencoding()=%s"%(sys.getdefaultencoding()))
#print("locale.getpreferredencoding()=%s"%(locale.getpreferredencoding()))

class VerboseLevel(IntEnum):
	vb0 = 0
	vb1 = 1
	vb2 = 2
	vb3 = 3

def find_dups(fh, fields_to_chk, vb, vblines_list_max=10):

	assert(type(fields_to_chk)==list and (len(fields_to_chk)==0 or type(fields_to_chk[0])==int))

	fh.seek(0)
	reader = csv.reader(fh)
	ireader = iter(reader)
	
	# read the first line(line #0)
	ar_headers = next(ireader)
	count_fields = len(ar_headers)

	if len(fields_to_chk)==0:
		fields_to_chk = list(range(0, len(ar_headers)))

	# find longest header chars
	max_header_width = max([len(ar_headers[fidx]) for fidx in fields_to_chk])
	max_header_prnwidth = min(max_header_width, 40)

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
	#
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
						print_vb23_detail(vb, 
							ar_field_stats[idx_field][dup_text], 
							vblines_list_max,
							fh)

	return 0
	
def print_vb23_detail(vb, ar_idxlines, list_max, fh):
	total = len(ar_idxlines)
	list_count = min(total, list_max)
	is_elps = total>list_max 
	
	s = ",".join(["%d"%(idx) for idx in ar_idxlines[0:list_count]])
	elps = "..." if is_elps else ""
	
	print("    Appears at #lines: %s %s"%(s, elps))

	if vb>=VerboseLevel.vb3:
		
		# For each field, we have to rescan the csv file again.
		# And we know that int-s in ar_idxlines are already sorted.

		fh.seek(0)
		done = 0
		for idx_line, row in enumerate(fh):
			idx = bisect.bisect_left(ar_idxlines, idx_line)
			
			if idx<len(ar_idxlines) and ar_idxlines[idx]==idx_line: # found
				print("    [#%d] %s"%(idx_line, row), end="")

				done += 1
				if done==list_count:
					break
		if is_elps:
			print("    ...")


def my_parse_args():
	
	ap = argparse.ArgumentParser(
		add_help=False, # bcz we (once) need to define -h ourselves
		formatter_class=argparse.RawTextHelpFormatter,
		description='Check for text duplicates for all/some CSV columns.'
	)
	# --use RawTextHelpFormatter so that "\n" in help="..." will be preserved.
	# https://stackoverflow.com/a/3853776/151453
	
	ap.add_argument('csv_filename', type=str, # nargs='?',
		help='Input a CSV file name to check.'
	)
	
	ap.add_argument('-f', '--field', type=int, action='append',
		help='Tell which CSV fields to check, identified by field index(0, 1, 2 etc).\n'
			' * If no -f option is provided, scan every field for duplicates. \n'
			' * If one or more -f options are provided, only those fields are scanned.\n'
	)
	
	ap.add_argument('-v', '--verbose', action='count', default=0,
		help='Enable verbose output.\n'
			' * No -v : list only count of duplicate groups.\n'
			' *    -v : list duplicate text as well.\n'
			' *   -vv : list #line of each duplicate appearance.\n'
			' *  -vvv : list line content as well.\n'
	)

	ap.add_argument('-e', '--encoding', type=str, default='',
		help='Assign text encoding of the input csv file. If omitted, system default will be used.\n'
			'Typical encodings: utf8, gbk, big5, utf16le.'
	)

	ap.add_argument('-x', '--max-verbose-lines', type=int, dest='max_verbose_lines', default=10,
		help='When -vv, showing duplicate lines is limited to this number.'
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
	fields_to_chk = args.field
	text_encoding = locale.getpreferredencoding() if not args.encoding else args.encoding
	vb = VerboseLevel(args.verbose)

	fh = open(csvfile, "r", encoding=text_encoding)
	
	if fields_to_chk==None:
		ret = find_dups(fh, [], vb)
	else:
		ret = find_dups(fh, fields_to_chk, vb, args.max_verbose_lines)

	if vb==VerboseLevel.vb0:
		print("")
		print("Hint: add option -v or -vv to show more duplicate details.")


if __name__=="__main__":
	ret = main()
	exit(ret)
