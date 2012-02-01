/* global.h -- useful macros and types used across the entire package */


#ifndef GLOBAL_H
#define GLOBAL_H

/*#define UNODEBUG //*/

#define STACK_MAX 512

#define FRAME_COLS 16
#define FRAME_ROWS 512


typedef int IVAL;
typedef unsigned int UVAL;
typedef char * SVAL;
typedef unsigned int PVAL;
typedef unsigned int UFLAG;


#define print_error(fatal, ...) do { \
								fprintf(stderr, __VA_ARGS__); \
								fatalerr = fatal; \
								} while (0);

#define ERRCHECK do { if (fatalerr) exit(0); } while (0);

// Type differentiation for lines with arguments
#define TYPE_IVAL 0x0
#define TYPE_UVAL 0x1
#define TYPE_STR  0x2
#define TYPE_EMPTY 0x3
#define TYPE_PROC 0x4
#define TYPE_POINT 0x10
#define TYPE_EVENT 0x1F

// Flags

#define F_BOUNDSERR 0
#define F_STACKSUCCESS 1
#define F_COMPILESUCCESS 2
#define F_EXECSUCCESS 3
#define F_WRITESUCCESS 4

// Program modes

#define MODE_COMPILE 1
#define MODE_WRITE   2
#define MODE_EXECUTE 4


struct ullist_member
{
	void *ent;
	struct ullist_member *next;
	struct ullist_member *prev;
};

typedef struct ullist_member ullist_m;

struct ullist
{ 
	struct ullist_member *head;
	struct ullist_member *tail;
	int count;
};

typedef struct ullist ullist;

typedef struct 
{
	int pp; // location in the code
	int endpp; // where its closing '.' is
	unsigned int hash; // hash of the name
} routine;

typedef struct
{
	int type;
	union
	{
		UVAL 	uval;
		SVAL   	*str;
		IVAL    ival;
		PVAL 	point;
	} v;
} p_val;

typedef struct
{
	int opcode;
	p_val arg;
} line;

struct header
{
	int magic;
	char *id; 
};

struct metadata
{
	unsigned int proccount;
	unsigned int linecount;
};

typedef struct 
{
	line *inst;
	routine *proc;
	struct metadata *pdata;
} vmdata;


void uno_mem_init(void);

void *uno_malloc(unsigned int);

void uno_free(void *);

vmdata vmdata_init(void);

#define LINE_START 128

#define LINE_GROW 128

#endif