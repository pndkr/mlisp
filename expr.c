#include "lisp.h"

#define MATH_EXPR(name, op) \
	Value eval_ ## name(Value *ctx, Value *args) \
	{ \
		Value r = nil, a = nil; \
		int i; \
 		\
		set(&a, prepargs(ctx, args)); \
		if (a.list->len > 0) \
			set(&r, list(&a, 0)); \
		for (i = 1; i < a.list->len; i++) \
			r.number op ## = list(&a, i).number; \
		delete(&a); \
		unmark(&r); \
		return r; \
	}

#define CMPR_EXPR(name, op) \
	Value eval_ ## name(Value *ctx, Value *args) \
	{ \
		Value r = nil, a = nil; \
		int i; \
 		\
		set(&a, prepargs(ctx, args)); \
		if (a.list->len > 0) \
			set(&r, list(&a, 0)); \
		for (i = 1; i < a.list->len; i++) { \
			if (r.number op list(&a, i).number) { \
				set(&r, list(&a, i)); \
			} else { \
				set(&r, make(TNil)); \
				break; \
			} \
		} \
		delete(&a); \
		unmark(&r); \
		return r; \
	}

#define BOOL_EXPR(name, op) \
	Value eval_ ## name(Value *ctx, Value *args) \
	{ \
		Value v = nil; \
		int i; \
		\
		for (i = 1; i < args->list->len; i++) { \
			set(&v, eval(ctx, &list(args, i))); \
			if (v.type op TNil) \
				break; \
		} \
		unmark(&v); \
		return v; \
	}

Value prepargs(Value *ctx, Value *args)
{
	Value t = nil, v = make(TList);
	int i, j;

	for (i = 1, j = 0; i < args->list->len; i++) {
		set(&t, eval(ctx, &list(args, i)));
		if (t.type != TNumber)
			continue;
		set(&list(&v, j++), t);
	}
	delete(&t);
	unmark(&v);
	return v;
}

Value eval_do(Value *ctx, Value *args)
{
	Value r = nil;
	int i;

	for (i = 1; i < args->list->len; i++)
		set(&r, eval(ctx, &list(args, i)));
	unmark(&r);
	return r;
}

Value eval_if(Value *ctx, Value *args)
{
	Value r = nil;

	set(&r, eval(ctx, &list(args, 1)));
	if (r.type != TNil)
		set(&r, eval(ctx, &list(args, 2)));
	else
		set(&r, eval(ctx, &list(args, 3)));
	unmark(&r);
	return r;
}

Value eval_while(Value *ctx, Value *args)
{
	Value r = nil, c = nil;
	int i;

	for (;;) {
		set(&c, eval(ctx, &list(args, 1)));
		if (c.type == TNil)
			break;
		for (i = 2; i < args->list->len; i++)
			set(&r, eval(ctx, &list(args, i)));
	}
	delete(&c);
	unmark(&r);
	return r;
}

Value eval_len(Value *ctx, Value *args)
{
	Value v = nil, l = make(TNumber);

	set(&v, eval(ctx, &list(args, 1)));
	if (isobject(v) && !isother(v))
		l.number = v.list->len;
	delete(&v);
	return l;
}

Value eval_mod(Value *ctx, Value *args)
{
	Value r = nil, a = nil;
	int i;

	set(&a, prepargs(ctx, args));
	if (a.list->len > 0)
		set(&r, list(&a, 0));
	for (i = 1; i < a.list->len; i++)
		r.number = (long int)r.number % (long int)list(&a, i).number;
	delete(&a);
	unmark(&r);
	return r;
}

Value eval_not(Value *ctx, Value *args)
{
	Value v = nil;

	set(&v, eval(ctx, &list(args, 1)));
	set(&v, make(v.type == TNil? TNumber : TNil));
	unmark(&v);
	return v;
}

MATH_EXPR(add, +)
MATH_EXPR(sub, -)
MATH_EXPR(mul, *)
MATH_EXPR(div, /)
CMPR_EXPR(gt, >)
CMPR_EXPR(lt, <)
CMPR_EXPR(ge, >=)
CMPR_EXPR(le, <=)
CMPR_EXPR(ne, !=)
CMPR_EXPR(eq, ==)
BOOL_EXPR(or, !=)
BOOL_EXPR(and, ==)
