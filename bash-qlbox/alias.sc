
alias lpath='l.py PATH'
alias lso='l.py LD_LIBRARY_PATH'

alias sl='tmux ls'

alias la='ls --color -laF --time-style="+%Y-%m-%d %H:%M:%S"'
alias ll='ls -l'

# alias cp='cp -i'
# alias ret='echo $?'

# alias cd='pushd > /dev/null'

alias sev='export|grep $1'

alias svn-setexe='svn propset svn:executable yes'
alias svn-native='svn propset svn:eol-style native'
alias svn-lf='svn propset svn:eol-style LF'

# Special tweaks for macOS
if [ "$(uname)" = "Darwin" ]; then
  # -G: ls color output ; -T: full timestamp
  alias ls='ls -GFh'
  alias la='ls -laT'
fi
