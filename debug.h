/*
* IncFile1.h
*
* Created: 04/11/2012 8:20:36 PM
*  Author: michal
*/

#ifndef DEBUG_H_
#define DEBUG_H_

#include "project.h"

void runtime_msg(const char *msg);
void runtime_debug(const char *msg1, const char *msg2);
void runtime_error(const char *msg1, const char *msg2);

// error handler
#define RUNTIME_DEBUG(error) runtime_debug(error, (const char *)__func__);

#define RUNTIME_DEBUG_VA(...) { char __buf[40]; \
snprintf(__buf, sizeof(__buf)-1, __VA_ARGS__); \
__buf[sizeof(__buf)-1] = 0; \
  runtime_debug(&__buf[0], (char *)__func__); }

#define RUNTIME_ERROR(error) runtime_error(error, (const char *)__func__);

#define RUNTIME_MSG(msg) runtime_msg(msg);

#endif /* DEBUG_H_ */
