#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
void _int_err(const char *format, ...) {
	fprintf(stderr, "_int_err %d ", errno);
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputc('\n', stderr);
}
