export RSYNC_URL=rsync://cfuser@192.168.5.181/confluence
export RSYNC_PASSWORD=passcfu
export TARGET_DIR=.

export PYTHONPATH=~/gitw/pyutils/pycode
python3 -m cheese.incremental_rsync.irsync_cmd "$RSYNC_URL" "$TARGET_DIR" --datetime-pattern=YYYYMMDD --old-days=30 --fetch-finish-dir-from=finishdir.txt --finish-dir-relative --rsync --progress