#pragma once

/**
 * PLATFORM COMPATIBILITY HEADER
 *
 * This header provides cross-platform compatibility for Windows and Linux,
 * including safe file operations (like fopen, localtime), string manipulation
 * (tokenization, concatenation), and file system utilities (directory
 * iteration, stat, mkdir, path manipulation). It aims to handle compiler
 * warnings and maintain portability.
 */

// Standard C library includes
#include <errno.h>    // For errno, errno_t
#include <limits.h>   // For INT_MAX
#include <stdbool.h>  // For bool type (C99 and later)
#include <stdio.h>    // For FILE, fopen, fopen_s, snprintf, NULL
#include <string.h>   // For memcpy, memchr, snprintf, strlen, strrchr, strtok_r, strspn, strcspn
#include <time.h>     // For time_t, struct tm, mktime, localtime, localtime_s, localtime_r

// Cross-platform safe file opening function.
// Uses fopen_s when available (Windows with compatible compiler),
// falls back to standard fopen with proper error handling.
static inline FILE* safe_fopen(const char* filename, const char* mode) {
  FILE* file = NULL;

#if defined(_WIN32) && defined(__STDC_SECURE_LIB__) && \
    __STDC_WANT_SECURE_LIB__
  // Use fopen_s if available (Windows with secure CRT)
  errno_t err = fopen_s(&file, filename, mode);
  if (err != 0) {
    file = NULL; // fopen_s should set file to NULL on error, but to be safe.
  }
#else
  // Use standard fopen for all other cases
  file = fopen(filename, mode);
#endif

  return file;
}

// Cross-platform safe localtime function.
// Returns 0 on success, non-zero on failure.
// Error codes:
// -1: NULL pointer provided for timer or result
// -2: localtime_s failed (Windows)
// -3: localtime_r failed (POSIX)
// -4: localtime failed (fallback)
static inline int safe_localtime(const time_t* timer, struct tm* result) {
  if (timer == NULL || result == NULL) {
    return -1; // Indicate error: NULL pointer provided
  }

// POSIX localtime_r detection
#if defined(_POSIX_C_SOURCE) || defined(_GNU_SOURCE) || defined(__unix__) || \
    defined(__APPLE__)
  if (localtime_r(timer, result) == NULL) {
    return -3; // Specific error for localtime_r failure
  }
  return 0; // Success

// Windows localtime_s detection
#elif defined(_WIN32) && defined(__STDC_SECURE_LIB__) && \
    __STDC_WANT_SECURE_LIB__
  errno_t err = localtime_s(result, timer);
  if (err != 0) {
    return -2; // Specific error for localtime_s failure
  }
  return 0; // Success

#else
  // Fallback to standard localtime (NOT thread-safe without external sync)
  struct tm* temp_tm = localtime(timer);
  if (temp_tm == NULL) {
    return -4; // Specific error for localtime failure
  }
  // Copy the contents immediately from the static buffer
  memcpy(result, temp_tm, sizeof(struct tm));
  return 0; // Success
#endif
}

/**
 * Cross-platform thread-safe string tokenizer.
 * Uses strtok_r on POSIX-compliant systems.
 * Falls back to a manual implementation using strspn/strcspn otherwise.
 *
 * @param str The string to be tokenized. On the first call, this should be
 *            the C string to tokenize. In subsequent calls, it should be NULL.
 *            This string is modified by placing null characters ('\0') at
 *            the end of each token.
 * @param delim A C string containing the delimiter characters.
 * @param saveptr A pointer to a char* variable that is used internally by
 *                safe_strtok to maintain context between calls. The caller
 *                provides the storage for this variable.
 * @return A pointer to the next token found in the string, or NULL if no
 *         more tokens are found. The tokens are null-terminated.
 */
static inline char* safe_strtok(char* str, const char* delim, char** saveptr) {
  // POSIX strtok_r detection
#if (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 1) || \
    defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE) || \
    defined(__APPLE__)
  return strtok_r(str, delim, saveptr);
#else
  // Manual fallback implementation (e.g., for MSVC or other non-POSIX)
  char* s;
  char* token_start;

  if (str != NULL) {
    s = str;
  } else {
    if (*saveptr == NULL) { // No more tokens from previous state
      return NULL;
    }
    s = *saveptr;
  }

  // If s is NULL (from *saveptr being NULL) or points to an empty string
  if (s == NULL || *s == '\0') {
    *saveptr = s; // Ensure saveptr reflects the end (NULL or points to '\0')
    return NULL;
  }

  // Skip leading delimiters
  s += strspn(s, delim);
  if (*s == '\0') { // String consists only of delimiters from this point
    *saveptr = s; // s points to the null terminator
    return NULL;
  }

  token_start = s;

  // Find the end of the token (the next delimiter)
  s += strcspn(s, delim);

  if (*s != '\0') {   // Found a delimiter
    *s = '\0';        // Null-terminate the token
    *saveptr = s + 1; // Next call should start after this delimiter
  } else {            // End of string is the end of the token
    *saveptr = s; // s points to the null terminator, next call will get NULL
  }

  return token_start;
#endif
}

/**
 * Safely concatenates the source string to the destination string.
 * Uses snprintf for the concatenation operation, ensuring buffer safety.
 * The destination buffer is always null-terminated if dest_size > 0.
 *
 * @param dest The destination buffer. Must be a valid pointer.
 *             It should ideally be null-terminated within its first dest_size bytes.
 *             This function will determine its current length safely.
 * @param dest_size The total size of the destination buffer.
 * @param src The null-terminated source string to append. Must be a valid pointer.
 * @return The new length of the string in 'dest' (equivalent to strlen(dest)
 *         after the call) on success. This value will always be less than 'dest_size'.
 *         If truncation of 'src' occurred, the returned length will be
 *         'dest_size - 1'.
 *         Returns a negative value on error:
 *         -1: dest or src is NULL.
 *         -2: dest_size is 0.
 *         -3: dest buffer was initially full and not null-terminated (it is
 *             now forced to be null-terminated at dest[dest_size-1]).
 *         -4: snprintf encoding error or other snprintf failure.
 */
static inline int safe_strcat(char *dest, size_t dest_size, const char *src) {
    if (dest == NULL || src == NULL) {
        return -1; // Invalid arguments: NULL pointers
    }
    if (dest_size == 0) {
        return -2; // Destination buffer has no space at all
    }

    size_t current_len;
    char *null_terminator_in_dest = (char*)memchr(dest, '\0', dest_size);

    if (null_terminator_in_dest == NULL) {
        // No null terminator found within dest_size.
        dest[dest_size - 1] = '\0';
        return -3;
    } else {
        current_len = (size_t)(null_terminator_in_dest - dest);
    }

    size_t remaining_space = dest_size - current_len;
    char *write_ptr = dest + current_len;

    int chars_that_would_be_written = snprintf(write_ptr, remaining_space, "%s", src);

    if (chars_that_would_be_written < 0) {
        return -4; // snprintf error
    }

    if ((size_t)chars_that_would_be_written >= remaining_space) {
        return (int)(dest_size - 1); // Truncation occurred
    } else {
        return (int)(current_len + (size_t)chars_that_would_be_written);
    }
}

/**
 * Safely concatenates at most 'n' characters from the source string to the
 * destination string. Uses snprintf for the concatenation operation, ensuring
 * buffer safety. The destination buffer is always null-terminated if dest_size > 0.
 *
 * @param dest The destination buffer. Must be a valid pointer.
 *             It should ideally be null-terminated within its first dest_size bytes.
 *             This function will determine its current length safely.
 * @param dest_size The total size of the destination buffer.
 * @param src The null-terminated source string to append. Must be a valid pointer.
 * @param n The maximum number of characters to append from 'src'. If 'src' is
 *          shorter than 'n' characters (excluding its null terminator), only
 *          the actual characters from 'src' are appended.
 * @return The new length of the string in 'dest' (equivalent to strlen(dest)
 *         after the call) on success. This value will always be less than 'dest_size'.
 *         If truncation of 'src' (either due to 'n' or 'dest_size') occurred,
 *         the returned length will reflect the actual characters written, up to
 *         'dest_size - 1'.
 *         Returns a negative value on error:
 *         -1: dest or src is NULL.
 *         -2: dest_size is 0.
 *         -3: dest buffer was initially full and not null-terminated (it is
 *             now forced to be null-terminated at dest[dest_size-1]).
 *         -4: snprintf encoding error or other snprintf failure.
 */
static inline int safe_strncat(char *dest, size_t dest_size, const char *src, size_t n) {
    if (dest == NULL || src == NULL) {
        return -1; // Invalid arguments: NULL pointers
    }
    if (dest_size == 0) {
        return -2; // Destination buffer has no space at all
    }

    size_t current_len_dest;
    char *null_terminator_in_dest = (char*)memchr(dest, '\0', dest_size);

    if (null_terminator_in_dest == NULL) {
        // No null terminator found within dest_size.
        dest[dest_size - 1] = '\0'; // Force null-termination
        return -3; // Indicate dest was full and not properly terminated
    } else {
        current_len_dest = (size_t)(null_terminator_in_dest - dest);
    }

    // Space available in dest for appending new characters AND a null terminator
    size_t append_buf_size = dest_size - current_len_dest;

    if (append_buf_size <= 1) {
        // No space to append any characters from src (only space for existing null or less)
        return (int)current_len_dest; // Nothing appended
    }

    // Determine the actual 'n' for snprintf's precision (%.*s)
    // Cap 'n' at INT_MAX because snprintf's '*' precision takes an int.
    // If n is 0, actual_n_for_snprintf will be 0, and "%.0s" writes nothing.
    int actual_n_for_snprintf = (n > INT_MAX) ? INT_MAX : (int)n;

    char *write_ptr = dest + current_len_dest;

    // snprintf will write at most 'actual_n_for_snprintf' chars from src,
    // or fewer if src is shorter. It will also not write more than
    // 'append_buf_size - 1' characters into write_ptr, plus a null terminator.
    int chars_that_would_be_written = snprintf(write_ptr, append_buf_size, "%.*s", actual_n_for_snprintf, src);

    if (chars_that_would_be_written < 0) {
        // snprintf error (e.g., encoding error).
        return -4;
    }

    // chars_that_would_be_written is the number of characters (excluding null)
    // that *would have been* written if append_buf_size was large enough,
    // considering the 'actual_n_for_snprintf' limit from src.

    if ((size_t)chars_that_would_be_written >= append_buf_size) {
        // Truncation by snprintf due to append_buf_size limit occurred.
        // snprintf has written (append_buf_size - 1) chars and a null.
        // The new total length of dest is current_len_dest + (append_buf_size - 1)
        // which simplifies to dest_size - 1.
        return (int)(dest_size - 1);
    } else {
        // No truncation by snprintf's buffer limit.
        // All 'chars_that_would_be_written' (respecting 'n' and src length) fit.
        // The new total length is current_len_dest + chars_that_would_be_written.
        return (int)(current_len_dest + (size_t)chars_that_would_be_written);
    }
}


#ifdef _WIN32
// Windows-specific includes and definitions
#include <direct.h>  // For _mkdir
#include <io.h>      // For _findfirst etc. (though using FindFirstFile)
#include <windows.h>
// For _stat (MSVC)
#include <sys/stat.h>

// Platform-specific stat definitions
#define platform_stat _stat
#define platform_stat_struct struct _stat

// Directory operations
#define MKDIR(dir) _mkdir(dir)
#define PATH_SEPARATOR "\\"
#define PATH_SEPARATOR_CHAR '\\'

// File path maximum length
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

// File enumeration structures and functions
typedef struct {
  WIN32_FIND_DATA findData;
  HANDLE handle;
  bool firstCall;
  char pattern[MAX_PATH]; // Store the original pattern for reference
} DirectoryIterator;

// Function to start directory enumeration
static inline bool platform_find_first_file(DirectoryIterator* iter,
                                            const char* pattern_in) {
  // Copy pattern to iterator, ensuring null termination
  snprintf(iter->pattern, MAX_PATH, "%s", pattern_in);
  iter->pattern[MAX_PATH - 1] = '\0'; // Ensure null termination

  iter->handle = FindFirstFileA(iter->pattern, &iter->findData); // Use A for char*
  iter->firstCall = true;
  return iter->handle != INVALID_HANDLE_VALUE;
}

// Function to get next file in enumeration
static inline bool platform_find_next_file(DirectoryIterator* iter) {
  if (iter->firstCall) {
    iter->firstCall = false;
    // Data for the first file is already in findData from FindFirstFile
    return iter->handle != INVALID_HANDLE_VALUE;
  }
  return FindNextFileA(iter->handle, &iter->findData) != 0; // Use A for char*
}

// Function to get current filename
static inline const char* platform_get_filename(DirectoryIterator* iter) {
  return iter->findData.cFileName;
}

// Function to check if current entry is a directory
static inline bool platform_is_directory(DirectoryIterator* iter) {
    return (iter->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

// Function to get file modification time
static inline time_t platform_get_file_time(DirectoryIterator* iter) {
  SYSTEMTIME sysTime;
  if (FileTimeToSystemTime(&iter->findData.ftLastWriteTime, &sysTime)) {
    struct tm timeStruct = {0};
    timeStruct.tm_year = sysTime.wYear - 1900;
    timeStruct.tm_mon = sysTime.wMonth - 1;
    timeStruct.tm_mday = sysTime.wDay;
    timeStruct.tm_hour = sysTime.wHour;
    timeStruct.tm_min = sysTime.wMinute;
    timeStruct.tm_sec = sysTime.wSecond;
    timeStruct.tm_isdst = -1; // Let mktime determine DST
    return mktime(&timeStruct);
  }
  return 0;
}

// Function to close directory enumeration
static inline void platform_find_close(DirectoryIterator* iter) {
  if (iter->handle != INVALID_HANDLE_VALUE) {
    FindClose(iter->handle);
    iter->handle = INVALID_HANDLE_VALUE;
  }
}

#else
// Linux/Unix-specific includes and definitions
#include <dirent.h>     // For opendir, readdir, closedir, struct dirent
#include <fnmatch.h>    // For fnmatch
#include <sys/stat.h>   // For stat, mkdir, S_ISREG, S_ISDIR
#include <sys/types.h>  // For mode_t, etc.
#include <unistd.h>     // For access, getcwd, rmdir (mkdir is often here too)

// Platform-specific stat definitions
#define platform_stat stat
#define platform_stat_struct struct stat

// Directory operations
#define MKDIR(dir) mkdir(dir, 0755) // POSIX mkdir
#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_CHAR '/'

// File path maximum length
#ifndef MAX_PATH
#define MAX_PATH 4096 // A common value on Linux for PATH_MAX
#endif

// File enumeration structures and functions
typedef struct {
  DIR* dir;
  struct dirent* entry;
  struct stat fileStat;    // Stat info for the current entry
  char pattern[MAX_PATH];  // Filename pattern part
  char dirPath[MAX_PATH];  // Directory path part
  char currentFileFullName[MAX_PATH]; // Full path of current file
} DirectoryIterator;

// Function to start directory enumeration
static inline bool platform_find_first_file(DirectoryIterator* iter,
                                            const char* pattern_in) {
  const char* filename_component = strrchr(pattern_in, PATH_SEPARATOR_CHAR);

  if (filename_component != NULL) { // Pattern includes a path
    size_t dir_len = filename_component - pattern_in;
    if (dir_len == 0) { // e.g., "/pattern"
      snprintf(iter->dirPath, sizeof(iter->dirPath), "%c",
               PATH_SEPARATOR_CHAR);
    } else {
      snprintf(iter->dirPath, sizeof(iter->dirPath), "%.*s", (int)dir_len,
               pattern_in);
    }
    filename_component++; // Move past the separator
  } else { // Pattern is for current directory
    snprintf(iter->dirPath, sizeof(iter->dirPath), ".");
    filename_component = pattern_in;
  }
  iter->dirPath[sizeof(iter->dirPath) - 1] = '\0';

  snprintf(iter->pattern, sizeof(iter->pattern), "%s", filename_component);
  iter->pattern[sizeof(iter->pattern) - 1] = '\0';

  iter->dir = opendir(iter->dirPath);
  if (!iter->dir) {
    return false;
  }

  // Find first matching file (delegating to platform_find_next_file logic)
  return platform_find_next_file(iter);
}

// Function to get next file in enumeration
static inline bool platform_find_next_file(DirectoryIterator* iter) {
  if (!iter->dir) {
    return false;
  }

  while ((iter->entry = readdir(iter->dir)) != NULL) {
    if (strcmp(iter->entry->d_name, ".") == 0 ||
        strcmp(iter->entry->d_name, "..") == 0) {
      continue; // Skip . and ..
    }

    if (fnmatch(iter->pattern, iter->entry->d_name, 0) == 0) {
      snprintf(iter->currentFileFullName, sizeof(iter->currentFileFullName),
               "%s%c%s", iter->dirPath, PATH_SEPARATOR_CHAR,
               iter->entry->d_name);
      iter->currentFileFullName[sizeof(iter->currentFileFullName) - 1] = '\0';

      if (platform_stat(iter->currentFileFullName, &iter->fileStat) == 0) {
        // Original code filtered for S_ISREG.
        // To be more general, one might remove this or make it optional.
        // For now, keeping it to match original behavior.
        if (S_ISREG(iter->fileStat.st_mode)) {
            return true;
        }
      }
    }
  }
  return false; // No more matching files
}

// Function to get current filename
static inline const char* platform_get_filename(DirectoryIterator* iter) {
  return iter->entry ? iter->entry->d_name : NULL;
}

// Function to check if current entry is a directory
static inline bool platform_is_directory(DirectoryIterator* iter) {
    // Ensure entry and fileStat are valid (e.g., after a successful find_next_file)
    if (iter->entry) {
        // The current find_next_file filters for S_ISREG, so this would be false
        // for entries returned by it. If that filter is removed/changed,
        // this check becomes more broadly useful.
        return S_ISDIR(iter->fileStat.st_mode);
    }
    return false;
}


// Function to get file modification time
static inline time_t platform_get_file_time(DirectoryIterator* iter) {
  // fileStat should be populated by platform_find_next_file
  return iter->entry ? iter->fileStat.st_mtime : 0;
}

// Function to close directory enumeration
static inline void platform_find_close(DirectoryIterator* iter) {
  if (iter->dir) {
    closedir(iter->dir);
    iter->dir = NULL;
  }
  iter->entry = NULL;
}

#endif

// Common utility functions (platform-independent logic using platform-defined macros)

/**
 * Constructs a full path from a directory and a filename.
 * Ensures proper path separator is used.
 *
 * @param dest Buffer to store the resulting path.
 * @param destSize Size of the destination buffer.
 * @param dir Directory path.
 * @param filename Filename.
 */
static inline void platform_make_path(char* dest, size_t destSize,
                                      const char* dir, const char* filename) {
  if (dir == NULL || filename == NULL || dest == NULL || destSize == 0) {
    if (dest && destSize > 0) dest[0] = '\0';
    return;
  }
  snprintf(dest, destSize, "%s%s%s", dir, PATH_SEPARATOR, filename);
  dest[destSize - 1] = '\0'; // Ensure null-termination
}

/**
 * Constructs a full path pattern from a directory and a filename pattern.
 * This is typically used as input for platform_find_first_file.
 *
 * @param dest Buffer to store the resulting pattern.
 * @param destSize Size of the destination buffer.
 * @param dir Directory path.
 * @param pattern Filename pattern (e.g., "*.txt").
 */
static inline void platform_make_pattern(char* dest, size_t destSize,
                                         const char* dir,
                                         const char* pattern) {
  if (dir == NULL || pattern == NULL || dest == NULL || destSize == 0) {
    if (dest && destSize > 0) dest[0] = '\0';
    return;
  }
  // If dir is empty or ".", just use the pattern.
  // Otherwise, combine dir, separator, and pattern.
  if (dir[0] == '\0' || (dir[0] == '.' && dir[1] == '\0')) {
    snprintf(dest, destSize, "%s", pattern);
  } else {
    snprintf(dest, destSize, "%s%s%s", dir, PATH_SEPARATOR, pattern);
  }
  dest[destSize - 1] = '\0'; // Ensure null-termination
}