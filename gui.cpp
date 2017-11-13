#include "gui.h"
int row, col;
RAM *ram;
processor *proc;
WINDOW *ram_win, *proc_win;

WINDOW *create_win( int height, int width, int starty, int startx )
{
	WINDOW *win;
	win = newwin( height, width, starty, startx );
	box( win, 0, 0 );
	wrefresh( win );

	return win;
}

void delete_win( WINDOW* win ){
	wborder( win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' );
	wrefresh( win );
	delwin( win );
}

void tidy_up_ncurses()
{
	endwin();
}

void init_ncurses( RAM* ram_in, processor* proc_in )
{
	initscr();
	cbreak();
	noecho();
	getmaxyx( stdscr, row, col );
	ram = ram_in;
	proc = proc_in;

	refresh();

	ram_win = create_win( LINES, COLS/4, 0, 0 );
	proc_win = create_win( LINES, COLS*3/4, 0, COLS/4 );

	refresh();
}
