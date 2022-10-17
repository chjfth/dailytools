## Summary

In this directory, are some sample .bat files that will be invoked by VSPG framework at some points, and we call such a bat file "triggering bat (file)".

For example, if `VSPU-CopyOrClean.bat` exists in the directory of vcxproj, then `VSPU-CopyOrClean.bat` will be called 

- in **Postbuild** stage of the project-build(typically, after the EXE file is generated), and
- in **Clean** stage (when you tell VSIDE to clean your project).

That filename implies that you should do "copy" or "clean" in `VSPU-CopyOrClean.bat`, and writing batch statements to do these actions reliably is usually harder than first imagined.

Luckily, in this "copy-or-clean" case, you don't have to write those batch statements yourself, VSPG framework has most code for doing this. Just grab a copy of `VSPU-CopyOrClean.bat.sample`, put it into your own vcxproj directory and rename it to `VSPU-CopyOrClean.bat`, then edit the .bat to have correct var values for your current Visual Studio project.

`VSPU-CopyOrClean.bat.sample` has a `.sample` suffix, because it wants to imply that, you need to make some modification to its content before you can actually use it. [VSPU-CopyOrClean.bat.md](VSPU-CopyOrClean.bat.md) talks about how you should modify it.

The above rules apply to other `.sample` files in this directory.

## Where to place these sample files(after deleting `.sample` extension)

(pending...)

## Sample list

| Sample file | Used for ... |
| ----------- | ------------ |
| [VSPU-CopyOrClean.bat.sample](VSPU-CopyOrClean.bat.md) | VSPU-CopyOrClean.bat |
| [Team-Postbuild.bat.sample](Team-Postbuild.bat.md) | Team-Prebuild.bat <br/>Team-Postbuild.bat <br/>Personal-Prebuild.bat <br/>Personal-Postbuild.bat <br/> |
| VSPU-StartEnv.bat.sample | VSPU-StartEnv.bat |
