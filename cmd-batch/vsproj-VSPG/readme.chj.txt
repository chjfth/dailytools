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

