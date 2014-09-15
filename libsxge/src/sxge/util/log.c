#include <sxge/util/log.h>
#include <stdio.h>
#include <stdarg.h>

enum {
	SXGE_LOG_SIZE = 512,
};

void
sxge_log(enum sxge_log_level level, const char *format, ...)
{
	char buf[SXGE_LOG_SIZE];
	va_list args;
	(void)level;
	va_start(args, format);
	int count = vsnprintf(buf, SXGE_LOG_SIZE, format, args);
	va_end(args);

	if (count >= 0 && count < SXGE_LOG_SIZE) {
		buf[count] = '0';
		fwrite(buf, count, 1, stderr);
	}
}
