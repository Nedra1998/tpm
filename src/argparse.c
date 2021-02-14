#include "argparse.h"

#include <argp.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "stb.h"
#include "version.h"

const char *argp_program_version = TPM_VERSION_STRING;
const char *argp_program_bug_address =
    "<https://github.com/Nedra1998/tpm/issues>";
// Add a '\v' to add postfix comments
static char doc[] =
    "Tiny Path Marching physically based renderer\v"
    "A GPU accelerated path marcher written entirely in C. tpm "
    "has been constructed to have no dependencies other than "
    "OpenCL for GPGPU computations. It focuses on implementing ray marching "
    "algorithms to provide novel rendering opportunities, with complex user "
    "defined signed distance functions.";

static char args_doc[] = "INPUT";

static struct argp_option options[] = {
    {"output", 'o', "FILE", 0, "override output filename"},
    {0, 0, 0, 0, "Logging:", 1},
    {"verbose", 'v', 0, 0, "run verbosely"},
    {"quiet", 'q', 0, 0, "suppress all output"},
    {"color", 'c', "COLOR", OPTION_ARG_OPTIONAL, "use color in the output"},
    {0}};

static const char *color_choices[] = {"always", "never", "auto"};
static int n_color_choices = sizeof(color_choices) / sizeof(color_choices[0]);
static const char *input_extensions[] = {".cl"};
static int n_input_extensions =
    sizeof(input_extensions) / sizeof(input_extensions[0]);
static const char *output_extensions[] = {
    ".bmp", ".dip", ".hdr", ".icb", ".jfi", ".jfif", ".jif", ".jpe", ".jpeg",
    ".jpg", ".pbm", ".pgm", ".png", ".pnm", ".ppm",  ".tga", ".vda", ".vst",
};
static int n_output_extensions =
    sizeof(output_extensions) / sizeof(output_extensions[0]);

bool filename_validator(const char *arg, const char **extensions,
                        int n_extensions) {
  char ext[8];
  stb_splitpath(ext, (char *)arg, STB_EXT);
  for (int i = 0; i < n_extensions; ++i) {
    if (strncmp(ext, extensions[i], 8) == 0)
      return true;
  }
  return false;
}
bool file_validator(const char *arg) {
  if (access(arg, F_OK) != 0) {
    return false;
  }
  return true;
}
bool choice_validator(const char *arg, const char **choices, int n_choices) {
  for (int i = 0; i < n_choices; ++i) {
    if (strncmp(arg, choices[i], strlen(choices[i])) == 0)
      return true;
  }
  return false;
}

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
    if (!filename_validator(arg, output_extensions, n_output_extensions))
      argp_failure(state, 1, 22,
                   "invalid file extension '%s' (must be one of"
                   "'.bmp', '.dip', '.hdr', '.icb', '.jfi', '.jfif', '.jif', "
                   "'.jpe', '.jpeg', '.jpg', '.pbm', '.pgm', '.png', '.pnm', "
                   "'.ppm', '.tga', '.vda', '.vst')",
                   arg);
    else
      arguments->output = arg;
    break;
  case 'c':
    if (arg == NULL)
      arguments->color = "always";
    else if (!choice_validator(arg, color_choices, n_color_choices))
      argp_failure(state, 1, 22, "%s", arg);
    else
      arguments->color = arg;
    break;
  case ARGP_KEY_ARG:
    if (state->arg_num >= 1)
      argp_usage(state);
    else if (!file_validator(arg))
      argp_failure(state, 1, 2, "%s", arg);
    else if (!filename_validator(arg, input_extensions, n_input_extensions))
      argp_failure(state, 1, 22, "%s", arg);
    else
      arguments->input = arg;
    break;
  case ARGP_KEY_END:
    if (state->arg_num < 1)
      argp_usage(state);
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int parse_args(int argc, char *argv[], struct Args *args) {
  args->output = "output.png";
  args->quiet = false;
  args->verbose = false;
  args->color = "auto";
  return argp_parse(&argp, argc, argv, 0, 0, args);
}
