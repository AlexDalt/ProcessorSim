#include "gui.h"
int row, col;
RAM *ram;
processor *proc;
WINDOW *ram_win, *proc_win, *help_win;

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

	help_win = create_win( 5, COLS - 10, LINES - 5, 5 );
	mvwprintw( help_win, 2, (COLS - 58)/2, "<ENTER> - Step execition, <r> - run until finish");
	wrefresh( help_win );
	ram_win = create_win( LINES - 5, COLS/4, 0, 0 );
	proc_win = create_win( LINES - 5, COLS*3/4, 0, COLS/4 );

	refresh();
}
