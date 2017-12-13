#include "components.h"

int exec_times[13] = { 1, 1, 1, 1, 1, 4, 20, 4, 1, 3, 2, 1, 1 };

instruction::instruction ( string inst, string d, string b1, string b2 )
{
	d1 = false;
	d2 = false;
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

write_back::write_back ( register_file *reg_pointer, RAM *ram_in )
{
	rf = reg_pointer;
	ram = ram_in;
}

void write_back::insert_place_holder ( int inst_num )
{
	instruction i;
	i.num = inst_num;
	i.op = PLACE_HOLDER;
	buffer.push_back( i );
}

void write_back::flush ( int num )
{
	while( buffer.back().num > num )
		buffer.pop_back();
}

void write_back::buffer_write ( instruction inst )
{
	int i = 0;
	while( buffer[ i ].num != inst.num )
		i++;

	buffer[ i ].op = inst.op;
	buffer[ i ].dest = inst.dest;
	buffer[ i ].a1 = inst.a1;
}

int write_back::write ()
{
	int comp = 0;
	while( !buffer.empty() && buffer[ 0 ].op != PLACE_HOLDER )
	{
		instruction i = buffer.front();
		buffer.pop_front();
		switch ( i.op )
		{
			case ADD:
			case ADDI:
			case SUB:
			case SUBI:
			case MUL:
			case DIV:
			case LD:
			case LDI:
				rf->r[ i.dest ] = i.a1;
				rf->dirty[ i.dest ] = false;
				break;
			case ST:
			case STI:
				ram->data[ i.a1 ] = i.dest;

			default:
				break;
		}
		comp++;
	}
	return comp;
}

execute::execute( processor *proc_in, RAM *rp, register_file *rf_in, write_back *out )
{
	proc = proc_in;
	ram = rp;
	rf = rf_in;
	wb = out;
	halt = true;
	wait = false;
	finished = true;
}

void execute::exec ()
{
	finished = false;
	if ( !halt ){
		inst_out = inst_in;
		if ( exec_times[ inst_out.op ] > 1 && !wait )
		{
			wait = true;
			rem_exec = exec_times[ inst_out.op ] - 1;
		}
		else if ( wait && rem_exec > 1 )
		{
			rem_exec--;
		}
		else
		{
			wait = false;
			switch ( inst_out.op )
			{
				case NOP:
					break;

				case ADD:
				case ADDI:
					inst_out.a1 = inst_out.a1 + inst_out.a2;
					break;

				case SUB:
				case SUBI:
					inst_out.a1 = inst_out.a1 - inst_out.a2;
					break;

				case MUL:
					inst_out.a1 = inst_out.a1 * inst_out.a2;
					break;

				case DIV:
					inst_out.a1 = inst_out.a1 / inst_out.a2;
					break;

				case LD:
					inst_out.a1 = ram->data[ inst_out.a1 ];
					break;

				case LDI:
					break;

				case BLEQ:
					if ( inst_out.dest <= inst_out.a1 )
					{
						rf->pc = inst_out.pc + inst_out.a2;
						proc->flush( inst_out.num );
					}
					break;

				case B:
					rf->pc = inst_out.pc + inst_out.dest;
					proc->flush( inst_out.num );
					break;

				default:
					break;
			}
			finished = true;
		}
	}
}

void execute::push ()
{
	if ( finished && !halt )
	{
		wb->buffer_write ( inst_out );
		halt = true;
	}
}

void execute::flush ( int num )
{
	if( inst_in.num > num )
	{
		halt = true;
	}
}

void execute::buffer_exec ( instruction i )
{
	halt = false;
	wait = false;
	inst_in = i;
}

reservation_station::reservation_station ( execute *exec_in, register_file *rf_in )
{
	exec = exec_in;
	rf = rf_in;
}

void reservation_station::buffer_inst ( instruction inst )
{
	if ( inst.d1 || inst.d2 )
		wait_buffer.push_back( inst );
	else
		out_buffer.push_back( inst );
}

void reservation_station::fetch_operands ()
{
	for ( int i = 0; i < wait_buffer.size(); i++)
	{
		instruction inst = wait_buffer.front();
		wait_buffer.pop_front();
		switch( inst.op )
		{
			case ADD:
			case ADDI:
			case SUB:
			case SUBI:
			case MUL:
			case DIV:
			case LD:
				if ( inst.d1 && !rf->dirty[ inst.a1 ] )
				{
					inst.a1 = rf->r[ inst.a1 ];
					inst.d1 = false;
				}
				if ( inst.d2 && !rf->dirty[ inst.a2 ] )
				{
					inst.a2 = rf->r[ inst.a2 ];
					inst.d2 = false;
				}
				break;

			case BLEQ:
			case ST:
			case STI:
				if ( inst.d1 && !rf->dirty[ inst.dest ] )
				{
					inst.dest = rf->r[ inst.dest ];
					inst.d1 = false;
				}
				if ( inst.d2 && !rf->dirty[ inst.a1 ] )
				{
					inst.a1 = rf->r[ inst.a1 ];
					inst.d2 = false;
				}
				break;

			default:
				break;
		}

		if ( !inst.d1 && !inst.d2 )
		{
			out_buffer.push_back( inst );
			switch( inst.op )
			{
				case ADD:
				case ADDI:
				case SUB:
				case SUBI:
				case MUL:
				case DIV:
				case LD:
				case LDI:
					rf->dirty[ inst.dest ] = true;
					break;
				default:
					break;
			}
		}
		else
			wait_buffer.push_back( inst );
	}
}

void reservation_station::flush ( int num )
{
	instruction inst;
	for( int i = 0; i < wait_buffer.size(); i++ )
	{
		inst = wait_buffer.front();
		wait_buffer.pop_front();
		if ( inst.num <= num )
			wait_buffer.push_back( inst );
	}

	for( int i = 0; i < out_buffer.size(); i++ )
	{
		inst = out_buffer.front();
		out_buffer.pop_front();
		if ( inst.num <= num )
			out_buffer.push_back( inst );
		else
		{
			switch( inst.op )
			{
				case ADD:
				case ADDI:
				case SUB:
				case SUBI:
				case MUL:
				case DIV:
				case LD:
				case LDI:
					rf->dirty[ inst.dest ] = false;
					break;
				default:
					break;
			}
		}
	}
}

void reservation_station::push ()
{
	if ( !out_buffer.empty() && !exec->wait )
	{
		instruction inst = out_buffer.front();
		out_buffer.pop_front();
		exec->buffer_exec( inst );

		
	}
}

decode::decode ( register_file *rf_in, reservation_station *rs_in )
{
	rf = rf_in;
	rs = rs_in;
	halt = true;
	wait = false;
}

void decode::flush ( int num )
{
	if ( inst_out.num > num )
	{
		halt = true;
		wait = false;
		switch( inst_out.op )
		{
			case ADD:
			case ADDI:
			case SUB:
			case SUBI:
			case MUL:
			case DIV:
			case LD:
			case LDI:
				rf->dirty[ inst_out.dest ] = false;
				break;
			default:
				break;
		}
	}
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
				if ( !rf->dirty[ inst_out.a1 ] )
					inst_out.a1 = rf->r[ inst_out.a1 ];
				else
					inst_out.d1 = true;

				if ( !rf->dirty[ inst_out.a2 ] )
					inst_out.a2 = rf->r[ inst_out.a2 ];
				else
					inst_out.d2 = true;

				break;

			case BLEQ:
			case ST:
				if ( !rf->dirty[ inst_out.dest ] )
					inst_out.dest = rf->r[ inst_out.dest ];
				else
					inst_out.d1 = true;

				if ( !rf->dirty[ inst_out.a1 ] )
					inst_out.a1 = rf->r[ inst_out.a1 ];
				else
					inst_out.d2 = true;

				break;

			// one register
			case ADDI:
			case SUBI:
				if ( !rf->dirty[ inst_out.a1 ] )
					inst_out.a1 = rf->r[ inst_out.a1 ];
				else
					inst_out.d1 = true;

				break;

			case LD:
				if ( !rf->dirty[ inst_out.a1 ] )
					inst_out.a1 = rf->r[ inst_out.a1 ];
				else
					inst_out.d1 = true;

				break;

			case STI:
				if ( !rf->dirty[ inst_out.dest ] )
					inst_out.dest = rf->r[ inst_out.dest ];
				else
					inst_out.d1 = true;

				break;

			default:
				break;
		}
		
	}
}

void decode::push ()
{
	if ( rs->wait_buffer.size() + rs->out_buffer.size() >= RES_SIZE )
		wait = true;
	else
		wait = false;
	if ( !wait && !halt )
	{
		rs->buffer_inst( inst_out );
		halt = true;
		switch ( inst_out.op )
		{
			case ADD:
			case ADDI:
			case SUB:
			case SUBI:
			case MUL:
			case DIV:
			case LD:
			case LDI:
				rf->dirty[ inst_out.dest ] = true;
				break;
			default:
				break;

		}
	}
}

fetch::fetch( RAM *rp, register_file *rf_in, decode *d_in, write_back *wb_in )
{
	ram = rp;
	rf = rf_in;
	wb = wb_in;
	d = d_in;
	halt = false;
	inst_count = 0;
}

void fetch::flush ( int num )
{
	halt = false;
}

void fetch::fetch_instruction ()
{
	if ( !halt )
	{
		inst = ram->code[ rf->pc ];
		inst.num = inst_count;
		inst.pc = rf->pc;
		inst_count++;
		wb->insert_place_holder( inst.num );
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
	, wb ( &rf, rp )
	, exec ( this, rp, &rf , &wb )
	, rs ( &exec, &rf )
	, d ( &rf, &rs )
	, f ( rp, &rf, &d, &wb )
{
	cycles = 0;
	num_code = code;
	num_data = data;
	completed_instructions = 0;
}

void processor::flush ( int num )
{
	f.flush( num );
	d.flush( num );
	rs.flush( num );
	exec.flush( num );
	wb.flush( num );

	for( int i = 0; i < NUM_PHYS_REG; i++ )
		rf.dirty[ i ] = false;
}

int processor::tick ()
{
	completed_instructions += wb.write();
	exec.exec();
	rs.fetch_operands();
	d.fetch_operands();
	f.fetch_instruction();

	return 0;
}

int processor::tock ()
{
	exec.push();
	rs.push();
	d.push();
	f.push();

	cycles++;
	inst_per_cycle = (float) completed_instructions / (float) cycles;

	if ( rf.pc >= num_code + 1 )
		return 1;
	else
		return 0;
}
