#include <unistd.h>
#include "gui.h"

int main ( int argc, char *argv[] )
{
	init_ncurses();
	string directory = "test_code/";
	string file_name = selection_menu();

	while ( file_name != "Exit" )
	{
		string file_path = directory + file_name;

		cout << file_name;

		ifstream file ( file_path );
		string line;
		int num_code= 0;
		int data = 1;
		int num_data = 0;

		while (getline ( file, line ))
		{
			if ( data )
			{
				if ( line.empty() )
					data = 0;
				else
					num_data++;
			}
			else
			{
				num_code++;
			}
		}

		file.clear ();
		file.seekg ( 0, ios::beg );
		RAM ram ( num_code, num_data );
		processor p ( num_code, num_data, &ram );
		int inst_index = 0;
		int data_index = 0;

		init_params( &ram, &p );

		data = 1;

		while (getline ( file, line ))
		{
			if ( data )
			{
				if ( line.empty() )
					data = 0;
				else
				{
					p.ram->add ( data_index, stoi( line ) );
					data_index++;
				}
			}
			else
			{
				istringstream iss( line );
				string inst = "";
				string dest = "";
				string a1 = "";
				string a2 = "";

				iss >> inst;

				if ( !( iss >> dest && iss >> a1 ) || inst.compare("NOP") )
				{
					iss >> a2;
					instruction next_inst ( inst, dest, a1, a2 );
					p.ram->add ( inst_index, next_inst );
				}
				else
				{
					cerr << "Error: unparseable assembly\n";
					return -1;
				}

				inst_index++;
			}
		}

		redraw();

		char a = getchar();
		int finished = 0;
		bool run = false;
		int i = 0;

		a = getch();

		while ( (run || a != 'x') && !finished )
		{
			if ( a == 'r' )
				run = true;

			p.tick();
			finished = p.tock();
			redraw();

			if ( !run )
				a = getch();
			else
				usleep(50000);

			i++;
		}

		file_name = selection_menu();
	}

	tidy_up_ncurses();

	return 0;
}
