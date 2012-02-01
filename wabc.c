/* wabc.c -- compiler for Uno, a stack-oriented language */

/* exports: UNO_read UNO_write */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "global.h"
#include "ops.h"
#include "wabc.h"

#define IN_LINESZ 96

#define STRT_LINECT 128

static int fatalerr = 0;

unsigned int RSHash(char *str)
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

routine * proctab_add(routine *pt, unsigned int *ptp, char *add, unsigned int lp)
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



line * linetab_add(line *tab, line *add, int lp)
{
	/* problem: tab is relocated by realloc() but for some reason
	vm_pass.inst isn't being given the proper new address - return 
	pointer from this function of current location of tab
	*/

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

routine * find_proc(routine *pt, int ptp, unsigned int hash)
{
	int i;

	for (i = ptp-1; i >= 0; --i)
	{
		if (pt[i].hash == hash)
			return &pt[i];
	}
	return NULL;
}

int find_procid(routine *pt, int ptp, unsigned int hash)
{
	int i;

	for (i = ptp=1; i >= 0; --i)
	{
		if (pt[i].hash == hash)
			return i;
	}

	return -1;
}

vmdata UNO_read(FILE *infile)
{
	unsigned int lp = 0;	// Line
	unsigned int ip = lp;	// Instruction
	
	int nest = ip;		// Total nest level
	int localinit = nest;		// Local nest level (initial)
	//int hasarg = localinit;


	char *in_line_o;
	unsigned int linesz = IN_LINESZ;
	in_line_o = (char *) malloc(linesz + 1);


	vmdata vm_pass = vmdata_init(); // start the main data structure
	unsigned int linesallocated = LINE_START;

	vm_pass.inst[linesallocated-1] = GUARD_LINE;


	char *word = (char *) malloc(25);


	routine *cur;

	char *procn = (char *) malloc(25);

	char *newrout = (char *) malloc(25);

	struct
	{
		unsigned int bp;
		struct b *values;
	} block;

	block.bp = -1;
	block.values = calloc(16, 2*sizeof(int));

	if (block.values == NULL)
		print_error(1, "Error: unable to allocate block stack\n");

	ERRCHECK

	struct b popblock;

	line *n_l = calloc(1, sizeof(line));

	while (1)
	{
		/**********************************************************
		 ***************************INPUT**************************
		 **********************************************************/
		 
		/* This is the order we have to go in to find out what type of word we've
		 * been given:
		 * * Check if it begins with a digit (number literal)
		 * * Check if it ends with a colon (routine definition)
		 * * Check if it begins with a parenthesis (routine call)
		 */

		 if (feof(infile)) break;

		fscanf(infile, " %s ", word);
		
		if (word == NULL)
		{
			++lp;
			continue;
		}

		if (isdigit(word[0]))
		{
			sscanf(word, " %i ", &n_l->arg.v.ival);
			n_l->opcode = O_PUSH;

			n_l->arg.type = TYPE_IVAL;

			vm_pass.inst = linetab_add(vm_pass.inst, n_l, ip);

			ip++;
			continue;
		}

		if (strchr(word, ':') - word > 0)
		{
			n_l->opcode = O_ROUTINE;

			BPUSH(n_l->opcode, ip);

			sscanf(word, "%[^:]", newrout);

			cur = proctab_add(vm_pass.proc, 
							  &vm_pass.pdata->proccount, 
							  newrout, 
							  ip);

			
			//printf("Adding %s at %d (%d), (%d)\n", newrout, ip, 
			//	vm_pass.proc[0].hash, vm_pass.pdata->proccount);
			++nest;

			++ip;

			continue;
		}

		if (word[0] == '(')
		{
			if (sscanf(word, "(%[^)])", procn) < 1)
			{
				fprintf(stderr, "Error: Unclosed `(' at line %d\n", lp+1);
				exit(0);
			}

			n_l->opcode = O_CALL;

			int prid = find_procid(vm_pass.proc, 
							   vm_pass.pdata->proccount, 
							   RSHash(procn));

			if (prid < 0)
			{
				fprintf(stderr, "Error: Undefined routine %s\n", procn);
				exit(0);
			}

			n_l->arg.type = TYPE_IVAL;
			n_l->arg.v.ival = prid;

			vm_pass.inst = linetab_add(vm_pass.inst, n_l, ip);

			ip++;

			continue;
		}
		//printf("Parsing: %s(%s)\n", opcode, (argument == NULL) ? "" : argument);

		/*if (OPCODE_IS("routine")
		{
			n_l->opcode = O_ROUTINE;

			BPUSH(n_l->opcode, ip);

			if (hasarg)
				cur = proctab_add(vm_pass.proc, 
								  &vm_pass.pdata->proccount, 
								  argument, 
								  ip);

			++nest;
		}*/
		if (OPCODE_IS("while"))
		{
			n_l->opcode = O_JF;
			
			BPUSH(B_WH, ip);
			
			++nest;

			if (localinit)
				localinit = nest;
		}
		OP_CASE("end")
		{
			n_l->opcode = O_PER;

			popblock = BPOP();

			if (localinit)
				--localinit;

			if (popblock.type == O_ROUTINE)
			{
				cur->endpp = ip;
				
				vm_pass.inst[cur->pp - 1].arg.v.ival = cur->endpp;

			}
			
			if (Blk_iswhile(popblock.type))
			{
				line *g = &vm_pass.inst[popblock.ip];
				n_l->opcode = O_JT;
				set_ival(n_l, popblock.ip);
				set_ival(g, ip);
			}

			if (Blk_isif(popblock.type))
			{
				line *g = &vm_pass.inst[popblock.ip];
				n_l->opcode = O_NOP;
				set_ival(g, ip);
			}

			--nest;
		}
		OP_CASE("leave")
		{
			n_l->opcode = O_LEAVE;

			n_l->arg.type = TYPE_IVAL;

			n_l->arg.v.ival = cur->endpp;
		}
		OP_CASE("dup")
			n_l->opcode = O_DUP;
		OP_CASE("swap")
			n_l->opcode = O_SWAP;
		OP_CASE("over")
			n_l->opcode = O_OVER;
		OP_CASE("drop")
			n_l->opcode = O_DROP;
		SINGLE_OP_CASE('+')
			n_l->opcode = O_ADD;
		SINGLE_OP_CASE('-')
			n_l->opcode = O_SUB;
		SINGLE_OP_CASE('*')
			n_l->opcode = O_MULT;
		SINGLE_OP_CASE('/')
			n_l->opcode = O_DIV;
		OP_CASE("out")
			n_l->opcode = O_OUT;
		OP_CASE("outc")
			n_l->opcode = O_OUTC;
		OP_CASE("peek")
			n_l->opcode = O_PEEK;
		OP_CASE("if")
		{
			n_l->opcode = O_JF;
			BPUSH(B_IF, ip);
			++nest;
		}
		SINGLE_OP_CASE('=')
			n_l->opcode = O_EQ;
		OP_CASE("!=")
			n_l->opcode = O_NE;
		SINGLE_OP_CASE('!')
			n_l->opcode = O_NEG;
		OP_CASE("<<")
			n_l->opcode = O_SHL;
		OP_CASE(">>")
			n_l->opcode = O_SHR;
		OP_CASE("<=")
			n_l->opcode = O_LTE;
		SINGLE_OP_CASE('<')
			n_l->opcode = O_LT;
		OP_CASE(">=")
			n_l->opcode = O_GTE;
		SINGLE_OP_CASE('>')
			n_l->opcode = O_GT;
		SINGLE_OP_CASE('#')
			n_l->opcode = O_DEBUG;
		SINGLE_OP_CASE('&')
			n_l->opcode = O_AND;
		SINGLE_OP_CASE('|')
			n_l->opcode = O_OR;
		SINGLE_OP_CASE('^')
			n_l->opcode = O_XOR;
		SINGLE_OP_CASE('~')
			n_l->opcode = O_NOT;
		OP_CASE("rot")
			n_l->opcode = O_ROT;
		OP_CASE("st")
			n_l->opcode = O_ST;
		OP_CASE("die")
			n_l->opcode = O_DIE;
		OP_CASE(":=")
			n_l->opcode = O_SET;
		else
			print_error(1, 
				"Error: unrecognized opcode %s at line %d\n", word, lp+1);

		//printf("%d Adding: %d, %d\n", ip, n_l->opcode, GETVAL(n_l));
	
		vm_pass.inst = linetab_add(vm_pass.inst, n_l, ip);
		ERRCHECK

		++lp;
		++ip; // the only difference between lp and ip is that lp increases on blank lines, while ip does not. this is to give nice error messages with a line number identical to the physical file.
		//getchar();
	}
	printf("Finishing read on %d\n", ip);

	if (nest != 0)
		print_error(1, "Error: unclosed block\n");

	ERRCHECK

	n_l->opcode = O_DIE;
	n_l->arg = (p_val){TYPE_EMPTY, {0}};

	printf("ip - %d\n", ip);

	vm_pass.inst = linetab_add(vm_pass.inst, n_l, ip);

	vm_pass.pdata->linecount = ip+1;

	free(word);
	free(procn);
	free(newrout);
	free(block.values);
	//free(argument_o);
	free(n_l);
	free(in_line_o);

	return vm_pass;
}

UFLAG UNO_write(FILE *destfile, vmdata commit)
{
	
	struct header phdr = 
	{
		0x1885,
		"UNO"
	};

	if (fwrite(&phdr, sizeof(struct header), 1, destfile) < 1)
		print_error(1, "Error: couldn't write program header\n");

	if (fwrite(commit.pdata, sizeof(struct metadata), 1, destfile) < 1)
		print_error(1, "Error: couldn't write metadata\n");

	ERRCHECK

	unsigned int i;

	printf("UNO_write received: proccount=%d linecount=%d\n", commit.pdata->proccount, commit.pdata->linecount);

	for (i = 0 ; i < commit.pdata->proccount; ++i)
		fwrite(&commit.proc[i], sizeof(routine), 1, destfile);

	for (i = 0 ; i < commit.pdata->linecount; ++i)
	{
		//commit.inst[i].opcode -= OP_START;
		fwrite(&commit.inst[i], sizeof(line), 1, destfile);
	}

	return F_WRITESUCCESS;
}