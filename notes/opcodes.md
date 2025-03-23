# Opcodes
Number = 32-bit binary number
Register = A .. Z

| Opcode | Description                   | Arguments          |
|--------|-------------------------------|--------------------|
| 0x0    | Nop                           | No argumenns       |
| 0x1    | Push                          | Number             |
| 0x2    | Pop                           | Register           |
| 0x3    | LoadReg                       | Register           |
| 0x4    | PushReg                       | Register           |
| 0x5    | CmpReg*                       | Register, Register |
| 0x5    | CmpStack*                     | No argumenns       |
| 0x6    | Jump                          | Number             |
| 0x7    | Jump if greater than or equal | Number             |
| 0x8    | Jump if greater than          | Number             |
| 0x9    | Jump if less than or equal    | Number             |
| 0x10   | Jump if less than             | Number             |

# Extras
For the all the cmp operations: 
They push a number (↓) to the top of the stack.
| Equation | Number |
|----------|--------|
| a == b   | 0      |
| a >= b   | 1      |
| a > b    | 2      |
| a <= b   | 3      |
| a < b    | 4      |
