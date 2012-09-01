/* parse.c -- parser for Uno, a stack-oriented language */

/* exports: Uno_read Uno_write */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "global.h"
#include "ops.h"
#include "exec.h"
#include "doop.h"
#include "scan.h"
#include "parse.h"

#define IN_LINESZ 96

#define STRT_LINECT 128

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
	&op_incat,
	&op_decat,
};


static unsigned int RSHash(char *str)
{

   unsigned int b = 378551;
   unsigned int a = 63689;
   unsigned int hash = 0;
   unsigned int i;
   for(i = 0; str[i] != '\0'; i++)
   {
      hash = hash*a+str[i];
      a = a*b;
   }

   return (hash & 0x7FFFFFFF);

}

size_t hash(const char* str)
{
  unsigned int i = 0, c, hash = 0;
  while ((c = str[i++]))
  {
  	hash = ((c << i) + c) ^ hash;
  }
  return hash & 511;
}

static routine * proctab_add(routine *pt, 
							unsigned int *ptp, 
							char *add, 
							const unsigned int lp)
{
	if (*ptp+1 >= STACK_MAX)
		print_error(1, "Error: routine table full at line %d\n", lp+1);
	pt[*ptp  ].pp = lp + 1;
	//pt[*ptp  ].frameaddr = *ptp*FRAME_COLS;
	//pt[*ptp  ].frameaddr_o = pt[*ptp].frameaddr;
	pt[*ptp  ].endpp = lp;
	pt[*ptp].hash = RSHash(add);

	(*ptp)++;
	return &pt[*ptp-1];
}



static line * linetab_add(line *tab, line *add, const int lp)
{

	if (tab[lp].opcode == O_GUARD)
	{
		fprintf(stderr, "Wabc: Allocating...\n");

		unsigned int new_index = lp + LINE_GROW;

		tab = (line *)realloc(tab, 
			new_index * sizeof(line));

		if ( !tab )
			fprintf(stderr, "Error: Failure to grow line table\n");
		
		int i;

		for (i = lp; i < new_index; tab[i++] = (line){-1,{0,{0}}});
		tab[new_index - 1] = GUARD_LINE;
	}

	tab[lp] = *add;

	return tab;
}

static routine * find_proc(routine *pt, const int ptp, const unsigned int hash)
{
	int i;

	for (i = ptp-1; i >= 0; --i)
	{
		if (pt[i].hash == hash)
			return &pt[i];
	}
	return NULL;
}

static int find_procid(routine *pt, const int ptp, const unsigned int hash)
{
	int i;

	for (i = ptp; i >= 0; --i)
	{
		if (pt[i].hash == hash)
			return i;
	}

	return -1;
}

line *Uno_parse(line *n_l, 				/*	New line to add */
				vmdata *data, 			/*	Big data structure */
				blockstack *block, 		/*	Scope/block stack */
				const int ip)			/*	Instruction pointer */
{	

	register int pbt, pbip;
	
	TokenType token = unolex();


#ifdef UNODEBUG
	printf("Token is %s (%d)\n", tok2str(token), token);
	getchar();
#endif

	switch (token)
	{
		case END:
			pbt =
				(block->values[block->bp]).type;
			
			pbip = 
				(block->values[block->bp]).ip;
			
			block->bp--;

			switch (pbt)
			{
				case O_ROUTINE:
					puts("a");
					data->inst[pbip].arg.v.ival = ip;
					data->inst[pbip].arg.type = TYPE_IVAL;

					n_l->opcode = O_PER;	// O_PER

					n_l->exec = op_exectab[O_PER];

					return n_l;
				case B_WH:
					n_l->opcode = O_JT;
					n_l->arg.v.ival = pbip;
					n_l->arg.type = TYPE_IVAL;
					n_l->exec = op_exectab[O_JT];

					data->inst[pbip].opcode = O_JF;
					data->inst[pbip].exec = op_exectab[O_JF];
					data->inst[pbip].arg.type = TYPE_IVAL;
					data->inst[pbip].arg.v.ival = ip;

					return n_l;
				
				case B_IF:
					n_l->opcode = O_NOP;
					n_l->arg.type = TYPE_EMPTY;
					n_l->arg.v.ival = 0;
					n_l->exec = op_exectab[O_NOP];

					data->inst[pbip].opcode = O_JF;
					data->inst[pbip].exec = op_exectab[O_JF];
					data->inst[pbip].arg.type = TYPE_IVAL;
					data->inst[pbip].arg.v.ival = ip;

					return n_l;
				default:
					Uno_error(parser_error, 
						"Error: block mismatch near `%s'\n", tokenstr);
			}
		case WHILE:
			BPUSH(B_WH, ip);
			n_l->opcode = O_WHILE;
			n_l->arg.type = TYPE_IVAL;
			n_l->arg.v.ival = 0;

			return n_l;
		case IF:
			printf("parse.c: IF\n");
			BPUSH(B_IF, ip);
			printf("parse.c: bpushed\n");
			n_l->opcode = O_JF;
			n_l->arg.type = TYPE_IVAL;
			n_l->arg.v.ival = 0;
			printf("parse.c: n_l mostly assigned\n");
			n_l->exec = op_exectab[O_JF];
			printf("parse.c: /IF\n");
			return n_l;
		
		case DUP:
			newemptyRESWORD(O_DUP);
		case SWAP:
			newemptyRESWORD(O_SWAP);
		case ROT:
			newemptyRESWORD(O_ROT);
		case OVER:
			newemptyRESWORD(O_OVER);
		case DROP:
			newemptyRESWORD(O_DROP);
		case OUT:
			newemptyRESWORD(O_OUT);
		case OUTC:
			newemptyRESWORD(O_OUTC);
		case DEBUG:
			newemptyRESWORD(O_DEBUG);
		case ST:
			newemptyRESWORD(O_ST);
		case EOI:
		case DIE:
			newemptyRESWORD(O_DIE);
		case INCAT:
			newemptyRESWORD(O_INCAT);
		case DECAT:
			newemptyRESWORD(O_DECAT)
		case LEAVE:
			newemptyRESWORD(O_LEAVE);
		case PLUS:
			newemptyRESWORD(O_ADD);
		case MINUS:
			newemptyRESWORD(O_SUB);
		case TIMES:
			newemptyRESWORD(O_MULT);
		case SLASH:
			newemptyRESWORD(O_DIV);
		case AND:
			newemptyRESWORD(O_AND);
		case OR:
			newemptyRESWORD(O_OR);
		case XOR:
			newemptyRESWORD(O_XOR);
		case SHR:
			newemptyRESWORD(O_SHR);
		case SHL:
			newemptyRESWORD(O_SHL);
		case ASSIGN:
			newemptyRESWORD(O_SET);
		case EQ:
			newemptyRESWORD(O_EQ);
		case NE:
			newemptyRESWORD(O_NE);
		case GT:
			newemptyRESWORD(O_GT);
		case GTE:
			newemptyRESWORD(O_GTE);
		case LT:
			newemptyRESWORD(O_LT);
		case LTE:
			newemptyRESWORD(O_LTE);

		case NUM:
			n_l->opcode = O_PUSH;
			n_l->arg.type = TYPE_IVAL;
			n_l->arg.v.ival = tokenval.v.ival;
			n_l->exec = op_exectab[O_PUSH];
			return n_l;
		
		case ID:
			token = unolex();

			if (token == COLON)
			{
				proctab_add(data->proc, 
							&data->pdata->proccount, 
							tokenval.v.strval, 
							ip);

				BPUSH(O_ROUTINE, ip);

				n_l->opcode = O_ROUTINE;
				n_l->exec = op_exectab[O_ROUTINE];

				return n_l;
			}
			else
			{
				Uno_error(1, "Undefined identifier %s at line %d\n",
					tokenval.v.strval, lineno);
			}
		
		case LPAREN:
			token = unolex();

			if (token == ID)
			{
				n_l->opcode = O_CALL;
				n_l->exec = op_exectab[O_CALL];

				n_l->arg.type = TYPE_IVAL;

				n_l->arg.v.ival = 
					find_procid(data->proc, 
								data->pdata->proccount, 
								RSHash(tokenval.v.strval));
				
				if (n_l->arg.v.ival < 0)
					Uno_error(parser_error, 
					"Error: undefined routine called %s at line %d\n", 
						tokenval.v.strval, lineno);


				token = unolex();

				if (token != RPAREN)
					fprintf(stderr, "Syntax error: unexpected %s at %d (expected ')')\n",
						tokenstr, lineno);

				return n_l;
			}
			else
			{
				fprintf(stderr, "Syntax error: unexpected %s at line %d\n",
					tokenstr, lineno);
			}

		default:
			fprintf(stderr, "Unrecognized token %d\n", token);
			exit(0);
	}
}

void Uno_parsemain(UnoMS *ums)
{
	register int ip = 0;

	lineno = hiteof = 0;

	do
	{
		Uno_ParseLine(source, TLINE);

		Uno_ParseLine(source, TLINE);

		Uno_ParseLine(source, TLINE);

		Uno_ParseLine(source, TLINE);

	} while ( !hiteof );

	printf("Finishing read on %d\n", ip);

	LINE->opcode = O_DIE;
	LINE->arg = (p_val){ TYPE_EMPTY, {0}};
	LINE->exec = op_exectab[O_DIE];

	VD->inst = linetab_add(VD->inst, LINE, ip);

	VD->pdata->linecount = ip + 1;
}

UFLAG Uno_write(FILE *destfile, vmdata commit)
{
	
	struct header phdr = 
	{
		0x1885,
		"Uno"
	};

	if (fwrite(&phdr, sizeof(struct header), 1, destfile) < 1)
		print_error(1, "Error: couldn't write program header\n");

	if (fwrite(commit.pdata, sizeof(struct metadata), 1, destfile) < 1)
		print_error(1, "Error: couldn't write metadata\n");

	ERRCHECK

	unsigned int i;

	printf("Uno_write received: proccount=%d linecount=%d\n", commit.pdata->proccount, commit.pdata->linecount);

	for (i = 0 ; i < commit.pdata->proccount; ++i)
		fwrite(&commit.proc[i], sizeof(routine), 1, destfile);

	for (i = 0 ; i < commit.pdata->linecount; ++i)
	{
		//commit.inst[i].opcode -= OP_START;
		fwrite(&commit.inst[i], sizeof(line), 1, destfile);
	}

	return F_WRITESUCCESS;
}