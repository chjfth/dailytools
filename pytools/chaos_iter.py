#!/usr/bin/env python3
#coding: utf-8

import os, sys

import math
import matplotlib.pyplot as plt

mycolors = ['#50a0e0', '#f86030', '#50c878', '#c030dd']
mycolors_dark = ['#90e0ff', '#ffe8e0', '#b0e0b0', '#e0c0ff']
def getcolor(idx): return mycolors[idx%len(mycolors)]

def getcolor_dark(idx): return mycolors_dark[idx%len(mycolors)]


def Enable_chs_font(fontname='Microsoft YaHei'):
    # Thanks https://my.oschina.net/u/1180306/blog/279818
    from pylab import mpl
    mpl.rcParams['font.sans-serif'] = fontname

def is_iter(obj):
    try:
        it = iter(obj)
        return True
    except TypeError:
        return False

def do_plot_iter(fx, yinits, xcount, ylimit=None, fr=False, draw_axis=None):
    
    # fx(t) is the function to calculate next iteration-value
    
    if fr: # fixed aspect ratio, a circle looks like a circle
        plt.axis('equal')
    
    if draw_axis:
        plt.axhline(y=draw_axis[1], color='grey')
        plt.axvline(x=draw_axis[0], color='grey')

    fx_list = fx if is_iter(fx) else [fx]
    yinits = yinits if is_iter(yinits) else [yinits]
   
    if is_iter(ylimit):
        ylimits = (ylimit[0], ylimit[1])
    elif ylimit:
        ylimits = (-ylimit, ylimit)
    else:
        ylimits = None

    x_list = range(0, xcount+1)
   
    for ifx, fx in enumerate(fx_list):
        # Each function from user:
        y_list = [ yinits[ifx] ]
        y = y_list[0]
        for i in range(1, xcount+1):
            # Each point:
            try:
                y = fx(y)
                if ylimits and (y<ylimits[0] or y>ylimits[1]):
                    y = math.nan
            except (ZeroDivisionError, OverflowError, ValueError):
                y = math.nan
       
            y_list.append(y)
           
        plt.plot(x_list, y_list, linewidth=1, color=getcolor_dark(ifx))

        plt.plot(x_list, y_list, 'o', markersize=2,
        	label=getattr(fx,'label',None), color=getcolor(ifx))
       
    plt.legend() # to show user-given label
    plt.grid(linestyle='dotted')
    
    plt.xlabel("Iteration count (n)")
    plt.ylabel("X Value after iteration (Xₙ)")
    
    Enable_chs_font('Tahoma')
    plt.show()


# Chaos iteration:
#	 Xn+1=R*Xn*(1-Xn)

def nextX(R, x):
	return R*x*(1-x)

def make_gnx(R):
	def gen_nextX(x):
		return R*x*(1-x)
	return gen_nextX

def do_plot_iter_Rs(Rs, x0s, itercount):
	
	if itercount>10000:
		print("Info： itercount is too large, I cannot do plotting.")
		return
	
	Rs = Rs if is_iter(Rs) else [Rs]
	x0s = x0s if is_iter(x0s) else [x0s]*len(Rs)
	
	list_gnx = []
	for i, R in enumerate(Rs):
		gnx = make_gnx(R)
		gnx.label = 'R={} , X₀={}'.format(R, x0s[i])
		list_gnx.append(gnx)
	
	do_plot_iter(list_gnx, x0s, itercount)

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
	if tail_details<gens:
		print('---- fine print final %d values ----'%(min(tail_details, gens)))
	
	for i in range(max(0, gens-tail_details), gens):
		x = nextX(R, x)
		print("[%d] %.8f"%(i+1, x))


if __name__=='__main__':
	if len(sys.argv)==1:
		# Use examples from Dedao.cn Joker 2016.11.18 《卓克·卓老板聊科技》
		
		do_plot_iter_Rs(
			[2, 2],
			[0.2, 0.7],
			10)
		
		# 13:00
		do_plot_iter_Rs([2.5]*2, [0.2, 0.7], 10)
		
		# 13:48
		do_plot_iter_Rs([3.1]*2, [0.2, 0.7], 10)

		do_plot_iter_Rs([3.4]*2, [0.2, 0.7], 50)

		do_plot_iter_Rs([3.5]*2, [0.2, 0.7], 50)

		do_plot_iter_Rs([3.55]*2, [0.2, 0.7], 200)
		
		# 15:50 chaos
		do_plot_iter_Rs([3.58]*2, [0.2, 0.20000001], 200)
		do_plot_iter_Rs([4.00]*2, [0.2, 0.20000001], 200)
		
	else:
		# Use parameters from command line
		R = float(sys.argv[1])
		x0 = float(sys.argv[2])
		itercount = int(sys.argv[3])
		
		breed(R, x0, itercount)
		
		do_plot_iter_Rs(R, x0, itercount)
		
		# do_plot_iter_Rs([R, R+0.2], x0, itercount) # you can try this: chaos_iter.py 2.9 0.1 100
	
# 3.445 -> 3.45
# 3.0 -> 3.01