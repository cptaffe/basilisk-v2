
(add 1 2) ; simple form should work

; parens
((add 1 2)) ; fail, first must be operator
(add 1 2)) ; too many parens
(add (list 1 2 3)) ; list argument

; operator testing
("add" 1 2) ; operator must be identifier | symbol
(_x9 1 2) ; allowed operator
(9x 1 2) ; not allowed operator
($ 1 2) ; symbol as operator (permitted)
(~! 1 2) ; symbol

; argument testing
(add "add" 1) ; string argument
(add 'c' 1) ; character argument
(add 078 34) ; octal
(add 0x46 86) ; hexadecimal
(add 1)
(add 1 2)
(add 1 2 3)
(add 1 2 (add 1 (add 1 2))) ; multiple nested lists

; char testing
(putc 'c') ; simple char printing
(putc 'cc') ; too many characters