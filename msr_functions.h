/* Helper functions for using Model Specific Registers (MSR) */

#include "events.h"

/* 2 programmable monitoring counters */
#define PMC0 0xc1
#define PMC1 0xc2

#define PMC0_ctrl 0x186
#define PMC1_ctrl 0x187

/* 3 fixed function counters
FFC0 - instructions retired
FFC1 - unhalted cycle count
FFC2 - reference cycle count (invariant to frequency changes -- eq. to time)
*/
#define FFC0 0x309
#define FFC1 0x30a
#define FFC2 0x30b


/* read_msr - Read from a model specific register
@msr_address : register to read from

Returns value stored in specified register
*/
unsigned long read_msr(uint32_t msr_address)
{
  uint32_t lo=0, hi=0;
  /*
    input:
      ecx - msr address
    output:
      edx:eax - 40 bit value
  */
  __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr_address));
  return ((long long)lo) | (((long long)hi) << 32);
}


/* write_msr - Write 40 bit value to model specific register
@msr_address : register to write to
@lo : lower 32 bits to write
@hi : higher 8 bits to write

Returns nonthing
*/
void write_msr(uint32_t msr_address, uint32_t lo, uint32_t hi)
{
  /*
    input:
      exc - msr address
      edx:eax - value to write
  */
  __asm__ volatile("wrmsr"::"c"(msr_address),"a"(lo),"d"(hi));
}


/* reset_counter - Reset the performance monitoring counter
@msr_address - register to clear

Returns nothing
*/
void reset_counter(uint32_t msr_address)
{
  write_msr(msr_address,0x00,0x00);
}


/* init_fixed_function_counters - Enable fixed function and programmable performance counters

Returns nothing
*/
void init_counters(void)
{
  /* Disable all 3 counters */
  write_msr(0x38f, 0x00, 0x00);

  reset_counter(FFC0); reset_counter(FFC1); reset_counter(FFC2);
  reset_counter(PMC0); reset_counter(PMC1);

  /* Enable 3 ffcs and 4 pmcs  */
  write_msr(0x38d, 0x222, 0x00);  // ia32_perf_fixed_ctr_ctrl
  write_msr(0x38f, 0x0f, 0x07); // ia32_perf_global_ctrl
}


/* set_pmc - Use a PMC to monitor specified event
@pmc_number : pmc to be used
@event : One of the events from "events.h" to monitor in this pmc

Returns nothing
*/
void set_pmc(uint32_t pmc_number, uint16_t event)
{
  uint32_t pmc = (pmc_number==PMC1)?PMC1_ctrl:PMC0_ctrl;
  uint32_t event_select = (0x41 << 16) | event;
  write_msr(pmc,event_select,0x0);
}
