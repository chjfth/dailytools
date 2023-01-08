## A bat file to live-register your WSL1 instance into Windows 10.

`WSL-green.bat` here registers the WSL1 file-system at current directory into your 
Windows 10 system, so that you can use `wsl -d FoobarDistributionName` to run it. 

This makes your WSL1 instance become totally **portable**, that is, no longer 
need to install it from Microsoft Store every time you move/copy your WSL1 folder 
to a new windows 10 machine.

## How to use

### [STEP 1] 

You need to first have an already installed WSL1 file-system in an NTFS folder.

Assume that folder is `D:\WSL-Ubuntu-2204`, that is, you have these sub-folders:

	D:\WSL-Ubuntu-2204\rootfs
	D:\WSL-Ubuntu-2204\rootfs\root
	D:\WSL-Ubuntu-2204\rootfs\etc\init.d
	
### [STEP 2]

Copy files here to your WSL1 folder, that is you should finally have:

	D:\WSL-Ubuntu-2204\WSL-green.reg.0
	D:\WSL-Ubuntu-2204\WSL-green.bat

### [STEP 3]

Create a simple wrapper bat, any name you like, for example, `start-Ubuntu-2204.bat`, 
with only one line:

	"%~dp0WSL-green.bat" Ubuntu-22.04-portable bob

Note:

* Ubuntu-22.04-portable is the so-called "distribution name" recognized by `wsl -d`.
  You select this name by yourself.
* The "distribution name" **must not contains space-char**, at least for Win10.21H2.
* If the distribution name conflicts with an existing one on current Win10 machine. 
  That name will point to this "new" distribution, and the "old" one is masked off.
  So, to ensure no conflict, please use `wsl -l` to know what names have existed.
* The `bob` tells the auto-login user, it will be passed to `wsl -d` as `-u bob`. 
  If you omit this param, `root` will be the auto-login user.

## Comments

Inside `WSL-green.bat`, it will use WSL-green.reg.0 as template, generate real 
WSL-green.reg, and then import that .reg so to live-register 
your `D:\WSL-Ubuntu-2204` as a new WSL1 instance.

Upon registering done, `wsl -d my_distribution_name` is immediately executed to launch
that WSL instance.

There is no harm registering the same WSL instance repeatedly each time you run `WSL-green.bat`.

Run as Administrator is not required.
