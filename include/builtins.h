#ifndef _BUILTINS_H_
#define _BUILTINS_H_

#define BUILTIN_ERROR 2

#include "io.h"

typedef struct {
	char* name;
	int (*fun)(std_io *io, char**); 
} builtin_pair;

extern builtin_pair builtins_table[];

#endif /* !_BUILTINS_H_ */
