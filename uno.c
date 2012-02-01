/* uno.c -- toplevel program functions - main function and those called by it */

/* exports: none */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "global.h"
#include "wabc.h"
#include "vm.h"

static unsigned int mode;

UFLAG MODE_compile(FILE *infile, FILE *outfile)
{
	vmdata commit = UNO_read(infile);

	UNO_write(outfile, commit);

	fclose(infile);
	fclose(outfile);
	free(commit.inst);
	free(commit.proc);
	free(commit.pdata);

	return F_COMPILESUCCESS;
}

UFLAG MODE_run(FILE *execfile, int offset, int argc, char **argv)
{
	vmdata vd;
	vd = VM_read(execfile);
	
	STACK *beginstack;

	beginstack = VM_init(offset, argc, argv);
	
	VM_exec(vd, beginstack);

	fclose(execfile);
	free(beginstack->values);
	free(beginstack);
	free(vd.inst);
	free(vd.proc);
	free(vd.pdata);

	return F_EXECSUCCESS;
}

UFLAG MODE_interpret(FILE *infile, int argc, char **argv)
{
	vmdata vd;
	vd = UNO_read(infile);

	STACK *beginstack;

	beginstack = VM_init(2, argc, argv);

	VM_exec(vd, beginstack);

	fclose(infile);
	free(beginstack->values);
	free(beginstack);
	free(vd.inst);
	free(vd.proc);
	free(vd.pdata);

	return F_EXECSUCCESS | F_COMPILESUCCESS;
}

int main(int argc, char **argv)
{
	
	mode = 0;

	int c;

	char *infile, *outfile;

	c = getopt(argc, argv, "c:e:o:");

	do
	{

		switch (c)
		{
			case 'c':
				mode |= (MODE_COMPILE | MODE_WRITE);
				infile = strdup(optarg);
				break;
			case 'e':
				mode |= MODE_EXECUTE;
				infile = strdup(optarg);
				break;
			case 'o':
				outfile = strdup(optarg);
				break;
		}
	} while ((c = getopt(argc, argv, "c:e:o:")) != -1);

	if (mode == 0)
	{
		mode = MODE_COMPILE | MODE_EXECUTE;
		infile = argv[1];

		if (infile == NULL)
		{
			fprintf(stderr, "UnoError: No filename supplied\n");
			exit(0);
		}
	}

	FILE *in, *out;

	in = fopen(infile, "r");
	if ( !in )
	{
		fprintf(stderr, "UnoError: Couldn't open %s for reading\n", argv[1]);
		exit(0);
	}
	
	if (mode != (MODE_COMPILE | MODE_EXECUTE))
		free(infile);


	if (mode & MODE_WRITE)
	{
		out = fopen(outfile, "w");

		if ( !out )
		{
			fprintf(stderr, "UnoError: Couldn't open %s for writing\n", argv[2]);
			exit(0);
		}
		
		free(outfile);	
	}

	if (mode == (MODE_COMPILE | MODE_WRITE))
		MODE_compile(in, out);
	else if (mode == (MODE_COMPILE | MODE_EXECUTE))
		MODE_interpret(in, argc, argv);
	else if (mode == MODE_EXECUTE)
		MODE_run(in, optind, argc, argv);

	return 0;
}