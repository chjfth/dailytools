#!/usr/bin/env python3
#coding: utf-8

# Need python 3.8.
# shutil.copytree()'s `dirs_exist_ok` parameter requires Python 3.8

import os, sys, shutil
import time
from datetime import datetime, timedelta
import getopt
import configparser
from fnmatch import fnmatch

g_version = '20240401.1'

KEYNAME_SRCDIR = 'srcdir'
KEYNAME_DSTDIR = 'dstdir'
KEYNAME_INCLUDE_PTNS = 'include'
KEYNAME_PRESERVE_DAYS = 'preserve_days'

INDENT2 = "  "

class ErrMsg(Exception):
	def __init__(self, msg):
		self.msg = msg

g_exename = os.path.split(__file__)[1]
g_verbose = 0

# ign_callback = shutil.ignore_patterns('*.bak', '.svn', '.git')

DIRNAM_FMT_ONEDAY = '%Y%m%d.dailycopy'

def get_dirnam_today():
	now = time.localtime()
	dirnam = time.strftime(DIRNAM_FMT_ONEDAY, now)
	return dirnam

def is_fnmatch_ptns(path, ptns:[]):
	for ptn in ptns:
		if fnmatch(path, ptn):
			return True
	return False

def delete_outdated_dirs(dstdir_base, preserve_days):
	assert(preserve_days>0)
	dt_now = datetime.now()

	entries = os.listdir(dstdir_base)
	for entry in entries:
		dstpath = os.path.join(dstdir_base, entry)

		if not os.path.isdir(dstpath):
			continue

		try:
			dt_past = datetime.strptime(entry, '%Y%m%d.dailycopy')
		except ValueError:
			continue # not the desired dir to delete

		past_seconds = (dt_now - dt_past).total_seconds()
		if past_seconds >= preserve_days*3600*24:
			print(f"{INDENT2}[Delete]  {dstpath} ({preserve_days}+ days)")

			shutil.rmtree(dstpath)



def make_ignore(srcroot:str, dstroot:str, include_ptns:[str]):

	def ignore_existed_by_time(cur_srcdir, entries):
		assert(cur_srcdir.startswith(srcroot))
		inset_dir = cur_srcdir[len(srcroot)+1:] # +1 is for "\"
		cur_dstdir = os.path.join(dstroot, inset_dir)

		ignores = []
		for entry in entries:
			# entry can be a file or a dir.

			srcpath = os.path.join(cur_srcdir, entry)
			dstpath = os.path.join(cur_dstdir, entry)

			# Check 'include=' fnmatch
			inset_path = os.path.join(inset_dir, entry)
			if include_ptns and not is_fnmatch_ptns(inset_path, include_ptns):
				if g_verbose>=2:
					print(f"{INDENT2}[ignored] {dstpath}")
				ignores.append(entry)
				continue

			srctime = os.path.getmtime(srcpath)

			# Note: Do NOT check a directory's modification-time(that's not reliable).
			# If we see a dir, we always tell copytree() to recurse into it.
			if os.path.isfile(dstpath):
				dsttime = os.path.getmtime(dstpath)
			else:
				dsttime = 0

			if srctime==dsttime:
				if g_verbose>=1:
					print(f"{INDENT2}[existed] {dstpath}")
				ignores.append(entry)
			elif os.path.isfile(srcpath):
				print(f"{INDENT2}[copying] {dstpath}")

		return ignores

	return ignore_existed_by_time


def do_work(sys_argv):
	optlist, inilist = getopt.getopt(sys_argv[1:], 'vd:')
	optdict = dict(optlist)

	global g_verbose
	g_verbose = [opt[0] for opt in optlist].count('-v')

	dstroot = optdict.setdefault('-d', '')

	# Convert to abspaths
	inilist = [os.path.abspath(ini) for ini in inilist]

	print(f"Loading configs from:")
	for inifile in inilist:
		print(f"   {inifile}")

	iniobj = configparser.ConfigParser()

	iniobj.read(inilist)
	allsec = iniobj.sections()

	for secname in allsec:
		print("")
		print(f"[{secname}] Processing...")
		try:
			# 'srcdir=' and 'dstdir=' are required.

			srcdir = iniobj[secname][KEYNAME_SRCDIR]

			# TODO: When srcdir is relative-dir, make it relative to INI file.

			dstdir0 = iniobj[secname][KEYNAME_DSTDIR]
			dstdir_base = os.path.join(dstroot, dstdir0) # base does not contain YYYYMMDD
			dstdir_date = os.path.join(dstdir_base, get_dirnam_today())

			print(f"srcdir = {srcdir}")
			print(f"dstdir = {dstdir_date}")

		except KeyError as e:
			raise ErrMsg(f'In INI file, [{secname}] lacks keyname: {e.args[0]}')

		include = iniobj.get(secname, KEYNAME_INCLUDE_PTNS, fallback='') # '*.txt|*.doc' etc
		if include:
			include_ptns = include.split('|')
		else:
			include_ptns = []

		shutil.copytree(srcdir, dstdir_date,
		                ignore=make_ignore(srcdir, dstdir_date, include_ptns),
		                dirs_exist_ok=True)

		# Remove outdated copies from dstdir.
		preserve_days = iniobj.getint(secname, KEYNAME_PRESERVE_DAYS, fallback=30)
		if preserve_days<=0:
			raise ErrMsg(f'In INI file, [{secname}] has bad value {KEYNAME_PRESERVE_DAYS}={preserve_days} .')
		delete_outdated_dirs(dstdir_base, preserve_days)

	pass


def print_help():
	print(f"A program to copy files to everyday subdir. (version {g_version})")
	print(f"Usage:")
	print(f"    {g_exename} [options] <dailycopy-ini-file1> <dailycopy-ini-file2> ...")
	print(f"")
	print(f"Each section in the ini tells a pair of source-dir and dest-dir to do the copy.")
	print(f"")
	print(f"Examples:")
	print(f"")
	print(f"{g_exename} dailycopy.ini")
	print(f"")
	print(f"{g_exename} -d E:\\backups dailycopy.ini")
	print(f"    -d assigns the backup destination root dir. When a 'dstdir' in the INI is")
	print(f"    a relative dir, it will be relative to E:\\backups .")
	print(f"")
	print(f"{g_exename} -v dailycopy.ini")
	print(f"    -v prints those existed files in dest-dir, implying no copy action required.")
	print(f"")
	print(f"{g_exename} -v -v dailycopy.ini")
	print(f"    -v -v also prints those ignored files that do not match include patterns.")

if __name__=='__main__':

	if len(sys.argv)==1:
		print_help()
		exit(1)

	try:
		do_work(sys.argv)
		exit(0)
	except ErrMsg as e:
		print(f"[ERROR] {e.msg}")
		exit(4)

