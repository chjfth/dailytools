#!/usr/bin/env python3
#coding: utf-8

import os, sys
import sqlite3
import uuid

hexdm = lambda bytes :  ''.join('{:02X}'.format(x) for x in bytes)

def gen_uuid(dbfilename):
	# 连接到 SQLite 数据库
	conn = sqlite3.connect(dbfilename)
	cursor = conn.cursor()

	# 查找所有 guid IS NULL 的记录
	cursor.execute("SELECT uid FROM items WHERE guid IS NULL and type=1")

	# 遍历结果并更新每条记录的 guid 字段
	for i, row in enumerate(cursor.fetchall()):
		uid = row[0]
		guid = uuid.uuid4()  # 生成一个新的随机 GUID
		
		print("[#%d] uid=%d => creating guid: {%s}"%(i+1, uid, guid))
		
		sql_update = "UPDATE items SET guid=X'%s' WHERE uid=%d"%(
			hexdm(guid.bytes_le), uid)
		
		cursor.execute(sql_update)

	# 提交更改并关闭连接
	conn.commit()
	conn.close()

if __name__=='__main__':
	if len(sys.argv)==1:
		print("Need an Evernote .exb file as input.")
		exit(1)
	
	dbfilename = sys.argv[1]
	if not os.path.isfile(dbfilename):
		print("Cannot find disk file:")
		print("    " + dbfilename)
		exit(1)
	
	gen_uuid(dbfilename)
