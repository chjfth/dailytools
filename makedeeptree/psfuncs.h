#ifndef __psfuncs_h_
#define __psfuncs_h_

// Note: This header file is shared across Windows and Linux.

#include <stdio.h>
#include <string.h>

struct ErrMsg
{
	char *m_func;
	char *m_errmsg;

//	static const char *s_unknown = "Unknown_api_error";

	ErrMsg(const char *func, const char *errmsg) :
		m_func(nullptr), m_errmsg(nullptr)
	{
		int slen = (int)strlen(func);
		m_func = new char[slen+1];
		strcpy(m_func, func);

		slen = (int)strlen(errmsg);
		m_errmsg = new char[slen+1];
		strcpy(m_errmsg, errmsg);
	}

	~ErrMsg()
	{
		delete m_func;
		delete m_errmsg;
	}
};


#ifndef ARRAYSIZE
extern"C++"
template <typename TElement, int N>
char(*
	RtlpNumberOf(TElement(&rparam)[N])
	)[N];

#define ARRAYSIZE(arr)  ( sizeof(*RtlpNumberOf(arr)) )
#endif


unsigned int ps_GetMillisec();

void ps_create_dir_if_not_exist(const char *dirpath);


typedef void* filehandle_t;

filehandle_t ps_create_new_file(const char *filepath);
void ps_write_file(filehandle_t hfile, const void *pbytes, int nbytes);
void ps_close_file(filehandle_t hfile);

void ps_create_file_write_string(const char *filepath, const char *text);


#endif
