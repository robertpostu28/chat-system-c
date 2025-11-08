#ifndef LAB3_UTIL_H
#define LAB3_UTIL_H

#include <time.h>

void str_trim(char *str); // trims in-place spaces + CR / LF
const char* format_utc_now(char *buf, size_t size); // returns buf

#endif //LAB3_UTIL_H