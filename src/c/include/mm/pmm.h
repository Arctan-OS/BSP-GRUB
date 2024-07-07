#ifndef ARC_MM_PMM_H
#define ARC_MM_PMM_H

void *Arc_AllocPMM();
void *Arc_FreePMM(void *address);
void *Arc_ContiguousAllocPMM(int objects);
void *Arc_ContiguousFreePMM(void *address, int objects);

int Arc_InitPMM();

#endif
