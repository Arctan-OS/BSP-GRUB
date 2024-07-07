#ifndef ARC_MM_VMM_H
#define ARC_MM_VMM_H

#include <stdint.h>

#define ARC_VMM_NO_EXEC (1 << 0)
#define ARC_VMM_2MIB (1 << 1)
#define ARC_VMM_1GIB (1 << 2)

int Arc_MapPageVMM(uint64_t physical, uint64_t virtual, uint32_t flags);
int Arc_InitVMM();

#endif
