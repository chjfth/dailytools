#!/usr/bin/env python3
#coding: utf-8

import os, sys
import getopt
from fnmatch import fnmatch
import sqlite3
import uuid
from enum import Enum

import msvcrt

version = '1.1'

hexdm = lambda bytes :  ''.join('{:02X}'.format(x) for x in bytes)

DoWhat = Enum('DoWhat', 'CheckOnly SilentModify Interactive')

def db_fetchone(conn, sql):
	cursor = conn.cursor()
	cursor.execute(sql)
	result = cursor.fetchone()
	return result

def gen_uuid(dbfilepath, is_gen_now=False):
	# 连接到 SQLite 数据库
	conn = sqlite3.connect(dbfilepath)
	cursor = conn.cursor()

	# 查找所有 guid IS NULL 的记录
	cursor.execute("SELECT uid FROM items WHERE (guid IS NULL OR guid='') and type=1")

	count = 0
	# 遍历结果并更新每条记录的 guid 字段
	for i, row in enumerate(cursor.fetchall()):
		uid = row[0]
		guid = uuid.uuid4()  # 生成一个新的随机 GUID
		
		title = db_fetchone(conn, "SELECT title FROM note_attr where uid=%d"%(uid))[0]

		if not is_gen_now:
			print("[#%d] uid=%d (empty guid)"%(i+1, uid))
			print("    " + title)
		else:
			print("[#%d] uid=%d => creating guid: {%s}"%(i+1, uid, guid))
			print("    " + title)
		
			sql_update = "UPDATE items SET guid=X'%s' WHERE uid=%d"%(
				hexdm(guid.bytes_le), uid)
		
			cursor.execute(sql_update)
		
		count+=1
	
	# 提交更改并关闭连接
	conn.commit()
	conn.close()
	return count

def print_help():
	print("%s version %s"%(os.path.basename(__file__), version))
	print("Need an Evernote .exb file as first param.")
	print("  Check whether some evclip entry has missing GUID.")
	print("Optional second param:")
	print("  -s : Silent mode. Fill missing GUID(s) silently(will change .exb)")
	print("  -i : Interactive mode. Check first then ask for confirmation.")
	print("")
	print(
		"Note: If you run this program without parameter, it tries to find a single .exb file "
		"in the same directory as the program. If found, it will operate on that .exb with '-i'."
		)

def get_script_dir():
	# 检查 PyInstaller 打包的标志
	if getattr(sys, 'frozen', False):
	    # 如果是 PyInstaller 打包的，则执行下面的代码
	    mydir = os.path.dirname(sys.executable)
	else:
	    # 如果不是 PyInstaller 打包的，则执行下面的代码
	    mydir = os.path.dirname(__file__)

	abspath = os.path.abspath(mydir if mydir else '.')
	return abspath

def find_exbfile():
	# Find the only .exb side-by-side with this script.
	mydir = get_script_dir()
	exb_list = [f for f in os.listdir(mydir) if fnmatch(f, '*.exb')]
	exb_count = len(exb_list)
	if exb_count==0:
		print('[ERROR] No .exb file found in "%s"'%(mydir))
		
		if len(sys.argv)==1:
			print("")
			print_help()
		
		sys.exit(4)
	elif exb_count==1:
		return os.path.join(mydir, exb_list[0])
	else:
		print('[ERROR] More than one .exb file found. You need to specify one exactly as parameter.')
		for f in exb_list:
			print('    %s'%(os.path.join(mydir, f)))
		sys.exit(4)

def do_work():

	optlist, arglist = getopt.getopt(sys.argv[1:], 'si', ['help'])
	optdict = dict(optlist)
	
	if '--help' in optdict:
		print_help()
		sys.exit(0)
	
	if '-s' in optdict:
		dw = DoWhat.SilentModify
	elif '-i' in optdict:
		dw = DoWhat.Interactive
	else:
		dw = DoWhat.CheckOnly
	
	if arglist:
		dbfilepath = arglist[0]
	else:
		dbfilepath = find_exbfile()
		dw = DoWhat.Interactive
	
	assert(dbfilepath!='')
	
	if not os.path.isfile(dbfilepath):
		print("[ERROR] Cannot find disk file:")
		print("    " + dbfilepath)
		sys.exit(1)
	
	do_gen = False
	
	if dw in [DoWhat.CheckOnly, DoWhat.Interactive]:
		count = gen_uuid(dbfilepath, False)
		if count==0:
			print("Good. No evclip has missing GUID.")
		elif count>0:
			print("")
			if dw==DoWhat.CheckOnly:
				print("Found %d evclips with empty GUID. (run with -s to fill)"%(count))
			else:
				print("Generate GUID for above %d evclips? [y/n] "%(count), end='', flush=True)
				ans = msvcrt.getch()
				print("")
				if ans in [b'y', b'Y']:
					do_gen = True
	else:
		do_gen = True

	if do_gen:
		print("")
		count = gen_uuid(dbfilepath, True)
		if count==0:
			print("No evclip has missing GUID. Nothing to do.")
		else:
			print("")
			print("Success. %d GUIDs Generated."%(count))
	
	if dw==DoWhat.Interactive:
		print("")
		print("<Press a key to quit.>")
		msvcrt.getch()
		
	
	sys.exit(0)

if __name__=='__main__':
	do_work()
