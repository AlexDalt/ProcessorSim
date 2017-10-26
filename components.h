#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <queue>

using namespace std;

#define NUM_ARCH_REG 8
#define NUM_PHYS_REG 32

enum Operations { NOP, ADD, ADDI, SUB, SUBI, MUL, DIV, LD, LDI, BLEQ, B, ST, STI };

class instruction
{
public:
	Operations op;
	int dest;
	int a1;
	int a2;

	instruction( string inst="NOP", string d="", string b1="", string b2="" );
};

class RAM
{
public:
	instruction *code;
	int *data;

	RAM ( int code_size = 0, int data_size=0 );
	int add ( int index, instruction i );
	int add ( int index, int d );
};

class register_file
{
public:
	int pc;
	int r[NUM_ARCH_REG];
	bool dirty[NUM_ARCH_REG];

	register_file();
};

class write_back
{
public:
	register_file *rf;
	queue <instruction> buffer;

	write_back ( register_file *reg_pointer );
	void buffer_write ( instruction *inst );
	void write ();
};

class fetch_decode_execute
{
public:
	write_back *wb;
	RAM *ram;
	register_file *rf;
	instruction inst;
	bool halt;

	fetch_decode_execute ( RAM *rp, register_file *rf_in, write_back *out );
	int execute ();
	void push ();
};

class processor
{
public:
	RAM *ram;
	register_file rf;
	write_back wb;
	fetch_decode_execute fde;
	int cycles;
	int completed_instructions;
	int num_code;
	int num_data;

	processor ( int code, int data, RAM *rp );
	int tick ();
	int tock ();
};
