#include "components.h"

instruction::instruction ( string inst, string d, string b1, string b2 )
{
	if ( inst.compare ( "NOP" ) == 0 )
		op = NOP;
	else if ( inst.compare ( "ADD" ) == 0 )
		op = ADD;
	else if ( inst.compare ( "ADDI" ) == 0 )
		op = ADDI;
	else if ( inst.compare ( "SUB" ) == 0 )
		op = SUB;
	else if ( inst.compare ( "SUBI" ) == 0 )
		op = SUBI;
	else if ( inst.compare ( "MUL" ) == 0 )
		op = MUL;
	else if ( inst.compare ( "DIV" ) == 0 )
		op = DIV;
	else if ( inst.compare ( "LD" ) == 0 )
		op = LD;
	else if ( inst.compare ( "LDI" ) == 0 )
		op = LDI;
	else if ( inst.compare ( "BLEQ" ) == 0 )
		op = BLEQ;
	else if ( inst.compare ( "B" ) == 0 )
		op = B;
	else if ( inst.compare ( "ST" ) == 0 )
		op = ST;
	else if ( inst.compare ( "STI" ) == 0 )
		op = STI;

	try
	{
		if ( !(d.empty()) )
			dest = stoi ( d, nullptr, 10 );
		if ( !(b1.empty()) )
			a1 = stoi ( b1, nullptr, 10 );
		if ( !(b2.empty())  )
			a2 = stoi ( b2, nullptr, 10 );
	}
	catch ( exception e )
	{
		cerr << "Error: unable to parse assembly into internal instructions" << endl;
		cerr << "inst: " << inst << ", dest: " << dest << ", arg1: " << b1 << ", arg2: " << b2 <<endl;
	}
}

RAM::RAM ( int code_size, int data_size )
{
	code = new instruction[code_size];
	data = new int[data_size];
	c_size = code_size;
	d_size = data_size;
}

int RAM::add ( int index, instruction i )
{
	try
	{
		code[index] = i;
	}
	catch ( int e )
	{
		cerr << "inserting instruction in RAM that doesn't exist" << endl;
	}
	return 0;
}

int RAM::add ( int index, int d )
{
	try
	{
		data[index] = d;
	}
	catch ( int e )
	{
		cerr << "inserting data in RAM that doesn't exist";
	}
	return 0;
}

register_file::register_file()
{
	pc = 0;
	for ( int i = 0 ; i < NUM_ARCH_REG ; i++ )
	{
		dirty[ i ] = false;
	}
}

write_back::write_back ( register_file *reg_pointer )
{
	rf = reg_pointer;
}

void write_back::buffer_write ( instruction inst )
{
	instruction i = inst;
	buffer.push ( i );
}

void write_back::write ()
{
	if ( !buffer.empty() ){
		instruction i = buffer.front();
		buffer.pop();
		//cout << "write - [r" << i.dest << " " << i.a1 << "]";
		rf->r[ i.dest ] = i.a1;
		rf->dirty[ i.dest ] = false;
	}
	//else
		//cout << "write - [buffer empty]";
}

void write_back::flush ()
{
	buffer.empty ();
}

execute::execute( processor *proc_in, RAM *rp, register_file *rf_in, write_back *out )
{
	proc = proc_in;
	ram = rp;
	rf = rf_in;
	wb = out;
	halt = true;
}

int execute::exec ()
{
	write = false;
	if ( !halt ){
		inst2 = inst1;
		switch ( inst2.op )
		{
			case NOP:
				//cout << "execute - [NOP]";
				break;

			case ADD:
			case ADDI:
				//cout << "execute - [ADD r" << inst2.dest << " " << inst2.a1 << " " << inst2.a2 << "]";
				inst2.a1 = inst2.a1 + inst2.a2;
				write = true;
				break;

			case SUB:
			case SUBI:
				//cout << "execute - [SUB r" << inst2.dest << " " << inst2.a1 << " " << inst2.a2 << "]";
				inst2.a1 = inst2.a1 - inst2.a2;
				write = true;
				break;

			case MUL:
				//cout << "execute - [MUL r" << inst2.dest << " " << inst2.a1 << " " << inst2.a2 << "]";
				inst2.a1 = inst2.a1 * inst2.a2;
				write = true;
				break;

			case DIV:
				//cout << "execute - [DIV r" << inst2.dest << " " << inst2.a1 << " " << inst2.a2 << "]";
				inst2.a1 = inst2.a1 / inst2.a2;
				write = true;
				break;

			case LD:
				//cout << "execute - [LD r" << inst2.dest << " " << inst2.a1 << "]";
				inst2.a1 = ram->data[ inst2.a1 ];
				write = true;
				break;

			case LDI:
				//cout << "execute - [LDI r" << inst2.dest << " " << inst2.a1 << "]";
				write = true;
				break;

			case BLEQ:
				//cout << "execute - [BLEQ r" << inst2.dest << " r" << inst2.a1 << " " << inst2.a2 << "]";
				if ( inst2.dest <= inst2.a1 )
				{
					rf->pc += inst2.a2 - 1;
					proc->flush();
				}
				break;

			case B:
				//cout << "execute - [B " << inst2.dest << "]";
				rf->pc += inst2.dest - 2;
				break;

			case ST:
			case STI:
				//cout << "execute - [ST r" << inst2.dest << " " << inst2.a1 << "]";
				ram->data[ inst2.a1 ] = inst2.dest;
				break;
		}
		halt = true;

		return 1;
	}
	else
	{
		//cout << "execute - [buffer empty]";
		return 0;
	}
}

void execute::push ()
{
	if ( write )
	{
		//cout << "execute - [buffer write]";
		wb->buffer_write ( inst2 );
		write = false;
	}
	else
	{
		//cout << "execute - [nothing to write to write_back buffer]";
	}
}

void execute::buffer_exec ( instruction i )
{
	halt = false;
	inst1 = i;
}

fetch_decode::fetch_decode ( RAM *rp, register_file *rf_in, execute *e_in )
{
	ram = rp;
	rf = rf_in;
	exec = e_in;
}

void fetch_decode::fetch_instruction ()
{
	halt = false;
	inst = ram->code[ rf->pc ];
	switch ( inst.op )
	{
		case NOP:
			//cout << "fd - [" << rf->pc << ": NOP]";
			break;

		case ADD:
			if ( rf->dirty[ inst.a1 ] || rf->dirty[ inst.a2 ] )
			{
				halt = true;
				//cout << "fd - [blocking instrucion]";
			}
			else
			{
				//cout << "fd - [" << rf->pc << ": ADD r" << inst.dest << " r" << inst.a1 << " r" << inst.a2 << "]";
				inst.a1 = rf->r[ inst.a1 ];
				inst.a2 = rf->r[ inst.a2 ];
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case ADDI:
			if ( rf->dirty[ inst.a1 ] )
			{
				halt = true;
				//cout << "fd - [blocking instrucion]";
			}
			else
			{
				//cout << "fd - [" << rf->pc << ": ADDI r" << inst.dest << " r" << inst.a1 << " " << inst.a2 << "]";
				inst.a1 = rf->r[ inst.a1 ];
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case SUB:
			if ( rf->dirty[ inst.a1 ] || rf->dirty[ inst.a2 ] )
			{
				halt = true;
				//cout << "fd - [blocking instructon]";
			}
			else
			{
				//cout << "fd - [" << rf->pc << ": SUB r" << inst.dest << " r" << inst.a1 << " r" << inst.a2 << "]";
				inst.a1 = rf->r[ inst.a1 ];
				inst.a2 = rf->r[ inst.a2 ];
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case SUBI:
			if ( rf->dirty[ inst.a1 ] )
			{
				halt = true;
				//cout << "fd	- [blocking instructon]";
			}
			else
			{
				//cout << "fd - [" << rf->pc << ": SUBI r" << inst.dest << " r" << inst.a1 << " " << inst.a2 << "]";
				inst.a1 = rf->r[ inst.a1 ];
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case MUL:
			if ( rf->dirty[ inst.a1 ] || rf->dirty[ inst.a2 ] )
			{
				halt = true;
				//cout << "fd - [blocking instructon]";
			}
			else
			{
				//cout << "fd - [" << rf->pc << ": MUL r" << inst.dest << " r" << inst.a1 << " r" << inst.a2 << "]";
				inst.a1 = rf->r[ inst.a1 ];
				inst.a2 = rf->r[ inst.a2 ];
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case DIV:
			if ( rf->dirty[ inst.a1 ] || rf->dirty[ inst.a2 ] )
			{
				halt = true;
				//cout << "fd - [blocking instructon]";
			}
			else
			{
				//cout << "fd - " << rf->pc << ": DIV r" << inst.dest << " r" << inst.a1 << " r" << inst.a2 << "]";
				inst.a1 = rf->r[ inst.a1 ];
				inst.a2 = rf->r[ inst.a2 ];
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case LD:
			if ( rf->dirty[ inst.a1 ] )
			{
				halt = true;
				//cout << "fd - [blocking instructon]";
			}
			else
			{
				//cout << "fd - [" << rf->pc << ": LD r" << inst.dest << " r" << inst.a1 << "]";
				inst.a1 = rf->r[ inst.a1 ];
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case LDI:
			//cout << "fd - [" << rf->pc << ": LDI r" << inst.dest << " " << inst.a1 << "]";
			rf->dirty[ inst.dest ] = true;
			break;

		case BLEQ:
			if ( rf->dirty[ inst.dest ] || rf->dirty[ inst.a1 ] )
			{
				halt = true;
				//cout << "fd - [blocking instructon]";
			}
			else
			{
				//cout << "fd - [" << rf->pc << ": BLEQ r" << inst.dest << " r" << inst.a1 << " " << inst.a2 << "]";
				inst.dest = rf->r[ inst.dest ];
				inst.a1 = rf->r[ inst.a1 ];
			}
			break;

		case B:
			//cout << "fd - [" << rf->pc << ": B " << inst.dest << "]";
			break;

		case ST:
			if ( rf->dirty[ inst.a1 ] )
			{
				halt = true;
				//cout << "fd - [blocking instructon]";
			}
			else
			{
				//cout << "fd - [" << rf->pc << ": ST r" << inst.dest << " r" << inst.a1 << "]";
				inst.dest = rf->r[ inst.dest ];
				inst.a1 = rf->r[ inst.a1 ];
			}
			break;

		case STI:
			//cout << "fd - [" << rf->pc << ": STI r" << inst.dest << " " << inst.a1 << "]";
			inst.dest = rf->r[ inst.dest ];
			break;
	}
}

void fetch_decode::push ()
{
	if ( !halt )
	{
		exec->buffer_exec( inst );
		rf->pc++;
		//cout << "fd - [pushed to execute]";
	}
	else
	{
		//cout << "fd - [nothing to push to execute]";
	}
}

void fetch_decode::flush ()
{
	halt = true;
}

processor::processor ( int code, int data, RAM *rp )
	: ram ( rp )
	, wb ( &rf )
	, exec ( this, rp, &rf , &wb )
	, fd ( rp, &rf, &exec )
{
	cycles = 0;
	num_code = code;
	num_data = data;
	completed_instructions = 0;
}

void processor::flush ()
{
	wb.flush();
	fd.flush();
}

int processor::tick ()
{
	//cout << "Tick: ";
	fd.fetch_instruction();
	//cout << " | ";
	completed_instructions += exec.exec();
	//cout << " | ";
	wb.write();
	//cout << endl << "registers [ ";
	//for ( int i = 0 ; i < NUM_ARCH_REG ; i++ )
		//cout << rf.r[ i ] << " ";
	//cout << "]" << endl << "memory [ ";
	//for ( int j = 0 ; j < num_data ; j++)
		//cout << ram->data[j] << " ";
	//cout << "]" << endl;

	return 0;
}

int processor::tock ()
{
	//cout << "Tock: ";
	fd.push();
	//cout << " | ";
	exec.push();
	//cout << endl;
	cycles++;
	float inst_per_cycle = (float) completed_instructions / (float) cycles;

	//cout << "cycles: " << cycles << ", inst/cycle: " << inst_per_cycle << endl;

	if ( rf.pc >= num_code + 1 )
		return 1;
	else
		return 0;
}
