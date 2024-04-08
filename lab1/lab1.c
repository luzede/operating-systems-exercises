#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


int main(int argc, char const *argv[])
{
  // USER ERROR CHECKING
  // -----------------------------------------------------
  if (argc == 1) {
    printf("Usage: ./a.out filename\n");
    return 1;
  }
  if (strcmp(argv[1], "--help") == 0) {
    printf("Usage: ./a.out filename\n");
    return 0;
  }
  if (argc > 2) {
    printf("Usage: ./a.out filename\n");
    return 1;
  }

  struct stat file_stat;
  if (stat(argv[1], &file_stat) == 0) {
    printf("Error: %s already exists\n", argv[1]);
    return 1;
  }
  // ------------------------------------------------------

  char buf[100];
  int status;
  pid_t child = fork();

  pid_t parent_pid = getppid();
  pid_t self_pid = getpid();

  int fd = open(argv[1], O_CREAT | O_APPEND | O_WRONLY, 0622);
  if (fd == -1) {
    perror("open function failed");
    return 1;
  }

  if (child < 0) {
    perror("fork function failed");
    return 1;
  }
  if (child > 0) {
    // Parent's code
    wait(&status);
    snprintf(buf, 100, "[PARENT] getpid()= %d, getppid()=%d\n", self_pid, parent_pid);
  }
  else {
    // Child's code
    snprintf(buf, 100, "[CHILD] getpid()= %d, getppid()=%d\n", self_pid, parent_pid);
  }


  if (write(fd, buf, strlen(buf)) < strlen(buf)) {
      perror("write function failed");
      return 1;
  }

  close(fd);

  exit(0);
}
