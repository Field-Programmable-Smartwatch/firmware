#ifndef ASM_H
#define ASM_H

#define asm_svc(system_call, args) asm("mov r0, %1\n" "svc %0\n" :: "I" (system_call), "r" (r0))

#endif
