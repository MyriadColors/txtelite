#ifndef ELITE_UTILS_H
#define ELITE_UTILS_H

#include "elite_state.h" // Unified header for constants, structures, and globals
#include <ctype.h>		 // For isspace, toupper functions

// Note: NativeRand and ExitStatus are now defined in elite_state.h

static unsigned int lastrand_for_my_rand = 0; // Renamed to avoid potential conflicts if lastrand is used elsewhere

static inline void my_srand(unsigned int initialSeed)
{
	srand(initialSeed);
	lastrand_for_my_rand = initialSeed - 1;
}

static inline int my_rand(void)
{
	int r;

	if (NativeRand)
		r = rand();
	else
	{ // As supplied by D McDonnell	from SAS Insititute C
		r = (((((((((((lastrand_for_my_rand << 3) - lastrand_for_my_rand) << 3) + lastrand_for_my_rand) << 1) + lastrand_for_my_rand) << 4) - lastrand_for_my_rand) << 1) - lastrand_for_my_rand) + 0xe60) & 0x7fffffff;
		lastrand_for_my_rand = r - 1;
	}
	return (r);
}

static inline char random_byte(void)
{
	return (char)(my_rand() & 0xFF);
}

static inline uint16_t minimum_value(uint16_t valueA, uint16_t valueB)
{
	return valueA < valueB ? valueA : valueB;
}

static inline void stop(const char *messageString) // Made messageString const
{
	printf("\n%s", messageString);
	// ExitStatus will be used by the main exit call
	exit(EXIT_FAILURE); // Or some other non-success status
}

static inline signed int float_to_int_round(double inputValue)
{
	return ((signed int)floor(inputValue + 0.5));
}

static inline signed int float_to_int_floor(double inputValue)
{
	return ((signed int)floor(inputValue));
}

static inline void tweak_seed(struct SeedType *seedToTweak)
{
	uint16_t temp;
	temp = ((*seedToTweak).a) + ((*seedToTweak).b) + ((*seedToTweak).c); /* 2 byte aritmetic */
	(*seedToTweak).a = (*seedToTweak).b;
	(*seedToTweak).b = (*seedToTweak).c;
	(*seedToTweak).c = temp;
	// d is not updated in original algorithm, but should be handled for completeness
	// We could rotate d too, but it wasn't in the original algorithm
}

/* Remove all c's from string s */
static inline void strip_char_from_string(char *inputString, const char charToStrip)
{
	size_t i = 0, j = 0;

	while (i < strlen(inputString))
	{
		if (inputString[i] != charToStrip)
		{
			inputString[j] = inputString[i];
			j++;
		}
		i++;
	}

	inputString[j] = 0;
}

/* Return nonzero iff string t begins with non-empty string s */
static inline bool string_begins_with(const char *prefixString, const char *fullString) // Made params const
{
	size_t i = 0;
	size_t l = strlen(prefixString);
	if (l > 0)
	{
		// Check if fullString is long enough
		if (strlen(fullString) < l)
			return false;
		while ((i < l) & (toupper(prefixString[i]) == toupper(fullString[i])))
			i++;
		if (i == l)
			return true;
	}
	return false;
}

/*
 * Check string s against n options in string array a
 * If matches ith element return i+1 else return 0
 */
static inline uint16_t match_string_in_array(const char *searchString, const char stringArray[][MAX_LEN], uint16_t arraySize) // Made params const
{
	for (uint16_t i = 0; i < arraySize; i++)
	{
		if (string_begins_with(searchString, stringArray[i]))
			return i + 1;
	}
	return 0;
}

/* Strip leading and trailing space characters from the given string. */
static inline char *strip_leading_trailing_spaces(char *inputString)
{
	char *p;
	if (inputString == NULL)
		return NULL;													 // Handle NULL input
	while (*inputString != '\0' && isspace((unsigned char)*inputString)) // Cast to unsigned char for isspace
	{
		++inputString;
	}
	p = inputString + strlen(inputString);
	while (p > inputString && isspace((unsigned char)*(p - 1))) // Cast to unsigned char for isspace
	{
		--p;
		*p = '\0';
	}
	return inputString;
}

/* Split string s at first space, returning first 'word' in t & shortening s */
static inline void split_string_at_first_space(char *inputString, char *firstWord)
{
	if (inputString == NULL || firstWord == NULL)
		return; // Handle NULL input

	size_t l = strlen(inputString);
	size_t i = 0, j = 0;

	/* Strip leading spaces */
	while ((i < l) && isspace((unsigned char)inputString[i]))
		i++; // Cast for isspace

	if (i == l)
	{
		inputString[0] = 0;
		firstWord[0] = 0;
		return;
	};

	while ((i < l) && (inputString[i] != ' '))
	{
		firstWord[j] = inputString[i];
		i++;
		j++;
	}
	firstWord[j] = 0;

	// If there was a space, skip it
	if (i < l && inputString[i] == ' ')
		i++;

	j = 0;
	while (i < l)
	{
		inputString[j] = inputString[i];
		i++;
		j++;
	}
	inputString[j] = 0;
}

#endif // ELITE_UTILS_H
