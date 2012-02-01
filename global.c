/* global.c -- define functions common to all parts of the package */

/* exports: uno_mem_init uno_malloc uno_free uno_cleanup vmdata_init */

#include <stdlib.h>
#include <stdio.h>
#include "global.h"
#include "vm.h"

ullist *mem_trk;

static int fatalerr;

void uno_mem_init(void)
{
	mem_trk->head = NULL;
	mem_trk->tail = NULL;
	mem_trk->count = 0;
}

void *uno_malloc(unsigned int sz)
{
	void *newmem = malloc(sz);

	if ( !newmem )
		print_error(1, "Error: Failure to allocate memory\n");
	ERRCHECK
	
	ullist_m *newlistm = malloc(sizeof(ullist_m));

	if ( !newlistm)
		print_error(1, "Error: Failure to allocate new memory entity\n");
	ERRCHECK

	newlistm->ent = newmem;
	newlistm->next = NULL;
	newlistm->prev = NULL;

	if (mem_trk->tail)
	{
		mem_trk->tail->next = newlistm;
		newlistm->prev = mem_trk->tail;
	}
	
	mem_trk->tail = newlistm;

	if ( !mem_trk->head )
	{
		mem_trk->head = newlistm;
		newlistm->prev = NULL;
	}
	
	return newmem;
}

void uno_free(void *mem)
{
	ullist_m *tmp = mem_trk->head;

	for (; tmp; tmp = tmp->next)
	{
		if (tmp->ent == mem)
			free(mem);
		return;
	}

	print_error(1, 
		"Error: attempt to free memory at %p not allocated by umalloc()\n", mem);
}

void uno_cleanup(void)
{
	ullist_m *tmp, *tmp2;

	for (tmp = mem_trk->tail; tmp; tmp = tmp->prev)
		free(tmp->ent);
	
	for (tmp = mem_trk->head; tmp; tmp = tmp->next, tmp2 = tmp)
		free(tmp2);
}

vmdata vmdata_init(void)
{
	vmdata vm_pass;

	vm_pass.inst = (line *)calloc(LINE_START, sizeof(line));

	if (!vm_pass.inst) 
		print_error(1, "Error: Failed to allocate linetab\n");

	ERRCHECK

	vm_pass.proc = (routine *)calloc(64, sizeof(routine));

	if (!vm_pass.proc) 
		print_error(1, "Error: Failed to allocate proctab\n");

	ERRCHECK

	vm_pass.pdata = (struct metadata *)malloc(sizeof(struct metadata));

	if (!vm_pass.pdata) 
		print_error(1, "Error: Failed to allocate metadata\n");

	ERRCHECK

	vm_pass.pdata->proccount = 0;
	vm_pass.pdata->linecount = 0;

	return vm_pass;
}