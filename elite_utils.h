#pragma once

#include "elite_state.h" // Unified header for constants, structures, and globals
#include <ctype.h>       // For isspace, toupper functions
#include <math.h>        // For floor, etc.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Note: NativeRand and ExitStatus are now members of g_state in elite_state.h

// Internal PRNG states
static uint32_t g_s_sas_rand_state = 0;
static thread_local uint32_t g_s_xorshift_state = 1U;

// 32-bit Xorshift (Marsaglia, 2003) clamped to [0, 2^31 - 1]
static inline uint32_t step_xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x & 0x7FFF'FFFFU;
}

// SAS/C LCG: evaluates (3677 * state + 3680) mod 2^31
static inline uint32_t step_sas_lcg(uint32_t *state) {
    uint32_t r = ((3'677U * (*state)) + 0x0E60U) & 0x7FFF'FFFFU;
    *state = r - 1U;
    return r;
}

[[maybe_unused]] static inline void my_srand(uint32_t seed) {
    srand(seed);
    g_s_sas_rand_state = seed - 1U;
    g_s_xorshift_state = (seed != 0U) ? seed : 1U;
}

static inline int my_rand(void) {
    uint32_t r = (int)g_state.NativeRand ? step_xorshift32(&g_s_xorshift_state) : step_sas_lcg(&g_s_sas_rand_state);

    return (int)r;
}

[[maybe_unused]] static inline char random_byte(void) { return (char)(my_rand() & 0xFF); }

[[maybe_unused]] static inline uint16_t minimum_value(uint16_t value_a, uint16_t value_b) {
    return value_a < value_b ? value_a : value_b;
}

[[maybe_unused]] static inline void stop(const char *message_string) // Made messageString const
{
    printf("\n%s", message_string);
    // ExitStatus will be used by the main exit call
    // Defer termination to the owning thread instead of terminating the
    // entire process from an arbitrary worker thread.
    g_state.ExitStatus = EXIT_FAILURE;
}

[[maybe_unused]] static inline signed int float_to_int_round(double input_value) {
    const double ROUNDED_VALUE = floor(input_value + 0.5);
    return (signed int)ROUNDED_VALUE;
}

[[maybe_unused]] static inline signed int float_to_int_floor(double input_value) {
    const double FLOORED_VALUE = floor(input_value);
    return (signed int)FLOORED_VALUE;
}

[[maybe_unused]] static inline void tweak_seed(struct seed_type_t *seed_to_tweak) {
    uint16_t temp;
    temp = ((*seed_to_tweak).a) + ((*seed_to_tweak).b) + ((*seed_to_tweak).c); /* 2 byte aritmetic */
    (*seed_to_tweak).a = (*seed_to_tweak).b;
    (*seed_to_tweak).b = (*seed_to_tweak).c;
    (*seed_to_tweak).c = temp;
    // d is not updated in original algorithm, but should be handled for completeness
    // We could rotate d too, but it wasn't in the original algorithm
}

/* Remove all c's from string s */
[[maybe_unused]] static inline void strip_char_from_string(char *input_string, const char CHAR_TO_STRIP) {
    size_t index = 0;
    size_t j = 0;

    while (index < strlen(input_string)) {
        if (input_string[index] != CHAR_TO_STRIP) {
            input_string[j] = input_string[index];
            j++;
        }
        index++;
    }

    input_string[j] = 0;
}

/* Return nonzero iff string t begins with non-empty string s */
static inline bool string_begins_with(const char *prefix_string, const char *full_string) // Made params const
{
    size_t index = 0;
    size_t prefixStrLen = strlen(prefix_string);
    if (prefixStrLen > 0) {
        // Check if fullString is long enough
        if (strlen(full_string) < prefixStrLen) {
            return false;
        }
        while ((index < prefixStrLen) & (toupper(prefix_string[index]) == toupper(full_string[index]))) {
            index++;
        }
        if (index == prefixStrLen) {
            return true;
        }
    }
    return false;
}

/*
 * Check string s against n options in string array a
 * If matches ith element return i+1 else return 0
 */
[[maybe_unused]] static inline uint16_t match_string_in_array(const char *search_string,
                                                              const char string_array[][MAX_LEN],
                                                              uint16_t array_size) // Made params const
{
    for (uint16_t i = 0; i < array_size; i++) {
        if (string_begins_with(search_string, string_array[i])) {
            return i + 1;
        }
    }
    return 0;
}

/* Strip leading and trailing space characters from the given string. */
[[maybe_unused]] static inline char *strip_leading_trailing_spaces(char *input_string) {
    char *lineBuffer;
    if (input_string == nullptr) {
        return nullptr; // Handle nullptr input
    }
    while (*input_string != '\0' && isspace((unsigned char)*input_string)) // Cast to unsigned char for isspace
    {
        ++input_string;
    }
    lineBuffer = input_string + strlen(input_string);
    while (lineBuffer > input_string && isspace((unsigned char)*(lineBuffer - 1))) // Cast to unsigned char for isspace
    {
        --lineBuffer;
        *lineBuffer = '\0';
    }
    return input_string;
}

/* Split string s at first space, returning first 'word' in t & shortening s */
[[maybe_unused]] static inline void split_string_at_first_space(char *input_string, char *first_word) {
    if (input_string == nullptr || first_word == nullptr) {
        return; // Handle nullptr input
    }

    size_t l = strlen(input_string);
    size_t i = 0;
    size_t j = 0;

    /* Strip leading spaces */
    while ((i < l) && isspace((unsigned char)input_string[i])) {
        i++; // Cast for isspace
    }

    if (i == l) {
        input_string[0] = 0;
        first_word[0] = 0;
        return;
    };

    while ((i < l) && (input_string[i] != ' ')) {
        first_word[j] = input_string[i];
        i++;
        j++;
    }
    first_word[j] = 0;

    // If there was a space, skip it
    if (i < l && input_string[i] == ' ') {
        i++;
    }

    j = 0;
    while (i < l) {
        input_string[j] = input_string[i];
        i++;
        j++;
    }
    input_string[j] = 0;
}
