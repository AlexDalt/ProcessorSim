# ProcessorSim

## Instruction Set

### Implimented
```
ADD dest opp1 opp2		// *dest = *opp1 + *opp2
ADDI dest opp1 const	// *dest = *opp1 + const
SUB dest opp1 opp2 		// *dest = *opp1 - *opp2
SUBI dest opp1 const	// *dest = *opp1 - const
LDI dest const			// *dest = const
NOP						// no opperation

```

### Not Implimented (yet)
```
BEQ opp1 opp2 offset	// if (value in register opp1) == (value in register opp2) *pc += offset
	// add more branch instructions?
DIV dest opp1 opp2 		// *dest = *opp1 / *opp2
J target				// *pc = target
JR opp1					// *pc = *opp1
LD dest addr			// load value from memory address addr into register dest
MUL dest opp1 opp2 		// *dest = *opp1 * *opp2
ST opp1 addr			// store value in register opp1 into memory address addr
```
