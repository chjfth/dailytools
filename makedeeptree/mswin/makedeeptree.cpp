#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "psfuncs.h"


#define TOOBIG (-1)

#define MAX_NODES (2*1000*1000)

#define FILENAME_MAXLEN 64

#define STRLEN(s) ((int)strlen(s))

#ifndef ARRAYSIZE
extern"C++"
template <typename TElement, int N>
char(*
	RtlpNumberOf(TElement(&rparam)[N])
	)[N];

#define ARRAYSIZE(arr)  ( sizeof(*RtlpNumberOf(arr)) )
#endif

int safe_int_power(int base, int power)
{
	// only deal with positive number.
	// return TOOBIG(-1) if overflow

	if(base<0 || power<0)
		return -1;

	if(base==0)
		return 0;

	int threshold = INT_MAX / power;

	int result = 1;

	for(int i=0; i<power; i++)
	{
		result *= base;

		if(result>threshold || result<0)
			return TOOBIG; // overflow
	}

	return result;
}

enum StartDone_et { NodeStart = 0, NodeDone = 1 };

typedef void PROC_CreateNode(StartDone_et sd, const char *dirnow, void *userctx);

struct WorkParam_st
{
	int ndepth;
	int nsibling_dirs;
	int nsibling_files;
};

int generate_stemname(char *buf, int bufsize, int ndepth, int nfile)
{
	// nfile is zero-based
	// Example: 
	//	ndepth=1, nfile=0 : "deep1a"  ('a' is just like decimal-world '0')
	//	ndepth=1, nfile=1 : "deep1b"
	//	ndepth=1, nfile=26 : "deep1ba"
	//	ndepth=1, nfile=28 : "deep1bc"
	//
	// return generated stemname length

	assert(ndepth > 0 && nfile >= 0);

	char tail[40] = {};

	int i = 0;
	do
	{
		int remainder = nfile % 26;
		nfile /= 26;

		tail[i++] = remainder + 'a';

	} while (nfile > 0);

	tail[i] = '\0';

	_strrev(tail);

	snprintf(buf, bufsize, "deep%d%s", ndepth, tail);
	
	return STRLEN(tail);
}

void r_make_one_dirnode(WorkParam_st &workparam, const char *nowdir, int nowdepth,
	PROC_CreateNode *userproc, void *userctx)
{
	const char *thisfunc = "r_make_one_dirnode";

	// Starting param: nowpath="d:\mytree" , nowdepth=0 .
	// so "d:\mytree\deep1a" is the depth==1 dirnode. 

	if(userproc)
		userproc(NodeStart, nowdir, userctx);

	ps_create_dir_if_not_exist(nowdir);

	int nowdir_len = STRLEN(nowdir);

	char *newpathbuf = new char[nowdir_len + FILENAME_MAXLEN + 2];
	if (!newpathbuf)
		throw ErrMsg(thisfunc, "C++-new fails. No memory?!");

	snprintf(newpathbuf, nowdir_len+2, "%s" PATH_SEP_CHAR, nowdir);
	char *pfilename = newpathbuf + (nowdir_len + 1);

	int i = 0;
	for(i=0; i<workparam.nsibling_files; i++)
	{
		// create a file node
		int stemlen = generate_stemname(pfilename, FILENAME_MAXLEN+1, nowdepth+1, i);

		// append extname:
		snprintf(pfilename+stemlen, FILENAME_MAXLEN+1-stemlen, ".txt");
		// printf("File: %s.txt\n", newpathbuf);
		ps_create_file_write_string(newpathbuf, pfilename);
	}

	if(nowdepth < workparam.ndepth)
	{
		char *pnewdirname = newpathbuf + (nowdir_len + 1);
		for (i = 0; i < workparam.nsibling_dirs; i++)
		{
			// recursively create subdir node
			int stemlen = generate_stemname(pnewdirname, FILENAME_MAXLEN+1, nowdepth+1, i);

			// printf("Dir : %s\n", newpathbuf);
			r_make_one_dirnode(workparam, newpathbuf, nowdepth+1, userproc, userctx);
		}
	}

	if(userproc)
		userproc(NodeDone, nowdir, userctx);

	delete newpathbuf;
}


struct ProgressCtx_st
{
	int ntotal; // total dir nodes
	int nstart; // started dir nodes
	int ndone;  // done dir nodes
};

void Notify_CreateNode(StartDone_et sd, const char *dirnow, void *userctx)
{
	ProgressCtx_st &ctx = *(ProgressCtx_st*)userctx;

	if (sd == NodeStart)
		ctx.nstart++;
	else if (sd == NodeDone)
		ctx.ndone++;

	if(ctx.ndone%1000==0 || ctx.ndone==ctx.ntotal)
	{
		printf("\rTotal %d, Started %d , Done %d (%d%%) ...",
			ctx.ntotal, ctx.nstart, ctx.ndone, ctx.ndone * 100 / ctx.ntotal);
	}
}

int count_total_dirnodes(int nsibling_dirs, int ndepth)
{
	if (ndepth < 0 || ndepth==INT_MAX)
		return TOOBIG;

	if (nsibling_dirs == 1)
		return ndepth + 1;
	else
	{
		// Geometric sum of nsiblings^0 + nsiblings^1 + nsiblings^2 ...
		// until nsiblings^ndepth .

		int pw = safe_int_power(nsibling_dirs, ndepth + 1);
		if (pw == TOOBIG)
			return TOOBIG;

		return (pw - 1) / (nsibling_dirs - 1);
	}
}

void makedeeptree(const char *rootdir, int nsibling_dirs, int ndepth, int nsibling_files)
{
	int dir_nodes = count_total_dirnodes(nsibling_dirs, ndepth);
	if (dir_nodes == TOOBIG)
	{
		printf("Tree too large(probably power overflow).\n");
		exit(2);
	}
	else if (dir_nodes > MAX_NODES)
	{
		printf("[ERROR] Tree too large(%d nodes), max allowed is %d\n", dir_nodes, MAX_NODES);
		exit(2);
	}

	int total_files = dir_nodes * nsibling_files;
	if(total_files<0)
	{
		printf("[ERROR] You are creating two many files (> two billion).\n");
		exit(2);
	}

	printf("[dir_sibs=%d, deep=%d] Creating total %d dir nodes and %d files.\n", 
		nsibling_dirs, ndepth, dir_nodes, total_files);

	WorkParam_st wp = {};
	wp.ndepth = ndepth;
	wp.nsibling_dirs = nsibling_dirs;
	wp.nsibling_files = nsibling_files;

	ProgressCtx_st ctx = {};
	ctx.ntotal = dir_nodes;

	printf("\n");
	r_make_one_dirnode(wp, rootdir, 0, Notify_CreateNode, &ctx);
}

void print_help()
{
	const char *helptext =
"makedeeptree v1.0\n"
"Usage:\n"
"    makedeeptree <rootdir> [dir_sibs] [deep] [file_sibs]\n"
"Example (as default values):\n"
"\n"
"    makedeeptree d:\\mytree 3 2 1\n"
"\n"
"This will create 2 levels of subdirs inside d:\\mytree .\n"
"Each dirnode will contain 3 sub-dirs and 4 filenodes.\n"
"So the result will be:\n"
"    d:\\mytree \n"
"    d:\\mytree\\deep1a \n"
"    d:\\mytree\\deep1a\\deep2a \n"
"    d:\\mytree\\deep1a\\deep2b \n"
"    d:\\mytree\\deep1a\\deep2c \n"
"    d:\\mytree\\deep1b \n"
"    d:\\mytree\\deep1b\\deep2a \n"
"    d:\\mytree\\deep1b\\deep2b \n"
"    d:\\mytree\\deep1b\\deep2c \n"
"    d:\\mytree\\deep1c \n"
"    d:\\mytree\\deep1c\\deep2a \n"
"    d:\\mytree\\deep1c\\deep2b \n"
"    d:\\mytree\\deep1c\\deep2c \n"
"\n"
"Each above dirnode will have extra four files.\n"
	;
	printf(helptext);

/* Python counting code, Geometric sum:

def dirnodes(nsibs, ndepth):
	return (nsibs**(ndepth+1)-1) // (nsibs-1)
*/
}


int main(int argc, char* argv[])
{
	int nsibling_dirs = 3;
	int ndepth = 2;
	int nsibling_files = 1;

	if(argc<2)
	{
		print_help();
		exit(1);
	}

	const char *dirpath = argv[1];
	if(!dirpath[0]) {
		printf("Empty input dirpath.\n");
		exit(1);
	}

	if(argc>2) 
		nsibling_dirs = atoi(argv[2]);
	
	if(argc>3)
		ndepth = atoi(argv[3]);

	if(argc>4)
		nsibling_files = atoi(argv[4]);

	try 
	{
		unsigned int msec_start = ps_GetMillisec();
		
		makedeeptree(dirpath, nsibling_dirs, ndepth, nsibling_files);
		
		unsigned int msec_used = ps_GetMillisec() - msec_start;
		printf("\n");
		printf("Success. Time cost: %u millisec.\n", msec_used);

	}
	catch(const ErrMsg& e)
	{
		printf("\n");
		printf("====Error occured====\n");
		printf("In function: %s\n", e.m_func);
		printf("Message: %s\n", e.m_errmsg);
		exit(2);
	}
	
	return 0;
}

