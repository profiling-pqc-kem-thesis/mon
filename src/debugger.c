#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>

#ifdef __linux__
#include <sys/personality.h>
#include <sys/ptrace.h>
#else
#include <sys/types.h>
#include <sys/ptrace.h>
#define personality(x)
#define ADDR_NO_RANDOMIZE
#define PTRACE_TRACEME PT_TRACE_ME
#define PTRACE_GETREGS PT_GETREGS
#endif

#include "debugger.h"
#include "breakpoint.h"

static void debuggee_entrypoint(debugger_t *debugger) {
  // Disable ASLR (on Linux)
  personality(ADDR_NO_RANDOMIZE);
  // Enable tracing
  ptrace(PTRACE_TRACEME, 0, NULL, 0);
  // Execute the target program
  execl(debugger->program, debugger->program, NULL);
}

static uint64_t debugger_get_pic(debugger_t *debugger) {
  struct user_regs_struct registers;
  ptrace(PTRACE_GETREGS, debugger->pid, NULL, &registers);
  return registers.rip;
}

static void debugger_set_pic(debugger_t *debugger, uint64_t value) {
  struct user_regs_struct registers;
  ptrace(PTRACE_GETREGS, debugger->pid, NULL, &registers);
  registers.rip = value;
  ptrace(PTRACE_SETREGS, debugger->pid, NULL, &registers);
}

static int debugger_entrypoint(debugger_t *debugger) {
  printf("Started program with pid %d\n", debugger->pid);

  // Set a breakpoint for main
  breakpoint_t *breakpoint = breakpoint_create(debugger->pid, 0x4011a4);
  if (breakpoint == NULL) {
    printf("Unable to create breakpoint\n");
    return 1;
  }
  breakpoint_enable(breakpoint);

  while (1) {
    printf("Waiting for signal\n");

    // Wait for a signal from the child
    int status;
    int wpid = waitpid(debugger->pid, &status, WUNTRACED);
    if (wpid < 0) {
      perror("Failed to wait for child");
      return 1;
    }

    if (wpid != debugger->pid) {
      printf("Unexpected process has been reported\n");
      return 1;
    }

    if (!WIFSTOPPED(status)) {
      if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        printf("Program exited normally with code %d\n", exit_status);
        return 0;
      } else if (WIFSIGNALED(status)) {
        int exit_status = WEXITSTATUS(status);
        printf("Program was terminated and exited with code %d\n",
        exit_status); return 0;
      } else {
        printf("Child not stopped\n");
        return 1;
      }
    }

    siginfo_t signal;
    ptrace(PTRACE_GETSIGINFO, debugger->pid, NULL, &signal);

    if (signal.si_signo == SIGTRAP) {
      printf("Child stopped with SIGTRAP\n");
      uint64_t breakpoint_address = debugger_get_pic(debugger);
      printf("Hit breakpoint at 0x%x\n", breakpoint_address);

      if (breakpoint_address == breakpoint->address) {
        printf("Hit specified breakpoint\n");
        // Disable the breakpoint (restoring the overwritten data)
        breakpoint_disable(breakpoint);
        // Move past the breakpoint
        debugger_set_pic(debugger, breakpoint_address - 1);
      }

      ptrace(PTRACE_CONT, debugger->pid, NULL, 0);
    } else if (signal.si_signo == SIGSEGV) {
      printf("Caught signal %d - child segfaulted\n", signal.si_code);
      return 1;
    } else if (signal.si_signo == SIGILL) {
      printf("Caught signal %d - child was on an illegal instruction\n", signal.si_code);
      return 1;
    } else {
      printf("Uncaught signal: %d\n", signal.si_signo);
    }
  }

  free(breakpoint);
}

debugger_t *debugger_create(const char *program) {
  debugger_t *debugger = malloc(sizeof(debugger_t));
  if (debugger == NULL)
    return NULL;

  debugger->pid = 0;
  debugger->program = program;

  return debugger;
}

int debugger_start(debugger_t *debugger) {
  pid_t pid = fork();
  if (pid == 0) {
    debuggee_entrypoint(debugger);
  } else if (pid >= 1) {
    debugger->pid = pid;
    return debugger_entrypoint(debugger);
  }
}
