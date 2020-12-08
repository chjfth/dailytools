#!/usr/bin/env python3
#coding: utf-8

import os, sys
import csv

def count_dups(fh):
	fh.seek(0)
	reader = csv.reader(fh)
	ireader = iter(reader)
	
	ar_headers = next(ireader)
	count_fields = len(ar_headers)

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
		
		dup_groups = len(dict_dupcount)
		if dup_groups==0:
			print("Field %d (%s) has no duplicate items."%(idx_field, ar_headers[idx_field]))
		else:
			print("Field %d (%s) has %d group of duplicate items:"%(
				idx_field, ar_headers[idx_field], dup_groups))
			for dup_text, dup_count in dict_dupcount.items():
				print("  (%d*) %s"%(dup_count, dup_text))

	return 0
	


if __name__=="__main__":

	if len(sys.argv)==1:
		print("Need a csv filename as parameter.")
		exit(1)

	csvfile = sys.argv[1]
	fh = open(csvfile, "r")
	ret = count_dups(fh)
	exit(ret)

