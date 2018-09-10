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

	for (;;) {
		while (isspace(ch = getchar()))
			;
		if (ch == ';') {
			while ((ch = getchar()) != '\n')
				;
		} else break;
	}
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

Value eval_print(Value *ctx, Value *args)
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
	delete(&d);
	return nil;
}

Value eval_def(Value *ctx, Value *args)
{
	set(mapget(ctx, &list(args, 1)), eval(ctx, &list(args, 2)));
	return nil;
}

Value eval_lambda(Value *ctx, Value *args)
{
	Value v = nil;
	int i;

	set(&v, make(TList));
	for (i = 1; i < args->list->len; i++)
		set(&list(&v, i-1), list(args, i));
	unmark(&v);
	return v;
}

Value eval_eval(Value *ctx, Value *args)
{
	Value v = nil;
	int i;

	if (args->list->len > 1)
		set(&v, eval(ctx, &list(args, 1)));
	set(&v, eval(ctx, &v));
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
	else if (v.type == TList)
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

	//dump(lbd, 0); dump(args, 0); putchar('\n');
	for (i = 0; i < vars->list->len; i++)
		set(mapget(&lclctx, &list(vars, i)), eval(ctx, &list(args, i+1)));
	for (i = 1; i < lbd->list->len; i++)
		set(&v, eval(&lclctx, &list(lbd, i)));
	delete(&lclctx);
	unmark(&v);
	return v;
}

int init(Value *ctx)
{
	set(ctx, make(TList));

	/* base system */
	setvar(ctx, "eval", cfunc(eval_eval));
	setvar(ctx, "read", cfunc(eval_read));
	setvar(ctx, "print", cfunc(eval_print));
	setvar(ctx, "set", cfunc(eval_set));
	setvar(ctx, "def", cfunc(eval_def));
	setvar(ctx, "fn", cfunc(eval_lambda));

	/* debugging system */
	setvar(ctx, "info", cfunc(eval_info));
	setvar(ctx, "write", cfunc(eval_write));

	/* maps and lists system */
	setvar(ctx, "map", cfunc(eval_map_literal));
	setvar(ctx, "map-field", cfunc(eval_map_field));
	setvar(ctx, "list", cfunc(eval_list_literal));
	setvar(ctx, "list-field", cfunc(eval_list_field));

	/* expression system */
	setvar(ctx, "len", cfunc(eval_len));
	setvar(ctx, "while", cfunc(eval_while));
	setvar(ctx, "do", cfunc(eval_do));
	setvar(ctx, "if", cfunc(eval_if));
	setvar(ctx, "add", cfunc(eval_add));
	setvar(ctx, "sub", cfunc(eval_sub));
	setvar(ctx, "mul", cfunc(eval_mul));
	setvar(ctx, "div", cfunc(eval_div));
	setvar(ctx, "mod", cfunc(eval_mod));
	setvar(ctx, "gt", cfunc(eval_gt));
	setvar(ctx, "lt", cfunc(eval_lt));
	setvar(ctx, "ge", cfunc(eval_ge));
	setvar(ctx, "le", cfunc(eval_le));
	setvar(ctx, "eq", cfunc(eval_eq));
	setvar(ctx, "ne", cfunc(eval_ne));
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
