#include <argp.h>
#include <stdbool.h>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <windows.h>
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
#include <unistd.h>
#endif

#include "argparse.h"
#include "log.h"
#include "sysinfo.h"

int main(int argc, char *argv[]) {
  Args args;
  if (parse_args(argc, argv, &args) != 0) {
    return -1;
  }

  FILE *log_file = NULL;
  if (!args.quiet) {
    bool should_use_color = false;
    if (strncmp(args.color, "auto", 4) == 0) {
#if defined(_WIN32) || defined(_WIN64)
      should_use_color = _isatty(_fileno(stdout));
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
      should_use_color = isatty(fileno(stdout)) != 0;
#endif
    } else if (strncmp(args.color, "always", 6) == 0) {
      should_use_color = true;
    }
    bool ret = true;
    if (args.verbose) {
      ret &= log_add_sink((LogSink){stdout, LOG_TRACE, should_use_color, NULL});
    } else {
      ret &= log_add_sink((LogSink){stdout, LOG_INFO, should_use_color, NULL});
    }
    log_file = fopen("tpm.log", "a");
    if (log_file == NULL) {
      ret = false;
    } else {
      log_add_sink((LogSink){log_file, LOG_TRACE, false, NULL});
    }
    if (ret == false) {
      return -1;
    }
  }

  log_sysinfo();

  if (log_file != NULL)
    fclose(log_file);
  return 0;
}
