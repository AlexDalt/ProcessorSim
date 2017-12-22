# ProcessorSim

Processor simulator for experimenting with processor architecture

## Instruction Set
```
ADD dest opp1 opp2	// *dest = *opp1 + *opp2
ADDI dest opp1 const	// *dest = *opp1 + const
SUB dest opp1 opp2 	// *dest = *opp1 - *opp2
SUBI dest opp1 const	// *dest = *opp1 - const
MUL dest opp1 opp2 	// *dest = *opp1 * *opp2
DIV dest opp1 opp2 	// *dest = *opp1 / *opp2
LD dest addr		// load value from memory address in reg addr into register dest
LDI dest const		// *dest = const
BLEQ opp1 opp2 target	// if (value in register opp1) <= (value in register opp2) *pc += offset
B offset 		// *pc += target
ST opp1 opp2 		// store value in register opp1 into memory address in reg opp2
STI opp1 addr		// store value in register opp1 into memory address addr
NOP			// no opperation
```

## TODO
* Execution unit that supports vector instructions
* Cache memory
