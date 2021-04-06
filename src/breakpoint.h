#ifndef BREAKPOINT_H
#define BREAKPOINT_H

#include <stdint.h>
#include <unistd.h>

typedef struct {
  pid_t pid;
  uint64_t address;
  int enabled;
  uint8_t saved_data;
} breakpoint_t;

breakpoint_t *breakpoint_create(pid_t pid, uint64_t address);
void breakpoint_enable(breakpoint_t *breakpoint);
void breakpoint_disable(breakpoint_t *breakpoint);

#endif
