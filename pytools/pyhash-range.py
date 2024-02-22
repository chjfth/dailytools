#!/usr/bin/env python

import os
import sys
import hashlib

# Thanks to: http://stackoverflow.com/a/17782753/151453
def hash_for_file(method, path, block_size=256*128, offset=0, cal_length=0, hr=False):
	"""
	Block size directly depends on the block size of your filesystem
	to avoid performances issues
	Here I have blocks of 4096 octets (Default NTFS)
	cal_length==0 means calculating to the end of file.
	"""
	try:
		hashobj = eval("hashlib."+method+"()")
	except AttributeError:
		sys.stderr.write('Error: "%s" is not a valid hash method.\n'%(method))
		exit(2)
	
	done_length = 0
	with open(path,'rb') as f: 
		f.seek(offset)
		
		if cal_length==0:
			read_len = block_size
		else:
			read_len = min(block_size, cal_length-done_length)
			
		for chunk in iter(lambda: f.read(read_len), b''): 
			hashobj.update(chunk)
			done_length += len(chunk)
			# print('done_length =', done_length) # debug
			if cal_length>0 and done_length>=cal_length:
				break
	
	if offset>0 or cal_length>0:
		print('Range: [%d - %d) %d bytes'%(offset, offset+done_length, done_length))
	
	if hr:
		return hashobj.hexdigest()
	else:
		return hashobj.digest()


usage="""Usage:

  pyhash md5 <file> [offset] [bytes]

  pyhash sha1 <file> [offset] [bytes]

  pyhash sha256 <file> [offset] [bytes]
"""

if __name__=='__main__':
	if len(sys.argv)<3:
		sys.stderr.write(usage)
		exit(1)
	
	method = sys.argv[1]
	path = sys.argv[2]
	filename = os.path.split(path)[1]
	
	offset = int(sys.argv[3]) if len(sys.argv)>3 else 0
	cal_length = int(sys.argv[4]) if len(sys.argv)>4 else 0
	
	hash = hash_for_file(method, path, 1024*1024, offset, cal_length, True)

	print("%s(%s)=%s"%(method, filename, hash))

	exit(0)
