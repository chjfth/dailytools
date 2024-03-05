
# List env-vars matching some starting words.
# "lset" implies "linux set". You know, 'set PA' on Windows list all env-var 
# whose name start with PA .
lset () 
{ 
	for v in "$@";
	do
		env | grep "^$v";
	done
}

function addpath()
{
  PATH="$1:$PATH"
}

alias lpath="python -c \"for p in __import__('os').environ['PATH'].split(':'): print(p)\""

function L()
{
  # Usage example: 
  # L PATH
  # L LD_LIBRARY_PATH
  python -c "for p in __import__('os').environ['$1'].split(':'): print(p)"
}


hexadd0x() { echo "$1" | sed -r 's:( *)([0-9A-Fa-f]{2}): 0x\2:g'; }
# $ hexadd0x '35E7 94   b5'
# 0x35 0xE7 0x94 0xb5
#	See https://www.evernote.com/l/ABXShQHUijNFt5fpqOY857UQJtak6xSVxq8/

run-to-death()
{
	local i=0
	while true; do
		i=$(($i+1))
		echo "[running $i] $(date +%Y-%m-%d_%H:%M:%S) STY=$STY WINDOW=$WINDOW"
		sleep 1
	done
}

if [ ! -f ~/.inputrc ]; then
	cp "${BASH_SOURCE%/*}"/chj.input.rc ~/.inputrc
fi

pid-args()
{
	if [ "$1" = "" ]; then 
		echo "Usage: Pass a pid as parameter, to show its argv[] values, one line each."
		return 1
	fi
	
	cat /proc/$1/cmdline | xargs -0 printf "%s\n"; echo
}

pid-envs()
{
	if [ "$1" = "" ]; then 
		echo "Usage: Pass a pid as parameter, to show its environ[] values, one line each."
		return 1
	fi
	
	cat /proc/$1/environ | xargs -0 printf "%s\n"; echo
}

function diffdays()
{
	comment="
# Show date difference between two days.
# Example:
#	$ diffdays 040130 040203
#	2004-01-30 -> 2004-02-03
#	Difference: 4 days
#
# If a input is only four digits, it will be MMDD of this year.
# Second parameter is optional, default to today.
	"
	
	start_date=$1
	end_date=$2
	
	if [ -z "$2" ]; then end_date=$(date +%Y-%m-%d); fi;

	year=$(date +%Y)
	if [ ${#start_date} -eq 4 ]; then start_date=$year$start_date; fi
	if [ ${#end_date} -eq 4 ]; then end_date=$year$end_date; fi
	
	echo "$(date -d "$start_date" +%Y-%m-%d) -> $(date -d "$end_date" +%Y-%m-%d)"
	
	date_diff=$(( ($(date -d "$end_date" +%s) -$(date -d "$start_date" +%s)) / 86400 ))
	echo "Difference: $date_diff days"
}
