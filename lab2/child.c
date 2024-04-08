#define _GNU_SOURCE // This is necessary to fix intellisense errors on struct sigaction
#include <sys/wait.h> // Used to wait for child process to finish
#include <stdio.h> // printf and scanf, snprintf
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>

// This needs to be included before signal so, as the features.h contains
// #define _POST_C_SOURCE with latest value for compatibility purpose.
#include <features.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


// 0 - SIGUSR1/SIGALRM
// 1 - SIGUSR2
// 2 - SIGTERM
volatile sig_atomic_t sig_type = 0;

void handler(int signum) {
  switch(signum) {
    case SIGALRM:
    case SIGUSR1:
      sig_type = 0;
      break;
    case SIGUSR2:
      sig_type = 1;
      break;
    case SIGTERM:
      sig_type = 2;
      break;
  }
}

int main(int argc, char const *argv[])
{
  // total number of seconds elapsed since the Epoch (00:00:00 UTC, January 1, 1970)
  time_t begin = time(NULL);
  time_t end;

  int gate_open = 0;
  if (*argv[1] == 't') gate_open = 1;

  
  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = SA_RESTART;
  // It specifies the signals to be blocked during the execution of the signal handler
  sigfillset(&sa.sa_mask);

  sigaction(SIGUSR1, &sa, NULL);
  sigaction(SIGUSR2, &sa, NULL);
  sigaction(SIGALRM, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  
  while(1) {
    if (sig_type == 1) {
      // XOR operation
      gate_open = gate_open ^ 1;
    }
    if (sig_type == 2) exit(0);


    end = time(NULL);
    if (gate_open) {
      printf("[ID=%s]/PID=%d/TIME=%lds] The gates are open!\n", argv[2], getpid(), end - begin);
    }
    else {
      printf("[ID=%s]/PID=%d/TIME=%lds] The gates are closed!\n", argv[2], getpid(), end - begin);
    }
    pause();
  }
  
  exit(0);
}
