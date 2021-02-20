#!/usr/bin/env python3
#coding: utf-8

import os, sys, locale
import argparse
import csv
import codecs
import datetime
from enum import Enum,IntEnum # since Python 3.4
from collections import namedtuple, OrderedDict

#print("sys.getdefaultencoding()=%s"%(sys.getdefaultencoding()))
#print("locale.getpreferredencoding()=%s"%(locale.getpreferredencoding()))

g_default_encoding = locale.getpreferredencoding()

"""
Memo: 
 * celt: csv cell text
 * celx: csv cell's eXtra info (#line and file-offset)
"""

class CsvRow:
	def __init__(self, idxline, rowdict):
		self.idxline = idxline # zero based line number
		self.rowdict = rowdict # row content in dict type
		# Q: can I get rowtext as well with Python csv module?

	def __getitem__(self, key):
		return self.rowdict[key]

class CsvFieldInfo:
	def __init__(self, exists, keys=None, compares=None):
		self.exists = exists
		self.keys = keys
		self.cmps = compares

	def itemkey(self, csvrow):
		# csvrow is a dict-object representing a csv row.
		# return the string that will be used as dict-key indexing a specific csv row
		return ",".join([ csvrow[field] for field in self.keys ])

	def is_csvrow_equal(self, rowA, rowB):
		for cmpfield in self.cmps:
			if rowA[cmpfield] != rowB[cmpfield]:
				return False
		return True

class CsvWork:
	def __init__(self, csvfilename):
		self.csvfilename = csvfilename
		self.fh = open(csvfilename, encoding=g_default_encoding)
		self.csvreader = csv.DictReader(self.fh)
		self.csvfieldinfo = CsvFieldInfo(self.csvreader.fieldnames)
		self.rows = {} # each item in this dict is a CsvRow

	def __getitem__(self, key):
		return self.rows[key]

	@property
	def keyset(self):
		"""For the dict-object representing the whole csv file,
		return all the keys as a set.
		"""
		return set(self.rows.keys())

	def close(self):
		if self.fh:
			self.fh.close()

	def LoadDict(self):
		for idxline, row_now in enumerate(self.csvreader, start=1): # skip 1 due to CSV header line
			key = self.csvfieldinfo.itemkey(row_now)
			if key in self.rows.keys():
				keyvalues = "\n".join(["  [%s] %s"%(field, row_now[field]) for field in self.csvfieldinfo.keys ])
				oldline = "  #%d : %s"%(self.rows[key].idxline+1, ",".join(self.rows[key].rowdict.values()))
				newline = "  #%d : %s"%(idxline+1, ",".join(row_now.values()))
				raise SystemExit("Not allowed duplicate key(s) in %s:\n%s\n%s\n%s"%(
					self.csvfilename, keyvalues, oldline, newline))
			else:
				self.rows[key] = CsvRow(idxline, row_now)

class DiffWork:
	def __init__(self, args):
		# args is from my_parse_args()
		self.csvworkA = CsvWork(args.csvfileA)
		self.csvworkB = CsvWork(args.csvfileB)
		self.csvfieldinfo = None # set later

		# Check two csv field names match
		if self.csvworkA.csvfieldinfo.exists != self.csvworkB.csvfieldinfo.exists:
			msg = "Two csv files do NOT have the same header fields, so I cannot compare them."
			raise SystemExit(msg)

		existfields = self.csvworkA.csvfieldinfo.exists

		# Check if key-field is assigned.

		if args.key_fields == None:
			print_csv_header(existfields)
			raise SystemExit("No key-field(s) assigned. Please assign one or more from above(-k).")

		# Check if user-given field names are valid.

		keyfields = args.key_fields.split(',')
		invalid_fields = set(keyfields) - set(existfields)
		if invalid_fields:
			raise SystemExit("Some key-fields are invalid: " + ",".join(invalid_fields))

		# If -c is not given, assume all fields for comparing
		cmpfields = args.comparing_fields.split(',') if args.comparing_fields else existfields

		# ensure that cmpfields does not contain key fields, which is meaningless
		cmpfields = [f for f in cmpfields if f not in keyfields]

		invalid_fields = set(cmpfields) - set(existfields)
		if invalid_fields:
			raise SystemExit("Some comparing-fields are invalid: " + ",".join(invalid_fields))

		# OK. csv fields validated.
		#
		self.csvfieldinfo = CsvFieldInfo(self.csvworkA.csvfieldinfo.exists, keyfields, cmpfields)
		self.csvworkA.csvfieldinfo = self.csvworkB.csvfieldinfo = self.csvfieldinfo

	def close(self):
		self.csvworkA.close()
		self.csvworkB.close()

	def StartDiff(self):
		self.csvworkA.LoadDict()
		self.csvworkB.LoadDict()

		added_keys = self.csvworkB.keyset - self.csvworkA.keyset
#		print(added_keys)

		removed_keys = self.csvworkA.keyset - self.csvworkB.keyset
#		print(removed_keys)

		common_keys = self.csvworkA.keyset & self.csvworkB.keyset
#		print(common_keys)

		# Check which common-keys has changed content
		unchanged_keylist = []
		changed_keylist = []
		for key in common_keys:
			if self.csvfieldinfo.is_csvrow_equal(self.csvworkA[key], self.csvworkB[key]):
				unchanged_keylist.append(key)
			else:
				changed_keylist.append(key)

		print("Diff summary:")
		print("  csvfileA %d items => csvfileB %d items"%(
			len(self.csvworkA.keyset), len(self.csvworkB.keyset)))

		print("  Unchanged items: %d"%(len(unchanged_keylist)))
		print("    Changed items: %d"%(len(changed_keylist)))

		print("      Added items: %d"%(len(added_keys)))

		print("    Removed items: %d"%(len(removed_keys)))


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
		help='Assign text encoding of the input csv file. If omit, system default will be used. '
			'Typical encodings: utf8, gbk, big5, utf16le.'
	)

	args = ap.parse_args()

	if args.encoding:
		global g_default_encoding
		g_default_encoding = args.encoding

	return args

def print_csv_header(fieldnames):
	print("CSV field names:")
	for i, f in enumerate(fieldnames):
		print("[%d] %s"%(i+1, f))

#def make_key_dict(csvreader, ):

def main():
	args = my_parse_args()

	try:
		dw = DiffWork(args) # create DiffWork object
		dw.StartDiff()
		dw.close() # close csv file handle // todo: use with... ctx manager
	except SystemExit as e:
		print("ERROR EXIT: "+str(e.code))
		exit(1) # raise // `raise` will print the error message again

	print("ok")

if __name__=="__main__":
	ret = main()
	exit(ret)
