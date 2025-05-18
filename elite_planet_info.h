#pragma once

#include "elite_state.h" // Unified header for constants, structures, and globals

// Definition for planetary description generation
struct DescChoice
{
	const char *options[5];
};

// Data for goat_soup planetary description generator
static struct DescChoice descList[] =
	{
		/* 81 */ {{"fabled", "notable", "well known", "famous", "noted"}},
		/* 82 */ {{"very", "mildly", "most", "reasonably", ""}},
		/* 83 */ {{"ancient", "\x95", "great", "vast", "pink"}},
		/* 84 */ {{"\x9E \x9D plantations", "mountains", "\x9C", "\x94 forests", "oceans"}},
		/* 85 */ {{"shyness", "silliness", "mating traditions", "loathing of \x86", "love for \x86"}},
		/* 86 */ {{"food blenders", "tourists", "poetry", "discos", "\x8E"}},
		/* 87 */ {{"talking tree", "crab", "bat", "lobst", "\xB2"}},
		/* 88 */ {{"beset", "plagued", "ravaged", "cursed", "scourged"}},
		/* 89 */ {{"\x96 civil war", "\x9B \x98 \x99s", "a \x9B disease", "\x96 earthquakes", "\x96 solar activity"}},
		/* 8A */ {{"its \x83 \x84", "the \xB1 \x98 \x99", "its inhabitants' \x9A \x85", "\xA1", "its \x8D \x8E"}},
		/* 8B */ {{"juice", "brandy", "water", "brew", "gargle blasters"}},
		/* 8C */ {{"\xB2", "\xB1 \x99", "\xB1 \xB2", "\xB1 \x9B", "\x9B \xB2"}},
		/* 8D */ {{"fabulous", "exotic", "hoopy", "unusual", "exciting"}},
		/* 8E */ {{"cuisine", "night life", "casinos", "sit coms", " \xA1 "}},
		/* 8F */ {{"\xB0", "The planet \xB0", "The world \xB0", "This planet", "This world"}},
		/* 90 */ {{"n unremarkable", " boring", " dull", " tedious", " revolting"}},
		/* 91 */ {{"planet", "world", "place", "little planet", "dump"}},
		/* 92 */ {{"wasp", "moth", "grub", "ant", "\xB2"}},
		/* 93 */ {{"poet", "arts graduate", "yak", "snail", "slug"}},
		/* 94 */ {{"tropical", "dense", "rain", "impenetrable", "exuberant"}},
		/* 95 */ {{"funny", "wierd", "unusual", "strange", "peculiar"}},
		/* 96 */ {{"frequent", "occasional", "unpredictable", "dreadful", "deadly"}},
		/* 97 */ {{"\x82 \x81 for \x8A", "\x82 \x81 for \x8A and \x8A", "\x88 by \x89", "\x82 \x81 for \x8A but \x88 by \x89", "a\x90 \x91"}},
		/* 98 */ {{"\x9B", "mountain", "edible", "tree", "spotted"}},
		/* 99 */ {{"\x9F", "\xA0", "\x87oid", "\x93", "\x92"}},
		/* 9A */ {{"ancient", "exceptional", "eccentric", "ingrained", "\x95"}},
		/* 9B */ {{"killer", "deadly", "evil", "lethal", "vicious"}},
		/* 9C */ {{"parking meters", "dust clouds", "ice bergs", "rock formations", "volcanoes"}},
		/* 9D */ {{"plant", "tulip", "banana", "corn", "\xB2weed"}},
		/* 9E */ {{"\xB2", "\xB1 \xB2", "\xB1 \x9B", "inhabitant", "\xB1 \xB2"}},
		/* 9F */ {{"shrew", "beast", "bison", "snake", "wolf"}},
		/* A0 */ {{"leopard", "cat", "monkey", "goat", "fish"}},
		/* A1 */ {{"\x8C \x8B", "\xB1 \x9F \xA2", "its \x8D \xA0 \xA2", "\xA3 \xA4", "\x8C \x8B"}},
		/* A2 */ {{"meat", "cutlet", "steak", "burgers", "soup"}},
		/* A3 */ {{"ice", "mud", "Zero-G", "vacuum", "\xB1 ultra"}},
		/* A4 */ {{"hockey", "cricket", "karate", "polo", "tennis"}},
};

/**
 * @brief Generates a random number using a linear congruential generator (LCG) algorithm.
 * 
 * This function implements a custom random number generator with the following characteristics:
 * - Uses a global RndSeed structure with members a, b, c, and d
 * - Performs bitwise operations and modular arithmetic to generate pseudo-random values
 * - The algorithm maintains internal state between calls via the RndSeed structure
 * - All operations are constrained to 8-bit values (0-255 range)
 * 
 * @return An 8-bit random number in the range [0-255]
 * 
 * @note This is a deterministic PRNG that will produce the same sequence 
 *       of numbers for the same initial seed values.
 */
static inline int gen_rnd_number(void)
{
	int a, x;
	x = (RndSeed.a * 2) & 0xFF;
	a = x + RndSeed.c;
	if (RndSeed.a > 127)
		a++;
	RndSeed.a = a & 0xFF;
	RndSeed.c = x;

	a = a / 256; /* a = any carry left from above */
	x = RndSeed.b;
	a = (a + x + RndSeed.d) & 0xFF;
	RndSeed.b = a;
	RndSeed.d = x;
	return a;
}

/**
 * @brief Processes a template string and generates descriptive text for a planet.
 * 
 * This function implements the "Goat Soup" algorithm from Elite, which generates
 * descriptive text by processing template strings with special codes:
 * - ASCII characters (< 0x80) are printed directly
 * - Codes between 0x81-0xA4 select random text options from descList
 * - Special codes 0xB0-0xB2 insert planet-specific content:
 *   - 0xB0: Planet name (first letter capital, rest lowercase)
 *   - 0xB1: Planet name in adjective form (e.g., "Martian")
 *   - 0xB2: Random name generated using character pairs
 *
 * The function recursively processes templates, allowing for nested text generation.
 * 
 * @param sourceString The template string to process
 * @param planetSystem Pointer to the planet system data structure
 * 
 * @note There may be a potential bug in the 0xB2 case (random name generation)
 *       where the random index could potentially access out-of-bounds memory.
 */
static inline void goat_soup(const char *sourceString, struct PlanSys *planetSystem)
{
	for (;;)
	{
		int c = *(sourceString++);
		c &= 0xff; // Ensure char is treated as unsigned
		if (c == '\0')
			break;
		if (c < 0x80)
			printf("%c", c);
		else
		{
			if (c <= 0xA4)
			{
				int rnd = gen_rnd_number();
				goat_soup(descList[c - 0x81].options[(rnd >= 0x33) + (rnd >= 0x66) + (rnd >= 0x99) + (rnd >= 0xCC)], planetSystem);
			}
			else
				switch (c)
				{
				case 0xB0: /* planet name */
				{
					int i = 1;
					printf("%c", planetSystem->name[0]);
					while (planetSystem->name[i] != '\0')
						printf("%c", tolower(planetSystem->name[i++]));
				}
				break;
				case 0xB1: /* <planet name>ian */
				{
					int i = 1;
					printf("%c", planetSystem->name[0]);
					while (planetSystem->name[i] != '\0')
					{
						if ((planetSystem->name[i + 1] != '\0') || ((planetSystem->name[i] != 'E') && (planetSystem->name[i] != 'I')))
							printf("%c", tolower(planetSystem->name[i]));
						i++;
					}
					printf("ian");
				}
				break;
				case 0xB2: /* random name */
				{
					int i;
					int len = gen_rnd_number() & 3;
					for (i = 0; i <= len; i++)
					{
						int x = gen_rnd_number() & 0x3e; // pairs0 has 60 chars, so 0x3e (62) is out of bounds for pairs0[x+1] if x is 60 or 61. Max index is 59. (0x3A for 58, 0x3C for 60). pairs0 is 52 chars. "LEXEZASARESORARECETISOALATENNESSEESSTINIVEDALERQUANTEDLENETA" -> length 52. So max x should be 50 (0x32) to access x and x+1.
														 // Original code: gen_rnd_number() & 0x3e; this means x can be up to 62. pairs0 is 52 chars long.
														 // This was an existing bug. For now, I will keep it as is to match original, but this should be noted.
														 // The original pairs0 is "LEXEZASARESORARECETISOALATENNESSEESSTINIVEDALERQUANTEDLENETA" (52 chars)
														 // Accessing pairs0[x] and pairs0[x+1]. So x must be <= 50.
														 // gen_rnd_number() & 0x30 would give x up to 48.
														 // gen_rnd_number() % 51 would give x up to 50.
														 // Let's use gen_rnd_number() % (sizeof(pairs0)-2) if pairs0 is accessible here for sizeof.
														 // For now, keeping original logic: x = gen_rnd_number() & 0x3e;
						if (i == 0)
						{
							printf("%c", pairs0[x]);
						}
						else
						{
							printf("%c", tolower(pairs0[x]));
						}
						printf("%c", tolower(pairs0[x + 1]));
					}
				}
				break;
				default:
					printf("<bad char in data [%X]>", c);
					return;
				}
		}
	}
}

/**
 * @brief Prints information about a planetary system, either in a compressed or detailed format.
 * 
 * This function outputs details about a planetary system, including its name, position,
 * economy type, government type, technology level, productivity, radius, and population.
 * The format of the output depends on the `useCompressedOutput` parameter.
 * 
 * In compressed mode, it displays a single line with:
 * - System name
 * - Technology level (1-based)
 * - Economy type
 * - Government type
 * 
 * In detailed mode, it shows:
 * - System name
 * - Galactic position (x,y)
 * - Economy type (index and name)
 * - Government type (index and name)
 * - Technology level (1-based)
 * - Productivity (turnover)
 * - System radius
 * - Population (in billions)
 * - A descriptive sentence generated using the goat_soup function
 * 
 * @param planetSystemInfo The planetary system information structure to be displayed
 * @param useCompressedOutput If true, outputs in single-line format; if false, outputs in detailed format
 * 
 * @note Relies on global arrays EconNames and GovNames from elite_globals.h
 * @note When using detailed format, modifies the global RndSeed variable for goat_soup generation
 */
static inline void print_system_info(struct PlanSys planetSystemInfo, bool useCompressedOutput)
{
	if (useCompressedOutput)
	{
		printf("%10s", planetSystemInfo.name);
		printf(" TL: %2i ", (planetSystemInfo.techLev) + 1);
		printf("%12s", EconNames[planetSystemInfo.economy]); // EconNames from elite_globals.h
		printf(" %15s", GovNames[planetSystemInfo.govType]); // GovNames from elite_globals.h
	}
	else
	{
		printf("\n\nSystem:  ");
		printf("%s", planetSystemInfo.name);
		printf("\nPosition (%i,", planetSystemInfo.x);
		printf("%i)", planetSystemInfo.y);
		printf("\nEconomy: (%i) ", planetSystemInfo.economy);
		printf("%s", EconNames[planetSystemInfo.economy]); // EconNames from elite_globals.h
		printf("\nGovernment: (%i) ", planetSystemInfo.govType);
		printf("%s", GovNames[planetSystemInfo.govType]); // GovNames from elite_globals.h
		printf("\nTech Level: %2i", (planetSystemInfo.techLev) + 1);
		printf("\nTurnover: %u", (planetSystemInfo.productivity));
		printf("\nRadius: %u", planetSystemInfo.radius);
		printf("\nPopulation: %u Billion", (planetSystemInfo.population) >> 3);

		RndSeed = planetSystemInfo.goatSoupSeed; // RndSeed is global
		printf("\n");
		goat_soup("\x8F is \x97.", &planetSystemInfo);
	}
}