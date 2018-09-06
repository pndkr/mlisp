#include <stdlib.h>
#include <string.h>
#include "lisp.h"

void *access(Vector *v, int n, int sz)
{
	if (n >= v->len)
		v->len = n + 1;
	if (n >= v->cap) {
		void *new;

		new = calloc(2*v->len, sz);
		memmove(new, v->d, sz*v->cap);
		free(v->d);
		v->d = new;
		v->cap = 2*v->len;
	}
	return v->d + sz*n;
}
