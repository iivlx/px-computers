# px-assembly

The `px assembly` language is a **16-bit assembly language** designed for **MMIO interactions** and **low-level system control**. 
It uses a **stack-based** approach for multi-word operations, such as arithmetic, data movement, and control flow, rather than relying on flags for return values.
Overflow, multi-step calculations, and branching are managed through **pushing and popping values** on the stack.
The instructions are designed to be flexible and general, allowing **direct stack manipulation** per operation for conditional branching and smooth execution chains.

## Stack control

Values can be pushed onto the stack at the end of an operation by appending `<` to the instruction.

Values can be popped from the stack and used in an operation by appending `>` to the instruction.

If both `<` and `>` are appended to an operation, a value will be popped from the stack, used in the operation, and the result will be pushed back on the stack.

`^` can be used instead of `<` to push only the lower byte of the result, ignoring any overflow or regular results.

## Every operand is either an immediate value or an address

Every operand can be an **immediate value**, or either a **direct** or **indirect address**, which can be two different bit widths: **byte** or **word**.

**Immediate** - A direct numerical value used in the operation.

**Direct** - Accesses a specific memory location and uses its value. Specified by `[ ]`

**Indirect** - Accesses a memory location which serves as a pointer to the value. Specified by `( )`

**Byte** - Use either an immediate byte value, or load a byte from the address, specified by prepending a `#`.

**Word** - Use a word (two bytes) for the size of the immediate value, or load a word (two bytes) from the address, this is the default operand size.

## Examples

Multiply DWORDS:
```
       MUL< [B], [A]                
       MOV> [A]                     ; A:B = A*B
```

Add DWORDS:
```
       ADD< [Al], [Bl]
       ADD> [Ah], [Bh]              ; Ah:Al = Ah:Al + Bh:Bl
```

Add QWORDS:
```
      ADD<  [All], [Bll]            ; QWORD
      ADD<> [Alh], [Blh]
      ADD<> [Ahl], [Bhl]
      ADD>  [Ahh], [Bhh]            ; carry through... so now Ahh:Ahl:Alh:All has the QWORD A + B
```

Loop:
```
    MOV [t], 0                      ; t=0
Loop:
    INC [t]                         ; t++
    CMP< [t], 255                   
    JLT> Loop                       ; loop from t=0..255
Done:
                                    ; t=255
```


# Examples of stack operations explained for some instructions

```
MUL< 0x10, 0x10                     ; PUSH 0x100

MOV^ 0, [COLOR]                     ; PUSH [COLOR] && 0xFFFF

ADD_ [a], [b]                       ; [a] += [b]
ADD> [a], [b]                       ; [a] += [b] + POP()
ADD<> [a], [b]                      ; [a] += [b] + POP() ... PUSH [a] >> 0x10
ADD^> [a], [b]                      ; [a] += [b] + POP() ... PUSH [a] && 0xFFFF
ADD< [a], [b]                       ; [a] += [b] ... PUSH [a] >> 0x10
ADD^ [a], [b]                       ; [a] += [b] ... PUSH [a] && 0xFFFF
```