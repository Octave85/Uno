/* vmopts.c -- defines functions that execute opcodes */

/* exports: All functions beginning with 'OP_PROTO' */

#include <stdio.h>
#include "global.h"
#include "exec.h"
#include "ops.h"
#include "doop.h"

p_val t1, t2;

routine *r1;

OP_PROTO(op_routine)
{
	point->inst = arg.v.ival + 1;

	RETEMPTY;
}

OP_PROTO(op_while)
{
	RETEMPTY;
}
/* op_call is special */
BADOP(op_call);

OP_PROTO(op_per)
{
	return (p_val){ TYPE_EVENT, { (IVAL)EV_RETURN } };
}
OP_PROTO(op_push)
{
	NEXT;
	return (p_val)arg;
}
OP_PROTO(op_leave)
{
	return (p_val){ TYPE_EVENT, { (IVAL)EV_RETURN } };
}
OP_PROTO(op_dup)
{
	NEXT;

	return stackaccess(st, 0);
}
OP_PROTO(op_swap)
{
	t1 = stackpop(st);

	t2 = stackpop(st);

	stackpush(st, t1);

	NEXT;

	return t2;
}
OP_PROTO(op_over)
{
	NEXT;

	stackpush(st, stackaccess(st, 1));

	RETEMPTY;
}
OP_PROTO(op_drop)
{
	NEXT;

	stackpop(st);

	RETEMPTY;
}
OP_PROTO(op_add)
{
	t1 = stackpop(st);

	t2 = stackpop(st);

	NEXT;

	return (p_val){ t2.type, { Getval(t2) + Getval(t1) } };
}
OP_PROTO(op_sub)
{
	t1 = stackpop(st);

	t2 = stackpop(st);

	NEXT;

	return (p_val){ t2.type, { Getval(t2) - Getval(t1) } };
}
OP_PROTO(op_mult)
{
	t1 = stackpop(st);

	t2 = stackpop(st);

	NEXT;

	return (p_val){ t2.type, { Getval(t2) * Getval(t1) } };
}
OP_PROTO(op_div)
{
	t1 = stackpop(st);

	t2 = stackpop(st);

	NEXT;

	return (p_val){ t2.type, { Getval(t2) / Getval(t1) } };
}
OP_PROTO(op_out)
{
	t1 = stackpop(st);

	printf(IVAL_PRINTFORMAT, t1.v.ival);
	
	fflush(stdout);

	NEXT;

	RETEMPTY;
}
OP_PROTO(op_outc)
{
	printf("%c", (unsigned char)(stackpop(st)).v.ival);

	NEXT;

	RETEMPTY;
}
OP_PROTO(op_peek)
{
	NEXT;

	return (p_val)stackaccess(st, arg.v.ival);
}

BADOP(op_ife);
BADOP(op_ifn);
BADOP(op_ifg);
BADOP(op_ifge);
BADOP(op_ifl);
BADOP(op_ifle);


OP_PROTO(op_debug)
{
	int i;

	t1.type = TYPE_EMPTY;
	t1.v.ival = 0;

	for (i = 9; i >= 0; --i)
	{
		t1 = stackaccess(st, i);

		if (t1.type != TYPE_EMPTY)
		{
			if (t1.type == TYPE_IVAL)
				printf(IVAL_PRINTFORMAT, t1.v.ival);
		}
		else
			printf("N ");
	}
	putchar('\n');
	fflush(stdout);
	NEXT;
	RETEMPTY;
}
//unconditional jump
OP_PROTO(op_jmp)
{
	point->inst = arg.v.ival;
	RETEMPTY;
}
//jump if true
OP_PROTO(op_jt)
{
	NEXT;
	if (((stackpop(st)).v.ival))
		point->inst = arg.v.ival + 1;
	
	RETEMPTY;
}
//jump if false
OP_PROTO(op_jf)
{
	NEXT;

	if (!((stackpop(st)).v.ival))
		point->inst = arg.v.ival + 1;
	
	RETEMPTY;
}
OP_PROTO(op_eq)
{
	Cmp_op(==);	
}
OP_PROTO(op_ne)
{
	Cmp_op(!=);
}
OP_PROTO(op_lt)
{
	Cmp_op(<);
}
OP_PROTO(op_lte)
{
	Cmp_op(<=);
}
OP_PROTO(op_gt)
{
	Cmp_op(>);
}
OP_PROTO(op_gte)
{
	Cmp_op(>=);
}
OP_PROTO(op_shr)
{
	NEXT;

	t1 = stackpop(st);
	t2 = stackpop(st);

	return (p_val){ TYPE_IVAL, { t2.v.ival >> t1.v.ival } };
}

OP_PROTO(op_shl)
{
	NEXT;

	t1 = stackpop(st);
	t2 = stackpop(st);

	return (p_val){ TYPE_IVAL, { t2.v.ival << t1.v.ival } };
}
OP_PROTO(op_and)
{
	NEXT;

	t1 = stackpop(st);
	t2 = stackpop(st);

	return (p_val){ TYPE_IVAL, { t2.v.ival & t1.v.ival } };
}
OP_PROTO(op_or)
{
	NEXT;

	t1 = stackpop(st);
	t2 = stackpop(st);

	return (p_val){ TYPE_IVAL, { t2.v.ival | t1.v.ival } };
}
OP_PROTO(op_xor)
{
	NEXT;

	t1 = stackpop(st);
	t2 = stackpop(st);

	return (p_val){ TYPE_IVAL, { t2.v.ival ^ t1.v.ival } };
}
OP_PROTO(op_not)
{
	NEXT;

	t1 = stackpop(st);

	return (p_val){ TYPE_IVAL, { ~t1.v.ival } };
}
OP_PROTO(op_rot)
{
	t1 = stackaccess(st, 0);
	t2 = stackaccess(st, 1);

	stackset(st, 0, stackaccess(st, 2));

	stackset(st, 1, t1);

	stackset(st, 2, t2);

	NEXT;

	RETEMPTY;
}
OP_PROTO(op_st)
{
	NEXT;

	t1 = stackpop(st);

	return st->values[t1.v.ival];
}
OP_PROTO(op_die)
{
	t1.type = TYPE_EVENT;
	t1.v.ival = EV_TIMETODIE;

	return t1;
}
OP_PROTO(op_set)
{
	NEXT;

	t2 = stackpop(st);

	t1 = stackpop(st);

	stacksetabs(st, t1.v.ival, t2);

	RETEMPTY;
}
NOP(op_nop);
OP_PROTO(op_neg)
{
	NEXT;

	t1 = stackpop(st);

	t1.v.ival = !(t1.v.ival);

	return t1;
}
OP_PROTO(op_incat)
{
	NEXT;

	t1 = stackpop(st);

	t2 = st->values[t1.v.ival];

	t2.v.ival++;

	stacksetabs(st, t1.v.ival, t2);

	RETEMPTY;
}
OP_PROTO(op_decat)
{
	NEXT;

	t1 = stackpop(st);

	t2 = st->values[t1.v.ival];

	t2.v.ival--;

	stacksetabs(st, t1.v.ival, t2);

	RETEMPTY;
}