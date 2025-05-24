#pragma once

/**
 * PLATFORM COMPATIBILITY HEADER
 *
 * This header provides cross-platform compatibility for Windows and Linux
 * by defining platform-specific includes, macros, and function mappings.
 */

#ifdef _WIN32
    // Windows-specific includes and definitions
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
    
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
        char pattern[MAX_PATH];
    } DirectoryIterator;
    
    // Function to start directory enumeration
    static inline bool platform_find_first_file(DirectoryIterator* iter, const char* pattern) {
        strncpy(iter->pattern, pattern, MAX_PATH - 1);
        iter->pattern[MAX_PATH - 1] = '\0';
        iter->handle = FindFirstFile(pattern, &iter->findData);
        iter->firstCall = true;
        return iter->handle != INVALID_HANDLE_VALUE;
    }
    
    // Function to get next file in enumeration
    static inline bool platform_find_next_file(DirectoryIterator* iter) {
        if (iter->firstCall) {
            iter->firstCall = false;
            return iter->handle != INVALID_HANDLE_VALUE;
        }
        return FindNextFile(iter->handle, &iter->findData) != 0;
    }
    
    // Function to get current filename
    static inline const char* platform_get_filename(DirectoryIterator* iter) {
        return iter->findData.cFileName;
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
    #include <dirent.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <fnmatch.h>
    
    // Directory operations
    #define MKDIR(dir) mkdir(dir, 0755)
    #define PATH_SEPARATOR "/"
    #define PATH_SEPARATOR_CHAR '/'
    
    // File path maximum length
    #ifndef MAX_PATH
        #define MAX_PATH 4096
    #endif
    
    // File enumeration structures and functions
    typedef struct {
        DIR* dir;
        struct dirent* entry;
        char pattern[MAX_PATH];
        char dirPath[MAX_PATH];
        char currentFile[MAX_PATH];
        struct stat fileStat;
    } DirectoryIterator;
    
    // Function to start directory enumeration
    static inline bool platform_find_first_file(DirectoryIterator* iter, const char* pattern) {
        // Extract directory and filename pattern
        const char* lastSlash = strrchr(pattern, '/');
        if (lastSlash) {
            size_t dirLen = lastSlash - pattern;
            strncpy(iter->dirPath, pattern, dirLen);
            iter->dirPath[dirLen] = '\0';
            strcpy(iter->pattern, lastSlash + 1);
        } else {
            strcpy(iter->dirPath, ".");
            strcpy(iter->pattern, pattern);
        }
        
        iter->dir = opendir(iter->dirPath);
        if (!iter->dir) {
            return false;
        }
        
        // Find first matching file
        while ((iter->entry = readdir(iter->dir)) != NULL) {
            if (fnmatch(iter->pattern, iter->entry->d_name, 0) == 0) {
                snprintf(iter->currentFile, MAX_PATH, "%s/%s", iter->dirPath, iter->entry->d_name);
                if (stat(iter->currentFile, &iter->fileStat) == 0 && S_ISREG(iter->fileStat.st_mode)) {
                    return true;
                }
            }
        }
        
        closedir(iter->dir);
        iter->dir = NULL;
        return false;
    }
    
    // Function to get next file in enumeration
    static inline bool platform_find_next_file(DirectoryIterator* iter) {
        if (!iter->dir) {
            return false;
        }
        
        while ((iter->entry = readdir(iter->dir)) != NULL) {
            if (fnmatch(iter->pattern, iter->entry->d_name, 0) == 0) {
                snprintf(iter->currentFile, MAX_PATH, "%s/%s", iter->dirPath, iter->entry->d_name);
                if (stat(iter->currentFile, &iter->fileStat) == 0 && S_ISREG(iter->fileStat.st_mode)) {
                    return true;
                }
            }
        }
        
        return false;
    }
    
    // Function to get current filename
    static inline const char* platform_get_filename(DirectoryIterator* iter) {
        return iter->entry ? iter->entry->d_name : NULL;
    }
    
    // Function to get file modification time
    static inline time_t platform_get_file_time(DirectoryIterator* iter) {
        return iter->fileStat.st_mtime;
    }
    
    // Function to close directory enumeration
    static inline void platform_find_close(DirectoryIterator* iter) {
        if (iter->dir) {
            closedir(iter->dir);
            iter->dir = NULL;
        }
    }

#endif

// Common utility functions
static inline void platform_make_path(char* dest, size_t destSize, const char* dir, const char* filename) {
    snprintf(dest, destSize, "%s%s%s", dir, PATH_SEPARATOR, filename);
}

// Function to convert wildcard pattern to platform-appropriate format
static inline void platform_make_pattern(char* dest, size_t destSize, const char* dir, const char* pattern) {
    snprintf(dest, destSize, "%s%s%s", dir, PATH_SEPARATOR, pattern);
}
