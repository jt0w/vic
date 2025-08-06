# Docs

## Operations
>> TODO:
>> For now just look at [`src/common/vm.h`](../src/common/vm.h)

## Registers
There are a total of 26 registers: A - Z.
In the future registers like the stack pointer will also be accessible.

## Syscalls
> wip

| Val of A | Name  | Implemented |
|----------|-------|-------------|
| 0        | read  | no          |
| 1        | write | yes         |
| 2        | open  | no          |
| 3        | close | no          |
| 4        | exit  | yes         |
