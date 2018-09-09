#include <stdlib.h>
#include <stdio.h>
#include "lisp.h"

void dump(Value *v, int indent)
{
	switch (v->type) {
	default:
		printf("[type %d]", v->type); break;
	case TNil:
		printf("nil"); break;
	case TFunc:
		printf("[func %x]", (void*)v->func); break;
	case TWeak:
		printf("[ref %x]", (void*)v->weak); break;
	case TNumber:
		printf("%lg", v->number); break;
	case TSymbol:
		fwrite(v->symbol->d, sizeof(char), v->symbol->len, stdout); break;
	case TString:
		putchar('\"');
		fwrite(v->string->d, sizeof(char), v->string->len, stdout);
		putchar('\"');
		break;
	case TLambda: case TList: {
			int i, j;

			putchar('(');
			for (i = 0; i < v->list->len; i++) {
				dump(&list(v, i), indent+1);
				if (i + 1 < v->list->len) {
					if (islist(list(v, i))) { /* indentation formatting part */
						putchar('\n');
						for (j = 0; j <= indent; j++)
							putchar(' ');
					} else {
						putchar(' ');
					}
				}
			}
			putchar(')');
		}
		break;
	}
}

Value eval_write(Value *ctx, Value *args)
{
	Value v = nil;

	if (args->list->len > 1)
		set(&v, eval(ctx, &list(args, 1)));
	else
		set(&v, *ctx);
	dump(&v, 0);
	unmark(&v);
	putchar('\n');
	return v;
}

Value eval_info(Value *ctx, Value *args)
{
	extern int _objects;

	printf("objects in memory: %d\n", _objects);
	return nil;
}
