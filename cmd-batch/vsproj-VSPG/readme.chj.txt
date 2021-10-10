[2020-09-22]

VSPG is Visual Studio Project Pre-build/Post-build Generic processing framework.
It can ease the deployment of many Pre-build/Post-build actions for us.

REMEMBER: 
* We must place these .bat files inside a subdirectory named _VSPG.
* The _VSPG subdir should appear inside either $(ProjectDir) or $(SolutionDir) or both.

Features:
* Robust. All errors in .bat are reported and fail the build.
* Easy to deploy and apply customization. 
* Be little intrusive, so we can apply it team-wide.
* Extensibility of the framework is fairly OK.
* Co-existence with other Pre-build/Post-build actions.

==== How to deploy? ====

The key is to know how to "bootstrap" these .bat files. To do this, we need to 
configure Visual Studio project files(.vcxproj or .csproj) to execute the 
_VSPG\VSPG-Start9.bat at its Pre-build stage and Post-build stage. 

Please see comments in VSPG-Start9.bat to know the exact command-line to write.

Please be clear: Merely bootstrapping VSPG-Start9.bat in your .vcxproj/.csproj
will not actually change the project-building behavior, so it does not affect
your team fellows on another svn/git sandbox.

The project-building behavior changes only after you make a copy of 
PostBuild-SyncOutput4.bat.sample to PostBuild-SyncOutput4.bat. So, 
PostBuild-SyncOutput4.bat etc is your personal stuff to tune/tweak the build 
to meet your personal need(copy output EXE to a remote machine for live debugging etc).

==== Advanced Usage on .sln & .vcxproj ====

You may soon find that setting Pre-build/Post-build commands from VSIDE is very
cumbersome, because you have to set them in each .vcxproj, each Release/Debug 
variant, each x86/x64 variant. Luckily, there is a way to help you out.

[STEP 1] Create a new file named $(SolutionDir)_VSPG.props, with content:

<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemDefinitionGroup>
    <PreBuildEvent>
      <Command>$(SolutionDir)_VSPG\VSPG-Start9.bat $(SolutionDir)_VSPG\VSPG-PreBuild7.bat $(ProjectDir)$(VSPG_FeedbackFile) $(SolutionDir) $(ProjectDir) $(Configuration) $(PlatformName) $(TargetDir) $(TargetFileName) $(TargetName)</Command>
    </PreBuildEvent>
    <PostBuildEvent>
      <Command>$(SolutionDir)_VSPG\VSPG-Start9.bat $(SolutionDir)_VSPG\VSPG-PostBuild7.bat $(ProjectDir)$(VSPG_FeedbackFile) $(SolutionDir) $(ProjectDir) $(Configuration) $(PlatformName) $(TargetDir) $(TargetFileName) $(TargetName)</Command>
    </PostBuildEvent>
  </ItemDefinitionGroup>
</Project>

[STEP 2] Open foo.vcxproj in a text editor, at near-end of the file, add content:

  <PropertyGroup>
    <VSPG_FeedbackFile>.\winmain.cpp</VSPG_FeedbackFile>
  </PropertyGroup>
  <Import Project="$(SolutionDir)_VSPG.props" />

so the end of foo.vcxproj looks like this:

...
  <PropertyGroup>
    <VSPG_FeedbackFile>.\winmain.cpp</VSPG_FeedbackFile>
  </PropertyGroup>
  <Import Project="$(SolutionDir)_VSPG.props" />

  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets">
  </ImportGroup>
</Project>

STEP 2 tells two things:
(1) It defines a MSBuild variable named VSPG_FeedbackFile with value .\winmain.cpp .
(2) It imports(=includes) the _VSPG.props file you defined in STEP 1.

As you have noticed, VSPG_FeedbackFile's value is expanded in _VSPG.props, so that
_VSPG.props's content remains invariable across all .vcxproj files.

[STEP 3] Repeat STEP 2 for each .vcxproj of your interest, but remember to set new
value for VSPG_FeedbackFile for new .vcxproj.

OK now, with minor manual tweak of .vcxproj, we have applied VSPG project-wide.

