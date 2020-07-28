#include <stdio.h>
#include <string.h>
#include <dirent.h>

int
main(int argc, char *argv[])
{
	DIR             *dp;
	struct dirent   *dirp;

	if (argc != 2)
	{
		printf("usage: enum_indir <directory_name>\n");
		return 1;
	}

	if ((dp = opendir(argv[1])) == NULL)
	{
		printf("opendir(\"%s\") fail!\n", argv[1]);
		return 1;
	}

	int i;
	for(i=0; (dirp = readdir(dp)) != NULL; i++)
	{
		printf("[%d]: %lld bytes in filename\n", i+1, (long long)strlen(dirp->d_name));
 		printf("%s\n", dirp->d_name);

		int j;
		for(j=0; dirp->d_name[j]; j++)
			printf("%02X ", (unsigned char)dirp->d_name[j]);

		printf("\n\n");
	}

	closedir(dp);
	return 0;
}

/* [2006-10-03] Motivation of writing this program:
	I'm trying to use Linux's smbmount to mount a network share from 
 a Windows 2000 machine, but problem arises that the Chinese filenames
 from the Win2k machine always fail to show up in Linux's console.
 From the man page of smbmount, I know that proper usage of smbmount's 
 option `iocharset' and `codepage' is the key to solution, however, the 
 meaning of the very two options is really shrouded in mystery.
 Therefore, in order to dig one more step deeper, I have to know 
 what byte-sequence is given out by kernel for those Chinese filename.
 So comes this program.

 [2006-10-05] I've finally got the answer: 
	If you use smbmount to mount a Windows network share, Windows will
 always transfer MBCS filenames in SMB packets, and the MBCS filenames
 are always translated by the Windows machine's system codepage. So, the
 user use `codepage' options to tell smbmount(NOT make smbmount control)
 what codepage the Windows machine will use to translate Unicode chars
 to MBCS chars.
	Finally, if you want true Unicode chars sent in SMB packets, you 
 should use command `mount.cifs' provided by Samba version 3 instead of
 smbmount. Since mount.cifs causes the Windows SMB server to transfer
 Unicode in SMB packets, there is no `codepage' options for mount.cifs .
*/
