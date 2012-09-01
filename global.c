/* global.c -- define functions common to all parts of the package */

/* exports: uno_mem_init uno_malloc uno_free uno_cleanup vmdata_init */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "exec.h"

ullist *mem_trk;

static int fatalerr;

void uno_cleanup(void)
{
	register int i = mem_trk->count - 1;
	do
	{
		free(mem_trk->items[i]);
	} while (i >= 0);
}

void uno_mem_init(void)
{
	mem_trk->items = (void **)calloc(64, sizeof(void *));

	if ( !mem_trk->items )
		Uno_error(mem_error, 
			"Error: no initialization of memory tracking -- going it alone\n");
}

void *uno_malloc(unsigned int sz)
{
	void *newmem = malloc(sz);
	
	mem_trk->items[mem_trk->count++] = newmem;

	return newmem;
}

void uno_free(void *mem)
{

	if ( !mem )
	{
		free(mem_trk->items[mem_trk->count--]);
		return;
	}

	register int i = -1;

	do
	{
		i++;
	} while (mem_trk->items[i] != mem);

	if (i < mem_trk->count)
	{
		free(mem_trk->items[i]);
		mem_trk->count--;
	}
}

inline vmdata * vmdata_init(void)
{
	vmdata *vm_pass = malloc(sizeof(vmdata));

	vm_pass->inst = (line *)calloc(LINE_START, sizeof(line));

	if (!vm_pass->inst) 
		print_error(1, "Error: Failed to allocate linetab\n");

	ERRCHECK

	vm_pass->proc = (routine *)calloc(64, sizeof(routine));

	if (!vm_pass->proc) 
		print_error(1, "Error: Failed to allocate proctab\n");

	ERRCHECK

	vm_pass->pdata = (struct metadata *)malloc(sizeof(struct metadata));

	vm_pass->pdata->proccount = 0;
	vm_pass->pdata->linecount = 0;

	vm_pass->inst[LINE_START - 1] = GUARD_LINE;

	return vm_pass;
}

inline UnoMS *Uno_init(UnoMS *ums)
{
	ums = malloc(sizeof(UnoMS));

	memset(ums, 0, sizeof(UnoMS));

	ums->vd = vmdata_init();

	return ums;
}

inline void Wabc_init(UnoMS *ums)
{
	ums->block = malloc(sizeof(blockstack));

	ums->block->values = calloc(16, 2*sizeof(int));

	ums->parse = malloc(sizeof(parseunit));

	ums->parse->n_l = malloc(sizeof(line));

	ums->parse->t_l = malloc(sizeof(textline));

	ums->parse->t_l->text = calloc(50, sizeof(char));
}

inline void VM_init(UnoMS *ums, int offset, int argc, char **argv)
{
	ums->stack = malloc(sizeof(STACK));
	
	ums->stack->p = -1;

	ums->stack->values = (p_val *)calloc(STACK_START, sizeof(p_val));

	ums->stack->allocated = STACK_START;

	int i;

	if (argc > 2)
		for (i = offset; i < argc; i++)
		{
			ums->stack->values[i++] = (p_val){TYPE_IVAL, { atoi(argv[i]) } };
		}


	ums->callstack = malloc(sizeof(STACK));

	ums->callstack->p = -1;

	ums->callstack->values = (p_val *)calloc(STACK_START, sizeof(p_val));

	ums->callstack->allocated = STACK_START;


	ums->pointers = malloc(sizeof(pstruct));
	ums->pointers->inst = 0;
	ums->pointers->stack = &ums->stack->p;
}

void Uno_exit(UnoMS *ums)
{
	if (ums->block)
	{
		free(ums->block->values);
		free(ums->block);
		free(ums->parse->n_l);
		free(ums->parse->t_l->text);
		free(ums->parse->t_l);
		free(ums->parse);
	}

	if (ums->stack)
	{
		free(ums->stack->values);
		free(ums->stack);
		free(ums->callstack->values);
		free(ums->callstack);
		free(ums->pointers);
	}

	free(ums->vd);

	free(ums);
}