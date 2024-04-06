#!/usr/bin/env python3
#coding: utf-8

import os, sys
import sqlite3
import uuid
from enum import Enum

version = '1.0'

hexdm = lambda bytes :  ''.join('{:02X}'.format(x) for x in bytes)

DoWhat = Enum('DoWhat', 'CheckOnly SilentModify Interactive')

def db_fetchone(conn, sql):
	cursor = conn.cursor()
	cursor.execute(sql)
	result = cursor.fetchone()
	return result

def gen_uuid(dbfilename, is_gen_now=False):
	# 连接到 SQLite 数据库
	conn = sqlite3.connect(dbfilename)
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

if __name__=='__main__':
	if len(sys.argv)==1:
		print("%s version %s"%(os.path.basename(__file__), version))
		print("Need an Evernote .exb file as first param.")
		print("  Check whether some evclip entry has missing GUID.")
		print("Optional second param:")
		print("  -s : Silent mode, Add missing GUID(s) silently.")
		print("  -i : Interactive mode, check first then ask for confirmation.")
		sys.exit(1)
	
	dbfilename = sys.argv[1]
	argv2 = sys.argv[2] if len(sys.argv)>2 else ''
	
	if argv2=='-s':
		dw = DoWhat.SilentModify
	elif argv2=='-i':
		dw = DoWhat.Interactive
	else:
		dw = DoWhat.CheckOnly
	
	if not os.path.isfile(dbfilename):
		print("[ERROR] Cannot find disk file:")
		print("    " + dbfilename)
		sys.exit(1)
	
	do_gen = False
	
	if dw in [DoWhat.CheckOnly, DoWhat.Interactive]:
		count = gen_uuid(dbfilename, False)
		if count==0:
			print("Good. No evclip has missing GUID.")
		elif count>0:
			print("")
			if dw==DoWhat.CheckOnly:
				print("Found %d evclips with empty GUID. (run with -s to fill)"%(count))
			else:
				ans = input("Generate GUID for above %d evclips? [y/n] "%(count))
				if ans in ['y', 'Y']:
					do_gen = True
	else:
		do_gen = True

	if do_gen:
		print("")
		count = gen_uuid(dbfilename, True)
		if count==0:
			print("No evclip has missing GUID. Nothing to do.")
		else:
			print("")
			print("Success. %d GUIDs Generated."%(count))
	
	sys.exit(0)
