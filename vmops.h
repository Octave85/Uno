/* vmopts.h -- defines prototypes and table for opcode-executing functions */

#ifndef VMOPS_H
#define VMOPS_H

#define EV_TIMETODIE 1
#define EV_BADOP 2
#define EV_RETURN 3

#define OP_PROTO(NAME) p_val NAME(p_val arg, pstruct *point, STACK *st)

#define RETEMPTY return (p_val){ TYPE_EMPTY, { (IVAL)0 } }

#define RETBADOP return (p_val){ TYPE_EVENT, { (IVAL)EV_BADOP } }

#define NOP(NAME) OP_PROTO(NAME) { NEXT; RETEMPTY; }

#define TIMETODIE (p_val){ TYPE_EVENT, { (IVAL)EV_TIMETODIE } }

#define BADOP(NAME) OP_PROTO(NAME) { RETBADOP; }

#define NEXT point->inst++

#define Cmp_op(o) NEXT; \
				t1 = stackpop(st); \
				t2 = stackpop(st); \
				return (p_val){ TYPE_IVAL, \
					{ (t2.v.ival o t1.v.ival) } };


/* Functions made available to the ops from vm.c */
p_val stackpop(STACK *);

void stackpush(STACK *, p_val);

p_val stackaccess(STACK *, int);

UFLAG stackset(STACK *, int, p_val);

UFLAG stacksetabs(STACK *from, int offset, p_val val);


OP_PROTO(op_routine);
OP_PROTO(op_while);
OP_PROTO(op_call);
OP_PROTO(op_per);
OP_PROTO(op_push);
OP_PROTO(op_leave);
OP_PROTO(op_dup);
OP_PROTO(op_swap);
OP_PROTO(op_over);
OP_PROTO(op_drop);
OP_PROTO(op_add);
OP_PROTO(op_sub);
OP_PROTO(op_mult);
OP_PROTO(op_div);
OP_PROTO(op_out);
OP_PROTO(op_outc);
OP_PROTO(op_peek);
OP_PROTO(op_ife);
OP_PROTO(op_ifn);
OP_PROTO(op_ifg);
OP_PROTO(op_ifge);
OP_PROTO(op_ifl);
OP_PROTO(op_ifle);
OP_PROTO(op_debug);
OP_PROTO(op_jmp);
OP_PROTO(op_jt);
OP_PROTO(op_jf);
OP_PROTO(op_eq);
OP_PROTO(op_ne);
OP_PROTO(op_lt);
OP_PROTO(op_lte);
OP_PROTO(op_gt);
OP_PROTO(op_gte);
OP_PROTO(op_shr);
OP_PROTO(op_shl);
OP_PROTO(op_and);
OP_PROTO(op_or);
OP_PROTO(op_xor);
OP_PROTO(op_not);
OP_PROTO(op_rot);
OP_PROTO(op_st);
OP_PROTO(op_die);
OP_PROTO(op_set);
OP_PROTO(op_nop);
OP_PROTO(op_neg);

#endif