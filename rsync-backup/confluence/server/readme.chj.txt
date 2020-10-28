20200320.4 

On Window Server, we can use following command to install a cygwin program as a Windows Service.

	cygrunsrv -I "Rsync Daemon" --chdir /cygdrive/d/rsync-cfg -p /bin/rsync.exe -a "--daemon --config=rsyncd.conf --no-detach --log-file rsync.log"

So, rsyncd.conf should have been prepared in D:\rsync-cfg , and there will be D:\rsync-cfg\rsyncd.conf

======Memo======
Service key at:
[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Rsync_Daemon]

rsync exe parameter at:
[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Rsync_Daemon\Parameters]
"AppPath"="/bin/rsync.exe"
"AppArgs"="--daemon --config=rsyncd.conf --no-detach --log-file rsync.log"
"WorkingDirectory"="/cygdrive/d/rsync-cfg"	

