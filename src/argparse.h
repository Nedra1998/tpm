#ifndef ARGPARSE_H_VL3W9CDY
#define ARGPARSE_H_VL3W9CDY

#include <stdbool.h>

typedef struct Args {
  const char *output;
  bool quiet, verbose;
  const char *color;
  const char *input;
} Args;

bool argparse_exists(const char* arg);
bool argparse_isfile(const char *arg);
bool argparse_isdir(const char* arg);

bool argparse_choice(const char *arg, const char **choices, int n_choices);
bool argparse_choice_ext(const char *arg, const char **extensions,
                        int n_extensions);
int parse_args(int argc, char *argv[], struct Args *args);

#endif /* end of include guard: ARGPARSE_H_VL3W9CDY */
