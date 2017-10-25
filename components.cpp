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
	else if ( inst.compare ( "LDI" ) == 0 )
		op = LDI;
	else if ( inst.compare ( "BEQ" ) == 0 )
		op = BEQ;

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
}

fetch_decode_execute::fetch_decode_execute ( RAM *rp, register_file *rf_in/*, write_back *out */)
{
	ram = rp;
	rf = rf_in;
	//wb = out;
}

void fetch_decode_execute::execute ()
{
	inst = ram->code[rf->pc];
	switch ( inst.op )
	{
		case NOP:
			cout << "	fde - NOP" << endl;
			break;
		case ADD:
			cout << "	fde - ADD r" << inst.dest << " r" << inst.a1 << " r" << inst.a2 << endl;
			rf->r[ inst.dest ] = rf->r[ inst.a1 ] + rf->r[ inst.a2 ];
			break;
		case ADDI:
			cout << "	fde - ADDI r" << inst.dest << " r" << inst.a1 << " " << inst.a2 << endl;
			rf->r[ inst.dest ] = rf->r[ inst.a1 ] + inst.a2;
			break;
		case SUB:
			cout << "	fde - SUB r" << inst.dest << " r" << inst.a1 << " r" << inst.a2 << endl;
			rf->r[ inst.dest ] = rf->r[ inst.a1 ] - rf->r[ inst.a2 ];
			break;
		case SUBI:
			cout << "	fde - SUBI r" << inst.dest << " r" << inst.a1 << " " << inst.a2 << endl;
			rf->r[ inst.dest ] = rf->r[ inst.a1 ] - inst.a2;
			break;
		case MUL:
			cout << "	fde - MUL r" << inst.dest << " r" << inst.a1 << " r" << inst.a2 << endl;
			rf->r[ inst.dest ] = rf->r[ inst.a1 ] * rf->r[ inst.a2 ];
			break;
		case DIV:
			cout << "	fde - DIV r" << inst.dest << " r" << inst.a1 << " r" << inst.a2 << endl;
			rf->r[ inst.dest ] = rf->r[ inst.a1 ] / rf->r[ inst.a2 ];
			break;
		case LDI:
			cout << "	fde - LDI r" << inst.dest << " " << inst.a1 << endl;
			rf->r[ inst.dest ] = inst.a1;
			break;
		case BEQ:
			cout << "	fde - BEQ r" << inst.dest << " r" << inst.a1 << " " << inst.a2 << endl;
			if ( rf->r[ inst.dest ] == rf->r[ inst.a1 ] )
				rf->pc += inst.a2;
			break;
	}
}

void fetch_decode_execute::push ()
{
	//cout << "	fde - buffer write" << endl;
	//wb->buffer_write ( &inst );
}

processor::processor ( int lines, RAM *rp )
	:/* wb ( &rf )
	,*/ ram ( rp )
	, fde ( rp, &rf /*, &wb*/ )
{
	cycles = 0;
	num_lines = lines;
}

int processor::tick ()
{
	cout << "Tick:" << endl;
	for ( int i = 0 ; i < NUM_ARCH_REG ; i++ )
		cout << "r" << i << " - " << rf.r[ i ] << ", ";
	cout << endl;
	fde.execute();
	//wb.write();

	return 0;
}

int processor::tock ()
{
	cout << "Tock:" << endl;
	//fde.push();
	rf.pc++;
	cycles++;

	if ( rf.pc >= num_lines )
		return 1;
	else
		return 0;
}
