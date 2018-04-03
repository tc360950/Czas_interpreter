# Czas_interpreter

Implements interpreter of language 'czas' described by the following grammar:

Program → ε | Element Program

Element → Definition | Instruction

Definition → ':' Label

Instruction → Subtract | Jump | Call | Return | Read | Write

Subtract → Adress Adress

Jump → Adress Label

Call → Label

Return → ';'

Read → '^' Adress

Write → Adress '^'

Label → id

Adress → number

