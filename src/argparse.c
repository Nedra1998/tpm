#include "argparse.h"

#include <argp.h>
#include <stdbool.h>

#include "version.h"

const char *argp_program_version = TPM_VERSION_STRING;
static char doc[] = "tpm -- Tiny Path Marcher\n\n"
"A GPU accelerated path marcher written entierly in C.";

static char args_doc[] = "";

static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "run verbosely"},
    {"quiet", 'q', 0, 0, "suppress all output"},
    {"output", 'o', "FILE", 0, "override output filename"},
    {"input", 'i', "FILE...", 0, "opencl SDF file"},
    {"color", 'c', "COLOR", OPTION_ARG_OPTIONAL, "use color in the output"},
    {0}};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct Args *arguments = state->input;
  switch (key) {
  case 'q':
    arguments->quiet = true;
    break;
  case 'v':
    arguments->verbose = true;
    break;
  case 'o':
    arguments->output = arg;
    break;
  case 'c':
    if (arg != NULL)
      arguments->color = arg;
    else
      arguments->color = "always";
    break;
  case ARGP_KEY_ARG:
    argp_usage(state);
    break;
  case ARGP_KEY_END:
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, 0, doc};

int parse_args(int argc, char *argv[], struct Args *args) {
  args->output = "output.png";
  args->quiet = false;
  args->verbose = false;
  args->color = "auto";
  return argp_parse(&argp, argc, argv, 0, 0, args);
}
