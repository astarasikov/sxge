#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#define err(fmt, ...) \
	fprintf(stderr, "error at '%s':%d: " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)

#if 0
#define info(fmt, ...) \
	fprintf(stdout, fmt "\n" ##__VA_ARGS__)

#define dbg(fmt, ...) \
	fprintf(stderr, fmt "\n" ##__VA_ARGS__)
#else
#define info err
#define dbg err
#endif

#endif
