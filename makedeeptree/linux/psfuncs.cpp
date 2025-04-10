#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>

#include "../psfuncs.h"


unsigned int ps_GetMillisec()
{
	struct timespec ts;
	if(clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
		return 0; // 0 means fail!
	}
	
	long long ret = ts.tv_sec*1000 + ts.tv_nsec/1000000;
	return ret==0 ? 1 : (unsigned int)ret;
}

void ps_create_dir_if_not_exist(const char *dirpath)
{
	const char *thisfunc = "ps_create_dir_if_not_exist";

	char errmsg[4000] = {};
	struct stat info = {};
	
	// Check if path exists
	if (stat(dirpath, &info) == 0) 
	{
		// Path exists, check if it's a directory
		if (S_ISDIR(info.st_mode)) 
		{
			// It's a directory, nothing to do
			return;
		} 
		else 
		{
			snprintf(errmsg, ARRAYSIZE(errmsg),
				"Error: I see an existing path on disk that is not a directory:\n"
				"    %s\n"
				, dirpath);
			throw ErrMsg(thisfunc, errmsg);
		}
	} 
	else 
	{
		// Path doesn't exist, check if error was ENOENT (doesn't exist)
		if (errno == ENOENT) 
		{
			// Try to create directory with 0755 permissions (rwxr-xr-x)
			if (mkdir(dirpath, 0755) != 0) 
			{
				snprintf(errmsg, ARRAYSIZE(errmsg), 
					"CreateDirectory(\"%s\") fail with errno=%d, %s", dirpath, 
					errno, strerror(errno));
				throw ErrMsg(thisfunc, errmsg);
			}
		} 
		else 
		{
			snprintf(errmsg, ARRAYSIZE(errmsg),
				"Panic! stat(\"%s\") reports error! errno=%d, %s",
					dirpath, errno, strerror(errno));
			throw ErrMsg(thisfunc, errmsg);
		}
	}
}

filehandle_t ps_create_new_file(const char *filepath)
{
	const char *thisfunc = "ps_create_new_file";
	char errmsg[4000] = {};

	// First check if path exists and is a directory
	struct stat info = {};
	
	if (stat(filepath, &info) == 0) 
	{
		if (S_ISDIR(info.st_mode)) 
		{
			snprintf(errmsg, ARRAYSIZE(errmsg),
				"Unexpect! There is a directory in the way that prevents us creating it as a file.\n"
				"filepath: %s"
				, filepath);
			throw ErrMsg(thisfunc, errmsg);
		}
	}

	// Open file with O_CREAT | O_WRONLY | O_TRUNC
	// Mode 0644 (rw-r--r--)
	int fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1) {
		snprintf(errmsg, ARRAYSIZE(errmsg),
			"[ERROR] Creating new file via open(\"%s\") fails, errno=%d, %s",
			filepath, errno, strerror(errno));
		throw ErrMsg(thisfunc, errmsg);
	}
	
	return (void*)(intptr_t)fd;
}

void ps_write_file(filehandle_t hfile, const void *pbytes, int nbytes)
{
	const char *thisfunc = "ps_write_file";
	char errmsg[4000] = {};
	
	int fd = (intptr_t)hfile;
	int nbWritten = write(fd, pbytes, nbytes);
	if(nbWritten!=nbytes)
	{
		snprintf(errmsg, ARRAYSIZE(errmsg),
			"[ERROR] write(fd=%d) fails, errno=%d, %s",
			fd, errno, strerror(errno));
		throw ErrMsg(thisfunc, errmsg);
	}
}

void ps_close_file(filehandle_t hfile)
{
	close((intptr_t)hfile);
}


void ps_create_file_write_string(const char *filepath, const char *text)
{
	filehandle_t hfile = ps_create_new_file(filepath);
	ps_write_file(hfile, text, (int)strlen(text));
	ps_close_file(hfile);

}


