#!/usr/bin/env python3
#coding: utf-8

import os, sys

# Chaos iteration:
#	 Xn+1=R*Xn*(1-Xn)

def nextX(R, x):
	return R*x*(1-x)

def breed(R, x, gens):
	
	# Calculate print_hop so that total coarse print is limited to 100~1000 lines.
	scale = (gens-1)//1000
	print_hop = 1
	if scale>0:
		print_hop = 10**len("%d"%scale)
	
	print_next = print_hop

	tail_details = 50
	
	for i in range(gens-tail_details):
		x = nextX(R, x)
		
		if i+1==print_next:
			print("[%d] %.8f"%(i+1, x))
			print_next += print_hop

	# fine print final `tail_details` values one by one, no skip
	print('---- fine print final %d values ----'%(min(tail_details, gens)))
	for i in range(max(0, gens-tail_details), gens):
		x = nextX(R, x)
		print("[%d] %.8f"%(i+1, x))

if __name__=='__main__':
	R = float(sys.argv[1])
	x = float(sys.argv[2])
	gens = int(sys.argv[3])
	breed(R, x, gens)

# 3.445 -> 3.45
# 3.0 -> 3.01