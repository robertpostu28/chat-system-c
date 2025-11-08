#ifndef LAB3_LOG_H
#define LAB3_LOG_H

// open file, returns 0 on success, -1 on failure
int log_init(const char *path);

// close the log file if it was opened
void log_close(void); // (void) means the function log_close takes no parameters
                      // log_close() in C - may take any number of arguments

// logs an informational message with timestamp
// accepts printf-style format string and arguments
void log_info(const char *fmt, ...);  // ... - accept any number of arguments
                                      // used for flexible logging, printing, formatting

// logs a warning message with timestamp
// accepts printf-style format string and arguments
void log_warn(const char *fmt, ...);

// logs an error message with timestamps
// accepts printf-style format string and arguments
void log_error(const char *fmt, ...);

#endif //LAB3_LOG_H