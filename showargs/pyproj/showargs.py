#!/usr/bin/env python3
#coding: utf-8

import os, sys

hexdmpstr = lambda str : ' '.join('{:02X}'.format(ord(c)) for c in str)

argc = len(sys.argv)

print("Argument count(argc) = %d"%(argc))

for idx in range(argc):
	argv = sys.argv[idx]
	if os.environ.get('SHOWHEX') == '1':
		# print argv[] as hexdump
		print("(H%d)argv[%d]: %s"%(len(argv), idx, hexdmpstr(argv)))
	else:
		# print argv[] as text
		print("(%d)argc[%d]: %s"%(len(argv), idx, argv))
		



