#ifndef SCAN_H
#define SCAN_H

#define MAXTOKENLEN 45

char tokenstr[MAXTOKENLEN];
int lineno;
int hiteof;

p_val tokenval;

typedef enum
	{ Start, Inword, Innum, Inlt, Ingt, 
		Inassign, Incomment, Inexcl, Instr, Done 
	} StateType;

typedef enum
{ 
	/* general */
	ID, NUM, OP, LPAREN, RPAREN, COLON, EOI, STR,

	/* reserved words */
	DROP, DUP, OUT, OUTC, INCAT, DECAT, OVER, NOP,
	ST, DIE, LEAVE, WHILE, IF, PEEK, SWAP, ROT, END,

	/* binary operators */
	PLUS, MINUS, TIMES, SLASH, AND, OR, XOR, SHR, SHL, ASSIGN,

	/* unary operators */
	NOT, NEG,

	/* comparison operators */
	EQ, NE, GT, GTE, LT, LTE,

	/* miscellaneous */
	DEBUG, ERROR, DQUOTE

} TokenType;

TokenType unolex(void);

char *tok2str(TokenType);

#endif /* SCAN_H */