# ProcessorSim

Processor simulator for experimenting with processor architecture

## Instruction Set

### Implimented
```
ADD dest opp1 opp2		// *dest = *opp1 + *opp2
ADDI dest opp1 const	// *dest = *opp1 + const
SUB dest opp1 opp2 		// *dest = *opp1 - *opp2
SUBI dest opp1 const	// *dest = *opp1 - const
MUL dest opp1 opp2 		// *dest = *opp1 * *opp2
DIV dest opp1 opp2 		// *dest = *opp1 / *opp2
LD dest addr			// load value from memory address addr into register dest
LDI dest const			// *dest = const
BEQ opp1 opp2 offset	// if (value in register opp1) == (value in register opp2) *pc += offset
J target				// *pc = target
JR target				// *pc += target
NOP						// no opperation
```

### Not Implimented (yet)
```
// add more branch instructions?
ST opp1 addr			// store value in register opp1 into memory address addr
```
