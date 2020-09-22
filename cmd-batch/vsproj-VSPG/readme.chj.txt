[2020-09-22]

VSPG is Visual Studio Project Pre-build/Post-build Generic processing framework.
It can ease the deployment of many Pre-build/Post-build actions for us.

REMEMBER: 
* We must place these .bat files inside a subdirectory named _VSPG.
* The _VSPG subdir should appear inside either $(ProjectDir) or $(SolutionDir) or both.

Features:
* Robust. All errors in .bat are reported and fail the build.
* Easy to deploy and apply customization. 
* Extensibility of the framework is fairly OK.
