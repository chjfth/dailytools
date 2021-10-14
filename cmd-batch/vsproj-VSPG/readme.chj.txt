[2020-09-22]

VSPG is Visual Studio Project Pre-build/Post-build Generic processing framework.
It can ease the deployment of many Pre-build/Post-build actions for us.

A typical scenario is: You want your output EXE/DLL to be copied to a remote machine, 
F5 start remote debugging immediately, and do it robustly.

Features:
* Robust. All errors in .bat are reported and cause project-build to fail.
* Easy to deploy and apply customization. 
* Be little intrusive, so we can apply it team-wide.
* Extensibility of VSPG framework is fairly OK.
* Co-existence with other Pre-build/Post-build actions.

==== How to deploy? ====

The key is to know how to "bootstrap" these .bat files. To do this, we need to 
configure Visual Studio project (.vcxproj or .csproj) to execute the 
_VSPG\VSPG-Start9.bat at its Pre-build stage and/or Post-build stage. 

Please see comments in VSPG-Start9.bat to know the exact command-line inside.

Please be clear: Merely executing VSPG-Start9.bat in your .vcxproj/.csproj
will not actually change the project-building behavior, so it does not affect
your team fellows on another svn/git sandbox.

The project-building behavior changes only after you make a copy of 
PostBuild-SyncOutput4.bat.sample to PostBuild-SyncOutput4.bat. So, 
PostBuild-SyncOutput4.bat etc is your personal stuff to tune/tweak, so to 
meet your personal need(copy output EXE to a remote machine for live debugging etc).

==== Bootstrapping Best Practice (for vcxproj) ====

A versed Visual Studio user knows that setting Pre-build/Post-build commands 
from VSIDE is very cumbersome, because you have to set them in each .vcxproj, 
each Release/Debug variant, each x86/x64 variant, so many repetitive copy/paste. 

Luckily, there is a best practice to help you out.

[STEP 1] Copy the whole _VSPG directory to your own project.

Please keep the directory name _VSPG. Aside from that, you can place it anywhere
you like.

Assume that you place it inside $(SolutionDir), i.e. side-by-side with your .sln file.

[STEP 2] Open foo.vcxproj in a text editor, at near-end of the file, add content:

  <PropertyGroup>
    <VSPG_BatDir_NoTBS>$(SolutionDir)_VSPG</VSPG_BatDir_NoTBS>
    <VSPG_FeedbackFile>$(ProjectDir)winmain.cpp</VSPG_FeedbackFile>
  </PropertyGroup>
  <Import Project="$(VSPG_BatDir_NoTBS)\_VSPG.props" />

so the end of foo.vcxproj looks like this:

...
  <PropertyGroup>
    <VSPG_BatDir_NoTBS>$(SolutionDir)_VSPG</VSPG_BatDir_NoTBS>
    <VSPG_FeedbackFile>$(ProjectDir)winmain.cpp</VSPG_FeedbackFile>
  </PropertyGroup>
  <Import Project="$(VSPG_BatDir_NoTBS)\_VSPG.props" />

  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets">
  </ImportGroup>
</Project>

STEP 2 tells Visual Studio two things:
(1) It defines a MSBuild variable named VSPG_BatDir_NoTBS telling where you have placed
    the _VSPG folder, so that _VSPG.props can be imported(=included) in VS project.
(2) It defines a MSBuild variable named VSPG_FeedbackFile to act as a feedback file.
    You must assign an existing source file from your project to make VSPG work.
    On .bat exec fail, the .bat code will touch the feedback file so your project 
    becomes out-of-date. Purpose: On next build cycle, VSPG will run again.

These two variables are recognized in _VSPG.props, so that VSGP .bat files in turn 
see their values.

Of course, you can place _VSPG folder at other places, just change VSPG_BatDir_NoTBS's
directory-prefix accordingly.

[STEP 3] Repeat STEP 2 for each .vcxproj of your interest.

OK now, with above minor manual tweak of .vcxproj, we have applied VSPG project-wide,
that is, x86/x64, Debug/Release configuration variants all receive VSPG's benefits.

