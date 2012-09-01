/* scan.c - lexer for uno. Identifies next token */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "global.h"
#include "scan.h"


#define BUFLEN 1024

static char linebuf[BUFLEN];
static int linepos = 0;
static int bufsize = 0;

#define MAXRES 17

static char nextC(void)
{
	if (linepos >= bufsize)
	{
		lineno++;
		if ((fread(linebuf, 1, BUFLEN, source)))
		{
			bufsize = strlen(linebuf);
			linepos = 0;

			linebuf[bufsize] = 0; 
			
			return linebuf[linepos++];
		}
		else
		{
			hiteof = 1;
			return EOF;
		}
	}
	else
	{
		return linebuf[linepos++];
	}
}

static void prevC(void)
{
	linepos--;
}

struct resword
{
	char *str;
	TokenType tok;
};

#define TOTAL_KEYWORDS 17
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 5
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 30
/* maximum key range = 28, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (str, len)
	register const char *str;
	register unsigned int len;
{
	static unsigned char asso_values[] =
	{
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		10, 10,  0, 31,  0, 10, 31, 31,  0, 31,
		15,  0, 10, 31,  5,  5,  0,  0,  5,  5,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
		31, 31, 31, 31, 31, 31
	};
	
	return len + asso_values[(unsigned char)str[1]] + 
		asso_values[(unsigned char)str[0]];
}

#ifdef __GNUC__
__inline
#ifdef __GNUC_STDC_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
TokenType
resLookup (str, len)
register const char *str;
register const int len;
{
	static const struct resword wordlist[] =
	{
		{"", ERROR }, { "", ERROR }, { "", ERROR },
		{ "out", OUT }, { "outc", OUTC },
		{ "", ERROR }, { "", ERROR }, { "st", ST }, { "rot", ROT },
		{ "over", OVER }, { "while", WHILE }, { "", ERROR },
		{ "if", IF }, { "dup", DUP }, { "swap", SWAP },
		{ "leave", LEAVE }, { "", ERROR }, { "", ERROR },
		{ "nop", NOP }, { "drop", DROP }, { "", ERROR },
		{ "", ERROR }, { "", ERROR },
		{ "die", DIE }, { "peek", PEEK }, { "decat", DECAT },
		{ "", ERROR }, { "", ERROR }, { "end", END }, { "", ERROR },
		{ "incat", INCAT },
	};

	register int key = hash (str, len);

	if (key <= MAX_HASH_VALUE && key >= 0)
	{
		register const struct resword s = wordlist[key];

		if (*str == *s.str && !strcmp (str + 1, s.str + 1))
			return s.tok;
	}

	return ID;
}
/*
static TokenType resLookup(char *s)
{
	sortedreswords[MAXRES] = (struct resword){ s, ID };

	int i = -1;

	do
	{
		i++;
	} while (strcmp(sortedreswords[i].str, s));
	
	return (i == MAXRES) ? ID : sortedreswords[i].tok;
}
*/
TokenType unolex(void)
{
	int tokStrIndex = 0;

	TokenType curtok;

	StateType state = Start;

	int save, endstr = 1;

	while ((state != Done))
	{
		char c = nextC();
		save = 1;

		hiteof = (c == EOF);

		save = !hiteof;

		switch (state)
		{
			case Start:
				if (isdigit(c))
					state = Innum;
				else if (isalpha(c))
					state = Inword;
				else if (c == ':')
					state = Inassign;
				else if (isspace(c))
				{
					save = 0;
					lineno += c == '\n';
				}
				else if (c == '!')
					state = Inexcl;
				else if (c == '<')
					state = Inlt;
				else if (c == '>')
					state = Ingt;
				else if (c == '"')
				{
					save = 0;
					state = Instr;
				}
				else if (c == ';')
				{
					save = 0;
					state = Incomment;
				}
				else
				{
					state = Done;

					switch (c)
					{
						case EOF:
							curtok = EOI;
							break;
						case '=':
							curtok = EQ;
							break;
						case '(':
							curtok = LPAREN;
							break;
						case ')':
							curtok = RPAREN;
							break;
						case '+':
							curtok = PLUS;
							break;
						case '-':
							curtok = MINUS;
							break;
						case '*':
							curtok = TIMES;
							break;
						case '/':
							curtok = SLASH;
							break;
						case '^':
							curtok = XOR;
							break;
						case '&':
							curtok = AND;
							break;
						case '|':
							curtok = OR;
							break;
						case '~':
							curtok = NOT;
							break;
						case '#':
							curtok = DEBUG;
							break;
						default:
							curtok = ERROR;
							break;
					}
				}
			break;

			case Inword:
				printf("In Inword, c: %c\n", c);
				if (!isalpha(c))
				{
					prevC();
					save = 0;
					state = Done;
					tokenval.v.strval = strdup(tokenstr);
					tokenval.type = TYPE_STRVAL;
					curtok = ID;
				}
				break;
			case Innum:
				//printf("In Innum: c: %c\n", c);
				if (!isdigit(c))
				{
					prevC();
					save = 0;
					state = Done;
					curtok = NUM;
					tokenval.v.ival = u_strtolong(tokenstr);
					tokenval.type = TYPE_IVAL;
				}
				break;
			case Inassign:
				if (c == '=')
				{
					state = Done;
					curtok = ASSIGN;
				}
				else
				{
					prevC();
					save = 0;
					state = Done;
					curtok = COLON;
				}
				break;
			case Inlt:
				switch (c)
				{
					case '<':
						state = Done;
						curtok = SHL;
						break;
					case '=':
						state = Done;
						curtok = LTE;
						break;
					default:
						prevC();
						save = 0;
						state = Done;
						curtok = LT;
						break;
				}
				break;
			case Ingt:
				switch (c)
				{
					case '>':
						state = Done;
						curtok = SHR;
						break;
					case '=':
						state = Done;
						curtok = GTE;
						break;
					default:
						prevC();
						save = 0;
						state = Done;
						curtok = GT;
						break;
				}
				break;
			case Inexcl:
				if (c == '=')
				{
					state = Done;
					curtok = NE;
				}
				else
				{
					prevC();
					save = 0;
					state = Done;
					curtok = NEG;
				}
				break;
			case Incomment:
				save = 0;
				if (c == '\n')
				{
					state = Start;
				}
				break;
			case Instr:
				save = 1;
				if (c == '"' && endstr)
				{
					curtok = STR;
					tokenval.v.strval = strdup(tokenstr);
					tokenval.type = TYPE_STRVAL;
					state = Done;
					save = 0;
				}

				if (c == '\\' && endstr)
				{
					save = 0;
					endstr ^= endstr;
				}
				else
					endstr = 1;
				
				break;
			default:
				fprintf(stderr, 
				"SCANNER FUCKED UP: state = %d line = %d\n", state, lineno);
				state = Done;
				curtok = ERROR;
				break;
		}


		if (save && (tokStrIndex <= bufsize))
		{
			tokenstr[tokStrIndex++] = c;
		}

		if (state == Done)
		{
			tokenstr[tokStrIndex] = 0;

#			ifdef UNODEBUG
				printf("tokenstr = %s\n", tokenstr);
#			endif

			if (curtok == ID)
			{
				curtok = resLookup(tokenstr, strlen(tokenstr));

#				ifdef UNODEBUG
					printf("curtok on ID = %s\n", tok2str(curtok));
#				endif
			}
		}
	}

	//printf("Returning %d\n", curtok);

	return curtok;
}



char *tok2str(TokenType tok)
{
	char *str;

	switch (tok)
	{
		case ID:
			str = "ID";
			break;
		case NUM:
			str = "Number";
			break;
		case OP:
			str = "Operator";
			break;
		case LPAREN:
			str = "Left paren";
			break;
		case RPAREN:
			str = "Right paren";
			break;
		case COLON:
			str = "Colon";
			break;
		case EOI:
			str = "_End of Input";
			break;
		case DROP:
			str = "Drop";
			break;
		case DUP:
			str = "Dup";
			break;
		case OVER:
			str = "Over";
			break;
		case OUT:
			str = "Out";
			break;
		case OUTC:
			str = "Character out";
			break;
		case INCAT:
			str = "Inc at";
			break;
		case DECAT:
			str = "Dec at";
			break;
		case NOP:
			str = "Nop";
			break;
		case ST:
			str = "Stack acc";
			break;
		case DIE:
			str = "Die";
			break;
		case LEAVE:
			str = "Leave";
			break;
		case WHILE:
			str = "While";
			break;
		case IF:
			str = "If";
			break;
		case PEEK:
			str = "Peek";
			break;
		case SWAP:
			str = "Swap";
			break;
		case ROT:
			str = "Rot";
			break;
		case END:
			str = "End";
			break;
		case PLUS:
			str = "Plus";
			break;
		case MINUS:
			str = "Minus";
			break;
		case TIMES:
			str = "Times";
			break;
		case SLASH:
			str = "Slash";
			break;
		case AND:
			str = "And";
			break;
		case OR:
			str = "Or";
			break;
		case XOR:
			str = "Xor";
			break;
		case NOT:
			str = "Not";
			break;
		case SHR:
			str = "Right shift";
			break;
		case SHL:
			str = "Left shift";
			break;
		case ASSIGN:
			str = "Assignment";
			break;
		case NEG:
			str = "Negation";
			break;
		case EQ:
			str = "Equals";
			break;
		case NE:
			str = "Not equal";
			break;
		case LT:
			str = "Less than";
			break;
		case LTE:
			str = "Less than or equal to";
			break;
		case GT:
			str = "Greater than";
			break;
		case GTE:	
			str = "Greater than or equal to";
			break;
		case DEBUG:
			str = "Debug";
			break;
		case ERROR:
			str = "_Error";
			break;
		case STR:
			str = "Stringval";
			break;
		default:
			str = "_Unknown";
			break;
	}

	return str;
}


/*
int main(int argc, char **argv)
{
	if (argv[1])
	{
		source = fopen(argv[1], "r");

		listing = stdout;
	}
	else
	{
		fprintf(stderr, "No file supplied\n");
		exit(1);
	}

	TokenType t;

	while ( !hiteof )
	{
		t = unolex();
		fprintf(listing, "%d. %s: '%s'\n", lineno, 
			tok2str(t), tokenstr);
	}

	fclose(source);
	//free(linebuf);

	return 0;
}
*/