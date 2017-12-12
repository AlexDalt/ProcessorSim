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

void write_back::flush ()
{
	buffer.empty();
}

void write_back::buffer_write ( instruction inst )
{
	buffer.push_back ( inst );
}

int write_back::write ()
{
	if ( !buffer.empty() ){
		instruction i = buffer.front();
		buffer.pop_front();
		rf->r[ i.dest ] = i.a1;
		rf->dirty[ i.dest ] = false;
		return 1;
	}
	return 0;
}

execute::execute( processor *proc_in, RAM *rp, register_file *rf_in, write_back *out )
{
	proc = proc_in;
	ram = rp;
	rf = rf_in;
	wb = out;
	halt = true;
	finished = true;
}

int execute::exec ()
{
	write = false;
	finished = false;
	if ( !halt ){
		inst_out = inst_in;
		switch ( inst_out.op )
		{
			case NOP:
				break;

			case ADD:
			case ADDI:
				inst_out.a1 = inst_out.a1 + inst_out.a2;
				write = true;
				break;

			case SUB:
			case SUBI:
				inst_out.a1 = inst_out.a1 - inst_out.a2;
				write = true;
				break;

			case MUL:
				inst_out.a1 = inst_out.a1 * inst_out.a2;
				write = true;
				break;

			case DIV:
				inst_out.a1 = inst_out.a1 / inst_out.a2;
				write = true;
				break;

			case LD:
				inst_out.a1 = ram->data[ inst_out.a1 ];
				write = true;
				break;

			case LDI:
				write = true;
				break;

			case BLEQ:
				if ( inst_out.dest <= inst_out.a1 )
				{
					rf->pc += inst_out.a2 - 2;
					proc->flush();
				}
				break;

			case B:
				rf->pc += inst_out.dest - 2;
				proc->flush();
				break;

			case ST:
			case STI:
				ram->data[ inst_out.a1 ] = inst_out.dest;
				break;
		}
		finished = true;
		
		if( write == true )
			return 0;
		else
			return 1;
	}
	return 0;
}

void execute::push ()
{
	if ( finished && !halt )
	{
		if( write )
			wb->buffer_write ( inst_out );
		write = false;
		halt = true;
	}
}

void execute::buffer_exec ( instruction i )
{
	halt = false;
	inst_in = i;
}

decode::decode ( register_file *rf_in, execute *e_in )
{
	exec = e_in;
	rf = rf_in;
	halt = true;
	wait = false;
}

void decode::flush ()
{
	halt = true;
}

void decode::buffer_dec ( instruction i )
{
	if ( halt )
	{
		inst_in = i;
		halt = false;
		wait = false;
	}
}

void decode::fetch_operands ()
{
	inst_out = inst_in;
	if ( !halt )
	{
		switch( inst_out.op )
		{
			// two registers
			case ADD:
			case SUB:
			case MUL:
			case DIV:
				if ( rf->dirty[ inst_out.a1 ] || rf->dirty[ inst_out.a2 ] )
					wait = true;
				else
				{
					inst_out.a1 = rf->r[ inst_out.a1 ];
					inst_out.a2 = rf->r[ inst_out.a2 ];
					rf->dirty[ inst_out.dest ] = true;
					wait = false;
				}
				break;

			case BLEQ:
			case ST:
				if ( rf->dirty[ inst_out.dest ] || rf->dirty[ inst_out.a1 ] )
					wait = true;
				else
				{
					inst_out.dest = rf->r[ inst_out.dest ];
					inst_out.a1 = rf->r[ inst_out.a1 ];
					wait = false;
				}
				break;

			// one register
			case ADDI:
			case SUBI:
				if ( rf->dirty[ inst_out.a1 ] )
					wait = true;
				else
				{
					inst_out.a1 = rf->r[ inst_out.a1 ];
					rf->dirty[ inst_out.dest ] = true;
					wait = false;
				}
				break;

			case LD:
				if ( rf->dirty[ inst_out.a1 ] )
					wait = true;
				else
				{
					inst_out.a1 = rf->r[ inst_out.a1 ];
					rf->dirty[ inst_out.dest ] = true;
					wait = false;
				}
				break;

			case STI:
				if ( rf->dirty[ inst_out.dest ] )
					wait = true;
				else
				{
					inst_out.dest = rf->r[ inst_out.dest ];
					wait = false;
				}
				break;

			// no registers
			case LDI:
				rf->dirty[ inst_out.dest ] = true;
				wait = false;
				break;

			default:
				wait = false;
				break;
		}
	}
}

void decode::push ()
{
	if ( !wait && !halt )
	{
		exec->buffer_exec( inst_out );
		halt = true;
	}
}

fetch::fetch( RAM *rp, register_file *rf_in, decode *d_in )
{
	ram = rp;
	rf = rf_in;
	d = d_in;
	halt = false;
	inst_count = 0;
}

void fetch::flush ()
{
	halt = false;
}

void fetch::fetch_instruction ()
{
	if ( !halt )
	{
		inst = ram->code[ rf->pc ];
		inst.num = inst_count;
		inst_count++;
	}
}

void fetch::push ()
{
	if ( d->wait )
		halt = true;
	else
	{
		halt = false;
		d->buffer_dec( inst );
		rf->pc++;
	}
}

processor::processor ( int code, int data, RAM *rp )
	: ram ( rp )
	, wb ( &rf )
	, exec ( this, rp, &rf , &wb )
	, d ( &rf, &exec )
	, f ( rp, &rf, &d )
{
	cycles = 0;
	num_code = code;
	num_data = data;
	completed_instructions = 0;
}

void processor::flush ()
{
	f.flush();
	d.flush();
	wb.flush();
}

int processor::tick ()
{
	completed_instructions += wb.write();
	completed_instructions += exec.exec();
	d.fetch_operands();
	f.fetch_instruction();

	return 0;
}

int processor::tock ()
{
	exec.push();
	d.push();
	f.push();

	cycles++;
	inst_per_cycle = (float) completed_instructions / (float) cycles;

	if ( rf.pc >= num_code + 1 )
		return 1;
	else
		return 0;
}
