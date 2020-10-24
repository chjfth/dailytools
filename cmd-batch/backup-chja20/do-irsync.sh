# User can set IRSYNC_PARAMS and RSYNC_PARAMS to customize irsync's behavior.

export CHJHOST=10.22.3.84
export PYTHONPATH=~/gitw/pyutils/pycode

_irsync_="--datetime-pattern=YYYYMMDD --max-retry=3 --old-days=60"
_rsync_="--rsync --progress --exclude-from=excludes.list"

modlist=(myd myk mym myn) # define a Bash array
	
for mod in "${modlist[@]}"; do 
	echo ""
	echo "do-irsync.sh: Doing rsync://$CHJHOST:1873/$mod ..."
	python3 -m cheese.incremental_rsync.irsync_cmd rsync://$CHJHOST:1873/$mod . $_irsync_ $IRSYNC_PARAMS $_rsync_ $RSYNC_PARAMS
	if [ $? != 0 ]; then exit 4; fi
done
