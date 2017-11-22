#include <ncurses.h>
#include <curses.h>
#include <dirent.h>
#include "components.h"

void init_ncurses();
void init_params( RAM* ram_in, processor* proc_in );
void tidy_up_ncurses();
void redraw();
const char *selection_menu();
