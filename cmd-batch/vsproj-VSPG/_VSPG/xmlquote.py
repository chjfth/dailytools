#!/usr/bin/env python3
#coding: utf-8

import os, sys
from xml.sax.saxutils import escape

def do_quote(filename):
	
	intext = open(filename, "r").read()
	outtext = escape(intext, {'%' : '%25'})
	
	print(outtext)


if __name__=='__main__':
	if len(sys.argv)!=2:
		sys.stderr.write("Need a text file as parameter.\n")
		sys.stderr.write("Example.\n")
		sys.stderr.write("  xmlquote.py VSPG-PostBuild7.csproj.txt\n")
		exit(1)
	
	do_quote(sys.argv[1])

# Running 
#
#	python xmlquote.py VSPG-PostBuild7.csproj.txt > extra.props
#
# produces content below. Then we can 
#
#  <Import Project="extra.props" />
#  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
#  <ImportGroup Label="ExtensionTargets">
#
# at end of .vcxproj .
"""

<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">

<ItemDefinitionGroup>
    <PostBuildEvent>
      <Command>REM Paste these text into .csproj project-settings -&gt; Post-build event command line.
REM Customization: Change Program.cs below to be an existing file in your VS-project.
set FILE_TOUCH=$(ProjectDir)dllmain.cpp
REM
set BatFilenam=VSPG-PostBuild7.bat
if not exist "%25FILE_TOUCH%25" (
  echo [VSPG]ERROR: The filepath defined by FILE_TOUCH does NOT exist: %25FILE_TOUCH%25
  exit /b 4
)
set ALL_PARAMS=$(SolutionDir) $(ProjectDir) $(Configuration) $(PlatformName) $(TargetDir) $(TargetFileName) $(TargetName)
if exist $(ProjectDir)_VSPG\%25BatFilenam%25 (
  cmd /c $(ProjectDir)_VSPG\%25BatFilenam%25 %25ALL_PARAMS%25
) else if exist $(SolutionDir)_VSPG\%25BatFilenam%25 (
  cmd /c $(SolutionDir)_VSPG\%25BatFilenam%25 %25ALL_PARAMS%25
)
if errorlevel 1 ( copy /b "%25FILE_TOUCH%25"+,, "%25FILE_TOUCH%25" &gt;NUL 2&gt;&amp;1 &amp;&amp; exit /b 4 )

</Command>
    </PostBuildEvent>
</ItemDefinitionGroup>

</Project>

"""