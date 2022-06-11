# User can set IRSYNC_PARAMS and RSYNC_PARAMS to customize irsync's behavior.

export CHJHOST=10.22.3.84
export PYTHONPATH=~/gitw/pyutils/pycode

#datetime_pattern=YYYYMMDD
datetime_pattern=$(echo $(date +%Y.%m.%d) | sed 's/.$/x/')
# -- Get current date, and replace final char with 'x' (e.g. 2022.06.1x) , 
# so that we create a backup every 10 days.

_irsync_="--datetime-pattern=${datetime_pattern} --max-retry=3 --old-days=60"

_rsync_="--rsync --progress --exclude-from=excludes.list"

_has_error=FALSE
_err_list=()

modlist=(myk mym myn myo-vms myd) # define a Bash array

for mod in "${modlist[@]}"; do 
	echo ""
	echo "do-irsync.sh: Doing rsync://$CHJHOST:1873/$mod ..."
	python3 -m cheese.incremental_rsync.irsync_cmd rsync://$CHJHOST:1873/$mod . $_irsync_ $IRSYNC_PARAMS $_rsync_ $RSYNC_PARAMS
	if [ $? != 0 ]; then _has_error=TRUE; _err_list+=($mod); fi
done

if [ "$_has_error" = FALSE ]; then
	exit 0;
else
	echo ""
	echo "do-irsync.sh: Some irsync task(s) end with error:"
	for value in "${_err_list[@]}"; do echo "  $value"; done
	exit 4;
fi

