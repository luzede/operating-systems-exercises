#define _GNU_SOURCE // This is necessary to fix intellisense errors on struct sigaction
#include <sys/wait.h> // Used to wait for child process to finish
#include <stdio.h> // printf and scanf, snprintf
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>


// This is used to communicate between process and a handler
// Handler gives values to sig_type and the process gets the sig_type value or child_pid that send the signal
volatile sig_atomic_t sig_type;
volatile sig_atomic_t child_pid;

// This needs to be included before signal so, as the features.h contains
// #define _POST_C_SOURCE with latest value for compatibility purpose.
#include <features.h>
#include <signal.h>

size_t indexOf(pid_t p, pid_t *arr, size_t size);
void signal_handler(int signum, siginfo_t *info, void *ptr) {
  switch(signum) {
    case SIGUSR1:
      sig_type = SIGUSR1;
      break;
    case SIGUSR2:
      sig_type = SIGUSR2;
      break;
    case SIGALRM:
      sig_type = SIGALRM;
      break;
    case SIGCHLD:
      child_pid = info->si_pid;
      sig_type = info->si_code;
      break;
    case SIGTERM:
      sig_type = SIGTERM;
      break;

  }
}


int main(int argc, char const *argv[])
{
  // ERROR CHECKING
  if (argc < 2 || argc > 2) {
    printf("Error: invalid user input\n");
    return 1;
  };

  size_t number_of_gates = strlen(argv[1]);
  pid_t parent = getpid();
  pid_t children[number_of_gates];
  char gates[number_of_gates]; 

  for (size_t i = 0; i < number_of_gates; i++) {
    gates[i] = argv[1][i];
    children[i] = fork();

    // If it is a parent process, continue creating children
    if (children[i] > 0) {
      printf("[PARENT/PID=%d] Created child for gate %lu (PID=%d) and initial state '%c'\n", parent, i, children[i], gates[i]);
      continue;
    }

    // If it is a child process, replace the forked process to a new process with the same pid
    // If you call execv in the child process, it will replace the copy of the parent's program with the new program.
    else if (children[i] == 0) {
      char i_str[20];
      char gate_str[2];
      snprintf(gate_str, sizeof(gate_str), "%c", gates[i]);
      snprintf(i_str, sizeof(i_str), "%lu", i);
      char *args[] = {"/home/luzede/Documents/Projects/C/lab2/child", gate_str, i_str, NULL};
      int exec_status = execv(args[0], args);
      if (exec_status == -1) {
        perror("Failed to run 'execv' with a child program");
        exit(1);
      }
    }
    else {
      perror("Failed to fork the parent program");
      exit(1);
    }
  }

  // Defining sigaction
  struct sigaction sa;
  sa.sa_sigaction = signal_handler;
  sa.sa_flags = SA_RESTART | SA_SIGINFO;
  sigfillset(&sa.sa_mask);

  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
    perror("Error: cannot handle SIGUSR1");
    exit(1);
  }
  if (sigaction(SIGUSR2, &sa, NULL) == -1) {
    perror("Error: cannot handle SIGUSR2");
    exit(1);
  }
  if (sigaction(SIGALRM, &sa, NULL) == -1) {
    perror("Error: cannot handle SIGALRM");
    exit(1);
  }
  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    perror("Error: cannot handle SIGTERM");
    exit(1);
  }
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("Error: cannot handle SIGCHLD");
    exit(1);
  }

  
  // int status;
  // while(1) {
  //   // 0 in options means block the calling process until a child has exited (Not stopped)
  //   // WUNTRACED flag makes it return also when the child process is stopped
  //   pid_t child = waitpid(-1, &status, WUNTRACED);
  //   size_t c_index = indexOf(child, children, number_of_gates);
  //   char c_index_str[20];
  //   char gate_str[2];
  //   snprintf(gate_str, sizeof(gate_str), "%c", gates[c_index]);
  //   snprintf(c_index_str, sizeof(c_index_str), "%lu", c_index);

    
  //   printf("[PARENT/PID=%d] Child %lu with PID=%d exited", parent, c_index, child);
    

  //   // If child process terminated normally or by a signal
  //   if (WIFEXITED(status) || WIFSIGNALED(status)) {
  //     child = fork();
  //     // Replace the copy of the parent's program with the new program from 'child' executable
  //     if (child == 0) {
  //       char *args[] = {"/home/luzede/Documents/Projects/C/lab2/child", gate_str, c_index_str, NULL};
  //       int exec_status = execv(args[0], args);
  //       if (exec_status == -1) {
  //         perror("Failed to restart a terminated child process using 'execv'");
  //         exit(1);
  //       }
  //     }
  //     else if (child < 0) {
  //       perror("Failed to fork the parent program");
  //       exit(1);
  //     }
  //     else {
  //       children[c_index] = child;
  //       printf("[PARENT/PID=%d] Created child for gate %lu (PID=%d) and initial state '%c'\n", parent, c_index, children[c_index], gates[c_index]);
  //     }
  //   }
  //   else if (WIFSTOPPED(status)) {
  //     // Send a SIGCONT signal to child that is stopped
  //     if (kill(child, SIGCONT) == -1) {
  //       perror("Failed to send SIGCONT to child process");
  //       exit(1);
  //     }
  //   }
  //   else {
  //     perror("It did not terminate normally or by a signal");
  //     exit(1);
  //   }
  // }


  while(1) {
    alarm(15);
    pause();

    // Check if a process was killed or exited, SIGCHLD is signal number and CLD_EXITED and CLD_KILLED are signal codes
    if (sig_type == CLD_EXITED || sig_type == CLD_KILLED) {
      size_t c_index = indexOf(child_pid, children, number_of_gates);
      printf("[PARENT/PID=%d] Child %lu with PID=%d exited\n", parent, c_index, child_pid);


      pid_t new_child = fork();

      // If it is a child process, replace the forked process to a new process with the same pid
      // If you call execv in the child process, it will replace the copy of the parent's program with the new program.
      if (new_child == 0) {
        char c_index_str[20];
        char gate_str[2];
        snprintf(gate_str, sizeof(gate_str), "%c", gates[c_index]);
        snprintf(c_index_str, sizeof(c_index_str), "%lu", c_index);
        char *args[] = {"/home/luzede/Documents/Projects/C/lab2/child", gate_str, c_index_str, NULL};
        int exec_status = execv(args[0], args);
        if (exec_status == -1) {
          perror("Failed to restart a terminated child process using 'execv'");
          exit(1);
        }
      }
      else if (new_child < 0) {
        perror("Failed to fork the parent program");
        exit(1);
      }
      else {
        children[c_index] = new_child;
        printf("[PARENT/PID=%d] Created child for gate %lu (PID=%d) and initial state '%c'\n", parent, c_index, children[c_index], gates[c_index]);
      }
    }
    // If it is stopped, continue the child process through sending a SIGCONT signal
    else if (sig_type == CLD_STOPPED) {
      kill(child_pid, SIGCONT);
    }
    // If any of these signals are received, send them to all children
    else if (sig_type == SIGUSR1 || sig_type == SIGUSR2 || sig_type == SIGALRM) {
      for (size_t i = 0; i < number_of_gates; i++) {
        kill(children[i], sig_type);
      }
    }
    else if (sig_type == SIGTERM) {
      // Send SIGTERM to all the children
      for (size_t i = 0; i < number_of_gates; i++) {
        kill(children[i], SIGTERM);
      }
      
      // Wait for children to exit
      while(1) {
        printf("[PARENT/PID=%d] Waiting for %lu children to exit\n", parent, number_of_gates--);
        pid_t exited_child = waitpid(-1, NULL, 0);
        if (exited_child == -1) {
          printf("[PARENT/PID=%d] All children exited, terminating as well\n", parent);
          break;
        }
        printf("[PARENT/PID=%d] Child with PID=%d terminated successfully with exit status code 0!\n", parent, exited_child);
      }
      break;
    }
  }

  exit(0);
}




size_t indexOf(pid_t p, pid_t *arr, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (p == arr[i]) return i;
  }
  return -1;
}



