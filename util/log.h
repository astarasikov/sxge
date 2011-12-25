#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#define err(fmt, x...) \
	fprintf(stderr, "error at %s:%d: " fmt "\n", __func__, __LINE__, ##x)

#endif
