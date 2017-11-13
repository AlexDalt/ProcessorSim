#include "gui.h"

int main ( int argc, char *argv[] )
{
	if ( argc < 1 )
	{
		cerr << "Error: too few arguments\n";
		return -1;
	}

	ifstream file ( argv[1] );
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

	for ( int i = 0 ; i < num_code ; i++ )
	{
		cout << "Opp " << i << " :";
		cout << " op - " << p.ram->code[i].op;
		cout << " dest - " << p.ram->code[i].dest;
		cout << " a1 - " << p.ram->code[i].a1;
		cout << " a2 - " << p.ram->code[i].a2 << endl;
	}

	char a = getchar();
	int i = 0;
	int finished = 0;
	bool run = false;

	init_ncurses( &ram, &p );

	while ( (run || a != 'x') && !finished )
	{
		if ( a == 'r' )
			run = true;

		if ( i % 2 == 0 )
		{
			p.tick();
		}
		else
		{
			finished = p.tock();
		}
		i++;

		if ( !run )
			a = getchar();
	}

	tidy_up_ncurses();

	return 0;
}
