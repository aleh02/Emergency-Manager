#ifndef PARSE_ENV_H
#define PARSE_ENV_H

// Parsing del file di configurazione dell'ambiente. return -1 se errore.
// Alloca dinamicamente queue_out con strdup (da liberare nel main).
int parse_env(char *filename, char **queue_out, int *x_out, int *y_out);

#endif
