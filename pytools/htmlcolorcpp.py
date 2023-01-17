#!/usr/bin/env python3
#coding: utf-8

import os, re, sys
import stat
import shutil
from os.path import join as dj

COPY_SUFFIX = ".copy"
NEW_SUFFIX = ".newonly"

def EnumHtmlFile(root):
	for dirpath, dirnames, files in os.walk(root):
		for file in files:
			if file.endswith('.html') or file.endswith('.htm'):
				oldpath = dj(dirpath, file)
				yield oldpath, oldpath.replace(root, root+COPY_SUFFIX), oldpath.replace(root, root+NEW_SUFFIX)
			

class HtmlContent:
	def __init__(self, filepath, all_lines):
		self.filepath = filepath
		self.lines = all_lines.split(b'\n')
	def allbytes(self):
		return b'\n'.join(self.lines)

def makedir_for_file(filepath):
	dir = os.path.dirname(filepath)
	if not os.path.exists(dir):
		os.makedirs(dir)


def main(indir):
	dir_html_old = indir.rstrip('\\')
	dir_html_copy = dir_html_old + COPY_SUFFIX
	dir_html_new  = dir_html_old + NEW_SUFFIX
	
	print( 'Will generate a fresh "%s" subdir ...'%(dir_html_new) )
	
	# remove stale dirname_html_new output from last run.
	if os.path.exists(dir_html_copy):
		shutil.rmtree(dir_html_copy)
	if os.path.exists(dir_html_new):
		shutil.rmtree(dir_html_new)
	
	# Generate a new copy so that output content can be viewed just in-place.
	shutil.copytree(dir_html_old, dir_html_copy)
	
	for oldhtmlpath, copyhtmlpath, newhtmlpath in EnumHtmlFile(dir_html_old):

		is_filepath_printed = [False]
		
		def prn_html_path():
			if not is_filepath_printed[0]:
				print( '$'+oldhtmlpath[len(dir_html_old):] ) # print the file with rootdir stripped.
				is_filepath_printed[0] = True

		def cstrRepl(matchobj):
			if line_text.find(b'<')>=0 or line_text.find(b'>')>=0:
				return matchobj.group(0) # unchanged
			else:
				prn_html_path()
				c_string = matchobj.group(0)
				print( '  #%d: colorize string %s'%(line_idx+1, c_string) )
				return b'<span class="c-string">' + c_string + b'</span>'
		
		def ccRepl(matchobj):
			prn_html_path()
			c_comment = matchobj.group(0)
			print( '  #%d: // colorize C/C++ comment'%(line_idx+1) )
			return b'<span class="c-comment">' + c_comment + b'</span>'
		

		infh = open(oldhtmlpath, 'rb')
		htmlcnt = HtmlContent(oldhtmlpath, infh.read())
		infh.close()
		
		#outfh = open()
		
		for line_idx, line_text in enumerate(htmlcnt.lines):

			"""Make C/C++ string magenta.
			   .c-string { color: magenta; }
			Simple logic: Match each double-quoted string, but only if current line does not have < and > .
			So, double-quoted strings as html element's attribute will be excluded. As a side effect,
			html content like <p>He says "blah blah..."</p> will also be ignored, which is not a problem.
			"""
			line_text = re.sub(rb'".+?"', cstrRepl, line_text, flags=re.IGNORECASE)
			line_text = re.sub(rb'&quot;.+?&quot;', cstrRepl, line_text, flags=re.IGNORECASE)

			""" Make C/C++ comment line in green color. 
			    .c-comment { color: #00AA00; }
			Simple logic: find '//' and the char before '//' can only be a whitespace(\s).
			This gets rid of 'http://www.foobar.com', "-//W3C//DTD HTML 4.01 Transitional/..."
			"""
			line_text = line_text.strip(b'\r\n') # On Windows, '\r' is there and should be stripped.
			line_text = re.sub(rb'(?<![^\s])//.*$', ccRepl, line_text, flags=re.IGNORECASE)

			htmlcnt.lines[line_idx] = line_text
		
		if not is_filepath_printed[0]:
			continue # Nothing to change, no need to copy this html.
		
		makedir_for_file(copyhtmlpath)
		makedir_for_file(newhtmlpath)
		
		with open(copyhtmlpath, "wb") as outfh:
			outfh.write(htmlcnt.allbytes())
		
		with open(newhtmlpath, "wb") as outfh:
			outfh.write(htmlcnt.allbytes())
	

if __name__=='__main__':
	if len(sys.argv)==1:
		print("Need an input directory that contains html files.")
		print("Files in the input directory will not be changed, instead, a .newonly folder will be created side-by-side with modified content.");
		exit(1)
	
	main(sys.argv[1])
