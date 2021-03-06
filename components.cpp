#include "components.h"

int exec_times[13] = { 1, 1, 1, 1, 1, 4, 20, 4, 1, 3, 2, 1, 1 };

instruction::instruction ( string inst, string d, string b1, string b2 )
{
	d1 = true;
	d2 = true;
	taken = false;
	renamed = false;
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
	for ( int i = 0; i < NUM_PHYS_REG; i++ )
		dirty[ i ] = false;
	for ( int i = 0; i < NUM_ARCH_REG; i++ )
		r[ i ] = -1;
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
	instruction inst;
	while( !buffer.empty() && buffer[ 0 ].op != PLACE_HOLDER )
	{
		inst = buffer.front();
		buffer.pop_front();
		switch ( inst.op )
		{
			case ADD:
			case ADDI:
			case SUB:
			case SUBI:
			case MUL:
			case DIV:
			case LD:
			case LDI:
				rf->p[ inst.dest ] = inst.a1;
				rf->dirty[ inst.dest ] = false;
				break;
			case ST:
			case STI:
				ram->data[ inst.a1 ] = inst.dest;
				break;

			case NOP:
				comp--;
				break;

			default:
				break;
		}
		comp++;
	}

	return comp;
}

execute::execute( processor *proc_in, RAM *rp, register_file *rf_in, write_back *out, branch_predictor *bp_in )
{
	proc = proc_in;
	ram = rp;
	rf = rf_in;
	wb = out;
	halt = true;
	wait = false;
	finished = true;
	bp = bp_in;
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
					if ( inst_out.dest <= inst_out.a1 && !inst_out.taken )
					{
						rf->pc = inst_out.pc + inst_out.a2;
						proc->flush( inst_out.num );
						bp->result( inst_out, false );
						for ( int k = 0; k < NUM_ARCH_REG; k++ )
							rf->r[ k ] = inst_out.old_r[ k ];
					}
					else if ( inst_out.dest > inst_out.a1 && inst_out.taken )
					{
						rf->pc = inst_out.pc + 1;
						proc->flush( inst_out.num );
						bp->result( inst_out, false );
						for ( int k = 0; k < NUM_ARCH_REG; k++ )
							rf->r[ k ] = inst_out.old_r[ k ];
					}
					else
					{
						bp->result( inst_out, true );
					}
					break;

				case B:
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
		wait = false;
	}
}

void execute::buffer_exec ( instruction i )
{
	halt = false;
	wait = false;
	inst_in = i;
}

reservation_station::reservation_station ( processor *proc_in, execute *exec_in, register_file *rf_in )
{
	for ( int i = 0; i < NUM_ALU; i++ )
		exec[ i ] = &exec_in[ i ];
	rf = rf_in;
	proc = proc_in;
}

bool sort_inst ( instruction i1, instruction i2 )
{
	return ( i1.num < i2.num );
}

void reservation_station::buffer_inst ( instruction inst )
{
	if ( !inst.d1 && !inst.d2 )
		out_buffer.push_back( inst );
	else
		wait_buffer.push_back( inst );

	sort ( wait_buffer.begin(), wait_buffer.end(), sort_inst );
	sort ( out_buffer.begin(), out_buffer.end(), sort_inst );
}

void reservation_station::fetch_operands ()
{
	instruction inst;
	deque<instruction> wait_copy;
	deque<instruction> out_copy;

	sort( out_buffer.begin(), out_buffer.end(), sort_inst );
	sort( wait_buffer.begin(), wait_buffer.end(), sort_inst );

	int min_num = 0;

	while( !out_buffer.empty() || !wait_buffer.empty() )
	{
		if ( out_buffer.size() == 0 || (wait_buffer.size() > 0 && wait_buffer.front().num < out_buffer.front().num ) )
		{
			inst = wait_buffer.front();
			wait_buffer.pop_front();
			proc->check( inst.num );

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
						inst.a1 = rf->p[ inst.a1 ];
						inst.d1 = false;
					}
					if ( inst.d2 && !rf->dirty[ inst.a2 ] )
					{
						inst.a2 = rf->p[ inst.a2 ];
						inst.d2 = false;
					}
					rf->dirty[ inst.dest ] = true;
					break;

				case LDI:
					rf->dirty[ inst.dest ] = true;
					break;

				case BLEQ:
				case ST:
				case STI:
					if ( inst.d1 && !rf->dirty[ inst.dest ] )
					{
						inst.dest = rf->p[ inst.dest ];
						inst.d1 = false;
					}
					if ( inst.d2 && !rf->dirty[ inst.a1 ] )
					{
						inst.a1 = rf->p[ inst.a1 ];
						inst.d2 = false;
					}
					break;

				default:
					break;
			}

			wait_copy.push_back( inst );
			min_num = inst.num;
		}
		else
		{
			inst = out_buffer.front();
			out_buffer.pop_front();
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
			out_copy.push_back( inst );
			min_num = inst.num;
		}
	}
	out_buffer = out_copy;
	wait_buffer = wait_copy;
}

void reservation_station::flush ( int num )
{
	instruction inst;
	int max = wait_buffer.size();

	for( int i = 0; i < max; i++ )
	{
		inst = wait_buffer.front();
		wait_buffer.pop_front();
		if ( inst.num <= num )
			wait_buffer.push_back( inst );
	}

	max = out_buffer.size();

	for( int i = 0; i < max; i++ )
	{
		inst = out_buffer.front();
		out_buffer.pop_front();
		if ( inst.num <= num )
			out_buffer.push_back( inst );
	}

	sort( out_buffer.begin(), out_buffer.end(), sort_inst );
	sort( wait_buffer.begin(), wait_buffer.end(), sort_inst );
}

void reservation_station::push ()
{
	sort( out_buffer.begin(), out_buffer.end(), sort_inst );
	sort( wait_buffer.begin(), wait_buffer.end(), sort_inst );

	bool lsu = true;
	bool bu = true;
	int current_exec = 1;
	deque<instruction> copy;

	while ( !out_buffer.empty() )
	{
		instruction inst = out_buffer.front();
		out_buffer.pop_front();

		if ( inst.op == LD || inst.op == ST || inst.op == STI || NUM_ALU == 1 )
		{
			if ( !exec[ 0 ]->wait && bu )
			{
				exec[ 0 ]->buffer_exec( inst );
				bu = false;
			}
			else
				copy.push_back( inst );
		}
		else if ( inst.op == B || inst.op == BLEQ || NUM_ALU == 2 )
		{
			if ( !exec[ NUM_ALU - 1 ]->wait && lsu )
			{
				exec[ NUM_ALU - 1 ]->buffer_exec( inst );
				lsu = false;
			}
			else
				copy.push_back( inst );
		}
		else
		{
			while ( current_exec < NUM_ALU - 1 && exec[ current_exec ]->wait )
				current_exec++;
			if ( current_exec < NUM_ALU - 1)
			{
				exec[ current_exec ]->buffer_exec( inst );
				current_exec++;
			}
			else
				copy.push_back( inst );
		}
	}

	out_buffer = copy;

	int max = wait_buffer.size();
	instruction inst;
	for ( int i = 0; i < max; i++ )
	{
		inst = wait_buffer.front();
		wait_buffer.pop_front();
		if ( !inst.d1 && !inst.d2 )
			out_buffer.push_back( inst );
		else
			wait_buffer.push_back( inst );
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
	deque<instruction> copy;
	instruction inst;

	while ( !insts.empty() )
	{
		inst = insts.front();
		insts.pop_front();
		if ( inst.num < num )
			copy.push_back( inst );
	}

	insts = copy;
}

void decode::buffer_dec ( instruction i )
{
	if ( halt )
	{
		insts.push_back( i );
	}
}

instruction decode::rename ( instruction inst )
{
	int j = 0;
	bool used = false;
	int k;
	for ( k = 0; k < NUM_ARCH_REG; k++ )
	{
		if ( rf->r[ k ] == j )
			used = true;
	}
	while ( used )
	{
		j++;
		used = false;
		for ( k = 0; k < NUM_ARCH_REG; k++ )
		{
			if ( rf->r[ k ] == j )
				used = true;
		}
	}

	switch ( inst.op )
	{
		case ADD:
		case SUB:
		case MUL:
		case DIV:
			inst.a2 = rf->r[ inst.a2 ];
		case ADDI:
		case SUBI:
		case LD:
			inst.a1 = rf->r[ inst.a1 ];
		case LDI:
			rf->r[ inst.dest ] = j;
			inst.dest = j;
			break;

		case BLEQ:
		case ST:
			inst.a1 = rf->r[ inst.a1 ];
		case STI:
			inst.dest = rf->r[ inst.dest ];
			break;

		default:
			break;
	}
	if ( inst.op == BLEQ )
	{
		for ( int i = 0; i < NUM_ARCH_REG; i ++ )
			inst.old_r[ i ] = rf->r[ i ];
	}

	inst.renamed = true;

	return inst;
}

void decode::fetch_operands ()
{
	for ( int i = 0; i < insts.size(); i++ )
	{
		if ( !insts[ i ].renamed )
			insts[ i ] = rename( insts[ i ] );
		switch( insts[ i ].op )
		{
			// two registers
			case ADD:
			case SUB:
			case MUL:
			case DIV:
				if ( insts[ i ].d1 && !rf->dirty[ insts[ i ].a1 ] )
				{
					insts[ i ].a1 = rf->p[ insts[ i ].a1 ];
					insts[ i ].d1 = false;
				}
				if ( insts[ i ].d2 && !rf->dirty[ insts[ i ].a2 ] )
				{
					insts[ i ].a2 = rf->p[ insts[ i ].a2 ];
					insts[ i ].d2 = false;
				}
				rf->dirty[ insts[ i ].dest ] = true;
				break;

			case BLEQ:
			case ST:
				if ( insts[ i ].d1 && !rf->dirty[ insts[ i ].dest ] )
				{
					insts[ i ].dest = rf->p[ insts[ i ].dest ];
					insts[ i ].d1 = false;
				}

				if ( insts[ i ].d2 && !rf->dirty[ insts[ i ].a1 ] )
				{
					insts[ i ].a1 = rf->p[ insts[ i ].a1 ];
					insts[ i ].d2 = false;
				}
				break;

				// one register
			case ADDI:
			case SUBI:
			case LD:
				if ( insts[ i ].d1 && !rf->dirty[ insts[ i ].a1 ] )
				{
					insts[ i ].a1 = rf->p[ insts[ i ].a1 ];
					insts[ i ].d1 = false;
				}
				rf->dirty[ insts[ i ].dest ] = true;
				insts[ i ].d2 = false;
				break;

			case LDI:
				rf->dirty[ insts[ i ].dest ] = true;
				insts[ i ].d1 = false;
				insts[ i ].d2 = false;
				break;

			case STI:
				if ( !rf->dirty[ insts[ i ].dest ] )
				{
					insts[ i ].dest = rf->p[ insts[ i ].dest ];
					insts[ i ].d1 = false;
				}
				else
					insts[ i ].d1 = true;
				insts[ i ].d2 = false;
				break;

			default:
				insts[ i ].d1 = false;
				insts[ i ].d2 = false;
				break;
		}
	}
}

void decode::push ()
{
	while ( !insts.empty() && rs->wait_buffer.size() + rs->out_buffer.size() < RES_SIZE )
	{
		rs->buffer_inst( insts.front() );
		insts.pop_front();
	}
}

branch_predictor::branch_predictor ()
{
	predicted = 0;
	correct = 0;
}

bool branch_predictor::predict ( instruction inst )
{
	switch ( BRANCH )
	{
		case 0:
			return true;
		case 1:
			return false;
		case 2:
			if ( inst.a2 > 0 )
				return false;
			else
				return true;
		case 3:
			bool entry = false;
			int i;
			for ( i = 0; i < history.size(); i++ )
				if ( history[ i ].pc == inst.pc )
				{
					entry = true;
					break;
				}

			if ( entry )
			{
				if ( history[ i ].num > 1 )
					return true;
				else
					return false;
			}
			else
			{
				if ( inst.a2 > 0 )
				{
					inst.num = 1;
					history.push_back( inst );
					return false;
				}
				else
				{
					inst.num = 2;
					history.push_back( inst );
					return true;
				}
			}
	}
}

void branch_predictor::result ( instruction inst, bool cor )
{
	predicted++;
	if ( cor )
		correct++;

	if ( BRANCH == 3 )
	{
		int i;
		for ( i = 0; i < history.size(); i++ )
		{
			if ( history[ i ].pc == inst.pc )
				break;
		}
		if ( history[ i ].num > 1 && !cor )
			history[ i ].num--;
		else if ( history[ i ].num == 2 && cor )
			history[ i ].num++;
		else if ( history[ i ].num < 2 && !cor )
			history[ i ].num++;
		else if ( history[ i ].num == 1 && cor )
			history[ i ].num--;
	}
}

fetch::fetch( RAM *rp, register_file *rf_in, branch_predictor *bp_in, decode *d_in, write_back *wb_in )
{
	ram = rp;
	rf = rf_in;
	wb = wb_in;
	d = d_in;
	bp = bp_in;
	halt = false;
	inst_count = 0;
}

void fetch::flush ( int num )
{
	deque<instruction> copy;
	instruction inst;
	while ( !insts.empty() )
	{
		inst = insts.front();
		insts.pop_front();
		if ( inst.num < num )
			copy.push_back( inst );
	}
	insts = copy;
}

void fetch::fetch_instruction ()
{
	instruction inst;
	while ( insts.size() < NUM_ALU )
	{
		inst = ram->code[ rf->pc ];
		inst.num = inst_count;
		inst.pc = rf->pc;
		if ( inst.op == B )
		{
			rf->pc += inst.dest - 1;
			inst.taken = true;
		}
		else if ( inst.op == BLEQ )
		{
			if ( bp->predict( inst ) )
			{
				rf->pc += inst.a2 - 1;
				inst.taken = true;
			}
		}
		inst_count++;
		insts.push_back( inst );
		wb->insert_place_holder( inst.num );
		rf->pc++;
	}
}

void fetch::push ()
{
	if ( d->insts.size() == 0 )
		halt = false;
	else
		halt = true;
	while ( d->insts.size() < NUM_ALU )
	{
		d->buffer_dec( insts.front() );
		insts.pop_front();
	}
}

processor::processor ( int code, int data, RAM *rp )
	: ram ( rp )
	, wb ( &rf, rp )
	, rs ( this, exec, &rf )
	, d ( &rf, &rs )
	, bp ()
	, f ( rp, &rf, &bp, &d, &wb )
{
	for ( int i = 0; i < NUM_ALU; i++ )
		exec[ i ] = execute( this, rp, &rf, &wb, &bp );
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
	for( int i = 0; i < NUM_ALU; i++ )
		exec[ i ].flush( num );
	wb.flush( num );

	refresh_db();
}

void processor::check ( int num )
{
	int i;
	instruction inst;

	// writeback
	for ( i = 0; i < wb.buffer.size(); i++ )
	{
		inst = wb.buffer[ i ];
		if ( inst.num < num )
		{
			switch ( inst.op )
			{
				case ADD:
				case ADDI:
				case SUB:
				case SUBI:
				case MUL:
				case DIV:
				case LD:
				case LDI:
					rf.dirty[ inst.dest ] = true;
					break;
				default:
					break;
			}
		}
	}

	// execute
	for ( i = 0; i < NUM_ALU; i++ )
	{
		if ( !exec[ i ].halt )
		{
			inst = exec[ i ].inst_in;
			if ( inst.num < num )
			{
				switch ( inst.op )
				{
					case ADD:
					case ADDI:
					case SUB:
					case SUBI:
					case MUL:
					case DIV:
					case LD:
					case LDI:
						rf.dirty[ inst.dest ] = true;
						break;
					default:
						break;
				}
			}
		}
	}

	for ( i = 0; i < rs.out_buffer.size(); i++ )
	{
		inst = rs.out_buffer[ i ];
		if ( inst.num < num )
		{
			switch ( inst.op )
			{
				case ADD:
				case ADDI:
				case SUB:
				case SUBI:
				case MUL:
				case DIV:
				case LD:
				case LDI:
					rf.dirty[ inst.dest ] = true;
					break;
				default:
					break;
			}
		}
	}
}

void processor::refresh_db ()
{
	int i;
	deque<instruction> insts;
	instruction inst;

	for ( i = 0; i < NUM_PHYS_REG; i++ )
		rf.dirty[ i ] = false;

	for ( i = 0; i < wb.buffer.size(); i++ )
		insts.push_back( wb.buffer[ i ] );

	for ( i = 0; i < NUM_ALU; i++ )
	{
		if ( !exec[ i ].halt )
			insts.push_back( exec[ i ].inst_out );
	}

	for ( i = 0; i < rs.wait_buffer.size(); i++ )
		insts.push_back( rs.wait_buffer[ i ] );
	for ( i = 0; i < rs.out_buffer.size(); i++ )
		insts.push_back( rs.out_buffer[ i ] );

	sort( insts.begin(), insts.end(), sort_inst );

	for ( i = 0; i < insts.size(); i++ )
	{
		inst = insts[ i ];
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
				rf.dirty[ inst.dest ] = true;
				break;
			default:
				break;
		}
	}
}

int processor::tick ()
{
	for ( int i = 0; i < NUM_ALU; i++ )
		exec[ i ].exec();
	completed_instructions += wb.write();
	rs.fetch_operands();
	d.fetch_operands();
	f.fetch_instruction();

	return 0;
}

int processor::tock ()
{
	for ( int i = 0; i < NUM_ALU; i++ )
		exec[ i ].push();
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
