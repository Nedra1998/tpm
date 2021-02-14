#ifndef LOG_H_NLITVEQU
#define LOG_H_NLITVEQU

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef LOG_MAX_SINKS
#define LOG_MAX_SINKS 8
#endif

#ifndef LOG_MAX_PAYLOAD_LENGTH
#define LOG_MAX_PAYLOAD_LENGTH 256
#endif

#ifndef LOG_FNAME_LENGTH
#define LOG_FNAME_LENGTH 64
#endif

#ifndef LOG_MSG_LENGTH
#define LOG_MSG_LENGTH 512
#endif

#define ltrace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define ldebug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define linfo(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define lwarn(...) log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define lerr(...) log_log(LOG_ERR, __FILE__, __LINE__, __VA_ARGS__)
#define lcrit(...) log_log(LOG_CRIT, __FILE__, __LINE__, __VA_ARGS__)

enum LogLevel {
  LOG_TRACE = 0,
  LOG_DEBUG = 1,
  LOG_INFO = 2,
  LOG_WARN = 3,
  LOG_ERR = 4,
  LOG_CRIT = 5,
  LOG_NLEVELS
};
enum LogFields {
  LOG_TIME = 0,
  LOG_PAYLOAD = 1,
  LOG_LEVEL = 2,
  LOG_FILE = 3,
  LOG_LINE = 4,
  LOG_NFIELDS
};

extern const char *log_level_color[LOG_NLEVELS];
extern const char *log_field_color[LOG_NFIELDS];

typedef struct LogEvent {
  enum LogLevel level;
  const char *file;
  int line;
  struct tm *time;
  char *payload;
} LogEvent;

typedef struct LogSink {
  FILE *fifo;
  enum LogLevel level;
  bool color;
  const char *fmt;
} LogSink;
// typedef void (*LogSink)(const LogEvent *);

bool log_add_sink(LogSink sink);
static inline bool log_add_stdout_sink() {
  return log_add_sink((LogSink){stdout, LOG_TRACE, false, NULL});
}
static inline bool log_add_stderr_sink() {
  return log_add_sink((LogSink){stderr, LOG_TRACE, false, NULL});
}
static inline bool log_add_stdout_color_sink() {
  return log_add_sink((LogSink){stdout, LOG_TRACE, true, NULL});
}
static inline bool log_add_stderr_color_sink() {
  return log_add_sink((LogSink){stderr, LOG_TRACE, true, NULL});
}

void log_vlog(enum LogLevel level, const char *file, int line, const char *fmt,
              va_list args);
void log_log(enum LogLevel level, const char *file, int line, const char *fmt,
             ...);

void log_print(FILE *buffer, const char *fmt, bool colored, const LogEvent *ev);

#endif /* end of include guard: LOG_H_NLITVEQU */
