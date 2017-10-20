#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

enum Operations { NOP, ADD, LD };

class instruction
{
public:
	Operations op;
	std::string dest;
	std::string a1;
	std::string a2;

	instruction ( std::string inst="NOP", std::string d="", std::string b1="", std::string b2="" )
	{
		if ( inst.string::compare ( "NOP" ) == 0 )
			op = NOP;
		else if ( inst.string::compare ( "ADD" ) == 0 )
			op = ADD;
		else if ( inst.string::compare ( "LD" ) == 0 )
			op = LD;

		dest = d;
		a1 = b1;
		a2 = b2;
	}
};

class RAM
{
public:
	instruction *code;
	int *data;

	RAM ( int code_size=0, int data_size=0 )
	{
		code = new instruction[code_size];
		data = new int[data_size];
	}

	int add ( int index, instruction i )
	{
		try
		{
			code[index] = i;
		}
		catch ( int e )
		{
			cerr << "inserting instruction in RAM that doesn't exist";
		}
		return 0;
	}

	int add ( int index, int d )
	{
		try
		{
			data[index] = d;
		}
		catch ( int e )
		{
			cerr << "inserting data in RAM that doesn't exist";
		}
		return 0;
	}
};

class register_file
{
public:
	int pc;
	int r[8];
	int p[32];
};

class fetch_decode_execute
{

};

class write_back
{

};

class processor
{

};

int main ( int argc, char *argv[] )
{
	if ( argc < 1 )
	{
		cerr << "Error: too few arguments\n";
		return -1;
	}

	std::ifstream file ( argv[1] );
	std::string line;
	int num_lines = 0;

	while (std::getline ( file, line ))
		num_lines++;

	file.clear ();
	file.seekg ( 0, std::ios::beg );
	RAM ram ( num_lines );
	int inst_index = 0;

	while (std::getline ( file, line ))
	{
		std::istringstream iss( line );
		std::string inst = "";
		std::string dest = "";
		std::string a1 = "";
		std::string a2 = "";

		iss >> inst;

		if ( !( iss >> dest && iss >> a1 ) || inst.compare("NOP") )
		{
			iss >> a2;
			instruction next_inst ( inst, dest, a1, a2 );
			ram.add ( inst_index, next_inst );
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
		cout << " op - " << ram.code[i].op;
		cout << " dest - " << ram.code[i].dest;
		cout << " a1 - " << ram.code[i].a1;
		cout << " a2 - " << ram.code[i].a2 << endl;
	}
	return 0;
}
