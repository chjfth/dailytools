#!/usr/bin/env python

import os
import sys
import hashlib

# Thanks to: http://stackoverflow.com/a/17782753/151453
def hash_for_file(method, path, block_size=256*128, hr=False):
	"""
	Block size directly depends on the block size of your filesystem
	to avoid performances issues
	Here I have blocks of 4096 octets (Default NTFS)
	"""
	try:
		hashobj = eval("hashlib."+method+"()")
	except AttributeError:
		sys.stderr.write('Error: "%s" is not a valid hash method.\n'%(method))
		exit(2)
		
	with open(path,'rb') as f: 
		for chunk in iter(lambda: f.read(block_size), b''): 
			hashobj.update(chunk)
	if hr:
		return hashobj.hexdigest()
	else:
		return hashobj.digest()


usage="""Usage:

  pyhash md5 <file>

  pyhash sha1 <file>

  pyhash sha256 <file>
"""

if __name__=='__main__':
	if len(sys.argv)<3:
		sys.stderr.write(usage)
		exit(1)
	
	method = sys.argv[1]
	path = sys.argv[2]
	filename = os.path.split(path)[1]
	
	hash = hash_for_file(method, path, hr=True)

	print("%s(%s)=%s"%(method, filename, hash))

	exit(0)
