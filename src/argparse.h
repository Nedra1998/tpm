#ifndef ARGPARSE_H_VL3W9CDY
#define ARGPARSE_H_VL3W9CDY

typedef struct Args {
  char *output;
  bool quiet, verbose;
  char *color;
} Args;

int parse_args(int argc, char *argv[], struct Args *args);

#endif /* end of include guard: ARGPARSE_H_VL3W9CDY */
