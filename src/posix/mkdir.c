#include <sys/stat.h>
#include "../csv.h"
#include "../pl.h"
#include <stack.h>
#include "../copy.h"


int
makedir(char *path, int rights)
{
	return mkdir(path, rights);
}
