#!/usr/bin/env python3
# coding: utf-8

import os,subprocess

exe_prefix = '' if os.name=='nt' else './'
subproc = subprocess.Popen(exe_prefix+'sleep-print *3 101 204 1040 1001 304 0', 
	shell=True,
	stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

while True:
	line = subproc.stdout.readline()
	print(line.decode("utf8"), end="")
	if not line:
		break;

