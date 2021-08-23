#!/bin/sh

# [2021-08-22] Prerequisites:
# 1. bepracs.com is the server machine that holds SVN repos in his /var/lib/svn .
# 2. At rsync client side, we must have SSH private key in our ~/.ssh/id_rsa ,
#    so that SSH client(invoked by rsync client) can use that private key to
#    authenticate us to the server machine.
# 3. In order to human-type-in private-key's passphrase only one time, 
#    we have to add 
#    	AddKeysToAgent=yes
#    to
#    	~/.ssh/config
#    so that ssh client can register private-key info into ssh-agent process
#    once he receives our passphrase.

BACKUP_TARGET_DIR=$1
if [ "$BACKUP_TARGET_DIR" = "" ]; then
	echo "Error: No backup target directory parameter assigned."
	exit 4
fi

export SSH_AUTH_SOCK=/tmp/chj.single.ssh-agent
ssh-agent -a ${SSH_AUTH_SOCK}

# Note: This rsync command will create a chjsvn subfolder in current directory.
# So, I recommended you have a wrapper script that sets the current directory.
rsync -av --inplace chj@bepracs.com:/var/lib/svn/ "$BACKUP_TARGET_DIR"

# PENDING Q: Current user must be 'chj'?
