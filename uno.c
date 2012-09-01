/* uno.c -- toplevel program functions - main function and those called by it */

/* exports: none */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "global.h"
#include "parse.h"
#include "exec.h"

static unsigned int mode;

UFLAG MODE_interpret(int argc, char **argv)
{
	UnoMS *ums;

	ums = Uno_init(ums);

	Wabc_init(ums);

	Uno_parsemain(ums);

	VM_init(ums, 2, argc, argv);

	VM_exec(ums);

	fclose(source);
	Uno_exit(ums);

	return F_EXECSUCCESS | F_COMPILESUCCESS;
}

UFLAG MODE_console(int optind, int argc, char **argv)
{
	UnoMS *ums;

	Uno_init(ums);
	Wabc_init(ums);
	VM_init(ums, optind, argc, argv);

	register int ip = 0;

	printf(">>> ");

	scanf("%s", TLINE->text);
	
	do
	{
		TLINE->len = strlen(TLINE->text);
		LINE = Uno_parse(
								LINE,  
								VD, 
								BSTACK, 
								ip++);
		VM_exec_op(VD, MSTACK, CSTACK, POINT, LINE);
		printf("\n>>> ");
	} 
	while (scanf("%s", TLINE->text) > 0);


	Uno_exit(U);
}

int main(int argc, char **argv)
{
	
	mode = 0;

	int c;

	char *infile, *outfile;


#	define OPTSTR "i"
	c = getopt(argc, argv, OPTSTR);

	do
	{
		switch (c)
		{
			case 'i':
				mode = MODE_CONSOLE;
				break;
			default:
				mode = MODE_INTERP;
		}
	} while ((c = getopt(argc, argv, OPTSTR)) != -1);

	switch (mode)
	{
		case MODE_INTERP:
			if ( !argv[1] )
			{
				fprintf(stderr, "No file supplied!\n");
				exit(0);
			}

			listing = stdout;

			source = fopen(argv[1], "r");

			if ( !source )
				fprintf(stderr, "Couldn't open %s for reading\n", argv[1]);

			MODE_interpret(argc, argv);

			break;
		case MODE_CONSOLE:
			listing = stdout;
			source = stdin;

			MODE_console(optind, argc, argv);
			break;
		default:
			fprintf(stderr, "Unrecognized mode %d\n", mode);
			exit(1);
	}

	return 0;
}