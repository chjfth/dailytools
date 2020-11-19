# This rsync client takes local machine as server, so there will be only local-to-local file copy.
# User can set IRSYNC_PARAMS and RSYNC_PARAMS to customize irsync's behavior.

export SRCROOT=/mnt
export PYTHONPATH=~/gitw/pyutils/pycode
modlist=(d) # define a Bash array

_irsync_="--datetime-pattern=YYYYMMDD --max-retry=3 --old-days=30"
_rsync_="--rsync --progress --exclude-from=excludes.list"

meet_err=0
	
for mod in "${modlist[@]}"; do 
	echo ""
	echo "do-irsync.sh from $SRCROOT/$mod ..."
	python3 -m cheese.incremental_rsync.irsync_cmd $SRCROOT/$mod . $_irsync_ $IRSYNC_PARAMS $_rsync_ $RSYNC_PARAMS
	if [ $? != 0 ]; then meet_err=1; fi
done

exit $meet_err
