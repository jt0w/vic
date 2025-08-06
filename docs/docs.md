# Docs
> [!WARNING]
> This is just a draft 

## Operations
> [!NOTE]
> Not documented yet
> For now just look at [`src/common/vm.h`](../src/common/vm.h)

## Registers
There are a total of 27 registers: A - Z and the Stack Pointer SP.
In the future registers like the stack pointer will also be accessible.

## Syscalls

| Val of A | Name  | Implemented |
|----------|-------|-------------|
| 0        | read  | no          |
| 1        | write | yes         |
| 2        | open  | no          |
| 3        | close | no          |
| 4        | exit  | yes         |
