#include <stdio.h>
#include <stdarg.h>

#include "arg.h"
#include "io.h"

void
prnt(char *format, ...)
{
	if(quiet)
		return;

	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}

void
prnte(char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}
