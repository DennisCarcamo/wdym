#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  int child;
  size_t bufsize = 32;
  size_t read;
  char *buffer = (char *) malloc(bufsize);

  while (1) {
    printf("> ");
    read = getline(&buffer, &bufsize, stdin);
    buffer[read - 1] = 0;

    if (strcmp(buffer, "exit") == 0) {
      break;
    }
  }

  return 0;
}
