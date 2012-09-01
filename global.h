/* global.h -- useful macros, datatypes and functions used throughout the program */


#ifndef GLOBAL_H
#define GLOBAL_H

//#define UNODEBUG //*/

#define STACK_MAX 512

#define FRAME_COLS 16
#define FRAME_ROWS 512


// types used in Uno
typedef long IVAL;
typedef unsigned int UVAL;
typedef double DVAL;
typedef char * SVAL;
typedef unsigned int PVAL;
typedef unsigned int UFLAG;


// functions
#define print_error(fatal, ...) do { \
								fprintf(stderr, __VA_ARGS__); \
								fatalerr = fatal; \
								} while (0); // [deprecated?] Works just like fprintf to std err, sets an error flag

#define ERRCHECK do { if (fatalerr) exit(0); } while (0); // Needed to check for fatal errors. deprecated for sure

// The following are macros that point to important data structures in the Uno program.
#define U ums // The entire Uno data structure

#define VD U->vd // All data used to run a program - routine table, instruction list, metadata

#define MSTACK U->stack // The main stack manipulated by Uno programs

#define CSTACK U->callstack // The callstack

#define POINT U->pointers // Pointers that keep track of position in a program - instruction, stack, etc.

#define LINE U->parse->n_l // The current instruction to be entered into the instruction list. Used by the interpreter.

#define TLINE U->parse->t_l // The current text line (verbatim string instead of the opcode and argument)

#define BSTACK U->block // The block stack that keeps track of how many nested levels we're in

// Type differentiation for lines with arguments
#define TYPE_IVAL 0x1
#define TYPE_UVAL 0x2
#define TYPE_STRVAL  0x3
#define TYPE_EMPTY 0x4
#define TYPE_PROC 0x5
#define TYPE_DVAL 0x6
#define TYPE_POINT 0x10
#define TYPE_EVENT 0x1F

// Flags

#define F_BOUNDSERR 0
#define F_STACKSUCCESS 1
#define F_COMPILESUCCESS 2
#define F_EXECSUCCESS 3
#define F_WRITESUCCESS 4

// Program modes (mainly just interp and console used)

#define MODE_COMPILE 1
#define MODE_WRITE   2
#define MODE_EXECUTE 4
#define MODE_CONSOLE 8
#define MODE_INTERP 16

#define IVAL_PRINTFORMAT "%ld "

#define DVAL_PRINTFORMAT "%.4f "

#define E_SHOULDCLEAN 1

#define LEX_EOF -2

#define mem_error 1
#define parser_error 3

// formal error message system, I guess
#define Uno_error(type, format, rest...)		\
     	    fprintf (stderr, format, ##rest);	//
     	   // if (type & E_SHOULDCLEAN) { uno_cleanup(); exit(0); }

#define u_strtolong(str) (strtol(str, NULL, 0))

typedef struct ullist_member ullist_m;

struct ullist
{ 
	void **items;
	int count;
};

typedef struct ullist ullist;

// A single routine entry
typedef struct 
{
	int pp; // location in the code
	int endpp; // where its closing '.' is
	unsigned int hash; // hash of the name
} routine;


// An Uno value - any type
typedef struct
{
	int type;
	union
	{
		UVAL 	uval;
		SVAL   	strval;
		IVAL    ival;
		DVAL 	dval;
	} v;
} p_val;

typedef unsigned int UPOINT;

typedef struct 
{
	UPOINT inst;
	UPOINT state;
	int *stack; // For manipulating the stack
	UPOINT *call;
	UPOINT ispoint;
} pstruct; /* holds pointers */


// The main stack data structure
typedef struct
{
	unsigned int allocated;
	int p; // Stack pointer (?)
	p_val *values; // Stack values
} STACK;


// Interpreter/VM's representation of a line/instruction. Last member is a function pointer that exec's that opcode
typedef struct
{
	int opcode;
	p_val arg;
	p_val (*exec)(p_val, pstruct *, STACK *);
} line;


// Was used to write compiled programs to a file. Not currently used
struct header
{
	int magic;
	char *id; 
};


// Data about the program as a whole
struct metadata
{
	unsigned int proccount; // # of procedures
	unsigned int linecount; // # of instructions
};


// Data needed to run an Uno program
typedef struct 
{
	line *inst; 			// Instructions
	routine *proc;			// Procedures
	struct metadata *pdata; // Extra data
} vmdata;


// Represents a single block level
struct b
{
	unsigned int type; // B_WHILE or B_IF
	unsigned int ip;   // Starting location of block
};


// Stack of nested blocks
typedef struct
{
	unsigned int bp; // stack pointer
	struct b *values;// stack values
} blockstack;


// Single verbatim line
typedef struct
{
	char *text;
	int len;
} textline;


// Each line is made into one of these, I believe
typedef struct 
{
	line *n_l;
	textline *t_l;
} parseunit;


// Grand daddy - everything used to run the Uno program, encapsulated into one data structure.
typedef struct
{
	vmdata *vd;
	STACK *stack;
	STACK *callstack;
	pstruct *pointers;
	blockstack *block;
	parseunit *parse;
} UnoMS;


// These memory functions are unused
void uno_mem_init(void);

void *uno_malloc(unsigned int);

void uno_free(void *);

void uno_cleanup(void);

// End unused


// These next few functions are used at the beginning of execution to initialize every data structure
vmdata *vmdata_init(void);

UnoMS *Uno_init(UnoMS *);

void Wabc_init(UnoMS *);

void VM_init(UnoMS *, int, int, char **);

void Uno_exit(UnoMS *);

#define LINE_START 128

#define LINE_GROW 128

#define O_GUARD -85

#define GUARD_LINE (line){O_GUARD,{TYPE_EMPTY,{0}}}


FILE *listing, *source;

#endif