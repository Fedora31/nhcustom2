#include <direct.h>

int
makedir(char *path, int rights)
{
	return _mkdir(path);
}
