chja20 is the rsync-server that provides the source data to backup.

do-irsync.cygwin.bat and do-irsync.cygwin.sh are run from a rsync-client machine that will store backup data of chja20.

[2022-06-20]
If using WSL1 to run irsync, I suggest open a WSL Linux Bash shell, then execute sth like:

	[chj @Win10BlueSSD /mnt/f/backup-chja20]
	$ ./do-irsync.sh | tee do-irsync.sh.log


[2022-07-01] Alternatively, run it from within a WSL1 shell.

Open a WSL1 shell, then execute:

	./do-irsync.sh
