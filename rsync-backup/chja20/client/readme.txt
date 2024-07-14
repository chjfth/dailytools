chja20 is the rsync-server that provides the source data to backup.

do-irsync-wsl.bat and do-irsync.sh are run from a rsync-client machine that will store backup data of chja20.


[2022-06-20]

If using WSL1 to run irsync, I suggest open a WSL Linux Bash shell, then execute sth like:

	[chj @Win10BlueSSD /mnt/f/backup-chja20]
	$ ./do-irsync.sh | tee do-irsync.sh.log

Using Cygwin is discouraged from now on.

Cygwin may have a buggy behavior: When we kill the running rsync.exe, by Taskmgr or Ctrl+C 
from console, the caller may receive exit-code 0, so it is a fake success, and the fake 
success results in fake backup finishing. So it's a threat to backup integrity.

So Avoid using Cygwin. Use WSL1 instead.


[2022-07-01]

Be aware, do NOT use old WSL1 from Win10.1709, which has bug. Win10.1709 would not respect 
rsync's --link-dest option, and the "same" file from two backup sessions do NOT exhibit mutual 
hard-links, instead, it will do full copy of the "same" file, so that will be very disk-space 
hogging if you do periodical backups.

Win10.21H2 has fixed this problem.


[2024-07-14]

Client machine do: 
* Open a CMD window, and `cd` into this directory.
* Execute do-irsync-wsl.bat .
* Then backup content will appear right inside this directory.

