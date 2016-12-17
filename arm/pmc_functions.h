/* Helper functions for using the ARM Perfomance Monitoring Unit (PMU) */

#include "events.h"
#define MAX_COUNTERS 4
#define CYCLE_COUNTER 0
#define IDX_TO_COUNTER(x) \
          ((x - 1) & (MAX_COUNTERS-1))



/* select_event_counter - Select an event counter to read/write
Used in conjunction with either set_event_counter or read_counter
Sets a value in the PMSELR register to control register selection
@index : counter number to access, ranges from 1 to MAX_COUNTERS

Returns nothing
*/
static inline void select_event_counter(uint32_t index)
{
  uint32_t idx = IDX_TO_COUNTER(index);
  __asm__ volatile("MCR p15, 0, %0, c9, c12, 5" :: "r" (idx));
}


/* set_event_counter - Set a counter to monitor a specified event
Sets the PMXEVTYPER register
@index : counter to access, ranges from 1 to MAX_COUNTERS
@event : event to monitor

Returns nothing
*/
static inline void set_event_counter(uint32_t index, uint32_t event)
{
  select_event_counter(index);
  __asm__ volatile("MCR p15, 0, %0, c9, c13, 1" :: "r" (event));
}


/* read_event_counter - Read a performance monitoring counter
Reads using the PMXEVCNTR register
@index : counter to accesss

Returns value read from counter
*/
static inline uint32_t read_event_counter(uint32_t index)
{
  uint32_t value = 0;
  select_event_counter(index);
  if (index == CYCLE_COUNTER)
    __asm__ volatile("MRC p15, 0, %0, c9, c13, 0" : "=r" (value));

  else
    __asm__ volatile("MRC p15, 0, %0, c9, c13, 2" : "=r" (value));
  return value;
}


/* write_event_counter - Write to a performance monitoring counter
Reads using the PMXEVCNTR register
@index : counter to accesss
@value : value to write

Returns nothing
*/
static inline void write_event_counter(uint32_t index, uint32_t value)
{
  select_event_counter(index);
  __asm__ volatile("MCR p15, 0, %0, c9, c13, 2" :: "r" (value));
}


/* reset_counter - Reset the cycle counter

Returns nothing
*/
static inline void reset_counter(void)
{
  /* Enable and reset cycle counter through the PMCR
  PMCR  bit 0 : Enable user access
        bit 1 : Reset cycle counter
  */
  __asm__ volatile("MCR p15, 0, %0, c9, c12, 0":: "r"(0x3));
}


/* init_fixed_function_counters - Enable event counters

Returns nothing
*/
static inline void init_counters(void)
{
  /* Enable and reset event counters through the PMCR
  PMCR  bit 0 : Enable user access
        bit 1 : Reset cycle counter
  */
  __asm__ volatile("MCR p15, 0, %0, c9, c12, 0":: "r"(0x3));

  /* Enable cycle counter and 4 event counters
  PMCNTENSET  bit 31 : cycle counter
              bits 0-n : event counters (n is implementation specific)
  */
  __asm__ volatile("MCR p15, 0, %0, c9, c12, 1" :: "r"(0x8000000f));
}
