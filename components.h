#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <deque>

using namespace std;

#define NUM_ARCH_REG 8
#define NUM_PHYS_REG 32
#define NUM_ALU 1

enum Operations { NOP, ADD, ADDI, SUB, SUBI, MUL, DIV, LD, LDI, BLEQ, B, ST, STI, PLACE_HOLDER };

class instruction
{
public:
	Operations op;
	int dest;
	int a1;
	int a2;
	int num;

	instruction( string inst="NOP", string d="", string b1="", string b2="" );
};

class RAM
{
public:
	int c_size, d_size;
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
	deque <instruction> buffer;

	write_back ( register_file *reg_pointer );
	void insert_place_holder ( int inst_num );
	void buffer_write ( instruction inst );
	int write ();
	void flush ( int num );
};

class processor;

class execute
{
public:
	instruction inst_in, inst_out;
	bool halt, finished, wait;
	processor *proc;
	RAM *ram;
	register_file *rf;
	write_back *wb;
	int rem_exec;

	execute ( processor* proc_in, RAM *rp, register_file *rf_in, write_back *out );
	void buffer_exec ( instruction i );
	void flush ( int num );
	void exec ();
	void push ();
};

class decode
{
public:
	instruction inst_in, inst_out;
	execute *exec;
	register_file *rf;
	bool wait, halt;

	decode ( register_file *rf_in, execute *e_in );
	void buffer_dec ( instruction i );
	void fetch_operands ();
	void push ();
	void flush ( int num );
};

class fetch
{
public:
	instruction inst;
	RAM *ram;
	decode *d;
	write_back *wb;
	register_file *rf;
	bool halt;
	int inst_count;

	fetch ( RAM *rp, register_file *rf_in, decode *d_in, write_back *wb_in );
	void fetch_instruction ();
	void push ();
	void flush ( int num );
};

class processor
{
public:
	RAM *ram;
	register_file rf;
	write_back wb;
	execute exec;
	decode d;
	fetch f;
	int cycles;
	int completed_instructions;
	int num_code;
	int num_data;
	float inst_per_cycle;

	processor ( int code, int data, RAM *rp );
	void flush ( int num );
	int tick ();
	int tock ();
};
