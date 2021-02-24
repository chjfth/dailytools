#!/usr/bin/env python3
#coding: utf-8

"""TODO: To get really precise results, I should use Decimal instead of float."""

import os, sys

import math
import matplotlib
import matplotlib.pyplot as plt

from decimal import *
decimal_context = getcontext()
decimal_context.traps[FloatOperation] = True # Raise error on truncation
decimal_context.prec = 56 # fractional precision (sys default is 28, we can increase it to 2800)

CHKRECUR_HOP = 64 # must be power of 2

fontname_subscript = "Tahoma" # For character of ₙ ₀
fontname_chs = "Microsoft YaHei" # 微软雅黑

mycolors = ['#50a0e0', '#f86030', '#50c878', '#c030dd']
mycolors_dark = ['#90e0ff', '#ffe8e0', '#b0e0b0', '#e0c0ff']
def getcolor(idx): return mycolors[idx%len(mycolors)]

def getcolor_dark(idx): return mycolors_dark[idx%len(mycolors)]


def Enable_chs_font(fontname='Microsoft YaHei'):
    # Thanks https://my.oschina.net/u/1180306/blog/279818
    matplotlib.rcParams['font.sans-serif'] = fontname

def is_iter(obj):
    try:
        it = iter(obj)
        return True
    except TypeError:
        return False

def do_plot_iter(fx, yinits, xcount, title=None, ylimit=None, fr=False, draw_axis=None):
    
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
	            ### ITERATION ###
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

    #Enable_chs_font() # Sigh, I cannot use CHS font as default

    if title:
        plt.title(title, fontname=fontname_chs)

    plt.show()


# Chaos iteration:
def nextX(R, x):
	return R * x * (1-x)

def make_gnx(R):
	def gen_nextX(x):
		return nextX(R, x)
	return gen_nextX

#ChaosDraw = namedtuple('ChaosDraw', 'R X0 text') # cannot support default member value?
class ChaosDraw:
	def __init__(self, R, X0, text=None):
		self.R = R
		self.X0 = X0
		self.text = text

def do_plot_iter_Rs(params, itercount, title=None):

	chaosdraws = [ChaosDraw(*p) for p in params]

	if itercount>10000:
		print("Info： itercount is too large, I cannot do plotting.")
		return

	x0s = [c.X0 for c in chaosdraws]

	list_gnx = []
	for i, draw in enumerate(chaosdraws):
		gnx = make_gnx(draw.R)
		gnx.label = 'R={} , X₀={}%'.format(draw.R, float(draw.X0*100))
		# -- note: if not casting to float,
		#   "{}%".format( Decimal("0.12034")*100 )
		# will be result in
		#   12.03400%
		# which is weird.
		if draw.text:
			gnx.label += " (%s)"%(draw.text)
		list_gnx.append(gnx)
	
	do_plot_iter(list_gnx, x0s, itercount, title=title)

def breed(R, x, itercount):

	x0 = x
	is_recurrence_found = False

	# Calculate print_hop so that total coarse print is limited to 100~1000 lines.
	scale = (itercount - 1) // 1000
	print_hop = 1
	if scale>0:
		print_hop = 10**len("%d"%scale)

	chkrecur_inext = CHKRECUR_HOP
	chkrecur_value__3 = -3 # init to any negative value
	chkrecur_value__2 = -2 # init to any negative value
	chkrecur_value__1 = -1 # init to any negative value

	print_next = print_hop

	for i_ in range(1, itercount+1):
		x = nextX(R, x)

		if i_==print_next:
			print("[{}] {}".format(i_, x))
			print_next += print_hop

		if i_==chkrecur_inext:
			# Time to check: if we've met recurrence value. If so, we can stop iteration.
			if x == chkrecur_value__1:
				print("==== Recurrence value encountered ====")
				iBackStart = i_-CHKRECUR_HOP*3
				if iBackStart <= 0:
					found = find_first_recurrence_value(R, 0, x0,
					                            print_next-print_hop, print_hop)
				else:
					found = find_first_recurrence_value(R, iBackStart, chkrecur_value__3,
				                                print_next-print_hop, print_hop)
				assert found==True
				is_recurrence_found = found
				break
			else:
				chkrecur_value__3 = chkrecur_value__2;
				chkrecur_value__2 = chkrecur_value__1;
				chkrecur_value__1 = x

			chkrecur_inext = i_ + CHKRECUR_HOP
	else:
		if is_recurrence_found==False:
			# find_first_recurrence_value() has not been called even once.
			# We should call it at least once.
			is_recurrence_found = find_first_recurrence_value(R, 0, x0,
			                           print_next-print_hop, print_hop)
			if is_recurrence_found==False:
				if print_hop==1:
					print("No recurrence found. "+RnPsuffix(R))
				else:
					dump_final_values(i_, R, x)

def find_first_recurrence_value(R, istart, xstart, i_prevprint, print_hop):
	xarray = []
	istart_ = istart+1
	x = xstart
	CHKRECUR_HOP3 = CHKRECUR_HOP * 3
	# Recalculate from istart
	for j in range(CHKRECUR_HOP3):
		x = nextX(R, x)
		xarray.append(x)
		# print("[{}] {}".format(istart_+j, x)) # we'd better delay the print to avoid verbose print

	idx, count = _find_recur_forward(xarray)
	#assert idx>CHKRECUR_HOP
	if idx==None:
		return False

	if print_hop>1:
		print("==== Fine dumping final values ====")

	for j in range(CHKRECUR_HOP3):
		inow = istart_+j
		if (print_hop > 1 or inow > i_prevprint) and j < idx+count*2:
			print("[{}] {}".format(inow, xarray[j]))

	print("Recurrence@[{}], count={}. {}".format(istart_+idx, count, RnPsuffix(R)))
	return True

def _find_recur_forward(xarray):
	idxmax_ = len(xarray)
	for j in range(idxmax_):
		for k in [1, 2, 4, 8, 16, 32, 64, 128, 256]:
			if j+k>=idxmax_:
				continue
			if xarray[j]==xarray[j+k]:
				return j, k
	return None, None


def dump_final_values(iprev, R, xprev):
	print("==== Dumping some final values ====")
	x = xprev
	istart = iprev + 1
	iend = istart
	for j in range(CHKRECUR_HOP):
		x = nextX(R, x)
		print("[{}] {}".format(istart+j, x))
		iend += 1
		if x == xprev:
			assert False # Old code. This will not be executed.
			print("Recurrence values found. Recurrence-count={} (R={} precision: {})".format(
					iend - istart,
					R, decimal_context.prec
			))
			return
	else:
		print("No recurrence found. Final {} values are dumped above. {}".format(
			CHKRECUR_HOP, RnPsuffix(R)
		))

def RnPsuffix(R):
	return "(R={} precision:{})".format(R, decimal_context.prec)

if __name__=='__main__':
	if len(sys.argv)==1:
		# Use examples from Dedao.cn Joker 2016.11.18
		# 《卓克·卓老板聊科技》混沌世界是怎么出现的？

		do_plot_iter_Rs([
			[2, Decimal("0.2"), "@11:44"],
			[2, Decimal("0.9999"), "@12:22"],
			[Decimal("2.5"), Decimal("0.2"), "@13:00"],
			[Decimal("2.5"), Decimal("0.9"), ""],
			], 16, f"R=2,迭代结果很快稳定于0.5 ; R=2.5,稳定于0.6")

		R = Decimal("3.1")
		do_plot_iter_Rs([
			[R, Decimal("0.2"), "@13:46"],
			[R, Decimal("0.7"), ""],
			], 10, f"R={R}, 看到在 2 个数值上跳变")

		# 卓老板 14:10 说 R=3.4 时会出现4个跳变值，本图只看到收敛到2个值,
		# 是因为1号和3号收敛值过于接近, 2号和4号收敛值过于接近,
		# 迭代一百次后, 差别到了小数点后第 7 位, 肉眼看不出, 因此误认为收敛到2个值.
		# R=3.4 很让我困惑, 因为用 56 位精度来算, 1号值和3号值是在缓缓接近的，
		# 继续迭代到 875 次时才看到它们稳定在4个跳变值,
		# [875]	0.84215439943267057935970192568651098140747579679937374095
		# [876]	0.45196324762615295005206278019584195976899479143592037676
		# [877]	0.84215439943267057935970192568651098140747579679937374105
		# [878]	0.45196324762615295005206278019584195976899479143592037654
		# #875 和 #877 小数点后 53 位相同. 如果继续提高计算精度，它们不是会趋于重合吗？
		# 那样的话，我们还能说它有 4 个跳变点吗？
		R = Decimal("3.4")
		do_plot_iter_Rs([
			[R, Decimal("0.2"), "@14:10 *"],
			[R, Decimal("0.7"), ""],
			], 100, f"R={R}, 看到在 2 个数值上跳变（这个结论存疑!）")
		#
		# R=3.45 时, 会比较明显地看到收敛到四个点.
		# 0.4459676567920559802834429045...
		# 0.8524277453117333628737654796...
		# 0.4339916609539836091945485465...
		# 0.8474680021585322104281798105...
		R = Decimal("3.45")
		do_plot_iter_Rs([
			[R, Decimal("0.2"), "@14:10"],
			[R, Decimal("0.7"), ""],
			], 100, f"R={R}, 看到在 4 个数值上跳变")

		R = Decimal("3.56")
		do_plot_iter_Rs([
			[R, Decimal("0.2"), ""],
			[R, Decimal("0.7"), ""],
			], 200, f"R={R}, 看到在 8 个数值上跳变")

		R = Decimal("3.567")
		do_plot_iter_Rs([
			[R, Decimal("0.2"), ""],
			], 1000, f"R={R}, 看到在 16 个数值上跳变")

		# Memo:
		# R=3.56789 , prec=28,  [1728] 显示 64 跳变点 (精度不够的假象)
		# R=3.56789 , prec=280, [18528] 显示 32 跳变点, 精度继续提高是否会变成 64 未有定论
		# R=3.569 ,   prec=280, [30560] 显示 32 跳变点 (也许依然是精度不够的假象)

		R = Decimal("3.572")
		do_plot_iter_Rs([
			[R, Decimal("0.2"), "@14:25"],
			], 2000, "R 刚刚超过混沌临界点的样子")

		R = Decimal("3.572")
		do_plot_iter_Rs([
			[R, Decimal("0.2"), "@15:50"],
			[R, Decimal("0.2000000001"), "@15:50"],
		], 500, f"R={R}, X0微小差异,两根曲线伴随迭代200多次后突然分道扬镳")

		R = Decimal("4.0")
		do_plot_iter_Rs([
			[R, Decimal("0.2"), "@16:28"],
			[R, Decimal("0.2000000001"), "@16:28"],
			], 100, f"R={R}, R值越大, 两曲线分道越早")

	else:
		# Use parameters from command line, e.g.
		# 2.2  0.1 10
		# 2.2  0.2 100
		# 3.45 0.2 100
		R = Decimal(sys.argv[1])
		x0 = Decimal(sys.argv[2])
		itercount = int(sys.argv[3])

		title = None if len(sys.argv)<=4 else sys.argv[4]
		
		breed(R, x0, itercount)
		
		do_plot_iter_Rs([[R, x0]], itercount, title)
