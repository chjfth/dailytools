
# Set very beautiful and useful PS1 prompt for bash user:
if [ $UID != 0 ]; then PS1fmtuser='\u'; else PS1fmtuser='\[\033[1;31m\]\u\[\033[0m\]'; fi
PS1fmtcwd='[33m'
if [ $UID != 0 ]; then PS1char='$'; else PS1char='#'; fi
PS1_err_bell='$(if [ $? != 0 ];then echo -e \a; fi)'
PS1_TERM='$TERM'
PS1_DQ='$?'

_ShowOnErr1()
{
	# The input $1, is the actual value of $? from the outside world.
	if [ "$1" != 0 ]; then
		echo "ERR:$1"
	fi
	return $1
}

_ShowOnErr0()
{
	# The input $1, is the actual value of $? from the outside world.
	if [ "$1" = 0 ]; then
		echo "ERR:0"
	fi
	return $1 # relay the outside $? to _ShowOnErr0
}

_Ps1ChjPrefix()
{
	if [ "$PS1_chjprefixcmd" != "" ]; then
		echo "$(eval $PS1_chjprefixcmd) "
	fi
	
	# Relay the outside $? to caller, by returning $1 .
	# This is important trick. This is to ensure that, whenever Bash sees a '$?' inside the PS1, the '$?'
	# equals the initial $? value from outside. The "outside $?" is the $? value after the Bash user 
	# executes a shell command. For example, `ls` will produce $?=0, `ls non-exist-file` will produce $?=2 .
	return $1
}

_Ps1Jobs_not_working()
{
	# This does NOT work because the "\j" we return by echo will not get expanded by Bash.
	# Bash expanded \h \t \w \j etc BEFORE $(user command) is executed.
	if [ "$(jobs)" != "" ]; then
		echo "jobs:\j "
	fi
}

_Ps1Jobs()
{
	# Caller should pass in $1 for relaying purpose.
	if [ "$(jobs -r)" != "" ]; then
		# count `jobs -r` output lines, that is the running jobs count.
		running_jobs="$(jobs -r | wc -l)"
	else
		running_jobs=0
	fi
	if [ "$(jobs -s)" != "" ]; then
		# count stopped jobs
		stopped_jobs="$(jobs -s | wc -l)"
	else
		stopped_jobs=0
	fi
	
	if [ "$running_jobs" != 0 ] || [ "$stopped_jobs" != 0 ]; then
		echo "jobs:$running_jobs+$stopped_jobs "
	fi
	
	return $1
}

_Ps1HostExtra()
{
	# Usage: In user's ~/.bashrc , user can add 
	#	QLBOX_PS1_HOST_EXTRA=$WSL_DISTRO_NAME
	# so that, when the Bash is run from a WSL distro(WSL instance), 
	# current WSL instance name is displayed on the prompt string.
	# The helps user distinguish different WSL instances running on the same Windows host.
	
	if [ "$QLBOX_PS1_HOST_EXTRA" = "" ]; then
		echo ""
	else
		echo " ("$QLBOX_PS1_HOST_EXTRA")"
	fi
	return 0
}

# _ClearMyErr(){ return 0; } # [2020-11-04]no-effect on clearing inherited error
PS1_chj="\n[\[\033[1;37m\]\$(_Ps1ChjPrefix $PS1_DQ)\[\033[0m\]\[\033[32m\]`tty`($PS1_TERM) \D{%Y-%m-%d} \t \[\033[0;36m\]\$(_Ps1Jobs $PS1_DQ)\[\033[1;31m\]\$(_ShowOnErr1 $PS1_DQ)\[\033[0m\]\$(_ShowOnErr0 $PS1_DQ)]$PS1_err_bell\n[$PS1fmtuser @\h\$(_Ps1HostExtra) \[\033$PS1fmtcwd\w\033[0m\]]\n\[\033[1;37;44m\]$PS1char\[\033[0m\] "

PS1=$PS1_chj

# Hint for PS1_chjprefixcmd :
# User may set
#
#	PS1_chjprefixcmd='eval "echo PY-$(pyenv version-name)"'
#
# so to achieve a dynamic prefix like:
#	[PY-3.7.9 /dev/pts/5(screen-256color) 2021-04-18 13:09:43 ERR:0]
# or
#	[PY-3.9.3 /dev/pts/5(screen-256color) 2021-04-18 13:09:57 ERR:0]
