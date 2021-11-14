#!/usr/bin/env python3
#coding: utf-8

import re, os, sys, traceback
import time
from collections import namedtuple

version = '1.1'

FILEOP_CHUNK_SIZE = 1024*1024
SECTOR_SIZE = 4096

BadRange = namedtuple('BadRange', 'start end_')

disksector_size = 4096
is_truncate = False


def MergeAdjacentRanges(ranges1, ranges2):
	
	if ranges1 and ranges2:
		if ranges1[-1].end_ == ranges2[0].start:
			return ranges1[:-1] + \
				[BadRange(ranges1[-1].start, ranges2[0].end_)] + \
				ranges2[1:]
		else:
			return ranges1 + ranges2
	elif ranges1:
		return ranges1
	else:
		return ranges2
	

def in_CopyFile_FindBadRanges(Lv, start, end_, 
	start0, end0_,
	srcfh, dstfh, 
	Lv0_chunk_size, Lv1_align_size):

	"""
	Lv: Recurse level, increase 1 on each recursion call. 
	
	Lv0: Initial user call. Lv0_split_size normally set to a big buffer-size, like 1024*1024.
		This is the app read/write buffer when no file-reading error occurs.
	
	Lv1: Split a big buffer into small sector-size after file-reading error occurs, 
		reading small sectors one by one in hope that we can rescue those survived bytes.
		Normally, this is set to disk sector-size, like 4096.
	
	Lv2: Processing a single sector.
	
	Return: a list of BadRange tuples, telling the caller which parts are bad/lost.
	"""

	if start==end_:
		return []

	nbytes = end_ - start
	assert(nbytes>0)

	if Lv==0:
		assert start==start0
		split_size = Lv0_chunk_size
	elif Lv>=1:
		split_size = Lv1_align_size

	if Lv>0:

		# For Lv1 & Lv2, try one-shot read/write of whole data from start to end_
		got_error = False
		try:
			srcfh.seek(start)
			data = srcfh.read(nbytes) # May encounter file reading error, and we will catch it.
		except OSError:
			# catch file-reading error.
			got_error = True
		
		if not got_error:
			dstfh.seek(start)
			dstfh.write(data) # would let write error(exception) propagate 
			return []

	# Since big-chunk source-reading has failed, now cycle for each split_size.
	# Each split generate a "sector" (just borrow the jargon for analogy).

	start_sector = start // split_size
	end_sector = (end_-1) // split_size
	
	if Lv>=1 and start_sector==end_sector:
		# this [start, end_) has been the smallest split, no more splitting to go.
		
		# fill zeros to dstfh at those "bad" location, don't leave garbage there.
		dstfh.seek(start)
		dstfh.write(b'\x00'*nbytes)
		
		return [BadRange(start, end_)]
	
	badranges = [] # todo: rename to Lv0_badranges
	
	if Lv==1:
		# need a new line to escape from Lv0's same-line progress state
		print("") 

		print("  Error@[%d - %d), will find out sector-by-sector."%(start, end_))
		
		time.sleep(0.5) # debug
	
	for idx_sector in range(start_sector, end_sector+1):
		
		substart = max(idx_sector*split_size, start)
		subend_ = min((idx_sector+1)*split_size, end_)
		
		#print("  >> sub [%d, %d) %d bytes"%(substart, subend_, subend_-substart)) # debug
		
		if Lv==0:
			print("Progress at %d (%.2f%%)"%(substart, (substart-start0)/(end0_-start0)*100), end='')
			
			if badranges:
				print("  (BAD+%d)"%(len(badranges)), end='')
			
			print("  \r", end='') # better have some spaces to overwrite same-line previous chars.
		
		brs = in_CopyFile_FindBadRanges(Lv+1, substart, subend_,
			start0, end0_, srcfh, dstfh, Lv0_chunk_size, Lv1_align_size)
		
		badranges = MergeAdjacentRanges(badranges, brs)
	
	assert(badranges)
	return badranges

def CopyFile_FindBadRanges(srcfh, dstfh, start, end_):
	
	badranges = in_CopyFile_FindBadRanges(0, start, end_, 
		start, end_,
		srcfh, dstfh, 
		FILEOP_CHUNK_SIZE, SECTOR_SIZE);
	
	print("") # cursor move to next line
	
	return badranges


def print_my_version():
	print("%s version %s"%(os.path.basename(__file__), version))
	

def do_main():
	if len(sys.argv)<3:
		print_my_version()
		print("    Copy a file, report and skip bad sectors, filling zeros on error-chunks.")
		print("    This program can rescue still-readable stuff from your rotting SSD disk.")
		print("Usage: ")
		print("    copybadfile.py <srcfile> <dstfile>")
		print("    copybadfile.py <srcfile> <dstfile> <start_offset> [end_offset_]")
		print("    copybadfile.py <srcfile> <dstfile> <start_offset> +<bytes_from_start>")
		print("    copybadfile.py <srcfile> <dstfile> <start_offset> -<less_bytes_from_srctail>")
		print("")
		print("Examples: ")
		print(r"copybadfile.py M:\vms\win7.vmdk R:\vms\win7.vmdk")
		print(r"    Copy whole source file to destination. Dst-file will be truncated.")
		print(r"")
		print(r"copybadfile.py M:\vms\win7.vmdk R:\vms\win7.vmdk 6021971968 6032261120")
		print(r"    Copy from offset 6021971968 to 6032261120. No truncation of dst-file.")
		print(r"")
		print(r"copybadfile.py M:\vms\win7.vmdk R:\vms\win7.vmdk 6021971968 +1000240")
		print(r"    Copy from offset 6021971968, copy only 1000240 bytes. No truncation.")
		print(r"")
		print(r"copybadfile.py M:\vms\win7.vmdk R:\vms\win7.vmdk 6021971968 -8192")
		print(r"    Copy from offset 6021971968, until 8192 less of source file. No truncation.")
		print(r"")
		
		exit(1)

	global is_truncate

	srcfile = sys.argv[1]
	dstfile = sys.argv[2]
	
	if len(sys.argv)>3:
		offset = eval( sys.argv[3] )
	else:
		offset = 0
		is_truncate = True
	
	if len(sys.argv)>4:
		param4s = sys.argv[4] 
	else:
		param4s = "-0"
	
	param4 = int(param4s)

	srcfilesize = os.path.getsize(srcfile)

	if param4s[0:1] == "+":
		totbytes = param4
	elif param4s[0:1] == "-":
		assert(param4<=0)
		totbytes = srcfilesize + param4 - offset
	else:
		totbytes = param4 - offset

	if totbytes<=0:
		print("Parameter error: Total bytes(%d) is not positive number."%(totbytes))
		sys.exit(1)

	if offset > srcfilesize:
		print("Parameter error: start-offset(%d) exceeds srcfile size(%d)."%(offset, srcfilesize))
		sys.exit(1)
	elif offset+totbytes > srcfilesize:
		print("Parameter error: end-offset_(%d) exceeds srcfile size(%d)."%(offset+totbytes, srcfilesize))
		sys.exit(1)

	print_my_version()
	print("From offset %d to %d, total %d bytes."%(offset, offset+totbytes, totbytes))

	srcfh = open(srcfile, "rb")

	if os.path.exists(dstfile):
		dstfh = open(dstfile, "rb+")
	else:
		dstfh = open(dstfile, "wb")


	badranges = CopyFile_FindBadRanges(srcfh, dstfh, offset, offset+totbytes)

	badrange_count = len(badranges)
	badbytes_total = 0
	
	if badrange_count>0:

		print("!!!!!!!!!!!!!!!!!!!!!!!!")

		for i, br in enumerate(badranges):
			
			badbytes_c = br.end_-br.start
			badbytes_total += badbytes_c
			
			print("[Bad#%d] offset: %d - %d (%d bytes)"%(i+1, br.start, br.end_, badbytes_c))

		print("Source file total BAD range count: %d !"%(badrange_count))

		print("!!!!!!!!!!!!!!!!!!!!!!!!")
		
		pctbad = badbytes_total/totbytes*100
		str_pctbad = "%.2f%%"%(pctbad) if pctbad>=0.01 else ">0.00%"
		
		print("BAD SECTORS DETECTED! For total %d bytes, %d bytes(4KB*%g) are NOT copied (%s)!"%(
			totbytes, badbytes_total, badbytes_total/4096, str_pctbad))
	else:
		print("Success. Total %d bytes copied."%(totbytes))
	
	if is_truncate:
		dstfh.truncate(offset+totbytes)
	
	srcfh.close()
	dstfh.close()
	
	return (0 if badbytes_total==0 else 4)

if __name__=='__main__':
	errcode = do_main()
	sys.exit(errcode)
