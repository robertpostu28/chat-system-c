#include "util.h"

#include <ctype.h>
#include <string.h>

// trim leading and trailing whitespaces characters from a string in-place
// the function modifies the original string and does not allocate new memory
void str_trim(char *str) {
    if (!str) return; // return early if the input string is null

    // remove trailing whitespaces characters by scanning from the end
    size_t len = strlen(str);
    while (len && (str[len - 1] == '\n' || str[len - 1] == '\r' || isspace((unsigned char)str[len - 1]))) {
        str[--len] = '\0';
    }

    // find the index of the first non-whitespace character from the start
    size_t i = 0;
    while (str[i] && isspace((unsigned char)str[i])) {
        i++;
    }

    // shift the trimmed string to the beginning if leading whitespace was found
    if (i > 0) {
        memmove(str, str + i, strlen(str + i) + 1);
    }
}

// returns the current UTC time formatted as a human-readable string
const char* format_utc_now(char *buffer, size_t size) {
    // get the current time in seconds since the Unix epoch
    time_t t = time(NULL);

    // convert time to a UTC time structure (thread-safe)
    struct tm tmv;
    gmtime_r(&t, &tmv); // it avoids using internal static memory, which could be
                        // overwritten by other threads

    // format the UTC time into a readable string and store it in a buffer
    strftime(buffer, size, "%A, %d %B %Y %H:%M:%S UTC", &tmv);

    // return the formatted time string
    return buffer;
}