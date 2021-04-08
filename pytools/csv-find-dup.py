#!/usr/bin/env python3
#coding: utf-8

import os, sys, locale
import argparse
import csv
import codecs
import bisect
import datetime
from enum import Enum,IntEnum # since Python 3.4
from collections import namedtuple
import locale, functools

#print("sys.getdefaultencoding()=%s"%(sys.getdefaultencoding()))
#print("locale.getpreferredencoding()=%s"%(locale.getpreferredencoding()))

g_default_encoding = locale.getpreferredencoding()
locale.setlocale(locale.LC_ALL,"") # to enable locale.strcoll

"""
Memo: 
 * celt: csv cell text
 * celx: csv cell's eXtra info (#line and file-offset)
"""

def cmp_to_key_ex(user_mapping, cmp_internal): # an extension to cmp_to_ex()
	
	def make_cmp_ex(user_mapping, cmp_internal):
		def cmp_ex(cmpoA, cmpoB):
			# cmp_ex 这个函数对象是传给 cmp_to_key() 用的，不是传给 sorted() 的。
			# cmp_ex 返回比较结果 -1, 0, -1 三者之一。
			# cmp_ex 要拿到一个 mapping 函数, 将 cmpo 映射为真正的 cmpoX。
			# cmpoX 才是真正要交给实质比较函数 strcoll 的对象。
			cmpoAX = user_mapping(cmpoA)
			cmpoBX = user_mapping(cmpoB)
			return cmp_internal(cmpoAX, cmpoBX) # e.g. cmp_internal=strcoll
		return cmp_ex
	
	cmp_func = make_cmp_ex(user_mapping, cmp_internal)
	return functools.cmp_to_key(cmp_func)


LineExtra = namedtuple('LineExtra', 'idxline ofs')

class VerboseLevel(IntEnum):
	vb0 = 0
	vb1 = 1
	vb2 = 2
	vb3 = 3

def enum_csv_rows_by_filehandle(fh, is_yield_offset=False, text_encoding=''):
	
	idx_line = 0
	while True:
		
		if is_yield_offset:
			
			assert('b' in fh.mode)
			
			filepos = fh.tell()
			
			_octets_ = fh.readline() # binary mode readline()
			if _octets_==b'':
				return # meet end of file
			
			octets = _octets_.strip()
			if octets==b'':
				idx_line += 1
				continue # meet an empty line, skip it
			
			# todo: strip leading \r and fix filepos
			
			linetext = codecs.decode(octets, text_encoding if text_encoding else g_default_encoding)
			
		else:
			
			assert(not 'b' in fh.mode)
			
			filepos = -1
			linetext = fh.readline()
			
			if linetext=='':
				return # meet end of file
			
		csv_row = next( csv.reader([linetext]) )
		# -- csv_row is a list of celt.
		
		yield idx_line, filepos, csv_row
		idx_line += 1


def find_dups(csv_filename, fields_to_chk, 
	is_single_as_group=False,
	is_force_binary_fh=False, text_encoding='',
	vb=0, vblines_list_max=10, sort_rules=[]):

	assert(type(fields_to_chk)==list and (len(fields_to_chk)==0 or type(fields_to_chk[0])==int))

	if vb>=VerboseLevel.vb3:
		is_force_binary_fh = True

	if is_force_binary_fh:
		fh = open(csv_filename, 'rb')
	else:
		fh = open(csv_filename, 'r', encoding=text_encoding)


	fh.seek(0)
	csv_spliter = enum_csv_rows_by_filehandle(fh, is_force_binary_fh, text_encoding)
	
	# read the first line(line #0), take it as csv header text.
	_, _, ar_headers = next(csv_spliter)
	count_fields = len(ar_headers)

	if len(fields_to_chk)==0:
		fields_to_chk = list(range(0, len(ar_headers)))

	# find longest header chars
	max_header_width = max([len(ar_headers[fidx]) for fidx in fields_to_chk])
	max_header_prnwidth = min(max_header_width, 40)

	ar_field_stats = [ { header_name : [0] } for header_name in ar_headers ]
		# [0] above means, the header text appears in line #0
	
	for idx_line, ofs, row in csv_spliter:

		if len(row)<count_fields:
			if len(row)==0:
				continue # meet empty line, skip it
			else:
				errmsg = 'Meet abnormal CSV line(#%d). Field count %d, required %d.'%(
					idx_line+1, len(row), count_fields)
				raise ValueError(errmsg)

		for i in fields_to_chk:

			celt = row[i]
			if celt=="":
				continue
			dict_stat = ar_field_stats[i]
			if celt in dict_stat:
				dict_stat[celt].append(LineExtra(idx_line, ofs))
			else:
				dict_stat[celt] = [LineExtra(idx_line, ofs)]

	ar_dupcount = [ {} for header_name in ar_headers ]
	#
	for idx_field, dict_stat in enumerate(ar_field_stats):
		#>>>
		#[[UNSTABLE1]]
		#Chj memo: Each run of csv-find-dup.py with the SAME input csv, 
		#we get different list order from dict_stat.items(), why?
		#print(">>> %s"%(dict_stat.items()))
		#<<<
		for key, celt2celx in dict_stat.items():
			dupcount = len(celt2celx)
			if dupcount > (0 if is_single_as_group else 1) :
				ar_dupcount[idx_field][key] = dupcount

	for idx_field in fields_to_chk:
		
		dict_dupcount = ar_dupcount[idx_field]

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

				dup_items = dict_dupcount.items()
				# -- dup_item[0] is celt string, dup_item[1] is dupcount
				
				# Apply sort rules here.
				# Sort from small-scale to large scale, so that user sees large-scale to small-scale.
				
				for irule_ in range(len(sort_rules), 0, -1):
					irule = irule_ - 1
				
					if sort_rules[irule]=='duptext':
						
						dup_items = sorted(dup_items, key=cmp_to_key_ex(lambda x: x[0], locale.strcoll))
					
					elif sort_rules[irule]=='dupcount':

						dup_items = sorted(dup_items, key=lambda x: x[1])

				for dup_text, dup_count in dup_items:
					print("  (%d*) %s"%(dup_count, dup_text))

					if vb>=VerboseLevel.vb2:
						print_vb23_detail(vb, 
							ar_field_stats[idx_field][dup_text], 
							text_encoding,
							vblines_list_max,
							fh)
	return 0


def print_vb23_detail(vb, ar_linex, text_encoding, list_max, fh):
	
	# ar_linex is LineExtra[] array
	
	total = len(ar_linex)
	list_count = min(total, list_max)
	is_elps = total>list_max 
	
	s = ",".join(["%d"%(x.idxline+1) for x in ar_linex[0:list_count]])
	elps = "..." if is_elps else ""
	
	print("    Appears at #lines: %s %s"%(s, elps))

	if vb>=VerboseLevel.vb3:
		
		assert('b' in fh.mode)
		done = 0

		for linex in ar_linex:
			fh.seek(linex.ofs)
			octets = fh.readline()
			linetext = codecs.decode(octets, text_encoding)
			print("    [#%d] %s"%(linex.idxline+1, linetext.rstrip()))

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
			' *  -vvv : list line content as well (a bit time consuming).\n'
	)

	ap.add_argument('-e', '--encoding', type=str, default='',
		help='Assign text encoding of the input csv file. If omit, system default will be used.\n'
			'Typical encodings: utf8, gbk, big5, utf16le.'
	)

	ap.add_argument('--sort', choices=['dupcount', 'duptext'], action='append', default=[],
		help='When listing duplicate items, sort by duplicate-count and/or duplicate-text.\n'
			'This can be assigned multiple times, from larg-scale to small-scale.\n'
			'If omit, use natural order in CSV.'
	)

	ap.add_argument('-x', '--max-verbose-lines', type=int, dest='max_verbose_lines', default=10,
		help='When -vv, showing duplicate lines is limited to this number.'
	)
	
	ap.add_argument('-t', '--timing', action='store_true',
		help='Print timing result to stderr.\n'
			'Hint: To eliminate the time cost of printing bluk text to screen,\n'
			'it is suggested to redirect print output to a file or to null device.'
	)
	
	ap.add_argument('-1', '--single-as-group', action='store_true', dest='single_as_group',
		help='This is counter-literal. Consider each distinct field text as "duplicate",\n'
			'so that we are not counting duplicates, but counting distincts.'
	)
	
	ap.add_argument('--binary', action='store_true',
		help='[Developer] Force binary-mode file reading.'
	)

	if len(sys.argv)==1:
		# If no command-line parameter given, display help and quit.
		print("csv-find-dup.py version 1.2 .")
		print("You must provide a csv filename to process.")
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

	csv_filename = args.csv_filename
	fields_to_chk = args.field
	text_encoding = g_default_encoding if not args.encoding else args.encoding
	vb = VerboseLevel(args.verbose)

	if fields_to_chk==None:
		fields_to_chk = []
		
	if args.single_as_group:
		print('NOTE: Operating in "single-as-group" mode.')

	tstart = datetime.datetime.now()
	
	ret = find_dups(csv_filename, fields_to_chk, 
		args.single_as_group,
		is_force_binary_fh = args.binary,
		text_encoding = text_encoding,
		vb = vb, 
		vblines_list_max = args.max_verbose_lines, 
		sort_rules = args.sort)

	tend = datetime.datetime.now()

	if vb==VerboseLevel.vb0 and fields_to_chk==None:
		print("")
		print("Hint:")
		print(" * add option -v, -vv or -vvv to show more duplicate details.")
		print(" * add option -f 0 -f 2 etc to focus on fields of interest.")

	if args.timing:
		tcost = tend - tstart
		sys.stderr.write("Time cost: {} seconds\n".format(tcost.total_seconds()))


if __name__=="__main__":
	ret = main()
	exit(ret)
