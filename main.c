#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "lisp.h"

Value run_lambda(Value *ctx, Value *lbd, Value *args);

Value global = nil;

Value eval_read(Value *ctx, Value *args)
{
	Value v = nil;
	int ch, i;

	while (isspace(ch = getchar()))
		;
	if (ch == '\"') {
		set(&v, make(TString));
		for (i = 0; (ch = getchar()) != '\"' && ch > 0; i++)
			string(&v, i) = ch;
	} else if (isalpha(ch)) {
		ungetc(ch, stdin);
		set(&v, make(TSymbol));
		for (i = 0; isalpha(ch = getchar()) || ch == '-'; i++)
			string(&v, i) = ch;
		ungetc(ch, stdin);
	} else if (isdigit(ch) || ch == '-') {
		ungetc(ch, stdin);
		set(&v, make(TNumber));
		scanf("%lf", &v.number);
	} else if (ch == '(') {
		Value elem = nil;
		set(&v, make(TList));
		for (i = 0; (elem = eval_read(ctx, 0)).type != TNil; i++)
			set(&list(&v, i), elem);
		delete(&elem);
	} else if (ch == ')') {
		set(&v, make(TNil));
	}
	unmark(&v);
	return v;
}

Value eval_write(Value *ctx, Value *args)
{
	Value v = nil;
	int i;

	for (i = 1; i < args->list->len; i++) {
		set(&v, eval(ctx, &list(args, i)));
		switch (v.type) {
		case TNumber:
			printf("%lg", v.number);
			break;
		case TString: case TSymbol:
			fwrite(v.string->d, sizeof(char), v.string->len, stdout);
			break;
		case TNil:
			printf("nil");
			break;
		default:
			printf("(type %d)", v.type);
			break;
		}
		putchar(' ');
	}
	printf("\n");
	unmark(&v);
	return v;
}

Value eval_set(Value *ctx, Value *args)
{
	Value d = nil;

	set(&d, eval_weak(ctx, &list(args, 1)));
	if (d.type == TWeak)
		set(d.weak, eval(ctx, &list(args, 2)));
	unmark(&d);
	return d;
}

Value eval_def(Value *ctx, Value *args)
{
	set(mapget(ctx, &list(args, 1)), eval(ctx, &list(args, 2)));
	return make(TNil);
}

Value eval_lambda(Value *ctx, Value *args)
{
	int i;
	Value v = nil;

	set(&v, make(TLambda));
	for (i = 1; i < args->list->len; i++)
		set(&list(&v, i-1), list(args, i));
	unmark(&v);
	return v;
}

Value eval_weak(Value *ctx, Value *args)
{
	Value v = nil;
	int i;

	if (args->type == TSymbol) {
		set(&v, make(TWeak));
		v.weak = mapget(ctx, args);
		if (v.weak->type == TNil)
			v.weak = mapget(&global, args);
		unmark(&v);
		return v;
	}
	if (args->type != TList)
		return *args;
	set(&v, eval(ctx, &list(args, 0)));
	if (v.type == TFunc)
		set(&v, v.func(ctx, args));
	else if (v.type == TLambda)
		set(&v, run_lambda(ctx, &v, args));
	unmark(&v);
	return v;
}

Value eval(Value *ctx, Value *args)
{
	Value v = nil;

	set(&v, eval_weak(ctx, args));
	if (v.type == TWeak)
		set(&v, *v.weak);
	unmark(&v);
	return v;
}

Value run_lambda(Value *ctx, Value *lbd, Value *args)
{
	Value lclctx = make(TList);
	Value *vars = &list(lbd, 0);
	Value v = nil;
	int i;

	for (i = 0; i < vars->list->len; i++)
		set(mapget(&lclctx, &list(vars, i)), eval(ctx, &list(args, i+1)));
	for (i = 1; i < lbd->lambda->len; i++)
		set(&v, eval(&lclctx, &list(lbd, i)));
	delete(&lclctx);
	unmark(&v);
	return v;
}

int init(Value *ctx)
{
	set(ctx, make(TList));

	/* base system */
	setstr(ctx, "eval", cfunc(eval));
	setstr(ctx, "read", cfunc(eval_read));
	setstr(ctx, "write", cfunc(eval_write));
	setstr(ctx, "set", cfunc(eval_set));
	setstr(ctx, "def", cfunc(eval_def));
	setstr(ctx, "fn", cfunc(eval_lambda));

	/* debugging system */
	setstr(ctx, "info", cfunc(eval_info));
	setstr(ctx, "dump", cfunc(eval_dump));

	/* maps and lists system */
	setstr(ctx, "map", cfunc(eval_map_literal));
	setstr(ctx, "map-field", cfunc(eval_map_field));
	setstr(ctx, "list", cfunc(eval_list_literal));
	setstr(ctx, "list-field", cfunc(eval_list_field));

	/* expression system */
	setstr(ctx, "while", cfunc(eval_while));
	setstr(ctx, "if", cfunc(eval_if));
	setstr(ctx, "add", cfunc(eval_add));
	setstr(ctx, "sub", cfunc(eval_sub));
	setstr(ctx, "mul", cfunc(eval_mul));
	setstr(ctx, "div", cfunc(eval_div));
	setstr(ctx, "gt", cfunc(eval_gt));
	setstr(ctx, "lt", cfunc(eval_lt));
	setstr(ctx, "ge", cfunc(eval_ge));
	setstr(ctx, "le", cfunc(eval_le));
	setstr(ctx, "eq", cfunc(eval_eq));
}

int main(int argc)
{
	Value e = nil, r = nil;

	init(&global);
	for (;;) {
		set(&e, eval_read(&global, 0));
		set(&r, eval(&global, &e));
	}
	exit(0);
}
