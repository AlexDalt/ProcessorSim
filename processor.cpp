#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <queue>
using namespace std;

#define NUM_ARCH_REG 8
#define NUM_PHYS_REG 32

enum Operations { NOP, ADD, LDI };

class instruction
{
public:
	Operations op;
	int dest;
	int a1;
	int a2;

	instruction ( string inst="NOP", string d="", string b1="", string b2="" )
	{
		if ( inst.compare ( "NOP" ) == 0 )
			op = NOP;
		else if ( inst.compare ( "ADD" ) == 0 )
			op = ADD;
		else if ( inst.compare ( "LDI" ) == 0 )
			op = LDI;

		try
		{
			if ( !(d.empty()) )
				dest = stoi ( d, nullptr, 10 );
			if ( !(b1.empty()) )
				a1 = stoi ( b1, nullptr, 10 );
			if ( !(b2.empty())  )
				a2 = stoi ( b2, nullptr, 10 );
		}
		catch ( exception e )
		{
			cerr << "Error: unable to parse assembly into internal instructions" << endl;
			cerr << "inst: " << inst << ", dest: " << dest << ", arg1: " << b1 << ", arg2: " << b2 <<endl;
		}
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
			cerr << "inserting instruction in RAM that doesn't exist" << endl;
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
	int r[NUM_ARCH_REG];
	int p[NUM_PHYS_REG];

	register_file()
	{
		pc = 0;
	}
};

/*

class write_back
{
public:
	register_file *rf;
	queue <instruction> buffer;

	write_back ( register_file *reg_pointer )
	{
		rf = reg_pointer;
	}

	void buffer_write ( instruction *inst )
	{
		instruction i = *inst;
		buffer.push ( i );
	}

	void write ()
	{
		if ( !buffer.empty() ){
			instruction i = buffer.front();
			buffer.pop();
			cout << "	write - r" << i.dest << " " << i.a1 << endl;
			rf->r[ i.dest ] = i.a1;
		}
		else
			cout << "	write - buffer empty" << endl;
	}
};

*/

class fetch_decode_execute
{
public:
	//write_back *wb;
	RAM *ram;
	register_file *rf;
	instruction inst;

	fetch_decode_execute ( RAM *rp, register_file *rf_in/*, write_back *out */)
	{
		ram = rp;
		rf = rf_in;
		//wb = out;
	}

	void execute ()
	{
		inst = ram->code[rf->pc];
		switch ( inst.op )
		{
			case NOP:
				cout << "	fde - NOP" << endl;
				break;
			case ADD:
				cout << "	fde - ADD r" << inst.dest << " r" << inst.a1 << " r" << inst.a2 << endl;
				rf->r[ inst.dest ] = rf->r[ inst.a1 ] + rf->r[ inst.a2 ];
				break;
			case LDI:
				cout << "	fde - LDI r" << inst.dest << " " << inst.a1 << endl;
				rf->r[ inst.dest ] = inst.a1;
				break;
		}
	}

	void push ()
	{
		//cout << "	fde - buffer write" << endl;
		//wb->buffer_write ( &inst );
	}
};

class processor
{
public:
	RAM *ram;
	register_file rf;
	//write_back wb;
	fetch_decode_execute fde;
	int cycles;

	processor ( RAM *rp )
		:/* wb ( &rf )
		,*/ fde ( rp, &rf /*, &wb*/ )
		, ram ( rp )
	{
		cycles = 0;
	}

	void tick ()
	{
		cout << "Tick:" << endl;
		for ( int i = 0 ; i < NUM_ARCH_REG ; i++ )
			cout << "r" << i << " - " << rf.r[ i ] << ", ";
		cout << endl;
		fde.execute();
		//wb.write();
	}

	void tock ()
	{
		cout << "Tock:" << endl;
		//fde.push();
		rf.pc++;
		cycles++;
	}
};

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
	processor p ( &ram );
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

	while ( a != 'x' )
	{
		if ( i % 2 == 0 )
		{
			p.tick();
		}
		else
		{
			p.tock();
		}
		i++;
		a = getchar();
	}

	return 0;
}
