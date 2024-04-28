#!/usr/bin/env python3
#coding: utf-8

"""
Evernote .exb is SQLite3 database with all TEXT fields set to use collation
"NOCASEUTF8", so, when using standard client(like sqlite3 CLI or Navicat)
 to INSERT or UPDATE some records, sqlite engine asserts error

	no such collation sequence: NOCASEUTF8

Then how to workaround this? Perplexity.ai suggests using a Python program
that can create/register a custom collation function for "NOCASEUTF8".

That works. This program asks for an SQL statement and execute it under
the context of "NOCASEUTF8" to workaround that very issue.
"""

import os, sys
import sqlite3

def my_collation(s1, s2):
	# Implement your custom collation logic here
	compare_ret = (s1.lower() > s2.lower()) - (s1.lower() < s2.lower())
	return compare_ret

def do_work(dbfilepath):

	count = 0
	conn = sqlite3.connect(dbfilepath)
	conn.create_collation('NOCASEUTF8', my_collation)

	# Now you can use the custom collation in your SQL queries
	cursor = conn.cursor()

	print('Enter SQL statement to execute. Type ".quit" to quit.')
	while True:
		try:
			sql = input("[%d]SQL> "%(count+1))

			if sql=='':
				continue
			elif sql=='.quit':
				break

			cursor.execute(sql)
			conn.commit()
			print("[OK]")

		except sqlite3.Error as e:
			print("[ERROR] %s"%(e))
		except EOFError:
			# so that user can do:
			#	echo "some SQL statement" | evclip-execute-sql.py chjfth.exb 
			# to execute one shot SQL statement.
			sys.exit(0)

		count += 1

	conn.close()

if __name__=='__main__':
	if len(sys.argv)==1:
		print("Need an .exb file as parameter.")
		print('Execute SQLite SQL statements under collation "NOCASEUTF8".')
		sys.exit(1)

	dbfilepath = sys.argv[1]
	do_work(dbfilepath)