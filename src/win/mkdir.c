#include <direct.h>
#include "../csv.h"
#include "../pl.h"
#include "../copy.h"


int
makedir(char *path, int rights)
{
	return _mkdir(path);
}
