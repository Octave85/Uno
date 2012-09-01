/* wabc.h -- define macros, constants and types for the compiler */

#ifndef WABC_H
#define WABC_H

#define OPCODE_IS(cm) (!strcmp(word, cm))

#define set_uval(d, val) d->arg.v.uval = val; \
					   d->arg.type = TYPE_UVAL; 

#define set_ival(d, val) d->arg.v.ival = val; \
					   d->arg.type = TYPE_IVAL

#define IS_IVAL(d) (d->arg.type == TYPE_IVAL)

#define IS_UVAL(d) (d->arg.type == TYPE_UVAL)

#define IS_EMPTY(d) (d->arg.type == TYPE_EMPTY)

#define GETVAL(d) ((IS_IVAL(d)) ? d->arg.v.ival : ((IS_UVAL(d)) ? d->arg.v.uval : -1))

#define BPUSH(n, p) block->values[++block->bp] = (struct b){ n, p }

#define BPOP() block->values[block.bp--]

#define B_WH 127
#define B_IF 128

#define Blk_iswhile(x) ((x) == B_WH)

#define Blk_isif(x) ((x) == B_IF)

#define OP_CASE(c) else if (OPCODE_IS((c)))

#define SINGLE_OP_CASE(c) else if (word[0] == c)


#define Uno_ParseLine(file, tl) VD->inst = 				\
								linetab_add(VD->inst, 	\
									Uno_parse(LINE, 	\
										VD, 			\
										BSTACK, 		\
										ip), 			\
									ip);				\
							ip++;


#define STATE_NORM 1
#define STATE_COMM 2
#define STATE_CALL 4
#define STATE_ROUT 8


#define newemptyRESWORD(name) n_l->opcode = name; \
							  n_l->exec = op_exectab[name]; \
							  n_l->arg = (p_val){ TYPE_EMPTY, {0}}; \
							  return n_l;


/*typedef struct tree
{
	tree *par;
	tree *left;
	tree *right;
	parseenum (*testfun)(int, char *, line *, FILE *);
	parseenum result;
} tree;*/

void Uno_parsemain(UnoMS *);

UFLAG Uno_write(FILE *, vmdata);

line *Uno_parse(line *, vmdata *, blockstack *, int);

int Uno_lex(FILE *reader, char *token);

int Uno_keyword(char *, int);

#endif