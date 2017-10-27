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
	void buffer_write ( instruction inst );
	void write ();
	void flush ();
};

class processor;

class execute
{
public:
	instruction inst1, inst2;
	bool halt, write;
	processor *proc;
	RAM *ram;
	register_file *rf;
	write_back *wb;

	execute ( processor* proc_in, RAM *rp, register_file *rf_in, write_back *out );
	int exec ();
	void push ();
	void buffer_exec ( instruction i );
};

class fetch_decode
{
public:
	instruction inst;
	RAM *ram;
	execute *exec;
	register_file *rf;
	bool halt;

	fetch_decode ( RAM *rp, register_file *rf_in, execute *e_in );
	void fetch_instruction ();
	void push ();
	void flush ();
};

class processor
{
public:
	RAM *ram;
	register_file rf;
	write_back wb;
	execute exec;
	fetch_decode fd;
	int cycles;
	int completed_instructions;
	int num_code;
	int num_data;

	processor ( int code, int data, RAM *rp );
	void flush ();
	int tick ();
	int tock ();
};
