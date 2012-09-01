/* exec.c -- virtual machine for Uno, a stack-oriented language */

/* exports: VM_read VM_init VM_exec */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "ops.h"
#include "exec.h"
#include "doop.h"

static int fatalerr = 0;

void stackpush(STACK *to, p_val src)
{
	if (src.type == TYPE_EMPTY)
		return;

	to->p++;

	to->values[to->p] = src;
}

void stackcheck(STACK *st)
{
	/* Bounds check, grow if necessary */
	if (st->p + 1 >= st->allocated)
	{
		fprintf(stderr, "UnoVM: Allocating...\n");

		unsigned int new_alloc = (st->allocated + STACK_GROW);

		st->values = realloc(st->values, 
			new_alloc * sizeof(p_val));
		
		if ( !st->values )
			fprintf(stderr, "VMError: Failure to grow stack\n");
		
		st->allocated = new_alloc;
	}
}


p_val stackpop(STACK *from)
{
	p_val ret;

	if (from->p < -1)
		print_error(1, "RuntimeError: attempt to go below stack boundary\n"); 
	
	ERRCHECK

	ret = from->values[from->p];

	--from->p;

	return ret;
}

inline p_val stackaccess(STACK *from, int offset)
{
	if (from->p < offset)
		return (p_val){ TYPE_EMPTY, { 0 } };

	return from->values[from->p - offset];
}

inline UFLAG stackset(STACK *from, int offset, p_val val)
{
	if (offset > from->p)
		return F_BOUNDSERR;
	
	from->values[from->p - offset] = val;

	return F_STACKSUCCESS;
}

inline UFLAG stacksetabs(STACK *from, int offset, p_val val)
{
	if (offset > from->p)
		return F_BOUNDSERR;

	from->values[offset] = val;

	return F_STACKSUCCESS;
}

/*
vmdata VM_read(FILE *execfile)
{
	fatalerr = 0; // begin ok

	struct header phdr;

	if (fread(&phdr, sizeof(struct header), 1, execfile))
	{
		if (phdr.magic != 0x1885) 
			print_error(1, "VMError: Magic incorrect\n");
	}
	else
	{
		print_error(1, "VMError: unable to read file for header\n");
	}

	ERRCHECK

	vmdata *VM_data;

	VM_data = vmdata_init();

	if ( !fread(VM_data->pdata, sizeof(struct metadata), 1, execfile))
		print_error(1, "VMError: unable to read file for metadata\n");

	ERRCHECK

	printf("VM_read received: proccount=%d, linecount=%d\n", VM_data.pdata->proccount, VM_data.pdata->linecount);
	
	unsigned int i;

	for (i = 0; i < VM_data.pdata->proccount; ++i)
		if ( !fread(&VM_data.proc[i], sizeof(routine), 1, execfile))
			print_error(1, "VMError: read error of routine table at number %d\n", i);

	VM_data.proc[i].hash = 0; // should always test with hash

	for (i = 0; i < VM_data.pdata->linecount; ++i)
		if ( !fread(&VM_data.inst[i], sizeof(line), 1, execfile))
			print_error(1, "VMError: read error of instructions at number %d\n", i);

	VM_data.inst[i-1].opcode = -1; // should always test with opcode


	return VM_data;
}
*/

inline int VM_exec_op(STACK *thestack,
					  STACK *callstack,
					  pstruct *pointers,
					  vmdata *vd,
					  line *ex)
{

#	undef MSTACK
#	define MSTACK thestack

#	undef CSTACK
#	define CSTACK callstack

#	undef POINT
#	define POINT pointers

#	undef VD
#	define VD vd

	register int Op = -1;

	Op = ex->opcode;

	p_val ret;

	routine routp;

	/**	I profiled this and determined that a switch construct is a bit
	 *	faster than an indirect function call from a table. It's ugly,
	 *	but it works pretty well.
	 */

#ifdef UNODEBUG
		printf("Executing %s %d\n", 
			op2string[ex->opcode], ex->arg.v.ival);

		printf("IP = %d\n", POINT->inst);
		
		getchar();
#endif

	if (Op == O_CALL)
	{
		
		routp = VD->proc[ex->arg.v.ival];

		stackpush(CSTACK, ((p_val){TYPE_IVAL, { POINT->inst }}));

		POINT->inst = routp.pp;

		return 0;
	}
	
		
	/* Execute the procedure */
	ret = (*(ex->exec))(ex->arg, POINT, MSTACK);
	/* Done */

	
	if (ret.type == TYPE_EVENT)
	{
		switch (ret.v.ival)
		{
			case EV_TIMETODIE:
				return EV_TIMETODIE;
			case EV_BADOP:
				fprintf(stderr, "VMError: Bad opcode at instruction %d\n", 
					POINT->inst);

				return EV_BADOP;
			case EV_RETURN:
				ret = stackpop(CSTACK);
				POINT->inst = ret.v.ival + 1;
		}
		return;
	}

	stackpush(MSTACK, ret);

	return 0;
}

UFLAG VM_exec(UnoMS *ums)
{
	puts("VM_exec'ing\n");

	routine *routp;

	p_val ret;

#	undef MSTACK
#	define MSTACK ums->stack

#	undef CSTACK
#	define CSTACK ums->callstack

#	undef POINT
#	define POINT ums->pointers

#	undef VD
#	define VD ums->vd

	register int Ev = 0;

	int eop = VD->pdata->linecount;

	STACK *thestack = MSTACK;
	STACK *callstack = CSTACK;
	pstruct *pointers = POINT;
	vmdata *vd = VD;

#	undef MSTACK
#	define MSTACK thestack

#	undef CSTACK
#	define CSTACK callstack

#	undef POINT
#	define POINT pointers

#	undef VD
#	define VD vd

	//MSTACK->p = -1;

	/* Main execution loop */
	do
	{
		Ev |= VM_exec_op(MSTACK, CSTACK, POINT, VD, &CUR_LINE);	
		Ev |= VM_exec_op(MSTACK, CSTACK, POINT, VD, &CUR_LINE);	
		Ev |= VM_exec_op(MSTACK, CSTACK, POINT, VD, &CUR_LINE);	
		Ev |= VM_exec_op(MSTACK, CSTACK, POINT, VD, &CUR_LINE);	
		Ev |= VM_exec_op(MSTACK, CSTACK, POINT, VD, &CUR_LINE);	
		Ev |= VM_exec_op(MSTACK, CSTACK, POINT, VD, &CUR_LINE);	
		Ev |= VM_exec_op(MSTACK, CSTACK, POINT, VD, &CUR_LINE);	
		Ev |= VM_exec_op(MSTACK, CSTACK, POINT, VD, &CUR_LINE);	
		
		stackcheck(MSTACK);	
	} while ((~Ev & EV_TIMETODIE));


	return F_EXECSUCCESS;
}

void vm_quit(pstruct *pointers, STACK *call)
{
	free(pointers);
	free(call->values);
	free(call);
}