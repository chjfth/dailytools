chja20 is the rsync-server that provides the source data to backup.

do-irsync.cygwin.bat and do-irsync.cygwin.sh are run from a rsync-client machine that will store backup data of chja20.


[2022-06-20]
If using WSL1 to run irsync, I suggest open a WSL Linux Bash shell, then execute sth like:

	[chj @Win10BlueSSD /mnt/f/backup-chja20]
	$ ./do-irsync.sh | tee do-irsync.sh.log

Cygwin may have a buggy behavior: When we kill the running rsync.exe, by Taskmgr or Ctrl+C 
from console, the caller may receive exit-code 0, so it is a fake success, and the fake 
success assets fake backup finishing. So it's a threat to backup integrity.

So Avoid using Cygwin. Use WSL instead.


[2022-07-01] Alternative: run it from within a WSL1 shell.

Open a WSL1 shell, then execute:

	./do-irsync.sh

BUT be aware, do NOT use old WSL from Win10.1709, which has bug. Win10.1709 would not respect 
rsync's --link-dest option, and the "same" file from two backup sessions do NOT exhibit mutual 
hard links, instead, it will do full copy of the "same" file, so that will be very disk-space 
unfriendly if you do periodical backups.

Win10.21H2 has fixed this problem.
