/* ops.h -- opcode #defines */

#ifndef OPS_H
#define OPS_H

#define OP_COUNT 45

#define OP_START 0

#define O_ROUTINE 	(OP_START    )
#define O_WHILE		(OP_START + 1)
#define O_CALL		(OP_START + 2)
#define O_PER		(OP_START + 3)
#define O_PUSH		(OP_START + 4)
#define O_LEAVE		(OP_START + 5)
#define O_DUP		(OP_START + 6)
#define O_SWAP		(OP_START + 7)
#define O_OVER		(OP_START + 8)
#define O_DROP		(OP_START + 9)
#define O_ADD		(OP_START + 10)
#define O_SUB		(OP_START + 11)
#define O_MULT		(OP_START + 12)
#define O_DIV		(OP_START + 13)
#define O_OUT 		(OP_START + 14)
#define O_OUTC		(OP_START + 15)
#define O_PEEK		(OP_START + 16)
// #define O_JMP		(OP_START + 18) -- DEPRECATED
#define O_IFE		(OP_START + 17)
#define O_IFN		(OP_START + 18)
#define O_IFG		(OP_START + 19)
#define O_IFGE		(OP_START + 20)
#define O_IFL		(OP_START + 21)
#define O_IFLE		(OP_START + 22)
#define O_DEBUG		(OP_START + 23)
/* internal */
#define O_JMP		(OP_START + 24)
#define O_JT		(OP_START + 25)
#define O_JF		(OP_START + 26)
/* end internal */
#define O_EQ		(OP_START + 27)
#define O_NE		(OP_START + 28)
#define O_LT		(OP_START + 29)
#define O_LTE		(OP_START + 30)
#define O_GT		(OP_START + 31)
#define O_GTE		(OP_START + 32)
#define O_SHR		(OP_START + 33)
#define O_SHL		(OP_START + 34)
#define O_AND		(OP_START + 35)
#define O_OR 		(OP_START + 36)
#define O_XOR		(OP_START + 37)
#define O_NOT		(OP_START + 38)
#define O_ROT 		(OP_START + 39)
#define O_ST 		(OP_START + 40)
#define O_DIE		(OP_START + 41)
#define O_SET		(OP_START + 42)
#define O_NOP		(OP_START + 43)
#define O_NEG		(OP_START + 44)

#ifdef UNODEBUG
static const char* OP_tostring[OP_COUNT] =
{
	"routine",
	"while",
	"call",
	".",
	"push",
	"leave",
	"dup",
	"swap",
	"over",
	"drop",
	"add",
	"sub",
	"mult",
	"div",
	"out",
	"outc",
	"peek",
	"ife (nop)",
	"ifn (nop)",
	"ifg (nop)",
	"ifge (nop)",
	"ifl (nop)",
	"ifle (nop)",
	"debug",
	"jmp",
	"jt",
	"jf",
	"eq",
	"ne",
	"lt",
	"lte",
	"gt",
	"gte",
	"shr",
	"shl",
	"and",
	"or",
	"xor",
	"not",
	"rot",
	"st",
	"die",
	"set",
	"nop",
	"neg",
};

#endif /* UNODEBUG */

#endif /* OPS_H */