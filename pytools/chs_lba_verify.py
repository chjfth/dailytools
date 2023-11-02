#!/usr/bin/env python3
#coding: utf-8
# Python 3.5+ is required for string formatting

import os, sys, codecs

def print_help():
	print("This program interprets C/H/S values from a MBR partition table entry, and/or,\n"
	      "compares them to the LBA raw value in the same entry to see if they match.\n"
	      "Tech notes: C: Cylinder , H: Head , S: Sector \n"
	      "            These three params constitute the so-called disk geometry.\n"
	      )
	print("Usage example 1:")
	print("    chs_lba_verify.py 78 51 e6")
	print("")
	print("Usage example 2:")
	print("    chs_lba_verify.py 80 01 01 00  0C 78 51 E6  20 00 00 00  DF 3F 77 00")

class CHS: pass

def decode_1hexrp(hexrp):
	byte_value = ord( codecs.decode(hexrp, 'hex') )
	assert(byte_value>=0 and byte_value<=255)
	return byte_value

def chs_from_hexrp(argv):
	# argv[0:3] : the 3 raw bytes (in pt-entry) representing C/H/S
	# pt-entry: a 16-byte partition table entry
	b0 = decode_1hexrp(argv[0])
	b1 = decode_1hexrp(argv[1])
	b2 = decode_1hexrp(argv[2])
	chs = CHS()
	chs.c = ( (b1 & 0xC0)<<2 ) + b2
	chs.h = b0
	chs.s = b1 & 0x3f
	return chs

def lba_from_chs(chs, s1base):
	# s1base: True/False, is chs.s one-based or zero-based?
	lba = (((chs.c * 255) + chs.h) * 63) + chs.s
	if s1base:
		lba -= 1
	return lba

def lba_from_ptnentry(argv):
	b0 = decode_1hexrp(argv[0])
	b1 = decode_1hexrp(argv[1])
	b2 = decode_1hexrp(argv[2])
	b3 = decode_1hexrp(argv[3])
	return (b3<<24) + (b2<<16) + (b1<<8) + b0


def interpret_chs(argv):
	# We need arg[0:3], the CHS triple-byte group
	chs = chs_from_hexrp(argv[0:3])
	print("C: %d" % chs.c)
	print("H: %d" % chs.h)
	print("S: %d" % chs.s)
	lba = lba_from_chs(chs, True)
	print('CHS-calculated-LBA: {:,} (0x{:X})'.format(lba, lba))
	#print("CHS-calculated LBA: %d" % lba_from_chs(chs, True))

def chs_lba_checkmatch(argv):
	# We need arg[0:16], the whole pt-entry
	# Formatting example: https://stackoverflow.com/a/58568092/151453
	#
	chs1 = chs_from_hexrp(argv[1:4])
	chs2 = chs_from_hexrp(argv[5:8])
	print( f'C: {chs1.c:>15}    {chs2.c:<15}' )
	print( f'H: {chs1.h:>15} -> {chs2.h:<15}' )
	print( f'S: {chs1.s:>15}    {chs2.s:<15}' )

	chs_lba1 = lba_from_chs(chs1, True)
	chs_lba2 = lba_from_chs(chs2, True)
	chs_lba_count = chs_lba2 - chs_lba1 + 1
	lba1_0x = f"0x{chs_lba1:X}"
	lba2_0x = f"0x{chs_lba2:X}"
	lba_diff_0x = f"0x{chs_lba_count:X}"
	plus_minus = '+' if chs_lba_count>=0 else '-'
	print('CHS-calculated:')
	print( f'         start-LBA    end-LBA')
	print( f'   {chs_lba1:>15,} -> {chs_lba2:<15,} ({plus_minus}{chs_lba_count:,})' )
	print( f'   {lba1_0x:>15}    {lba2_0x:<15} ({plus_minus}{lba_diff_0x})' )

	print( '-'*36 )

	rawlba_start = lba_from_ptnentry(argv[8:12])
	rawlba_count = lba_from_ptnentry(argv[12:16])

	lba_start_bias = chs_lba1 - rawlba_start
	lba_count_bias = chs_lba_count - rawlba_count

	if lba_start_bias==0:
		lba_start_bias_report = "(OK! match CHS.)"
		lba_start_bias_report_hex = ""
	else:
		lba_start_bias_report = f"(CHS BAD! bias: {lba_start_bias:+d})" # f"{lba_start_bias:+d,}" errs on Python 3.9
		plus_minus = '+' if lba_start_bias>=0 else '-'
		lba_start_bias_report_hex = f"                {plus_minus}0x{abs(lba_start_bias):X}"

	if lba_count_bias==0:
		lba_count_bias_report = "(OK! match CHS.)"
		lba_count_bias_report_hex = ""
	else:
		lba_count_bias_report = f"(CHS BAD! bias: {lba_count_bias:+d})"
		plus_minus = '+' if lba_count_bias >= 0 else '-'
		lba_count_bias_report_hex = f"                {plus_minus}0x{abs(lba_count_bias):X}"

	print( f'Raw-LBA-start: {rawlba_start:<15,} {lba_start_bias_report}' )
	print( f'               0x{rawlba_start:<13X} {lba_start_bias_report_hex}' )

	print( f'Raw-LBA-count: {rawlba_count:<15,} {lba_count_bias_report}' )
	print( f'               0x{rawlba_count:<13X} {lba_count_bias_report_hex}')

def do_main():
	params = len(sys.argv) - 1
	argv = sys.argv[1:]
	if params==3:
		interpret_chs(argv)
	elif params==16:
		chs_lba_checkmatch(argv)
	else:
		print_help()
		exit(1)

if __name__=='__main__':
	do_main()
	exit(0)

"""
chs_lba_verify.py  80 01 01 00  0C 78 51 E6  20 00 00 00  DF 3F 77 00

Running output:

C:               0    486
H:               1 -> 120
S:               1    17
CHS-calculated:
         start-LBA    end-LBA
                63 -> 7,815,166       (+7,815,104)
              0x3F    0x773FFE        (+0x773FC0)
------------------------------------
Raw-LBA-start: 32              (CHS BAD! bias: +31)
               0x20                            +0x1F
Raw-LBA-count: 7,815,135       (CHS BAD! bias: -31)
               0x773FDF                        -0x1F
"""
