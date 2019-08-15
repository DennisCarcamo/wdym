#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **split(char *);

const char *supported[] = {
  "ls",
  "ping",
  "cat",
  "help",
  "top"
};

const char *teclado[] = {
  "QWERTYUIOP",
  "ASDFGHJKL ",
  "ZXCVBNM   "
};

// TODO:
// check if its a supported command
// if it is, execute
// check if isnt, check in the database
// if it isnt in the db, check if he meant something else
// if he did mean something else, ask to save to db
// save to database

int main() {
  int child;
  size_t bufsize = 128;
  size_t read;
  char *buffer = (char *) malloc(bufsize);

  while (1) {
    printf("> ");
    read = getline(&buffer, &bufsize, stdin);
    buffer[read - 1] = 0;

    if (strcmp(buffer, "exit") == 0) {
      break;
    }

    char **vector = split(buffer);

    child = fork();
    if (child == 0) {
      execvp(vector[0], vector);

      // si llega a este punto hubo un error
      printf("error: comando \"%s\" no se pudo ejecutar.\n", buffer);
      exit(1);
    } else {
      wait(&child);
      free(vector);
    }
  }

  free(buffer);
  return 0;
}

char **split(char *input) {
  // tomar una cadena y hacerle un split por cada espacio:
  // "ls -l" => ["ls", "-l"]

  int allocated = 4;
  char **res = (char **) malloc(sizeof(*res) * allocated);

  int size = 0;
  char *token = strtok(input, " \t");
  while (token != NULL) {
    if (size >= allocated) {
      allocated *= 2;
      res = realloc(res, sizeof(*res) * allocated);
    }

    res[size] = token;
    size += 1;
    token = strtok(NULL, " \t");
  }

  if (allocated > size) {
    res[size] = 0;
  }

  return res;
}
