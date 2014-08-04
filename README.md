basilisk-v2
===========

Redo of Basilisk in C

## Language

There is a complete EBNF language specification in the file `Language Spec`.

### Syntax

The syntax resembles the following

```lisp
(op args...)
```

where the symbol `op` is a symbol or an identifier and args is an identifier, number, character, or string.

```lisp
(+ 1 0x2)
(println "string")
```

Both `+` and `println` are viable function calls.
