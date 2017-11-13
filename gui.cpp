#include "gui.h"
int row, col;
RAM* ram;
processor* proc;

void init_ncurses( RAM* ram_in, processor* proc_in )
{
	initscr();
	raw(); 		// turns off line buffering
	noecho(); 	// doesn't echo input to stdout
	getmaxyx( stdscr, row, col );
	ram = ram_in;
	proc = proc_in;
}

void tidy_up_ncurses()
{
	endwin();
}
