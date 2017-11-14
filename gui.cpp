#include "gui.h"
int row, col;
RAM *ram;
processor *proc;
WINDOW *ram_win, *proc_win, *help_win, *data_win, *program_win, *rf_win;

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
	mvwprintw( win, 1, 1, "pc - %d", proc->rf.pc );
	for( int i = 0; i < NUM_ARCH_REG; i++)
	{
		mvwprintw( win, i+2, 1, "r%d - %d", i, proc->rf.r[ i ] );
	}
	box( win, 0, 0 );
	mvwprintw( win, 0, (x - 2)/2, "rf" );
	wrefresh( win );
}

void init_ncurses( RAM* ram_in, processor* proc_in )
{
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	getmaxyx( stdscr, row, col );
	ram = ram_in;
	proc = proc_in;

	refresh();

	help_win = create_win( 4, COLS - 10, LINES - 4, 5 );
	mvwprintw( help_win, 1, (COLS - 72)/2, "<anything> - Step execition, <r> - run until finish, <x> - exit");
	wrefresh( help_win );

	int ram_win_h = LINES - 5;
	int ram_win_w = COLS/4;
	ram_win = create_win( ram_win_h, ram_win_w, 1, 0 );
	mvwprintw( ram_win, 0, (ram_win_w - 7)/2, "MEMORY" );

	data_win = create_subwin( ram_win, ram_win_h/2 - 1, ram_win_w - 2, 2, 1 );
	refresh_data( data_win );

	program_win = create_subwin( ram_win, ram_win_h/2 - 1, ram_win_w - 2, ram_win_h/2 + 1, 1 );
	refresh_program( program_win );
	wrefresh( ram_win );

	int proc_win_h = LINES - 5;
	int proc_win_w = COLS - ram_win_w;
	proc_win = create_win( proc_win_h, proc_win_w, 1, ram_win_w );
	mvwprintw( proc_win, 0,(proc_win_w - 9)/2, "PROCESSOR" );
	wrefresh( proc_win );

	rf_win = create_subwin( proc_win, proc_win_h - 2, proc_win_w/5 - 1, 2, ram_win_w + proc_win_w - (proc_win_w/5));
	refresh_reg_file( rf_win );

	refresh();
}

void redraw()
{
	refresh_data( data_win );
	refresh_program( program_win );
	refresh_reg_file( rf_win );
}
