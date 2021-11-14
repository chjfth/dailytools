#!/usr/bin/env python3
#coding: utf-8

import re, os, sys, traceback
import time
from collections import namedtuple

version = '1.0'

BadRange = namedtuple('BadRange', 'start end_')

disksector_size = 4096
is_truncate = False

def sector_idx(byte_offset):
	return byte_offset // disksector_size

def sector_boundary(byte_offset):
	return byte_offset // disksector_size * disksector_size

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
	

def CopyFile_FindBadRanges(level, srcfh, dstfh, start, end_):

	if start==end_:
		return []

	nbytes = end_ - start
	assert(nbytes>0)

	got_error = False

	try:
		srcfh.seek(start)
		data = srcfh.read(nbytes) # May meet file reading error, and we will catch it.
	except OSError:
		got_error = True
	
	if not got_error:
		dstfh.seek(start)
		dstfh.write(data) # let write error(exception) propagate 
		return []

	# Due to source file reading error on big block,
	# now we'll break it down into two smaller chunks of disksector_size and find out 
	# which sector(s) cause read error.
	
	start_sector = sector_idx(start)
	end_sector = sector_idx(end_-1)
	
	if start_sector == end_sector:
		# range already in same sector, splitting helps nothing, so just fail it.
		
		# but fill zeros to dstfh at those "bad" location, don't leave garbage there.
		dstfh.seek(start)
		dstfh.write(b'\x00'*nbytes)
		
		return [BadRange(start, end_)]
	
	badranges_all = []
	
	if level==1:
		print("") # need a new line
	
	print("  [Lv%d] Error@[%d - %d), split into %d sectors."%(level, start, end_, end_sector+1-start_sector))
	
	for idx_sector in range(start_sector, end_sector+1):
		
		substart = max(idx_sector*disksector_size, start)
		subend_ = min((idx_sector+1)*disksector_size, end_)
		
		#print("    Retrying offset [%d , %d]..."%(substart, subend_))
		
		badranges = CopyFile_FindBadRanges(level+1, srcfh, dstfh, substart, subend_)
		
		badranges_all = MergeAdjacentRanges(badranges_all, badranges)
	
	assert(badranges_all)
	return badranges_all


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

	srcfh.seek(offset)
	dstfh.seek(offset)

	chunksize = 1024*1024
	donebytes = 0

	badranges = []

	while donebytes<totbytes:

		cur_offset = offset+donebytes
		print("Progress at %d (%.2f%%)"%(cur_offset, donebytes/totbytes*100), end='')
		
		badrange_count = len(badranges)
		
		if badrange_count>0 :
			print( " (BAD+%d)"%(badrange_count), end='')
		
		print("        \r", end='') 
		# -- cursor move to start to line and we need some spaces to overwrite previous chars.
		
		nbytes = min(chunksize, totbytes-donebytes)
		
		brs = CopyFile_FindBadRanges(1, srcfh, dstfh, cur_offset, cur_offset+nbytes)
		badranges = MergeAdjacentRanges(badranges, brs)
		
		donebytes += nbytes

	print("") # move to next line

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
