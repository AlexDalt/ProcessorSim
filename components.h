#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <deque>
#include <algorithm>

using namespace std;

#define NUM_ARCH_REG 8
#define NUM_PHYS_REG 8
#define NUM_ALU 6
#define RES_SIZE 8

#define BRANCH 3
/*
 * 0: static taken
 * 1: static not taken
 * 2: backwards taken, forwards not
 * 3: 2-bit saturating counter
 */

enum Operations { NOP, ADD, ADDI, SUB, SUBI, MUL, DIV, LD, LDI, BLEQ, B, ST, STI, PLACE_HOLDER };

class instruction
{
public:
	Operations op;
	int dest, a1, a2, num, pc;
	bool d1, d2, taken;

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
	RAM *ram;
	deque <instruction> buffer;

	write_back ( register_file *reg_pointer, RAM* ram_in);
	void insert_place_holder ( int inst_num );
	void buffer_write ( instruction inst );
	int write ();
	void flush ( int num );
};

class processor;

class branch_predictor
{
public:
	int correct, predicted;
	deque<instruction> history;

	branch_predictor ();
	bool predict ( instruction inst );
	void result ( instruction inst, bool cor );
};

class execute
{
public:
	instruction inst_in, inst_out;
	bool halt, finished, wait;
	processor *proc;
	RAM *ram;
	register_file *rf;
	write_back *wb;
	branch_predictor *bp;
	int rem_exec;

	execute ( processor *proc_in=NULL, RAM *rp=NULL, register_file *rf_in=NULL, write_back *out=NULL, branch_predictor *bp_in=NULL );
	void buffer_exec ( instruction i );
	void flush ( int num );
	void exec ();
	void push ();
};

class reservation_station
{
public:
	deque<instruction> wait_buffer, out_buffer;
	execute *exec[ NUM_ALU ];
	register_file *rf;
	processor *proc;

	reservation_station ( processor *proc_in, execute exec_in[ NUM_ALU ], register_file *rf_in );
	void buffer_inst ( instruction inst );
	void fetch_operands ();
	void flush ( int num );
	void push ();
};

class decode
{
public:
	deque<instruction> insts;
	reservation_station *rs;
	register_file *rf;
	bool wait, halt;

	decode ( register_file *rf_in, reservation_station *rs_in );
	void buffer_dec ( instruction i );
	void fetch_operands ();
	void push ();
	void flush ( int num );
};

class fetch
{
public:
	deque<instruction> insts;
	RAM *ram;
	decode *d;
	write_back *wb;
	branch_predictor *bp;
	register_file *rf;
	bool halt;
	int inst_count;

	fetch ( RAM *rp, register_file *rf_in, branch_predictor *bp, decode *d_in, write_back *wb_in );
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
	execute exec[ NUM_ALU ];
	reservation_station rs;
	decode d;
	branch_predictor bp;
	fetch f;
	int cycles;
	int completed_instructions;
	int num_code;
	int num_data;
	float inst_per_cycle;

	processor ( int code, int data, RAM *rp );
	void flush ( int num );
	void check ( int num );
	void refresh_db ();
	int tick ();
	int tock ();
};
