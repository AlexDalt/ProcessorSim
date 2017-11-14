#include "gui.h"
int row, col;
RAM *ram;
processor *proc;
WINDOW *ram_win, *proc_win, *help_win, *data_win, *program_win;

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

void refresh_data( WINDOW* win, RAM* ram )
{
	for( int i = 0; i < ram->d_size; i++ )
	{
		mvwprintw( win, i+1, 1, "%2d: %d", i, ram->data[ i ] );
	}
	box( win, 0, 0 );
	wrefresh( win );
}

void refresh_program( WINDOW* win, RAM* ram )
{
	instruction inst;
	for( int i = 0; i < ram->c_size; i++ )
	{
		inst = ram->code[ i ];
		switch ( inst.op )
		{
			case NOP:
				mvwprintw( win, i+1, 1, "%d: NOP", i);
				break;
			case ADD:
				mvwprintw( win, i+1, 1, "%d: ADD r%d r%d r%d", i, inst.dest, inst.a1, inst.a2 );
				break;
			case ADDI:
				mvwprintw( win, i+1, 1, "%d: ADDI r%d r%d %d", i, inst.dest, inst.a1, inst.a2 );
				break;
			case SUB:
				mvwprintw( win, i+1, 1, "%d: SUB r%d r%d r%d", i, inst.dest, inst.a1, inst.a2);
				break;
			case SUBI:
				mvwprintw( win, i+1, 1, "%d: SUBI r%d r%d %d", i, inst.dest, inst.a1, inst.a2);
				break;
			case MUL:
				mvwprintw( win, i+1, 1, "%d: MUL r%d r%d r%d", i, inst.dest, inst.a1, inst.a2);
				break;
			case DIV:
				mvwprintw( win, i+1, 1, "%d: DIV  r%d r%d r%d", i, inst.dest, inst.a1, inst.a2);
				break;
			case LD:
				mvwprintw( win, i+1, 1, "%d: LD r%d r%d", i, inst.dest, inst.a1 );
				break;
			case LDI:
				mvwprintw( win, i+1, 1, "%d: LDI r%d %d", i, inst.dest, inst.a1 );
				break;
			case BLEQ:
				mvwprintw( win, i+1, 1, "%d: BLEQ r%d r%d %d", i, inst.dest, inst.a1, inst.a2);
				break;
			case B:
				mvwprintw( win, i+1, 1, "%d: B %d", i, inst.dest );
				break;
			case ST:
				mvwprintw( win, i+1, 1, "%d: ST r%d r%d", i, inst.dest, inst.a1 );
				break;
			case STI:
				mvwprintw( win, i+1, 1, "%d: STI r%d %d", i, inst.dest, inst.a1 );
				break;
		}
	}
	box( win, 0, 0 );
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
	mvwprintw( help_win, 1, (COLS - 58)/2, "<ENTER> - Step execition, <r> - run until finish");
	wrefresh( help_win );

	ram_win = create_win( LINES - 4, COLS/4, 0, 0 );
	mvwprintw( ram_win, 1, (COLS/4 - 3)/2, "RAM" );
	mvwprintw( ram_win, 2, (COLS/4 - 4)/2, "data" );
	mvwprintw( ram_win, (LINES - 4)/2 + 1, (COLS/4 - 4)/2, "code" );

	data_win = create_subwin( ram_win, (LINES - 4)/2 - 2, COLS/4 - 2, 3, 1 );
	refresh_data( data_win, ram );

	program_win = create_subwin( ram_win, (LINES - 4)/2 - 2, COLS/4 - 2, (LINES - 4)/2 + 2, 1 );
	refresh_program( program_win, ram );
	wrefresh( ram_win );

	proc_win = create_win( LINES - 4, COLS*3/4, 0, COLS/4 );

	refresh();
}
