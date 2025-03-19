#ifndef PROXY_CACHE_MACROS_HEADER
#define PROXY_CACHE_MACROS_HEADER

#define DebugMsg(msg, ...) fprintf(stderr, msg, __VA_ARGS__);
#define DebugError(scope, msg) fprintf(stderr, "Error in %s: %s\n", scope, msg);

#endif
