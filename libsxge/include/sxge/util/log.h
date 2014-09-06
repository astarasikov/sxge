#ifndef __LOG_H__
#define __LOG_H__

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

enum sxge_log_level {
	SXGE_LOG_FATAL,
	SXGE_LOG_ERROR,
	SXGE_LOG_INFO,
	SXGE_LOG_DEBUG,
};

extern void
sxge_log(enum sxge_log_level level,
const char *format, ...) __attribute__((format(printf, 2, 3)));

#define sxge_err(fmt, ...) \
	sxge_log(SXGE_LOG_ERROR, "error at '%s':%d: " fmt "\n", \
		__func__, __LINE__, ##__VA_ARGS__)

#define sxge_info(fmt, ...) \
	sxge_log(SXGE_LOG_INFO, fmt "\n", ##__VA_ARGS__)

#define sxge_dbg sxge_err

#ifdef __cplusplus
} //extern "C"
#endif //__cplusplus

#endif
