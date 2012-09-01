/* vm.c -- virtual machine for Uno, a stack-oriented language */

/* exports: VM_read VM_init VM_exec */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "ops.h"
#include "vm.h"
#include "vmops.h"

static int fatalerr = 0;

p_val (*op_exectab[OP_COUNT])(p_val, pstruct *, STACK *) =
{
	&op_routine,
	&op_while,
	&op_call,
	&op_per,
	&op_push,
	&op_leave,
	&op_dup,
	&op_swap,
	&op_over,
	&op_drop,
	&op_add,
	&op_sub,
	&op_mult,
	&op_div,
	&op_out,
	&op_outc,
	&op_peek,
	&op_ife,
	&op_ifn,
	&op_ifg,
	&op_ifge,
	&op_ifl,
	&op_ifle,
	&op_debug,
	&op_jmp,
	&op_jt,
	&op_jf,
	&op_eq,
	&op_ne,
	&op_lt,
	&op_lte,
	&op_gt,
	&op_gte,
	&op_shr,
	&op_shl,
	&op_and,
	&op_or,
	&op_xor,
	&op_not,
	&op_rot,
	&op_st,
	&op_die,
	&op_set,
	&op_nop,
	&op_neg,
};

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

p_val stackaccess(STACK *from, int offset)
{
	if (from->p < offset)
		return (p_val){ TYPE_EMPTY, { 0 } };

	return from->values[from->p - offset];
}

UFLAG stackset(STACK *from, int offset, p_val val)
{
	if (offset > from->p)
		return F_BOUNDSERR;
	
	from->values[from->p - offset] = val;

	return F_STACKSUCCESS;
}

UFLAG stacksetabs(STACK *from, int offset, p_val val)
{
	if (offset > from->p)
		return F_BOUNDSERR;

	from->values[offset] = val;

	return F_STACKSUCCESS;
}

STACK * VM_init(int offset, int argc, char **argv)
{
	STACK *init;

	init = malloc(sizeof(STACK));

	if ( !init)
		print_error(1,
			"VMError: unable to initialize stack in main()\n");
	ERRCHECK
	
	init->p = -1;

	
	init->values = (p_val *)calloc(STACK_START, sizeof(p_val));

	if ( !init->values)
		print_error(1,
			"VMError: unable to allocate stack in VM_init()\n");
	ERRCHECK

	init->allocated = STACK_START;

	int i;

	for (i = 0; i < init->allocated; init->values[i++] = (p_val){ 0, { 0 }});

	if (argc > 2)
		for (i = offset; i < argc; i++)
			stackpush(init, (p_val){TYPE_IVAL, { atoi(argv[i]) } });

	return init;
}

STACK *VM_callstack(void)
{
	STACK *cstack;

	cstack = malloc(sizeof(STACK));

	if ( !cstack )
	{
		fprintf(stderr, "VMError: Unable to allocate callstack\n");
		exit(0);
	}

	cstack->p = -1;

	cstack->values = (p_val *)calloc(STACK_START, sizeof(p_val));

	if ( !cstack->values )
	{
		fprintf(stderr, "VMError: unable to allocate callstack contents\n");
		exit(0);
	}

	cstack->allocated = STACK_START;

	return cstack;
}

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

	vmdata VM_data;

	VM_data = vmdata_init();

	if ( !fread(VM_data.pdata, sizeof(struct metadata), 1, execfile))
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

UFLAG VM_exec(vmdata vd, STACK *thestack)
{
	pstruct *pointers;
	pointers = malloc(sizeof(pstruct));

	if ( !pointers)
		print_error(1, 
			"VMError: unable to allocate space for runtime pointers\n");
	ERRCHECK

	STACK *callstack = VM_callstack();

	puts("VM_exec'ing\n");

	pointers->stack = &thestack->p;

	pointers->inst = 0;

	routine *routp;

	p_val ret;

	unsigned int point = 0;

	/* Main execution loop */
	do
	{

		
#ifdef UNODEBUG
		printf("Executing %s ", 
			op2string[CUR_LINE.opcode]);
		
		if (CUR_LINE.arg.type == TYPE_IVAL)
			printf(IVAL_PRINTFORMAT, CUR_LINE.arg.v.ival);
		else
			printf(DVAL_PRINTFORMAT, CUR_LINE.arg.v.dval);

		printf("IP = %d\n", IP);
		
		//getchar();
#endif


		if (CUR_LINE.opcode == O_CALL)
		{
			routp = &vd.proc[CUR_LINE.arg.v.ival];

			stackpush(callstack, (p_val){TYPE_IVAL, { pointers->inst }});

			pointers->inst = routp->pp;

			continue;
		}

		ret = (*op_exectab[CUR_LINE.opcode])
				(CUR_LINE.arg, pointers, thestack);
		
		if (ret.type == TYPE_EVENT)
		{
			if (ret.v.ival == EV_TIMETODIE)
				goto VM_quit;
			else if (ret.v.ival == EV_BADOP)
			{
				fprintf(stderr, "VMError: Bad opcode at instruction %d\n", 
					pointers->inst);

				goto VM_quit;
			}
			else if (ret.v.ival == EV_RETURN)
			{
				ret = stackpop(callstack);
				pointers->inst = ret.v.ival + 1;
			}

			continue;
		}

		stackpush(thestack, ret);

	} while (CUR_LINE.opcode >= 0);

VM_quit:
	//printf("%d\n", thestack->values[thestack->p].v.ival);

	free(pointers);
	free(callstack->values);
	free(callstack);

	return F_EXECSUCCESS;
}