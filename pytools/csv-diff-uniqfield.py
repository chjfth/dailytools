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

#print("sys.getdefaultencoding()=%s"%(sys.getdefaultencoding()))
#print("locale.getpreferredencoding()=%s"%(locale.getpreferredencoding()))

g_default_encoding = locale.getpreferredencoding()

"""
Memo: 
 * celt: csv cell text
 * celx: csv cell's eXtra info (#line and file-offset)
"""

class CsvFieldInfo:
	def __init__(self, exists, keys=None, compares=None):
		self.exists = exists
		self.keys = keys
		self.cmps = compares

class CsvWork:
	def __init__(self, csvfilename):
		self.fh = open(csvfilename, encoding=g_default_encoding)
		self.csvreader = csv.DictReader(self.fh)
		self.csvfieldinfo = CsvFieldInfo(self.csvreader.fieldnames)

	def close(self):
		if self.fh:
			self.fh.close()

class DiffWork:
	def __init__(self, csvworkA, csvworkB):
		self.csvworks = [csvworkA, csvworkB]
		self.csvfieldinfo = None # set later

	def close(self):
		self.csvworks[0].close()
		self.csvworks[1].close()

	def itemkey(self, csvrow):
		# csvrow is a dict-object representing a csv row
		# return the string that should be used as dict-key indexing a specific csv row
		return ",".join([ csvrow[field] for field in self.csvfieldinfo.keys ])


def my_parse_args():
	
	ap = argparse.ArgumentParser(
		formatter_class=argparse.RawTextHelpFormatter,
		description='Compare two CSV files by some specific key-field(s), '
			'so we can see which rows are added, which rows are deleted and which rows are modified. '
			'The criteria for row-modifying is identified by one or more '
			'other comparing-field(s) at your will.'
	)

	ap.add_argument('csvfileA', type=str, help='First csv filename to compare.')
	ap.add_argument('csvfileB', type=str, help='Second csv filename to compare.')

	ap.add_argument('-k', '--key-fields', type=str,
		help='Tell which fields are key-fields, separated by comma.'
	)

	ap.add_argument('-c', '--comparing-fields', type=str,
		help='Tell which fields are comparing-fields, separated by comma.'
	)

	ap.add_argument('-e', '--encoding', type=str, default='',
		help='Assign text encoding of the input csv file. If omit, system default will be used.\n'
			'Typical encodings: utf8, gbk, big5, utf16le.'
	)

	args = ap.parse_args()

	if args.encoding:
		g_default_encoding = args.encoding

	return args

def print_csv_header(fieldnames):
	print("CSV field names:")
	for i, f in enumerate(fieldnames):
		print("[%d] %s"%(i+1, f))

def check_csv_validity(args):

	csvworkA = CsvWork(args.csvfileA)
	csvworkB = CsvWork(args.csvfileB)

	dw = DiffWork(csvworkA, csvworkB)


	# Check two csv field names match
	if dw.readerA.fieldnames != dw.readerB.fieldnames:
		msg = "Two csv files do NOT have the same header fields, so I cannot compare them."
		raise SystemExit(msg)

	dw.existfields = dw.readerA.fieldnames

	# Check if key-field is assigned.

	if args.key_fields == None:
		print_csv_header(dw.existfields)
		raise SystemExit("No key-field(s) assigned. Please assign one or more from above(-k).")

	# Check if user-given field names are valid.

	dw.keyfields = args.key_fields.split(',')
	invalid_fields = set(dw.keyfields) - set(dw.existfields)
	if invalid_fields:
		raise SystemExit("Some key-fields are invalid: " + ",".join(invalid_fields))

	# If -c is not given, assume all fields for comparing
	dw.cmpfields = args.comparing_fields.split(',') if args.comparing_fields else dw.existfields

	# ensure that cmpfields does not contain key fields, which is meaningless
	dw.cmpfields = [f for f in dw.cmpfields if f not in dw.keyfields]

	invalid_fields = set(dw.cmpfields) - set(dw.existfields)
	if invalid_fields:
		raise SystemExit("Some comparing-fields are invalid: " + ",".join(invalid_fields))

	return dw

#def make_key_dict(csvreader, ):

def StartDiff(dw):

	dictA = {}
	dictB = {}

	for row in dw.readerA:



def main():
	args = my_parse_args()

	try:
		dw = check_csv_validity(args) # get DiffWork object
		StartDiff(dw)
		dw.close() # close csv file handle // todo: use with... ctx manager
	except SystemExit as e:
		print("ERROR EXIT: "+str(e.code))
		raise



	print("ok")

if __name__=="__main__":
	ret = main()
	exit(ret)
