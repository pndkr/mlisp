#include <stdlib.h>
#include <string.h>
#include "lisp.h"

#define HASHSZ 101

static unsigned hash(Value *v)
{
	unsigned h = 0;
	int i;

	if (v->type == TNumber)
		return (int)v->number%HASHSZ;
	if (v->type != TString && v->type != TSymbol)
		return 0;
	for (i = 0; i < v->string->len; i++)
		h = string(v, i) + 31 * h;
	return h%HASHSZ;
}

int cmp(Value *a, Value *b)
{
	if (a->type == TNumber && b->type == TNumber) {
		double c = a->number - b->number;
		return c < 0? -1 : c > 0? 1 : 0;
	}
	if ((a->type != TString && a->type != TSymbol) &&
	    (b->type != TSymbol && b->type != TSymbol))
		return 0;
	if (a->string->len != b->string->len)
		return a->string->len - b->string->len;
	return strncmp(a->string->d, b->string->d, a->string->len);
}

Value *mapget(Value *map, Value *key)
{
	int i;
	unsigned h = hash(key);
	Value *group = &list(map, h);

	if (group->type != TList)
		set(group, make(TList));

	/* i += 2 because it's a list of key-val pairs */
	for (i = 0; i < group->list->len; i += 2)
		if (cmp(&list(group, i), key) == 0)
			return &list(group, i+1);
	set(&list(group, i), *key);
	return &list(group, i+1); /* new undefined nil value */
}

void setstr(Value *map, char *key, Value v)
{
	int len = strlen(key);
	Value k = nil;

	set(&k, make(TString));
	string(&k, len-1);
	strncpy(k.string->d, key, len);
	set(mapget(map, &k), v);
	delete(&k);
}

/* interfaces for map and list usage inside the language */
Value eval_map_literal(Value *ctx, Value *args)
{
	Value m = nil;
	int i;

	set(&m, make(TList));
	for (i = 1; i < args->list->len; i += 2)
		set(mapget(&m, &list(args, i)), list(args, i+1));
	unmark(&m);
	return m;
}

Value eval_map_field(Value *ctx, Value *args)
{
	Value m = nil, k = nil, v = make(TWeak);

	set(&m, eval(ctx, &list(args, 1)));
	set(&k, eval(ctx, &list(args, 2)));
	v.weak = mapget(&m, &k);
	delete(&m);
	delete(&k);
	return v;
}

Value eval_list_literal(Value *ctx, Value *args)
{
	Value l = nil;
	int i;

	set(&l, make(TList));
	for (i = 1; i < args->list->len; i++)
		set(&list(&l, i-1), list(args, i));
	unmark(&l);
	return l;
}

Value eval_list_field(Value *ctx, Value *args)
{
	Value l = nil, i = nil, v = make(TWeak);

	set(&l, eval(ctx, &list(args, 1)));
	set(&i, eval(ctx, &list(args, 2)));
	v.weak = &list(&l, i.number);
	delete(&l);
	delete(&i);
	return v;
}
