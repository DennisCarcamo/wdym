#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typedef struct thing_t {
  char searching[15];
  int found;
  char meant[15];
} thing_t;

typedef struct meaning_t {
  char said[15];
  char meant[15];
} meaning_t;

char **split(char *);
int supported(char *);
void execute(char **);
thing_t *search(sqlite3 *, char *);
int did_you_mean(char *);
int cb_default(void *, int, char **, char **);
int cb_check_exists(void *, int, char **, char **);

const int supp_size = 5;
const char *supp_commands[] = {
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
// if he did mean something else, ask to save to db
// save to database

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "parametros: ./<ejecutable> <base de datos sqlite>\n");
    fprintf(stderr, "parametros: ./main.exe ../wdym.db\n");
    return 1;
  }

  size_t bufsize = 128;
  size_t read;
  char *buffer = (char *) malloc(bufsize);

  sqlite3 *db;
  if (sqlite3_open(argv[1], &db)) {
    fprintf(stderr, "Error al abrir db: %s\n", sqlite3_errmsg(db));
    return 1;
  }

  while (1) {
    printf("> ");
    read = getline(&buffer, &bufsize, stdin);
    buffer[read - 1] = 0;

    if (strcmp(buffer, "exit") == 0) {
      break;
    }

    char **vector = split(buffer);

    if (!supported(vector[0])) {
      thing_t *thing = search(db, vector[0]);
      if (thing->found) {
        // se encontro la correccion en la db
        memcpy(vector[0], thing->meant, strlen(thing->meant));
      } else if (did_you_mean(vector[0])) {
      }

      free(thing);
    }

    // exec
    execute(vector);
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

int supported(char *command) {
  // revisar si el comando que envio esta en la lista
  for (int i = 0; i < supp_size; i += 1) {
    if (strcmp(command, supp_commands[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

void execute(char **vector) {
  int child = fork();
  if (child == 0) {
    execvp(vector[0], vector);

    // si llega a este punto hubo un error
    fprintf(stderr, "error: comando \"%s\" no se pudo ejecutar.\n", vector[0]);
    exit(1);
  } else {
    wait(&child);
    free(vector);
  }
}

thing_t *search(sqlite3 *db, char *command) {
  thing_t *thing = (thing_t *) malloc(sizeof(*thing));
  memcpy(thing->searching, command, strlen(command));

  char *error;
  char *sql = "select * from meanings";

  int code = sqlite3_exec(db, sql, cb_check_exists, &thing, &error);
  if (code != SQLITE_OK) {
    fprintf(stderr, "Error SQL: %s\n", error);
    sqlite3_free(error);
  }

  return thing;
}

int cb_default(void *passed, int count, char **data, char **columns) {
  // select * from <tabla>;
  for(int i = 0; i < count; i += 1) {
    printf("%s = %s\n", columns[i], data[i] ? data[i] : "NULL");
  }

  printf("\n");
  return 0;
}

int did_you_mean(char *command) {
  char *maybe_meant;
  int similarity = 0;
  int diff_pos;

  int len_cmd = strlen(command);

  for (int i = 0; i < supp_size; i += 1) {
    // mismo tamanio
    if (len_cmd == strlen(supp_commands[i])) {
      // mismos caracteres excepto uno
      int diferentes = 0;

      for (int j = 0; j < len_cmd; j += 1) {
        if (command[j] != supp_commands[i][j]) {
          diferentes += 1;
          diff_pos = j;
        }
      }

      if (diferentes == 1) {
        // es posible que typeo mal el comando
        memcpy(maybe_meant, supp_commands[i], strlen(supp_commands[i]));
        similarity = 1;
        break;
      }
    }
  }

  if (similarity) {
    // revisar en el teclado si typeo mal
    char typo = command[diff_pos];
    char correcto = maybe_meant[diff_pos];

    // buscar el typo en el teclado y ver si la tecla correcta esta alrededor
    for (int fila = 0; fila < 3; fila += 1) {
      for (int columna = 0; columna < 10; columna += 1) {
      }
    }
  }

  return 0;
}

int cb_check_exists(void *passed, int count, char **data, char **columns) {
  // revisar la db, viendo si ya se ha hecho una correccion con el comando
  meaning_t meaning;
  for(int i = 0; i < count; i += 1) {
    if (strcmp(columns[i], "said") == 0) {
      memcpy(meaning.said, data[i], strlen(data[i]));
    } else {
      memcpy(meaning.meant, data[i], strlen(data[i]));
    }
  }

  thing_t *thing = (thing_t *) passed;
  if (strcmp(thing->searching, meaning.said) == 0) {
    // se ha hecho una correccion anteriormente
    thing->found = 1;
    memcpy(thing->meant, meaning.meant, strlen(meaning.meant));
  }

  printf("\n");
  return 0;
}
