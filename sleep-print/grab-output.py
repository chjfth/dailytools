#!/usr/bin/env python3
# coding: utf-8

import os,subprocess

exe_prefix = '' if os.name=='nt' else './'
subproc = subprocess.Popen(exe_prefix+'sleep-print *3 101 204 1040 1001 304 0', 
	shell=True,
	stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

while True:
	line = subproc.stdout.readline()
	if not line:
		break;
	print(line.decode("utf8"), end="")

subproc.wait() # we know that child process should have exited.
print("Subprocess exit-code is: %d (%s)"%(
	subproc.returncode, 
	"success" if subproc.returncode==0 else "fail"
	))
