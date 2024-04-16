#include <sys/wait.h> // Used to wait for child process to finish
#include <stdio.h> // printf and scanf, snprintf
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <time.h>


// pipe is a FIFO queue, if we have 4 bytes and we read 2, we remove the first 2 bytes and the next time we read, we read the other bytes
// read is blocking, it will stop the execution of the program until data is passed through pipe with write

int main(int argc, char const *argv[])
{
  // USER ERROR CHECKING ----------------------------------------------------------
  if (argc != 3) {
    printf("Usage: %s <nChildren> [--random] [--round-robin]\n", argv[0]);
    exit(0);
  }
  if (strcmp(argv[2], "--round-robin") != 0 && strcmp(argv[2], "--random") != 0) {
    printf("Usage: %s <nChildren> [--random] [--round-robin]\n", argv[0]);
    exit(0);
  }
  
  long n = strtol(argv[1], NULL, 10);
  if (n == 0) {
    printf("Usage: %s <nChildren> [--random] [--round-robin]\n", argv[0]);
    exit(0);
  }

  // Checking if the value of 'n' is greater than the value that rand() function can give
  if (n > RAND_MAX) {
    printf("The value of 'n' is too high, enter a value in range(1, %d)", RAND_MAX);
    exit(0);
  }
  // ------------------------------------------------------------------------------

  // round-robin -> flag == 0
  // random -> flag != 0
  int flag = strcmp(argv[2], "--round-robin");



  // One-way communication to target
  // Write to children with this pipe
  int **to_child = malloc(n * sizeof(int*));
  // Write to parent with this pipe
  int **to_parent = malloc(n * sizeof(int*));
  for (long i = 0; i < n; i++) {
    to_child[i] = malloc(2 * sizeof(int));
    to_parent[i] = malloc(2 * sizeof(int));

    if (pipe(to_child[i]) < 0 || pipe(to_parent[i])) {
      perror("'pipe' function failed\n");
      exit(1);
    }
  }

  long child_num;
  pid_t parent_pid = getpid();

  // Creating child processes using fork -------------------------------------------------
  for (long i = 1; i <= n; i++) {
    pid_t child_pid = fork();
    
    // Child process
    if (child_pid == 0) {
      child_num = i;
      break;
    }
    // Parent process
    else if (child_pid > 0) continue;
    // Error
    else {
      perror("'fork' function failed\n");
      exit(1);
    }
  }
  // -------------------------------------------------------------------------------------

  pid_t self_pid = getpid();

  // Parent code
  if (parent_pid == self_pid) {
    // Creating an array of pollfds for n+1 pipes, one pipe will be stdin = 0
    struct pollfd *pollfds = malloc((n+1) * sizeof(struct pollfd));
    // 'stdin' file descriptor is 0 and we poll it together with pipes
    pollfds[0].fd = 0;
    pollfds[0].events = POLLIN;

    for (long i = 1; i <= n; i++) {
      // Close read pipe descriptors of children
      close(to_child[i-1][0]);
      // Close write pipe descriptor of the parent - So that parent can no longer write to itself
      close(to_parent[i-1][1]);

      // Polling the read pipe descriptors of parent
      pollfds[i].fd = to_parent[i-1][0];
      pollfds[i].events = POLLIN;
    }

    long child_turn = 0;
    while(1) {
      if (poll(pollfds, n+1, -1) == -1) {
        perror("'poll' function failed");
        exit(1);
      }

      // If there is an input in 'stdin'
      if (pollfds[0].revents & POLLIN) {
        char input[100];
        // read(0, input, sizeof(input));
        fgets(input, 100, stdin);

        // Searches the newline character '\n' in the 'input', if there is none, clear the buffer
        if (!strchr(input, '\n')) {
          int ch;
          while ((ch = getchar()) != '\n' && ch != EOF);
        }

        char *endptr;
        long num_to_pass = strtol(input, &endptr, 10);

        if (num_to_pass > RAND_MAX) {
          printf("The value entered is out of range of an 'int' type\n");
          continue;
        }

        
        if (strcmp(input, "exit\n") == 0) {
          long exit = LONG_MAX;
          for (long i = 0; i < n; i++) {

            // We map the exit to the highest value of 'long' type
            ssize_t nbytes = write(to_child[i][1], &exit, sizeof(long));
            // printf("Bytes %ld sent to child %ld\n", nbytes, i+1);
          }

          printf("Waiting for all children to terminate\n");
          while(waitpid(-1, NULL, 0) != -1);
          printf("All children terminated\n");
          break;
        }
        // If number was read and it was followed by a newline
        // Zero was read and was followed by newline, which means 'endptr' pointer != input pointer
        else if ((num_to_pass != 0 && *endptr == '\n') || (num_to_pass == 0 && endptr != input)) {
          // If round-robin mode
          if (flag == 0) {
            if (child_turn >= n) child_turn = 0;
            child_turn += 1;

            ssize_t nbytes = write(to_child[child_turn-1][1], &num_to_pass, sizeof(long));
            // printf("%ld bytes sent to child\n", nbytes);
          }
          // random mode
          else {
            child_turn = rand() % n + 1;

            ssize_t nbytes = write(to_child[child_turn - 1][1], &num_to_pass, sizeof(long));
            // printf("%ld bytes sent to child\n", nbytes);
          }

          printf("[Parent] Assigned %ld to child %ld\n", num_to_pass, child_turn);
        }
        else {
          printf("Type a number to send job to a child!\n");
        }
      }

      // Code to check the returned value of children
      for (long i = 1; i <= n; i++) {
        if (pollfds[i].revents & POLLIN) {
          long in;
          ssize_t nbytes = read(to_parent[i-1][0], &in, sizeof(long));
          // printf("%ld bytes read in parent sent from child\n", nbytes);
          printf("%ld returned to parent from child %ld\n", in, i);
        }
      }
    }

    free(pollfds);
  }
  // Child code
  else {
    for (long i = 0; i < n; i++) {
      // Close read pipe descriptor of parent
      close(to_parent[i][0]);
      // Close write pipe descriptor of children - So that child cannot write to itself or other children
      close(to_child[i][1]);
    }

    while (1) {
      long in;
      ssize_t nbytes = read(to_child[child_num - 1][0], &in, sizeof(long));
      // printf("%ld bytes read in child\n", nbytes);

      if (in == LONG_MAX) break;

      printf("[Child] [%ld] Child received %ld!\n", child_num, in);


      sleep(10);

      long out = in - 1;

      write(to_parent[child_num-1][1], &out, sizeof(long));
      printf("[Child] [%ld] Child finished hard work, writing back %ld\n", child_num, out);
    }

  }

  for (long i = 0; i < n; i++) {
    if (self_pid == parent_pid) {
      close(to_child[i][1]);
      close(to_parent[i][0]);
    }
    else {
      close(to_parent[i][1]);
      close(to_child[i][0]);
    }

    free(to_child[i]);
    free(to_parent[i]);
  }

  free(to_child);
  free(to_parent);

  return 0;
}
