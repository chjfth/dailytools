#!/usr/bin/env python3

"""
Copy a source directory tree with gitrepos to zipfiles in a target directory.
Each gitrepo results in a zip file with only .git folder in it.

Usage:
	dotgit_sync.py <srcdir> <dstdir>

Usage example:
	dotgit_sync.py g:\gitw k:\gitw
	
It will scan all sub-folders in <srcdir> for ones that have a .git subdir. For each one found,
it copies that folder with only its .git subdir to <dstdir>. By doing so, we copy the whole
git repository, and a `git revert` operation should recover the git-versioned files from .git 
on dst machine.

The copying operation is actually a zip operation.

Benefit of generating zip file in dst machine than raw .git folder copy:
TortoiseGit on dst machine will not display the harsh red ticks, You know, leaving only the .git
folder will render the repo working folder as "modified", so TortoiseGit alerts us with a red tick
on the .git's containing folder.

Of course, subdir tree structure will remain the same inside <srcdir> and <dstdir>.
"""

import sys
import os
import distutils.dir_util
from zipfile import ZipFile
from os.path import join

version = "1.0"

# Snippet from system lib zipfile.py
def addToZip(zf, path, zippath):
	if os.path.isfile(path):
		zf.write(path, zippath) #, ZIP_DEFLATED)
	elif os.path.isdir(path):
		if zippath:
			zf.write(path, zippath)
		for nm in os.listdir(path):
			addToZip(zf,
					 os.path.join(path, nm), os.path.join(zippath, nm))
	# else: ignore
#
def ZipADir(osdirpath, zipfilepath):
	with ZipFile(zipfilepath, 'w') as zf:
		path = osdirpath
		zippath = os.path.basename(path) # zippath: path within zipfile
		if not zippath:
			# This is due to `path` ends with os.sep char. So we ignore the trailing slash and still use the last path component.
			zippath = os.path.basename(os.path.dirname(path))
		if zippath in ('', os.curdir, os.pardir):
			zippath = ''
		addToZip(zf, path, zippath)

def is_gitrepo(dir, subdirs):

	if not subdirs:
		subdirs = os.listdir(dir)
	
	if '.git' in subdirs or '.svn' in subdirs:
		return True
	else:
		return False

def do_work(srcroot, dstroot):

	# Example: 
	# srcroot = G:\gitw
	# dstroot = K:\gitw
	#
	# There is  G:\gitw\AmHotkey\.git
	# We'll get K:\gitw\AmHotkey.zip [There will be a single .git folder inside this git.]
	
	len_srcroot = len(srcroot)
	for dirpath, dirnames, filenames in os.walk(srcroot):
#		print("Scanning " + dirpath) # `````````
		
		if not is_gitrepo(dirpath, dirnames):
			continue
		
		assert(dirpath.startswith(srcroot))
		dst__dirv = dstroot + dirpath[len_srcroot:] # v: implies virtual
		dst__zipfile = dst__dirv+'.zip'
		dst__zipdir =  os.path.dirname(dst__zipfile)
		
		print("Syncing %s => %s"%(dirpath, dst__zipfile))
		# Example:
		# Syncing G:\gitw\AmHotkey => K:\gitw\AmHotkey.zip

		if not os.path.exists(dst__zipdir):
			os.makedirs(dst__zipdir)
		
#		distutils.dir_util.copy_tree(join(dirpath, '.git'), join(dst__dirv, '.git'))
		ZipADir(join(dirpath, '.git'), dst__zipfile)
		
		dirnames[:] = []
		
if __name__=='__main__':
	if len(sys.argv)!=3:
		print("""
Sync all folders with .git subdir from <srcdir> to <dstdir>.

Usage:
	%s <srcdir> <dstdir>
		"""%(os.path.basename(__file__))
		)
		exit(1)
	
	srcdir = sys.argv[1]
	dstdir = sys.argv[2]
	
	if not os.path.isdir(srcdir):
		print("Error: Not a directory: %s"%(srcdir))
		exit(2)
	
	# dstdir must be empty
	is_empty_dir = os.path.isdir(dstdir) and os.listdir(dstdir)==[]
	not_exist_yet = not os.path.exists(dstdir)

	if not (is_empty_dir or not_exist_yet):
		print("Error: Dstdir already exists and not empty: %s"%(dstdir))
		exit(3)
	
	dstdir = os.path.abspath(dstdir)
	if not_exist_yet:
		try:
			os.makedirs(dstdir)
		except OSError:
			pass
		
		if not os.path.isdir(dstdir):
			print("Error: Cannot create directory: %s"%(dstdir))
			exit(4)
	
	do_work(srcdir, dstdir)
