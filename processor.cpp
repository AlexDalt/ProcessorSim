#include "components.h"

int main ( int argc, char *argv[] )
{
	if ( argc < 1 )
	{
		cerr << "Error: too few arguments\n";
		return -1;
	}

	ifstream file ( argv[1] );
	string line;
	int num_lines = 0;

	while (getline ( file, line ))
		num_lines++;

	file.clear ();
	file.seekg ( 0, ios::beg );
	RAM ram ( num_lines );
	processor p ( num_lines, &ram );
	int inst_index = 0;

	while (getline ( file, line ))
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

	for ( int i = 0 ; i < num_lines ; i++ )
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

	while ( a != 'x' && !finished)
	{
		if ( i % 2 == 0 )
		{
			p.tick();
		}
		else
		{
			finished = p.tock();
		}
		i++;
		a = getchar();
	}

	return 0;
}
