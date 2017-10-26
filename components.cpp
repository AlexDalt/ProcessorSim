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
	for ( int i = 0; i < NUM_ARCH_REG ; i++ )
	{
		dirty[ i ] = false;
	}
}

write_back::write_back ( register_file *reg_pointer )
{
	rf = reg_pointer;
}

void write_back::buffer_write ( instruction *inst )
{
	instruction i = *inst;
	buffer.push ( i );
}

void write_back::write ()
{
	if ( !buffer.empty() ){
		instruction i = buffer.front();
		buffer.pop();
		cout << "write - [r" << i.dest << " " << i.a1 << "]";
		rf->r[ i.dest ] = i.a1;
		rf->dirty[ i.dest ] = false;
	}
	else
		cout << "write - [buffer empty]";
}

fetch_decode_execute::fetch_decode_execute ( RAM *rp, register_file *rf_in, write_back *out )
{
	ram = rp;
	rf = rf_in;
	wb = out;
}

int fetch_decode_execute::execute ()
{
	halt = false;
	inst = ram->code[rf->pc];
	switch ( inst.op )
	{
		case NOP:
			cout << "fde - [" << rf->pc << ": NOP]";
			break;

		case ADD:
			if ( rf->dirty[ inst.a1 ] || rf->dirty[ inst.a2 ] )
			{
				halt = true;
				cout << "fde - [blocking instrucion]";
			}
			else
			{
				cout << "fde - [" << rf->pc << ": ADD r" << inst.dest << " r" << inst.a1 << " r" << inst.a2 << "]";
				inst.a1 = rf->r[ inst.a1 ] + rf->r[ inst.a2 ];
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case ADDI:
			if ( rf->dirty[ inst.a1 ] )
			{
				halt = true;
				cout << "fde - [blocking instrucion]";
			}
			else
			{
				cout << "fde - [" << rf->pc << ": ADDI r" << inst.dest << " r" << inst.a1 << " " << inst.a2 << "]";
				inst.a1 = rf->r[ inst.a1 ] + inst.a2;
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case SUB:
			if ( rf->dirty[ inst.a1 ] || rf->dirty[ inst.a2 ] )
			{
				halt = true;
				cout << "fde - [blocking instructon]";
			}
			else
			{
				cout << "fde - [" << rf->pc << ": SUB r" << inst.dest << " r" << inst.a1 << " r" << inst.a2 << "]";
				inst.a1 = rf->r[ inst.a1 ] - rf->r[ inst.a2 ];
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case SUBI:
			if ( rf->dirty[ inst.a1 ] )
			{
				halt = true;
				cout << "fde - [blocking instructon]";
			}
			else
			{
				cout << "fde - [" << rf->pc << ": SUBI r" << inst.dest << " r" << inst.a1 << " " << inst.a2 << "]";
				inst.a1 = rf->r[ inst.a1 ] - inst.a2;
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case MUL:
			if ( rf->dirty[ inst.a1 ] || rf->dirty[ inst.a2 ] )
			{
				halt = true;
				cout << "fde - [blocking instructon]";
			}
			else
			{
				cout << "fde - [" << rf->pc << ": MUL r" << inst.dest << " r" << inst.a1 << " r" << inst.a2 << "]";
				inst.a1 = rf->r[ inst.a1 ] * rf->r[ inst.a2 ];
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case DIV:
			if ( rf->dirty[ inst.a1 ] || rf->dirty[ inst.a2 ] )
			{
				halt = true;
				cout << "fde - [blocking instructon]";
			}
			else
			{
				cout << "	fde - " << rf->pc << ": DIV r" << inst.dest << " r" << inst.a1 << " r" << inst.a2 << endl;
				inst.a1 = rf->r[ inst.a1 ] / rf->r[ inst.a2 ];
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case LD:
			if ( rf->dirty[ inst.a1 ] )
			{
				halt = true;
				cout << "fde - [blocking instructon]";
			}
			else
			{
				cout << "	fde - " << rf->pc << ": LD r" << inst.dest << " r" << inst.a1 << endl;
				inst.a1 = ram->data[ rf->r[ inst.a1 ] ];
				rf->dirty[ inst.dest ] = true;
			}
			break;

		case LDI:
			cout << "fde - [" << rf->pc << ": LDI r" << inst.dest << " " << inst.a1 << "]";
			rf->dirty[ inst.dest ] = true;
			break;
		case BLEQ:
			cout << "	fde - " << rf->pc << ": BLEQ r" << inst.dest << " r" << inst.a1 << " " << inst.a2 << endl;
			if ( rf->r[ inst.dest ] <= rf->r[ inst.a1 ] )
				rf->pc += inst.a2 - 1;
			break;
		case B:
			cout << "	fde - " << rf->pc << ": B " << inst.dest << endl;
			rf->pc += inst.dest - 1;
			break;
		case ST:
			cout << "	fde - " << rf->pc << ": ST r" << inst.dest << " r" << inst.a1 << endl;
			ram->data[ rf->r[ inst.a1 ] ] = rf->r[ inst.dest ];
			break;
		case STI:
			cout << "	fde - " << rf->pc << ": STI r" << inst.dest << " " << inst.a1 << endl;
			ram->data[ inst.a1 ] = rf->r[ inst.dest ];
			break;
	}

	if ( !halt )
		rf->pc++;

	return 1; //number of instructions completed
}

void fetch_decode_execute::push ()
{
	if ( !halt )
	{
		cout << "fde - [buffer write]";
		wb->buffer_write ( &inst );
	}
}

processor::processor ( int code, int data, RAM *rp )
	: ram ( rp )
	, wb ( &rf )
	, fde ( rp, &rf , &wb )
{
	cycles = 0;
	num_code = code;
	num_data = data;
	completed_instructions = 0;
}

int processor::tick ()
{
	cout << "Tick: ";
	completed_instructions += fde.execute();
	cout << " | ";
	wb.write();
	cout << endl << "registers [ ";
	for ( int i = 0 ; i < NUM_ARCH_REG ; i++ )
		cout << rf.r[ i ] << " ";
	cout << "]" << endl << "memory [ ";
	for ( int j = 0 ; j < num_data ; j++)
		cout << ram->data[j] << " ";
	cout << "]" << endl;


	return 0;
}

int processor::tock ()
{
	cout << "Tock: ";
	fde.push();
	cout << endl;
	cycles++;

	cout << "cycles: " << cycles << ", inst/cycle: " << completed_instructions/cycles << endl;

	if ( rf.pc >= num_code )
		return 1;
	else
		return 0;
}
