#!/bin/bash

# User can set env-var IRSYNC_PARAMS and RSYNC_PARAMS to customize irsync's behavior.
# datetime_pattern=YYYYMMDD_hhmmss

export PYTHONPATH=/home/chj/gitw/pyutils/pycode
export SHDIR=${0%/*}

if [ -z "$datetime_pattern" ]; then
	datetime_pattern=YYYYMMDD
fi

_irsync_="--datetime-pattern=${datetime_pattern} --max-retry=1 --old-days=90"

_rsync_="--rsync --progress --exclude-from=${SHDIR}/excludes.list"

_has_error=FALSE
_err_list=()

if [ -z "$dirlist" ]; then
	# user can override dirlist from command line.
	dirlist=(/mnt/f/githubs /mnt/f/webui_forge_2024) # define a Bash array
fi

for onedir in "${dirlist[@]}"; do 
	echo ""
	echo "do-irsync.sh: Rsync backing up $onedir ..."
	python3 -m cheese.incremental_rsync.irsync_cmd $onedir . $_irsync_ $IRSYNC_PARAMS $_rsync_ $RSYNC_PARAMS
	if [ $? != 0 ]; then _has_error=TRUE; _err_list+=($onedir); fi
done

if [ "$_has_error" = FALSE ]; then
	exit 0;
else
	echo ""
	echo "do-irsync.sh: Some irsync task(s) end with error:"
	for value in "${_err_list[@]}"; do echo "  $value"; done
	exit 4;
fi
