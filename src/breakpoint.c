#include <stdlib.h>

#ifdef __linux__
#include <sys/ptrace.h>
#else
#include <sys/types.h>
#include <sys/ptrace.h>
#define PTRACE_PEEKDATA PT_READ_D
#define PTRACE_POKEDATA PT_WRITE_D
#endif

#include "breakpoint.h"

const uint8_t INTERRUPT_BYTE = 0xcc;

breakpoint_t *breakpoint_create(pid_t pid, uint64_t address) {
  breakpoint_t *breakpoint = malloc(sizeof(breakpoint_t));
  if (breakpoint == NULL)
    return NULL;

  breakpoint->pid = pid;
  breakpoint->address = address;
  breakpoint->enabled = 0;
  breakpoint->saved_data = 0;

  return breakpoint;
}

void breakpoint_enable(breakpoint_t *breakpoint) {
  int data = ptrace(PTRACE_PEEKDATA, breakpoint->pid, breakpoint->address, 0);
  // Save the lowest byte as we will overwrite it with an interrupt byte
  breakpoint->saved_data = data & 0xff;
  uint64_t breakpoint_data = ((data & ~0xff) | INTERRUPT_BYTE);
  ptrace(PTRACE_POKEDATA, breakpoint->pid, breakpoint->address, breakpoint_data);
  breakpoint->enabled = 1;
}

void breakpoint_disable(breakpoint_t *breakpoint) {
  int data = ptrace(PTRACE_PEEKDATA, breakpoint->pid, breakpoint->address, 0);
  // Restore the data we inserted an interrupt byte into previously
  uint64_t restored_data = ((data & ~0xff) | breakpoint->saved_data);
  ptrace(PTRACE_POKEDATA, breakpoint->pid, breakpoint->address, restored_data);
  breakpoint->enabled = 0;
}
