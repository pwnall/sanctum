#include "gtest/gtest.h"

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "bare/phys_ptr.h"

void signal_handler(int sig) {
  void *array[16];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 16);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

// This is the entry point for the CLI executable that runs all the monitor's
// unit tests.
int main(int argc, char** argv) {
  signal(SIGSEGV, signal_handler);

  sanctum::testing::init_phys_buffer(8192);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
