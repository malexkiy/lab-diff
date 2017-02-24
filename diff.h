#ifndef _DIFF_H_
#define _DIFF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

	typedef struct _str {
		const char* file;
		unsigned long offset;
		unsigned long len;
		unsigned long line;
		unsigned long hash;
	} StrHash_t;

	int diff(const char* file1, const char* file2, const char* outFile);

#ifdef __cplusplus
}
#endif

#endif