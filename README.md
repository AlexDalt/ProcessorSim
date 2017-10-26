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
* Write benchmark programs
	* Standard kernels (eg. "Livermore Loops")
	* Sorting algorithms (eg. bubble sort + quick sort)
	* Numerical algorithms (eg. GCD or Hamming weight)
	* Recursive algorithms (eg. Factorial)
* Write assembler to read benchmark programs and convert them into machine readable
* Fetch/Decode/Execute pipeline stages
* GUI
* Branch prediction into the fetch unit
* Multiple execute units (2xALU, 1xLSU, 1xBU) w/ blocking issue
* Reservation stationsj
* Extend branch prediction mechanism
* Reorder buffer
* Avoid dependancies
* Register renaming
* Execution unit that supports vector instructions
* Cache memory
