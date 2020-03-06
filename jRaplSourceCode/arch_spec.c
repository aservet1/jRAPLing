#include <stdio.h>
//#include <jni.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "arch_spec.h"


/** <Alejandro's Interpretation>
 *	- (?) Direct  CPUID  access  through  this  device
          should only be used in exceptional cases.
 *	Calls cpuid with eax=1 which returns info abt if its SANDYBRIDGE, BROADWELL, ...
 *	EAX	Version Information: Type, Family, Model, and Stepping ID
-	EBX	Bits 7-0: Brand Index
-	-	Bits 15-8: CLFLUSH line size (Value . 8 = cache line size in bytes)
-	-	Bits 23-16: Number of logical processors per physical processor; two for the Pentium 4 processor supporting Hyper-Threading Technology
 */
uint32_t
get_cpu_model(void)
{
	uint32_t eax, ebx, ecx, edx;
    eax = 0x01;
	ecx = 0x02; //might be the wrong value to put here, but had to put something to not make compiler warnings when ecx used in CPUID
	CPUID;
	return (((eax>>16)&0xFU)<<4) + ((eax>>4)&0xFU);
}

/** <Alejandro's Interpretation>
 * Gets the number of processors with sysconf. # processors == # cores (right(?))
 */
int core_num() {
	return sysconf(_SC_NPROCESSORS_CONF); //passed in is number of configured processors
}

/** <Alejandro's Interpretation>
 *	Gets info from cpuid call to identify where the APIC stuff is.
 *	To my understnading, APIC stuff can target parts of the process and interrupt it // APIC the advanced programmable interrupt controller
-	tech that intel developed to streamline interrupt handling on multiprocessor systems

 *	We care about this bc we want to interrupt processes that pass a certain energy level
 */
void
parse_apic_id(cpuid_info_t info_l0, cpuid_info_t info_l1, APIC_ID_t *my_id){

	// Get the SMT ID (SMT = Simultaneous MultiThreading)
	uint64_t smt_mask_width = info_l0.eax & 0x1f;
	uint64_t smt_mask = ~((-1) << smt_mask_width);
	my_id->smt_id = info_l0.edx & smt_mask;

	// Get the core ID
	uint64_t core_mask_width = info_l1.eax & 0x1f;
	uint64_t core_mask = (~((-1) << core_mask_width ) ) ^ smt_mask;
	my_id->core_id = (info_l1.edx & core_mask) >> smt_mask_width;

	// Get the package ID
	uint64_t pkg_mask = (-1) << core_mask_width;
	my_id->pkg_id = (info_l1.edx & pkg_mask) >> core_mask_width;
}

/** <Alejandro's Interpretation>
 *	Gets CPUID info given eax_in and ecx_in and eax and ecx x86 args. Stores
 *  reulting e[a/b/c/d]x values in a cpuid_info_t struct
 */
void cpuid(uint32_t eax_in, uint32_t ecx_in, cpuid_info_t *ci) {
	 asm (
#if defined(__LP64__)           /* 64-bit architecture */
	     "cpuid;"                /* execute the cpuid instruction */
	     "movl %%ebx, %[ebx];"   /* save ebx output */
#else                           /* 32-bit architecture */
	     "pushl %%ebx;"          /* save ebx */
	     "cpuid;"                /* execute the cpuid instruction */
	     "movl %%ebx, %[ebx];"   /* save ebx output */
	     "popl %%ebx;"           /* restore ebx */
#endif
             : "=a"(ci->eax), [ebx] "=r"(ci->ebx), "=c"(ci->ecx), "=d"(ci->edx)
             : "a"(eax_in), "c"(ecx_in)
        );
}


/** <Alejandro's Interpretation>
 *  Wraps up the cpuid function that gets cpuid information stuff. More abstract and easy to deal with
 *	no specific numbers or assembly or whatever. Always passes in 0xb because thats the cpuid() arg that
 *	gives info about packages and cores and APIC info
 *
 *	see intel manual pdf p. 771 for info about when eax_in = 0x0b; ebx bits 15-00 are number of logical preprocessors at this level type
 *		the number reflects configuration as shipped by Intel

	- INTEL: CPUID eax=0x0000000B
	  For Intel CPUs (and not AMD), this CPUID function tells you "Number of bits to shift right APIC ID to get next level APIC ID", and needs to be used twice. The 		  first time (with ECX=0) it tells you how many bits of the APIC ID is used to identify the logical CPU within each core (logical_CPU_bits). The second time (with
	  ECX=1) it tells you how many bits of the APIC ID is used to identify the core and logical CPU within the chip, and to get "core_bits" from this value you subtract
	  "logical_CPU_bits" from it.
 */
cpuid_info_t getProcessorTopology(uint32_t level) {
	cpuid_info_t info;
	cpuid(0xb, level, &info); ///define a constant for 0xb at some point...
	return info;
}

uint64_t get_num_core_thread()
{
	uint32_t level1 = 0;
	cpuid_info_t infol0 = getProcessorTopology(level1);
	return infol0.ebx & 0xffff;
}

uint64_t get_num_pkg_thread()
{
	uint32_t level2 = 1;
	cpuid_info_t infol1 = getProcessorTopology(level2);
	return infol1.ebx & 0xffff;
}

uint64_t get_num_pkg_core()
{
		uint32_t num_core_thread = get_num_core_thread();
		uint32_t num_pkg_thread = get_num_pkg_thread();
		return num_pkg_thread / num_core_thread;
}

/** <Alejandro's Interpretation>
 *	Initializes some data about the system, returns number of cores.

	Below used to be global variables
	  num_core_thread; 	//number of physical threads per core
	  num_pkg_thread; 	//number of physical threads per package
	  num_pkg_core;		//number of cores per package
	  num_pkg; 			//number of packages for current machine

 */
uint64_t getSocketNum() {
	int coreNum = core_num();
	uint64_t num_pkg_thread = get_num_pkg_thread();
	uint64_t num_pkg = coreNum / num_pkg_thread;
	return num_pkg;
}

int get_architecture_category(uint32_t cpu_model){
	if(
			cpu_model == SANDYBRIDGE_EP ||
			cpu_model == HASWELL1 ||
			cpu_model == HASWELL2 ||
			cpu_model == HASWELL3 ||
			cpu_model == HASWELL_EP ||
			cpu_model == SKYLAKE1 ||
			cpu_model == SKYLAKE2 ||
			cpu_model == BROADWELL ||
			cpu_model == BROADWELL2 ||
			cpu_model == APOLLOLAKE ||
			cpu_model == COFFEELAKE2
	) return READ_FROM_DRAM;

	if(
		cpu_model ==  SANDYBRIDGE ||
		cpu_model == IVYBRIDGE
	) return READ_FROM_GPU;

	return UNDEFINED_ARCHITECTURE;

}	

