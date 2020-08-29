# User can set IRSYNC_PARAMS and RSYNC_PARAMS to customize irsync's behavior.

export CHJHOST=10.22.3.84
export PYTHONPATH=~/gitw/pyutils/pycode

_irsync_="--datetime-pattern=YYYYMMDD --old-days=30"
_rsync_="--rsync --progress --exclude-from=excludes.list"

modlist=(myd myk mym) # define a Bash array
	
for mod in "${modlist[@]}"; do 
	python3 -m cheese.incremental_rsync.irsync_cmd rsync://$CHJHOST:1873/$mod . $_irsync_ $IRSYNC_PARAMS $_rsync_ $RSYNC_PARAMS
	if [ $? != 0 ]; then exit 4; fi
done