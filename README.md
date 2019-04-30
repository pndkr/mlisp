# _mlisp_ Early Documentation

## Data Types

 Name                  | Reference type | Index type
-----------------------|----------------|------------
 Nil                   | no             | no
 C Function            | no             | no
 Number                | no             | no
 Symbol                | yes            | no
 String                | yes            | no
 List                  | yes            | yes
 Weak reference **\*** | no             | no

**\*** weak references are only used internally by the interpreter

The _mlisp_ interpreter doesn't manage every object through reference count, some values are only copied, and _only_ the reference types are cared by the garbage collector. This is intended to save memory, since non-reference types are usually of similar size as pointers.

Based on everything above, types can be put in the following hierarchy:

- `value`
	- `object` (reference types)
		- `list`
		- `symbol`
		- `string`
	- `c function`
	- `number`
	- `weak` (weak reference)
	- `nil`

The terms above will be used in next section.

## Built-in functions

This section describes behaviour of built-in functions. The following conventions will be used here:

 Example                    | Name             | Meaning
----------------------------|------------------|---------
 `(f)`  → `type`            | Returns          | function `f` returns value of type `type`
 `argument...`              | Repeatable       | `argument` may be repeated
 `[argument]`               | Optional         | `argument` is optional
 `[argument argument]`      | Group            | both arguments are optional, but have to be passed together
 `[argument argument]...`   | Repeatable group | like above, but the arguments can be repeated
 <code>A&#124;B</code>      | One of           | the argument has to be of type or name `A` or `B`

Beware, the behaviour for most of these functions is still unspecified if they are given wrong type of arguments, however, at worst it only causes forever loops and null-pointer dereference, otherwise the functions just return wrong results without any other side effects.

### Base system

This sub-section describes the most basic functions in the language, without which the intepreter couldn't work correctly.

#### `(eval value)` → `value`

`eval` takes value of any type, tries to evaluate it, and returns the result.

#### `(read)` → `value`

`read` reads the standard input, parses it as _mlisp_ data, and returns it without evaluation.

#### `(print [value]...)` → `nil`

`print` prints its arguments to the output, it doesn't dive recursively into indexed types. It returns `nil` after it finishes its task.

#### `(quote value)` → `value`

`quote` returns its only argument as a literal, unevaluated token.

#### `(def symbol value)` → `nil`

`def` creates new variable in the local scope with the name of the first argument, then evaluates its second argument and assigns the result to the variable.

The function returns `nil`.

#### `(set value value)` → `nil`

`set` evaluates it's arguments, and assigns the second one, to the first one. The first argument might be a variable name or a reference to a list or map field, for instance:

	(def l (list "a" "b" "c" "d")) ; creates a new list
	(print (list-field l 2))       ; prints 'c'

	(set (list-field l 2) "g")     ; sets the "c" inside the list to "g"
	(print (list-field l 2))       ; prints 'g'

The difference between `set` and `def` is that `set` will look for an existing value rather than create one, if it doesn't find it, it will look for it in the global scope. But since the interpeter treats every non-declared variable as it were containing `nil`, the `set` function -- given a symbol as its first argument -- can be actually used to declare global variables from inside functions.

The function returns `nil`.

#### `(fn ([symbol]...) [value]...)` → `list`

`fn` creates and returns a new lambda, its first argument is assumed to be a list of symbols to be used as argument names, the next arguments are the lambda's body.

Every lambda, if called, returns the value of its last expression.

The idiom to declare and call functions in _mlisp_ is:

	(def twice (fn (n)
	  (mul 2 n)))      ; declares function 'twice'

	(print (twice 9))  ; prints '18'

Since lambdas are first-class values, and actually just lists, they can be passed as arguments to other lambdas or returned by them. The user can use this property to create solutions like the following one:

	(def double-op (fn (op n)
	  (op n n)))

	(def twice (fn (n)
	  (double-op add n)))

	(def squared (fn (n)
	  (double-op mul n)))

	(print (twice 4) (squared 4)) ; prints '8 16'

### Debugging system

This section describes pars of the language that are helpful when some internal actions of the intepreter are to be figured out.

#### `(info)` → `nil`

`info` prints information on number of _objects_ currently allocated in the memory, notice it doesn't include non-reference types.

The function returns `nil`.

#### `(write [value])` → `nil`

`write` prints it's only argument as an s-expression, if it's not given any arguments, it will print the contents of a scope it is run in.

The function returns `nil`.

### Map and list manipulation system

This section describes functions used to manage maps and lists.

#### `(list [value]...)` → `list`

`list` creates list of its evaluated arguments and returns it.

#### `(list-field index number)` → `value`

`list-field` returns value of `n`th element inside the given `index`.

Remember that lambdas are just lists, so they can be manipulated with some help of this function.

#### `(map [number|string|symbol value]...)` → `list`

The arguments for `map` have to come in key-value pairs, the function puts them inside a newly created map, with the values evaluated, and returns it.

The returned value is a `list` understanable by `map` family of functions, but since it's a `list`, it can be managed by `list` functions as well.

#### `(map-field list number|string|symbol)` → `value`

`map-field` returns value of the element assigned to the given key (the second argument) in the given map (the first argument).

### Expression system

This section describes functions used for evaluating mathematical and comparement expressions as well as functions used for control flow.

For these functions only `nil` is treated and returned as a false value, all the other values, including empty strings or 0s are truths.

#### `(len object)` → `number`

`len` returns length of given objects, if it's a string or a symbol, it returns the number of its characters, if it's a list, it returns the number of values inside it.

#### `(while value [value]...)` → `value`

`while` evaluates all its arguments as long as the first one is true,
it returns the last value of the last expression in its body.

#### `(if value value [value])` → `value`

`if` evaluates its first argument and checks whether its true, if it is, it evaluates and returns its second argument, otherwise it returns the evaluated value of the third argument. If the third argument isn't given, it is assumed to be `nil`.

#### `(do [value]...)` → `value`

`do` evaluates every each of its arguments once, and returns the value of the last one.

This function is obviously not directly related with mathematical expressions, but it can be used to group expressions; that might be especially handy inside ifs.

#### `(add|sub|mul|div|mod number [number]...)` → `number`

All of the functions above take their first argument, and then apply the appropriate mathematical operation to it -- using the next argument; if there are more arguments, the operation is repeated with the current result and the next arguments.

#### `(gt number [number]...)` → `number|nil`

`gt` checks whether it's arguments are sorted in decreasing order.

If so, it returns the last value on the list, otherwise it returns `nil`.

#### `(lt number [number]...)` → `number|nil`

`lt` checks whether it's arguments are sorted in rising order.

If so, it returns the last value on the list, otherwise it returns `nil`.

#### `(ge number [number]...)` → `number|nil`

`ge` checks whether it's arguments are sorted in non-rising order.

If so, it returns the last value on the list, otherwise it returns `nil`.

#### `(le number [number]...)` → `number|nil`

`le` checks whether it's arguments are sorted in non-decreasing order.

If so, it returns the last value on the list, otherwise it returns `nil`.

#### `(eq number [number]...)` → `number|nil`

`eq` checks whether all of its arguments are of the same numerical value.

If so, it returns the last value on the list, otherwise it returns `nil`.

#### `(ne number [number]...)` → `number|nil`

`ne` checks whether any _neighbouring_ pair of its arguments is of different value.

If so, it returns the last value on the list, otherwise it returns `nil`.

#### `(or value [value]...)` → `value|nil`

`or` checks if any of arguments passed to it are of type other than `nil`, if so, it returns the first non-nil value met, otherwise it returns `nil`.

#### `(and value [value]...)` → `value|nil`

`and` checks if any of arguments passed to it are of type `nil`, if so, it returns `nil`, otherwise it returns the last value passed to it.

#### `(not value)` → `number|nil`

`not` checks if the only argument passed to it is `nil`, if so, it returns a number, otherwise it returns `nil`.
