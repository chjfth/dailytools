#ifndef __psfuncs_h_
#define __psfuncs_h_

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

#define PATH_SEP_CHAR "\\"

unsigned int ps_GetMillisec();

void ps_create_dir_if_not_exist(const char *dirpath);

void ps_create_file_write_string(const char *filepath, const char *text);


#endif
