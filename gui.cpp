#include "gui.h"
#include "menu.h"

int row, col;
RAM *ram;
processor *proc;
WINDOW *ram_win, *proc_win, *help_win, *data_win, *program_win, *rf_win, *fetch_win, *decode_win, *exec_win, *wb_win, *bp_win;

WINDOW *create_win( int height, int width, int starty, int startx )
{
	WINDOW *win;
	win = newwin( height, width, starty, startx );
	box( win, 0, 0 );
	wrefresh( win );

	return win;
}

WINDOW *create_subwin( WINDOW *orig, int height, int width, int starty, int startx )
{
	WINDOW *win;
	win = subwin( orig, height, width, starty, startx );
	box( win, 0, 0 );
	wrefresh( win );
	wrefresh( orig );

	return win;
}

void delete_win( WINDOW* win )
{
	wborder( win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' );
	wrefresh( win );
	delwin( win );
}

void tidy_up_ncurses()
{
	delete_win( ram_win );
	delete_win( proc_win );
	delete_win( help_win );
	endwin();
}

void refresh_data( WINDOW* win )
{
	werase( win );
	for( int i = 0; i < ram->d_size; i++ )
	{
		mvwprintw( win, i+1, 1, "%2d: %d", i, ram->data[ i ] );
	}
	box( win, 0, 0 );
	mvwprintw( win, 0, (COLS/4 - 6)/2, "data" );
	wrefresh( win );
}

void refresh_program( WINDOW* win )
{
	instruction inst;

	werase( win );
	for( int i = 0; i < ram->c_size; i++ )
	{
		inst = ram->code[ i ];
		switch ( inst.op )
		{
			case NOP:
				mvwprintw( win, i+1, 1, "%2d: NOP", i);
				break;
			case ADD:
				mvwprintw( win, i+1, 1, "%2d: ADD r%d r%d r%d", i, inst.dest, inst.a1, inst.a2 );
				break;
			case ADDI:
				mvwprintw( win, i+1, 1, "%2d: ADDI r%d r%d %d", i, inst.dest, inst.a1, inst.a2 );
				break;
			case SUB:
				mvwprintw( win, i+1, 1, "%2d: SUB r%d r%d r%d", i, inst.dest, inst.a1, inst.a2);
				break;
			case SUBI:
				mvwprintw( win, i+1, 1, "%2d: SUBI r%d r%d %d", i, inst.dest, inst.a1, inst.a2);
				break;
			case MUL:
				mvwprintw( win, i+1, 1, "%2d: MUL r%d r%d r%d", i, inst.dest, inst.a1, inst.a2);
				break;
			case DIV:
				mvwprintw( win, i+1, 1, "%2d: DIV  r%d r%d r%d", i, inst.dest, inst.a1, inst.a2);
				break;
			case LD:
				mvwprintw( win, i+1, 1, "%2d: LD r%d r%d", i, inst.dest, inst.a1 );
				break;
			case LDI:
				mvwprintw( win, i+1, 1, "%2d: LDI r%d %d", i, inst.dest, inst.a1 );
				break;
			case BLEQ:
				mvwprintw( win, i+1, 1, "%2d: BLEQ r%d r%d %d", i, inst.dest, inst.a1, inst.a2);
				break;
			case B:
				mvwprintw( win, i+1, 1, "%2d: B %d", i, inst.dest );
				break;
			case ST:
				mvwprintw( win, i+1, 1, "%2d: ST r%d r%d", i, inst.dest, inst.a1 );
				break;
			case STI:
				mvwprintw( win, i+1, 1, "%2d: STI r%d %d", i, inst.dest, inst.a1 );
				break;
			default:
				break;
		}
	}
	box( win, 0, 0 );
	mvwprintw( win, 0, (COLS/4 - 6)/2, "code" );
	wrefresh( win );
}

void refresh_reg_file( WINDOW *win )
{
	int x, y;
	getmaxyx( win, y, x );
	werase( win );
	mvwprintw( win, 1, 1, "pc - %3d", proc->rf.pc );
	for( int i = 0; i < NUM_ARCH_REG; i++)
	{
		if( proc->rf.dirty[ i ] )
			wattron( win, COLOR_PAIR( 1 ) );
		mvwprintw( win, i+2, 1, "r%d - %3d", i, proc->rf.r[ i ] );
		wattroff( win, COLOR_PAIR( 1 ) );
	}
	box( win, 0, 0 );
	mvwprintw( win, 0, (x - 2)/2, "rf" );
	wrefresh( win );
}

void refresh_fetch( WINDOW *win )
{
	int x, y, maxx, maxy;
	instruction inst = ram->code[ proc->rf.pc ];
	inst.num = proc->f.inst_count;
	getmaxyx( win, maxy, maxx );
	werase( win );
	x = (maxx-15)/2;
	y = (maxy-2)/2;
	switch ( inst.op )
	{
		case NOP:
			mvwprintw( win, y, (maxx-3)/2, "%d: NOP", inst.num);
			break;
		case ADD:
			mvwprintw( win, y, x, "%d: ADD r%d r%d r%d", inst.num, inst.dest, inst.a1, inst.a2 );
			break;
		case ADDI:
			mvwprintw( win, y, x, "%d: ADDI r%d r%d %d", inst.num, inst.dest, inst.a1, inst.a2 );
			break;
		case SUB:
			mvwprintw( win, y, x, "%d: SUB r%d r%d r%d", inst.num, inst.dest, inst.a1, inst.a2);
			break;
		case SUBI:
			mvwprintw( win, y, x, "%d: SUBI r%d r%d %d", inst.num, inst.dest, inst.a1, inst.a2);
			break;
		case MUL:
			mvwprintw( win, y, x, "%d: MUL r%d r%d r%d", inst.num, inst.dest, inst.a1, inst.a2);
			break;
		case DIV:
			mvwprintw( win, y, x, "%d: DIV  r%d r%d r%d", inst.num, inst.dest, inst.a1, inst.a2);
			break;
		case LD:
			mvwprintw( win, y, (maxx - 8)/2, "%d: LD r%d r%d", inst.num, inst.dest, inst.a1 );
			break;
		case LDI:
			mvwprintw( win, y, (maxx - 8)/2, "%d: LDI r%d %d", inst.num, inst.dest, inst.a1 );
			break;
		case BLEQ:
			mvwprintw( win, y, x, "%d: BLEQ r%d r%d %d", inst.num, inst.dest, inst.a1, inst.a2);
			break;
		case B:
			mvwprintw( win, y, (maxx - 3)/2, "%d: B %d", inst.num, inst.dest );
			break;
		case ST:
			mvwprintw( win, y, (maxx - 8)/2, "%d: ST r%d r%d", inst.num, inst.dest, inst.a1 );
			break;
		case STI:
			mvwprintw( win, y, (maxx - 8)/2, "%d: STI r%d %d", inst.num, inst.dest, inst.a1 );
			break;
		default:
			break;
	}
	mvwprintw( win, maxy-2, (maxx - 6)/2, "halt=%d", proc->f.halt );
	box( win, 0, 0 );
	mvwprintw( win, 0, (maxx - 5)/2, "fetch" );
	wrefresh( win );
}

void refresh_bp( WINDOW *win )
{
	int maxx, maxy;
	getmaxyx( win, maxy, maxx );
	werase( win );
	box( win, 0, 0 );
	mvwprintw( win, 0, (maxx - 16)/2, "branch predictor" );
	wrefresh( win );
}

void refresh_decode( WINDOW *win )
{
	instruction inst = proc->d.inst_in;
	int x, y, maxx, maxy;
	getmaxyx( win, maxy, maxx );
	x = (maxx-15)/2;
	y = (maxy-2)/2;

	werase( win );
	if ( !(proc->d.halt) && !(inst.op == NOP) )
	{
		switch ( inst.op )
		{
			case NOP:
				mvwprintw( win, y, (maxx-3)/2, "%d: NOP");
				break;
			case ADD:
				mvwprintw( win, y, x, "%d: ADD r%d r%d r%d", inst.num, inst.dest, inst.a1, inst.a2 );
				break;
			case ADDI:
				mvwprintw( win, y, x, "%d: ADDI r%d r%d %d", inst.num, inst.dest, inst.a1, inst.a2 );
				break;
			case SUB:
				mvwprintw( win, y, x, "%d: SUB r%d r%d r%d", inst.num, inst.dest, inst.a1, inst.a2);
				break;
			case SUBI:
				mvwprintw( win, y, x, "%d: SUBI r%d r%d %d", inst.num, inst.dest, inst.a1, inst.a2);
				break;
			case MUL:
				mvwprintw( win, y, x, "%d: MUL r%d r%d r%d", inst.num, inst.dest, inst.a1, inst.a2);
				break;
			case DIV:
				mvwprintw( win, y, x, "%d: DIV  r%d r%d r%d", inst.num, inst.dest, inst.a1, inst.a2);
				break;
			case LD:
				mvwprintw( win, y, (maxx - 8)/2, "%d: LD r%d r%d", inst.num, inst.dest, inst.a1 );
				break;
			case LDI:
				mvwprintw( win, y, (maxx - 8)/2, "%d: LDI r%d %d", inst.num, inst.dest, inst.a1 );
				break;
			case BLEQ:
				mvwprintw( win, y, x, "%d: BLEQ r%d r%d %d", inst.num, inst.dest, inst.a1, inst.a2);
				break;
			case B:
				mvwprintw( win, y, (maxx - 3)/2, "%d: B %d", inst.num, inst.dest );
				break;
			case ST:
				mvwprintw( win, y, (maxx - 8)/2, "%d: ST r%d r%d", inst.num, inst.dest, inst.a1 );
				break;
			case STI:
				mvwprintw( win, y, (maxx - 8)/2, "%d: STI r%d %d", inst.num, inst.dest, inst.a1 );
				break;
			default:
				break;
		}
	}

	mvwprintw( win, maxy-2, (maxx - 6)/2, "halt=%d", proc->d.wait );
	box( win, 0, 0 );
	mvwprintw( win, 0, (maxx-6)/2, "decode" );
	wrefresh( win );
}

void refresh_exec( WINDOW *win )
{
	int x, y, maxx, maxy;
	instruction inst = proc->exec.inst_in;
	getmaxyx( win, maxy, maxx );
	werase( win );
	x = (maxx-13)/2;
	y = (maxy-2)/2;
	if( !(proc->exec.halt) ){
		switch ( inst.op )
		{
			case NOP:
				mvwprintw( win, y, (maxx-3)/2, "%d: NOP", inst.num);
				break;
			case ADD:
				mvwprintw( win, y, x, "%d: ADD r%d %d %d", inst.num, inst.dest, inst.a1, inst.a2 );
				break;
			case ADDI:
				mvwprintw( win, y, x, "%d: ADDI r%d %d %d", inst.num, inst.dest, inst.a1, inst.a2 );
				break;
			case SUB:
				mvwprintw( win, y, x, "%d: SUB r%d %d %d", inst.num, inst.dest, inst.a1, inst.a2);
				break;
			case SUBI:
				mvwprintw( win, y, x, "%d: SUBI r%d %d %d", inst.num, inst.dest, inst.a1, inst.a2);
				break;
			case MUL:
				mvwprintw( win, y, x, "%d: MUL r%d %d %d", inst.num, inst.dest, inst.a1, inst.a2);
				break;
			case DIV:
				mvwprintw( win, y, x, "%d: DIV r%d %d %d", inst.num, inst.dest, inst.a1, inst.a2);
				break;
			case LD:
				mvwprintw( win, y, (maxx - 7)/2, "%d: LD r%d %d", inst.num, inst.dest, inst.a1 );
				break;
			case LDI:
				mvwprintw( win, y, (maxx - 8)/2, "%d: LDI r%d %d", inst.num, inst.dest, inst.a1 );
				break;
			case BLEQ:
				mvwprintw( win, y, x, "%d: BLEQ %d %d %d", inst.num, inst.dest, inst.a1, inst.a2);
				break;
			case B:
				mvwprintw( win, y, (maxx - 3)/2, "%d: B %d", inst.num, inst.dest );
				break;
			case ST:
				mvwprintw( win, y, (maxx - 7)/2, "%d: ST %d %d", inst.num, inst.dest, inst.a1 );
				break;
			case STI:
				mvwprintw( win, y, (maxx - 8)/2, "%d: STI %d %d", inst.num, inst.dest, inst.a1 );
				break;
			default:
				break;
		}
	}
	mvwprintw( win, maxy-2, (maxx - 6)/2, "halt=%d", proc->exec.halt );
	box( win, 0, 0 );
	mvwprintw( win, 0, (maxx - 3)/2, "ALU" );
	wrefresh( win );
}

void refresh_wb( WINDOW *win )
{
	int x, maxx, maxy;
	getmaxyx( win, maxy, maxx );
	werase( win );
	x = (maxx-13)/2;

	for( int i = 0; i < proc->wb.buffer.size(); i++ )
	{
		instruction inst = proc->wb.buffer[ i ];
		switch ( inst.op )
		{
			case NOP:
				mvwprintw( win, i+1, 2, "%d: NOP", inst.num );
				break;
			case ADD:
				mvwprintw( win, i+1, 2, "%d: ADD r%d %d", inst.num, inst.dest, inst.a1, inst.a2 );
				break;
			case ADDI:
				mvwprintw( win, i+1, 2, "%d: ADDI r%d %d", inst.num, inst.dest, inst.a1, inst.a2 );
				break;
			case SUB:
				mvwprintw( win, i+1, 2, "%d: SUB r%d %d", inst.num, inst.dest, inst.a1, inst.a2 );
				break;
			case SUBI:
				mvwprintw( win, i+1, 2, "%d: SUBI r%d %d", inst.num, inst.dest, inst.a1, inst.a2 );
				break;
			case MUL:
				mvwprintw( win, i+1, 2, "%d: MUL r%d %d", inst.num, inst.dest, inst.a1, inst.a2 );
				break;
			case DIV:
				mvwprintw( win, i+1, 2, "%d: DIV  r%d %d", inst.num, inst.dest, inst.a1, inst.a2 );
				break;
			case LD:
				mvwprintw( win, i+1, 2, "%d: LD r%d %d", inst.num, inst.dest, inst.a1 );
				break;
			case LDI:
				mvwprintw( win, i+1, 2, "%d: LDI r%d %d", inst.num, inst.dest, inst.a1 );
				break;
			case BLEQ:
				mvwprintw( win, i+1, 2, "%d: BLEQ", inst.num );
				break;
			case B:
				mvwprintw( win, i+1, 2, "%d: B", inst.num );
				break;
			case ST:
				mvwprintw( win, i+1, 2, "%d: ST", inst.num );
				break;
			case STI:
				mvwprintw( win, i+1, 2, "%d: STI", inst.num );
				break;
			case PLACE_HOLDER:
				mvwprintw( win, i+1, 2, "%d: ", inst.num );
				break;
			default:
				break;
		}
	}
	mvwprintw( win, maxy - 2, (maxx - 6)/2, "halt=%d", (proc->wb.buffer.size() == 0) );
	box( win, 0, 0 );
	mvwprintw( win, 0, (maxx - 14)/2, "reorder buffer" );
	wrefresh( win );
}

void refresh_help( WINDOW *win )
{
	werase( win );
	box( win, 0, 0 );
	mvwprintw( win, 1, (COLS - 72)/2, "<anything> - step execition, <r> - run until finish, <x> - exit");
	mvwprintw( win, 2, (COLS - 82)/2, "completed instructions: %d, completed cycles: %d, instructions/cycle: %.3f", proc->completed_instructions, proc->cycles, proc->inst_per_cycle );
	wrefresh( win );
}

void init_ncurses()
{
	initscr();
	start_color();
	cbreak();
	noecho();
	curs_set(0);
	getmaxyx( stdscr, row, col );

	refresh();

	init_pair(1, COLOR_BLACK, COLOR_RED);

	help_win = create_win( 4, COLS - 10, LINES - 4, 5 );

	int ram_win_h = LINES - 5;
	int ram_win_w = COLS/4;
	ram_win = create_win( ram_win_h, ram_win_w, 1, 0 );
	mvwprintw( ram_win, 0, (ram_win_w - 7)/2, "MEMORY" );

	data_win = create_subwin( ram_win, ram_win_h/2 - 1, ram_win_w - 2, 2, 1 );

	program_win = create_subwin( ram_win, ram_win_h/2 - 1, ram_win_w - 2, ram_win_h/2 + 1, 1 );

	int proc_win_h = LINES - 5;
	int proc_win_w = COLS - ram_win_w;
	proc_win = create_win( proc_win_h, proc_win_w, 1, ram_win_w );
	mvwprintw( proc_win, 0,(proc_win_w - 9)/2, "PROCESSOR" );

	rf_win = create_subwin( proc_win, proc_win_h - 2, (proc_win_w-2)/5, 2, ram_win_w + proc_win_w - (proc_win_w/5) - 1);

	fetch_win = create_subwin( proc_win, (proc_win_h - 2)/4, (proc_win_w - (proc_win_w-2)/5 - 2)/2, 2, ram_win_w + 1);

	bp_win = create_subwin( proc_win, (proc_win_h - 2)/4, (proc_win_w - (proc_win_w-2)/5 - 2)/2, 2, ram_win_w + 2 + (proc_win_w - (proc_win_w-2)/5 - 2)/2 );

	decode_win = create_subwin( proc_win, (proc_win_h - 2)/4, proc_win_w - (proc_win_w-2)/5 - 2, (proc_win_h - 2)/4 + 2, ram_win_w + 1);

	exec_win = create_subwin( proc_win, (proc_win_h - 2)/4, proc_win_w - (proc_win_w-2)/5 - 2, 2*(proc_win_h - 2)/4 + 2, ram_win_w + 1 );

	wb_win = create_subwin( proc_win, (proc_win_h - 2)/4, proc_win_w - (proc_win_w-2)/5 - 2, 3*(proc_win_h - 2)/4 + 2, ram_win_w + 1 );

	refresh();
}

void init_params( RAM* ram_in, processor* proc_in )
{
	ram = ram_in;
	proc = proc_in;
}

void redraw()
{
	refresh_data( data_win );
	refresh_program( program_win )	;
	refresh_reg_file( rf_win );
	refresh_fetch( fetch_win );
	refresh_decode( decode_win );
	refresh_exec( exec_win );
	refresh_wb( wb_win );
	refresh_help( help_win );
	refresh_bp( bp_win );
}

const char *selection_menu()
{
	DIR *dir;
	struct dirent *ent;
	int i;

	int c;
	const char *r_value;
	ITEM **my_items;
	ITEM *cur_item;
	MENU *my_menu;
	WINDOW *my_menu_win;
	int num_items = 0;

	if( (dir = opendir( "test_code/" )) != NULL )
	{
		while( (ent = readdir (dir)) != NULL )
		{
			num_items++;
		}
		closedir( dir );
		string choices[ num_items + 2 ];
		dir = opendir( "test_code/" );
		for( i = 0; i < num_items; i++ )
		{
			ent = readdir( dir );
			choices[ i ] = ent->d_name;
		}
		closedir( dir );
		choices[ num_items ] = "Exit";

		my_items = (ITEM **)calloc( num_items, sizeof(ITEM *) );
		for( i = 2; i < num_items + 1; i++ )
			my_items[ i - 2 ] = new_item( choices[ i ].c_str(), (char *)NULL );

		my_items[ num_items - 1 ] = new_item( (char *)NULL, (char *)NULL );

		my_menu = new_menu( (ITEM **)my_items );

		my_menu_win = newwin( LINES-8, 40, 4, (COLS-40)/2 );
		keypad( my_menu_win, TRUE );

		set_menu_win( my_menu, my_menu_win );
		set_menu_sub( my_menu, derwin( my_menu_win, LINES-11, 38, 3, 1 ) );
		set_menu_mark( my_menu, " * " );
		box( my_menu_win, 0, 0 );
		mvwprintw( my_menu_win, 1, 2, "PROGRAMS" );
		mvwaddch( my_menu_win, 2, 0, ACS_LTEE );
		mvwhline( my_menu_win, 2, 1, ACS_HLINE, 38 );
		mvwaddch( my_menu_win, 2, 39, ACS_RTEE );

		post_menu( my_menu );
		wrefresh( my_menu_win );

		bool enter = false;
		while( (c = wgetch( my_menu_win )) && !enter )
		{
			switch(c)
			{
				case KEY_DOWN:
					menu_driver( my_menu, REQ_DOWN_ITEM );
					break;
				case KEY_UP:
					menu_driver( my_menu, REQ_UP_ITEM );
					break;
				case 10:
					cur_item = current_item( my_menu );
					r_value = item_name( cur_item );

					refresh();
					pos_menu_cursor( my_menu );
					enter = true;
					break;
			}
			wrefresh( my_menu_win );
		}
		unpost_menu( my_menu );
		free_menu( my_menu );
		for( i = 0; i < num_items; i++ )
			free_item( my_items[ i ] );
		wborder( my_menu_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' );
		wrefresh( my_menu_win );
		delwin( my_menu_win );
		box( proc_win, 0, 0 );
		int ram_win_w = COLS/4;
		int proc_win_w = COLS - ram_win_w;
		mvwprintw( proc_win, 0,(proc_win_w - 9)/2, "PROCESSOR" );
		wrefresh( proc_win );
		box( ram_win, 0, 0 );
		mvwprintw( ram_win, 0, (ram_win_w - 7)/2, "MEMORY" );
		wrefresh( ram_win );

		return r_value;
	}
	return NULL;
}
