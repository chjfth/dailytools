chja20 is the rsync-server that provides the source data to backup.

do-irsync.bat and do-irsync.sh are run from a rsync-client machine that will store backup data of chja20.

[2022-06-20]
If using WSL1 to run irsync, I suggest open a WSL Linux Bash shell, then execute sth like:

	[chj @Win10BlueSSD /mnt/f/backup-chja20]
	$ ./do-irsync.sh | tee do-irsync.sh.log

