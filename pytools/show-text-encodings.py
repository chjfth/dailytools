#!/usr/bin/env python
#-*- coding: utf-8 -*-

import os, sys, locale, tempfile
expressions = """
    locale.getpreferredencoding()
    sys.getfilesystemencoding()
    sys.getdefaultencoding()
    .
    type(disk_file)
    disk_file.encoding
    .
    sys.stdout.isatty()
    sys.stdout.encoding
    sys.stdin.isatty()
    sys.stdin.encoding
    sys.stderr.isatty()
    sys.stderr.encoding
"""

print("Python version %s on %s"%(sys.version.split()[0], sys.platform))
tmpdir = tempfile.gettempdir()
disk_file = open(os.path.join(tmpdir, 'dummy.tmp'), mode='w') # text-mode
for expression in expressions.split():
	if expression=='.':
		print('.'.rjust(30))
	else:
		value = eval(expression)
		print("%s -> %s"%(expression.rjust(30), repr(value)))
