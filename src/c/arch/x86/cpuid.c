#include <arch/x86/cpuid.h>
#include <cpuid.h>
#include <stdint.h>
#include <global.h>
#include <arch/x86/ctrl_regs.h>
#include <inttypes.h>

#define PAGE_ATTRIBUTE(n, val) (uint64_t)((uint64_t)(val & 0b111) << (n * 8))

int check_cpuid() {
	register uint32_t eax;
        register uint32_t ebx;
        register uint32_t ecx;
        register uint32_t edx;

	__cpuid(0x00, eax, ebx, ecx, edx);

	uint32_t max_basic_value = eax;

        __cpuid(0x1, eax, ebx, ecx, edx);

	if (((edx >> 6) & 1) == 0) {
		// PAE supported
		ARC_DEBUG(ERR, "PAE not supported\n")
		ARC_HANG
	}

	if (((edx >> 9) & 1) == 1) {
		// APIC On Chip
		// Commands to be submitted somewhere between 0xFFFE0000 and 0xFFFE0FFF
		ARC_DEBUG(INFO, "APIC On Chip\n")
	}

        if (((edx >> 16) & 1) == 1) {
                ARC_DEBUG(INFO, "PATs present, initializing\n");

                uint64_t msr = _x86_RDMSR(0x277);

                ARC_DEBUG(INFO, "Previous PATs: 0x%016"PRIX64"\n", msr);
                msr = 0;
                msr |= PAGE_ATTRIBUTE(0, 0x06); // WB
                msr |= PAGE_ATTRIBUTE(1, 0x00); // UC
                msr |= PAGE_ATTRIBUTE(2, 0x07); // UC-
                msr |= PAGE_ATTRIBUTE(3, 0x01); // WC
                msr |= PAGE_ATTRIBUTE(4, 0x04); // WT
                msr |= PAGE_ATTRIBUTE(5, 0x05); // WP
                ARC_DEBUG(INFO, "Current PATs : 0x%016"PRIX64"\n", msr);

                _x86_WRMSR(0x277, msr);
        }

	__cpuid(0x80000000, eax, ebx, ecx, edx);

	uint32_t max_extended_value = eax;

	if (max_extended_value < 0x80000001) {
		// Extended functions not supported
		ARC_DEBUG(ERR, "No CPUID extended functions")
		ARC_HANG
	}

	__cpuid(0x80000001, eax, ebx, ecx, edx);

	if (((edx >> 29) & 1) == 0) {
		// No LM support, for booting Arctan, cannot continue
		ARC_DEBUG(ERR, "No LM support\n")
		ARC_HANG
	}

	return 0;
}
