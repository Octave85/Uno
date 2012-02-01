/* vm.h -- defines macros and types specific to the VM */

#ifndef VM_H
#define VM_H

typedef unsigned int UPOINT;

typedef struct 
{
	UPOINT inst;
	UPOINT state;
	int *stack;
	UPOINT *call;
	UPOINT ispoint;
} pstruct; /* holds pointers */

typedef struct
{
	unsigned int allocated;
	int p;
	p_val *values;
} STACK;

#define VM_cleanup() free(vd.inst); \
					 free(vd.proc); \
					 free(vd.pdata); \
					 free(beginstack->values); \
					 free(beginstack); \
					 fclose(exec);					

#define CUR_LINE (vd.inst[pointers->inst])

#define IP pointers->inst

#define STATE_NORM 0

#define STATE_ROUT 1

#define STATE_SKIP 2

#define SET_STATE(s) (pointers->state = s)

#define STACK_START 64

#define STACK_GROW 128

STACK * VM_init(int, int, char **);

vmdata VM_read(FILE *);

UFLAG VM_exec(vmdata, STACK *);

#endif