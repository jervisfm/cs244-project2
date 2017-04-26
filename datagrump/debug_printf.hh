#ifndef DEBUG_PRINTF_HH
#define DEBUG_PRINTF_HH

#include <cstdarg>

// Debug logging definitions.
const int ERROR = 0;
const int WARN = 1;
const int INFO = 2;
const int VERBOSE = 3;
const int EXTRA_VERBOSE = 4;
//const int DEBUG_LEVEL = 0;  // 0 means only output upto Error Level.
const int DEBUG_LEVEL = 2;  // 2 means only output upto Info Level.

// TODO(jmuindi): Remember to disable this after development completed so that
// no debug output is printed for submission.
static const bool LOGGING_ENABLED = true;
//static const bool LOGGING_ENABLED = false;


inline void debug_printf(int level, const char* format_template, ...) {
  if (level < 0) {
    return;
  }
  bool should_log = level <= DEBUG_LEVEL;
  if (!should_log || !LOGGING_ENABLED) {
    return;
  }
  // We color our debug prefix messages.
  // See https://en.wikipedia.org/wiki/ANSI_escape_code#Colors for ANSI color codes.
  if (level == ERROR) {
    fprintf(stderr, "\x1b[31m");  // Begin Red Color
    fprintf(stderr, "[ERROR] ");
    fprintf(stderr, "\x1b[0m");  // End Color
  } else if (level == WARN) {
    fprintf(stderr, "\x1b[33m");  // Begin Yellow Color
    fprintf(stderr, "[WARNING] ");
    fprintf(stderr, "\x1b[0m");  // End Color
  } else if (level == INFO) {
    fprintf(stderr, "\x1b[32m");  // Begin Green Color
    fprintf(stderr, "[LOG INFO] ");
    fprintf(stderr, "\x1b[0m");  // End Color
  } else if (level == VERBOSE) {
    fprintf(stderr, "\x1b[34m");  // Begin Blue Color
    fprintf(stderr, "[VERBOSE] ");
    fprintf(stderr, "\x1b[0m");  // End Color
  } else if (level == EXTRA_VERBOSE) {
    fprintf(stderr, "\x1b[35m");  // Begin Magneta Color
    fprintf(stderr, "[EXTRA VERBOSE] ");
    fprintf(stderr, "\x1b[0m");  // End Color
  }
  va_list argument_list;
  va_start(argument_list, format_template);
  vfprintf(stderr, format_template, argument_list);
  va_end(argument_list);

  // Output a new line so that boundary between messages is clear.
  fprintf(stderr, "\n");

  return;
}

#endif
