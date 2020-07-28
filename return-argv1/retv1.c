#include <stdlib.h>

int main(int argc, char **argv)
{
	int ret = 0;

	if(argc==1)
		return 0;

	ret = atoi(argv[1]);
	return ret;
}
