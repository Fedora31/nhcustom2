#include <sys/stat.h>


int
makedir(char *path, int rights)
{
	return mkdir(path, rights);
}
