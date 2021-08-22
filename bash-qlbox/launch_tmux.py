#!/usr/bin/env python
# -*- coding: UTF-8 -*-

"""
Requirment:
* Python 2.6 or Python 3.5+
* tmux 1.3 and above (up-to at least 3.0)

This program "launch" tmux session with the following rules:

This rule(we) picks one of the two cases and acts accordingly.

Before checking, we query ``tmux ls'' to know how many tmux sessions currently exist.

CASE 1: 

  No session exists yet.

  Action: exec ``tmux new -s <sessname>`` to create a new session.
  Prompt: [banner_first_session] .

CASE 2:

  There exist one or more sessions.

  Action: List all existing sessions(no matter attached or detached),
  	and user can:
	[1] Choose which one to attach
	[2] Answer A to create another new session.
	[3] Answer 0 to cancel tmux launch (stay in normal shell).
  Prompt: [banner_second_session] .

特别提示：
如果客户当前的连接特性不是一个“终端”(比如使用 WinSCP 作客户端)，那么不能调用此该程序，
否则此程序会卡在 raw_input() 上一直等待用户输入（但永远等不到）。
.
建议的做法：在 Bash 脚本中调用此 py 之前进行判断：

if [ ! -t 0 ]; then
	# 不要调用此 py 。注: -t 后的 0 表示 STDIN
fi

=========
调试注意事项：
* 在终端上输出调试信息用 sys.stderr.write 而非用 print ，因为后者是用来给父进程捕获的。

"""

import os
import sys
import re
import getopt
import shlex
import time
import subprocess
import traceback

is_python2 = sys.version_info.major==2

#g_is_interactive = False
g_tmuxconf_filepath = ''

def subprocess_check_output(*popenargs, **kwargs):
	# This is from Python 2.7 subprocess.py, so that Python 2.6 can use it too.
    if 'stdout' in kwargs:
        raise ValueError('stdout argument not allowed, it will be overridden.')
    process = subprocess.Popen(stdout=subprocess.PIPE, *popenargs, **kwargs)
    output, unused_err = process.communicate()
    retcode = process.poll()
    if retcode:
        cmd = kwargs.get("args")
        if cmd is None:
            cmd = popenargs[0]
        cpe = subprocess.CalledProcessError(retcode, cmd)
        cpe.output = output
        raise cpe
    return output


class CSessinfo:
	# one screen session info
	
	Attached = 1
	Detached = 0
	
	def __init__(self, display_line):
		"""
		display_line is like:
		
			chj2s: 2 windows (created Sun Apr  8 17:26:40 2012) [100x35]
			panes: 3 windows (created Sun Apr  8 12:10:19 2012) [100x35] (attached)
		
		Note: detached lines are not marked with "(detached)".
		"""
		line_format = r'(.+?): ([0-9]+) windows \(created (.+?)\)[^\(\)]+((?:\(attached\))?)'
		r = re.search(line_format, display_line)
		if not r:
			raise "Session info line does not match regex '%s'"%(line_format)
		
		self.sessname = r.group(1)
		
		try:
			self.windows = int(r.group(2))
		except:
			self.windows = 0
		if self.windows == 0:
			raise "Unexpect: 'windows' number missing from the session line info."
		
		self.ctime = time.strptime(r.group(3))
		self.status = self.Attached if r.group(4)=='(attached)' else self.Detached
	

def get_all_sessions():
	"""
	Run ``tmux ls`` to get all session info. Output sample:
	
chj2s: 2 windows (created Sun Apr  8 17:26:40 2012) [100x35]
fromkde: 1 windows (created Sun Apr  8 19:31:42 2012) [100x35]
panes: 3 windows (created Sun Apr  8 12:10:19 2012) [100x35] (attached)
	
	"""
	cmd_tmuxls = 'tmux ls'
	try:
		Output=subprocess_check_output(shlex.split(cmd_tmuxls), 
			stderr=subprocess.STDOUT,
			stdin=subprocess.PIPE) 
		"""
				[2012-04-12] A strange thing here: If I omit stdin=PIPE, nlssuse114-x86 and nlssuse114-x64 
				will exhibit a strange behavior.
				
				After subprocess_check_output(['tmux', ls'], ...), later raw_input will assert

				python reading..Traceback (most recent call last):
				  File "./t2.py", line 13, in <module>
				    raw_input('python reading..')
				EOFError

				Using strace to peek inside, we see a line like:
				
					read(0, 0xb72a5000, 1024) = -1 EAGAIN (Resource temporarily unavailable)
				
				But, if we subprocess_check_output('ls', ...), this problem does not show up.
				
				As a workaround, I add stdin=subprocess.PIPE, so the 'real' stdin is not affected by 'tmux ls'
		"""
	except OSError as errinfo:
		print("Cannot execute '%s' command. Perhaps tmux is not installed on the server."%(cmd_tmuxls))
		exit(4)
	except subprocess.CalledProcessError as cpe: # Strange: cannot capture this exception from subprocess.CalledProcessError from subprocess_check_output.
		# Typical error:
		#	server not found: Connection refused 
		# that means no tmux session exists yet.
		return []
		#print "Error: '%s' execution fail, exit code is %d. Output is:\n%s"%(cmd_tmuxls, cpe.returncode , cpe.output)
		#exit(6)
	
	
	sess_infos = []
	output_str = Output if is_python2 else Output.decode('utf8')
	lines = str(output_str).strip().split('\n')
	for line in lines:
		try:
			sess_info = CSessinfo(line)
			sess_infos.append(sess_info)
		except:
			pass

	return sess_infos


def export_banner_env(banner_str, sessname):
	# A trick: replace \n to | , because \n cannot be env-var's value.
	#os.environ['launch_tmux_banner'] = banner_str.replace('\n', '|')
	# // <= not feasible, because the new tmux session is not the child-process of its launcher.
	open('/tmp/tmux.banner.%s'%os.environ['USER'], "w").write(banner_str)
		# note: change launch_tmux_aux.sc accordingly if this file get a new name.
	# Why sessname is not used?
	# I tried, but I can't use it until I find a way to have a bash in tmux session get its own name.
	

def gen_session_name():
	# Use login user-name and current timestamp as session name.
	timenow = time.time()
	tm = time.localtime(timenow)
	sessname = "%s-%02d%02d_%02d%02d%02d"%(os.environ['USER'],
		tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec)
	return sessname

def shcmd_create_new_session(sessname):
	tmuxconf = '-f "%s"'%(g_tmuxconf_filepath) if g_tmuxconf_filepath else ''
	return 'tmux %s new -s '%(tmuxconf)+sessname
	

def shcmd_attach_a_session(sessname):
	return 'tmux attach -t '+sessname

def banner_list_sessions(sess_infos):
	# Return a multiline string as result.
	linefmt = "  %d. [%s] %d windows (created: %s) %s"
	strret = ''
	for i,s in enumerate(sess_infos):
		linestr = linefmt%(
			i+1,
			s.sessname, 
			s.windows, 
			time.strftime("%Y-%m-%d %H:%M:%S", s.ctime),
			"(detached)" if s.status==CSessinfo.Detached else "(using)" )
		strret += linestr+'\n'
	return strret

banner_first_session ="""
==============================================================================
 Welcome! This is your first *tmux* session on this server.
 No need to type 'exit' when you finish, just close your terminal window.
 When you log in next time, your session will be automatically resumed.
 
 Type command 'tmuxhelp' to know what you can do with tmux.
==============================================================================



"""

banner_second_session = """==============================================================================
 You have %d existing tmux sessions. You can freely choose which to attach.
%s
 Hotkey for tmux session management:
 * F12    Choose another session to attach.
 * Execute 'srename <newname>' to rename current session to your preference.
==============================================================================

"""

template_session_choice = """
==============================================================================
 You have existing sessions on this server:
%s
==============================================================================
"""

def create_first_session():
	sessname = gen_session_name()
	export_banner_env(banner_first_session, sessname)
	shcmd = shcmd_create_new_session(sessname)
	return shcmd
	
def create_one_more_sesssion(sess_infos): 
	# memo: Diff between create_first_session & create_one_more_sesssion is their banner style.
	
	sessname = gen_session_name()
	forge_sess_display = "%s: 1 windows (created %s) [xxx,yyy] (attached)"%(
		sessname, time.asctime( time.localtime(time.time()) ) )
	
	sess_infos.append( CSessinfo(forge_sess_display) )
	
	b3 = banner_second_session % (len(sess_infos), banner_list_sessions(sess_infos))
	export_banner_env(b3, sessname)
	shcmd = shcmd_create_new_session(sessname)
	return shcmd


def main():
	
	global g_tmuxconf_filepath
	optlist, arglist = getopt.getopt(sys.argv[1:], '', ['tmuxconf='])
	optdict = dict(optlist)

	if '--tmuxconf' in optdict:
		g_tmuxconf_filepath = optdict['--tmuxconf']

	sess_infos = get_all_sessions()
	attaches = sum(1 for s in sess_infos if s.status==CSessinfo.Attached)
	detaches = sum(1 for s in sess_infos if s.status==CSessinfo.Detached)
	nsess = attaches+detaches
	first_detached_sess = next( (s for s in sess_infos if s.status==CSessinfo.Detached), None)

#	sys.stderr.write( "attaches:%d detaches:%d\n"%(attaches, detaches) ) # debug
#	exit(0)

	shcmd = ''
	if nsess==0:
		shcmd = create_first_session()
		
	else:
		choice_prompt1 = template_session_choice % (banner_list_sessions(sess_infos))
		sys.stderr.write(choice_prompt1)
		
		while True: # user-input cycle
			if nsess==1:
				choice_prompt2 = "Answer 'A' to create a new session, or '1' to attach to the existing one:"
			else:
				choice_prompt2 = "Answer 'A' to create a new session, or pick an existing one(0,%d...%d):"%(1, nsess)
			sys.stderr.write(choice_prompt2)
		
			try:
				inputc = raw_input() if is_python2 else input()
				idx = int(inputc)-1
			except ValueError: # inputc is not a number
				idx = 4444
			except KeyboardInterrupt: # user press Ctrl+C
				break
			
			if inputc in ['a', 'A']:
				shcmd = create_one_more_sesssion(sess_infos)
				break
			elif idx>=0 and idx<nsess:
				shcmd = shcmd_attach_a_session(sess_infos[idx].sessname)
				break
			elif idx==-1:
				break
			elif inputc == '' and detaches>0:
				# Choose the first detached one automatically.
				shcmd = shcmd_attach_a_session(first_detached_sess.sessname)
				break
	
#	sys.stderr.write( 'shcmd='+shcmd + '\n' ) # debug
	print(shcmd)
		# The caller(bash script) will capture this output and execute it as command.
	
	return 0 # success

if __name__ == '__main__':
    ret = main()
    exit(ret)

