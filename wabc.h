/* wabc.h -- define macros, constants and types for the compiler */

#ifndef WABC_H
#define WABC_H

#define O_GUARD -85

#define GUARD_LINE (line){O_GUARD,{TYPE_EMPTY,{0}}}

#define OPCODE_IS(cm) (!strcmp(word, cm))

#define set_uval(d, val) d->arg.v.uval = val; \
					   d->arg.type = TYPE_UVAL; 

#define set_ival(d, val) d->arg.v.ival = val; \
					   d->arg.type = TYPE_IVAL

#define IS_IVAL(d) (d->arg.type == TYPE_IVAL)

#define IS_UVAL(d) (d->arg.type == TYPE_UVAL)

#define IS_EMPTY(d) (d->arg.type == TYPE_EMPTY)

#define GETVAL(d) ((IS_IVAL(d)) ? d->arg.v.ival : ((IS_UVAL(d)) ? d->arg.v.uval : -1))

#define BPUSH(n, p) block.values[++block.bp] = (struct b){ n, p }

#define BPOP() block.values[block.bp--]

#define B_WH 127
#define B_IF 128

#define Blk_iswhile(x) ((x) == B_WH)

#define Blk_isif(x) ((x) == B_IF)

#define OP_CASE(c) else if (OPCODE_IS((c)))

#define SINGLE_OP_CASE(c) else if (word[0] == c)


struct b
{
	unsigned int type;
	unsigned int ip;
};

vmdata UNO_read(FILE *);

UFLAG UNO_write(FILE *, vmdata);

#endif