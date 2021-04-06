#include <stdio.h>
#include <stdlib.h>

#include "breakpoint.h"
#include "debugger.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Program name not specified\n");
    return -1;
  }

	const char *program = argv[1];

  debugger_t *debugger = debugger_create(program);
  if (!debugger) {
    printf("Unable to create debugger\n");
    return -1;
  }

  if (debugger_start(debugger)) {
    printf("Debugging failed\n");
  }

  free(debugger);
  printf("Done debugging. Exiting\n");
}
