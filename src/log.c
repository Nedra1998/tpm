#include "log.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "stb.h"
#include "stb_sprintf.h"

static LogSink sinks[LOG_MAX_SINKS];
static int sink_index = 0;

bool log_add_sink(LogSink sink) {
  if (sink_index == LOG_MAX_SINKS)
    return false;
  sinks[sink_index++] = sink;
  return true;
}

void log_vlog(enum LogLevel level, const char *file, int line, const char *fmt,
              va_list args) {
  char payload[LOG_MAX_PAYLOAD_LENGTH];
  stbsp_vsnprintf(payload, LOG_MAX_PAYLOAD_LENGTH, fmt, args);

  LogEvent ev = {level, file, line, NULL, payload};

  time_t t = time(NULL);
  ev.time = localtime(&t);

  for (int i = 0; i < sink_index; ++i) {
    if (ev.level >= sinks[i].level)
      log_print(sinks[i].fifo, sinks[i].fmt, sinks[i].color, &ev);
  }
}

void log_log(enum LogLevel level, const char *file, int line, const char *fmt,
             ...) {
  va_list va;
  va_start(va, fmt);
  log_vlog(level, file, line, fmt, va);
  va_end(va);
}

// %t -> HH:MM:SS
// %T -> YYYY-MM-DD HH:mm:ss
// %v -> <Payload>
// %l -> trace, debug, info, warning, error, critical
// %L -> TRACE, DEBUG, INFO, WARNING, ERROR, CRITICAL
// %f -> my_file.c
// %F -> /usr/bin/my_file.c
// %# -> 123
// %% -> %
// %^ -> start color range
// %$ -> end color range

static const char default_fmt[] = "[%T] %^%L%$ %f:%#: %v";
static const char *level_lower[] = {"trace   ", "debug   ", "info    ",
                                    "warning ", "error   ", "critical"};
static const char *level_upper[] = {"TRACE   ", "DEBUG   ", "INFO    ",
                                    "WARNING ", "ERROR   ", "CRITICAL"};
const char *log_level_color[] = {"\x1b[37m",   "\x1b[36m",   "\x1b[32m",
                                 "\x1b[1;33m", "\x1b[1;31m", "\x1b[1;41m"};
const char *log_field_color[] = {"\x1b[34m", NULL, NULL, "\x1b[35m",
                                 "\x1b[33m"};
static const char *reset = "\x1b[0m";

void log_print(FILE *fifo, const char *fmt, bool colored, const LogEvent *ev) {
  if (fmt == NULL)
    fmt = default_fmt;
  char buffer[LOG_MSG_LENGTH];
  int fmt_len = strlen(fmt);
  int idx = 0;
  bool flag = false;
  for (int i = 0; i < fmt_len && idx < LOG_MSG_LENGTH; ++i) {
    switch (fmt[i]) {
    case '%':
      if (!flag) {
        flag = true;
        break;
      }
    case 't':
      if (flag) {
        if (colored && log_field_color[LOG_TIME] != NULL) {
          strncpy(buffer + idx, log_field_color[LOG_TIME],
                  LOG_MSG_LENGTH - idx);
          idx += strlen(log_field_color[LOG_TIME]);
        }
        idx += strftime(buffer + idx, LOG_MSG_LENGTH - idx, "%T", ev->time);
        if (colored && log_field_color[LOG_TIME] != NULL) {
          strncpy(buffer + idx, reset, LOG_MSG_LENGTH - idx);
          idx += strlen(reset);
        }
        flag = false;
        break;
      }
    case 'T':
      if (flag) {
        if (colored && log_field_color[LOG_TIME] != NULL) {
          strncpy(buffer + idx, log_field_color[LOG_TIME],
                  LOG_MSG_LENGTH - idx);
          idx += strlen(log_field_color[LOG_TIME]);
        }
        idx += strftime(buffer + idx, LOG_MSG_LENGTH - idx, "%F %T", ev->time);
        if (colored && log_field_color[LOG_TIME] != NULL) {
          strncpy(buffer + idx, reset, LOG_MSG_LENGTH - idx);
          idx += strlen(reset);
        }
        flag = false;
        break;
      }
    case 'v':
      if (flag) {
        if (colored && log_field_color[LOG_PAYLOAD] != NULL) {
          strncpy(buffer + idx, log_field_color[LOG_PAYLOAD],
                  LOG_MSG_LENGTH - idx);
          idx += strlen(log_field_color[LOG_PAYLOAD]);
        }
        strncpy(buffer + idx, ev->payload, LOG_MSG_LENGTH - idx);
        idx += strlen(ev->payload);
        if (colored && log_field_color[LOG_PAYLOAD] != NULL) {
          strncpy(buffer + idx, reset, LOG_MSG_LENGTH - idx);
          idx += strlen(reset);
        }
        flag = false;
        break;
      }
    case 'l':
      if (flag) {
        if (colored && log_field_color[LOG_LEVEL] != NULL) {
          strncpy(buffer + idx, log_field_color[LOG_LEVEL],
                  LOG_MSG_LENGTH - idx);
          idx += strlen(log_field_color[LOG_LEVEL]);
        }
        strncpy(buffer + idx, level_lower[ev->level], LOG_MSG_LENGTH - idx);
        idx += 8;
        if (colored && log_field_color[LOG_LEVEL] != NULL) {
          strncpy(buffer + idx, reset, LOG_MSG_LENGTH - idx);
          idx += strlen(reset);
        }
        flag = false;
        break;
      }
    case 'L':
      if (flag) {
        if (colored && log_field_color[LOG_LEVEL] != NULL) {
          strncpy(buffer + idx, log_field_color[LOG_LEVEL],
                  LOG_MSG_LENGTH - idx);
          idx += strlen(log_field_color[LOG_LEVEL]);
        }
        strncpy(buffer + idx, level_upper[ev->level], LOG_MSG_LENGTH - idx);
        idx += 8;
        if (colored && log_field_color[LOG_LEVEL] != NULL) {
          strncpy(buffer + idx, reset, LOG_MSG_LENGTH - idx);
          idx += strlen(reset);
        }
        flag = false;
        break;
      }
    case 'f':
      if (flag) {
        char fname[LOG_FNAME_LENGTH];
        stb_splitpath(fname, (char *)ev->file, STB_FILE_EXT);
        if (colored && log_field_color[LOG_FILE] != NULL) {
          strncpy(buffer + idx, log_field_color[LOG_FILE],
                  LOG_MSG_LENGTH - idx);
          idx += strlen(log_field_color[LOG_FILE]);
        }
        strncpy(buffer + idx, fname, LOG_MSG_LENGTH - idx);
        idx += strlen(fname);
        if (colored && log_field_color[LOG_FILE] != NULL) {
          strncpy(buffer + idx, reset, LOG_MSG_LENGTH - idx);
          idx += strlen(reset);
        }
        flag = false;
        break;
      }
    case 'F':
      if (flag) {
        if (colored && log_field_color[LOG_FILE] != NULL) {
          strncpy(buffer + idx, log_field_color[LOG_FILE],
                  LOG_MSG_LENGTH - idx);
          idx += strlen(log_field_color[LOG_FILE]);
        }
        strncpy(buffer + idx, ev->file, LOG_MSG_LENGTH - idx);
        idx += strlen(ev->file);
        if (colored && log_field_color[LOG_FILE] != NULL) {
          strncpy(buffer + idx, reset, LOG_MSG_LENGTH - idx);
          idx += strlen(reset);
        }
        flag = false;
        break;
      }
    case '#':
      if (flag) {
        if (colored && log_field_color[LOG_LINE] != NULL) {
          strncpy(buffer + idx, log_field_color[LOG_LINE],
                  LOG_MSG_LENGTH - idx);
          idx += strlen(log_field_color[LOG_LINE]);
        }
        idx +=
            stbsp_snprintf(buffer + idx, LOG_MSG_LENGTH - idx, "%d", ev->line);
        if (colored && log_field_color[LOG_LINE] != NULL) {
          strncpy(buffer + idx, reset, LOG_MSG_LENGTH - idx);
          idx += strlen(reset);
        }
        flag = false;
        break;
      }
    case '^':
      if (flag) {
        if (colored) {
          strncpy(buffer + idx, log_level_color[ev->level],
                  LOG_MSG_LENGTH - idx);
          idx += strlen(log_level_color[ev->level]);
        }
        flag = false;
        break;
      }
    case '$':
      if (flag) {
        if (colored) {
          strncpy(buffer + idx, reset, LOG_MSG_LENGTH - idx);
          idx += strlen(reset);
        }
        flag = false;
        break;
      }
    default:
      buffer[idx++] = fmt[i];
    }
  }
  buffer[idx] = 0;
  fprintf(fifo, "%s\n", buffer);
}
