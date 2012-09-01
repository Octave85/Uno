/* exec.h -- defines macros and types specific to the VM */

#ifndef VM_H
#define VM_H



#define VM_cleanup() free(vd.inst); \
					 free(vd.proc); \
					 free(vd.pdata); \
					 free(beginstack->values); \
					 free(beginstack); \
					 fclose(exec);					

#define CUR_LINE (VD->inst[POINT->inst])

#define IP pointers->inst

#define SET_STATE(s) (pointers->state = s)

#define STACK_START 64

#define STACK_GROW 128

//#define stackpush(s, val) (s->values[++s->p] = val)

//#define stackpop(s) (s->values[--s->p])

#define Op_exec(name) ret = name(ex->arg, pointers, thestack)

vmdata VM_read(FILE *);

UFLAG VM_exec(UnoMS *);

STACK * VM_callstack();

#endif