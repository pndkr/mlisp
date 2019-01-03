#include <stdlib.h>
#include <stdio.h>
#include "lisp.h"

int _objects = 0;

Object *alloc(void)
{
	++_objects;
	Object *ob = calloc(1, sizeof(Object));
	return ob;
}

Value make(enum Type type)
{
	Value v = nil;

	v.type = type;
	if (isobject(v))
		v.object = alloc();
	return v;
}

Value pack(void *d, void (*delete)(void*))
{
	Value v = make(TOther);

	v.other->d = d;
	v.other->delete = delete;
	return v;
}

void mark(Value *v)
{
	if (isobject(*v))
		++v->object->refc;
}

void unmark(Value *v)
{
	if (isobject(*v))
		--v->object->refc;
}

void check(Value *v)
{
	if (isobject(*v) && v->object->refc <= 0) {
		if (isother(*v)) {
			if (v->other->delete)
				v->other->delete(v->other->d);
		} else {
			if (islist(*v)) {
				int i;

				for (i = 0; i < v->list->len; i++)
					delete(&vector(Value, v->list, i));
			}
			free(v->object->v.d);
		}
		free(v->object);
		v->type = TNil;
		--_objects;
	}
}

void delete(Value *v)
{
	unmark(v);
	check(v);
}

void set(Value *d, Value s)
{
	mark(&s);
	delete(d);
	*d = s;
}
