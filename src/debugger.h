#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <unistd.h>

typedef struct {
  pid_t pid;
  const char *program;
} debugger_t;

debugger_t *debugger_create(const char *program);
int debugger_start(debugger_t *debugger);

#endif
