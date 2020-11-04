
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
#	See https://www.evernote.com/shard/s21/sh/d28501d4-8a33-45b7-97e9-a8e63ce7b510/26d6a4eb1495c6af9dffc130f15ad567

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
