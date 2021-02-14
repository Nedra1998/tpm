#ifndef ARGPARSE_H_VL3W9CDY
#define ARGPARSE_H_VL3W9CDY

#include <stdbool.h>

typedef struct Args {
  const char *output;
  bool quiet, verbose;
  const char *color;
  const char *input;
} Args;

bool filename_validator(const char *arg, const char **extensions,
                        int n_extensions);
bool file_validator(const char *arg);
bool choice_validator(const char *arg, const char **choices, int n_choices);
int parse_args(int argc, char *argv[], struct Args *args);

#endif /* end of include guard: ARGPARSE_H_VL3W9CDY */
