#pragma once

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#include "elite_galaxy.h"
#include "elite_state.h" // Unified header for constants, structures, and globals

// Definition for planetary description generation
struct desc_choice_t {
    const char *options[5];
};

// Data for goat_soup planetary description generator
static struct desc_choice_t g_desc_list[] = {
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
    /* 97 */
    {{"\x82 \x81 for \x8A", "\x82 \x81 for \x8A and \x8A", "\x88 by \x89", "\x82 \x81 for \x8A but \x88 by \x89",
      "a\x90 \x91"}},
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
static inline int gen_rnd_number(void) {
    int a;
    int x;
    x = (g_state.RndSeed.a * 2) & 0xFF;
    a = x + g_state.RndSeed.c;
    if (g_state.RndSeed.a > 127) {
        a++;
    }
    g_state.RndSeed.a = (uint8_t)(a & 0xFF);
    g_state.RndSeed.c = (uint8_t)x;

    a = a / 256; /* a = any carry left from above */
    x = g_state.RndSeed.b;
    a = (a + x + g_state.RndSeed.d) & 0xFF;
    g_state.RndSeed.b = (uint8_t)a;
    g_state.RndSeed.d = (uint8_t)x;
    return a;
}

/**
 * @brief Helper to print the planet name with the first letter uppercase and the rest lowercase.
 */
static inline void goat_soup_print_planet_name(const struct plan_sys_t *planet_system) {
    int i = 1;
    printf("%c", planet_system->name[0]);
    while (planet_system->name[i] != '\0') {
        printf("%c", tolower(planet_system->name[i++]));
    }
}

/**
 * @brief Helper to print the planet name in adjective form (e.g. Martian).
 */
static inline void goat_soup_print_adjective_name(const struct plan_sys_t *planet_system) {
    int i = 1;
    printf("%c", planet_system->name[0]);
    while (planet_system->name[i] != '\0') {
        if ((planet_system->name[i + 1] != '\0') ||
            ((planet_system->name[i] != 'E') && (planet_system->name[i] != 'I'))) {
            printf("%c", tolower(planet_system->name[i]));
        }
        i++;
    }
    printf("ian");
}

/**
 * @brief Helper to print a random planet-like name using character pairs.
 */
static inline void goat_soup_print_random_name(void) {
    int len = gen_rnd_number() & 3;
    const size_t MAX_PAIRS = (sizeof(PLANET_NAME_PAIRS) / 2) - 1;
    for (int i = 0; i <= len; i++) {
        // The random name generation uses the same pairs as planet naming.
        // The index is carefully calculated to prevent out-of-bounds access.
        size_t x = 2 * ((size_t)gen_rnd_number() % MAX_PAIRS);
        if (i == 0) {
            printf("%c", PLANET_NAME_PAIRS[x]);
        } else {
            printf("%c", tolower(PLANET_NAME_PAIRS[x]));
        }
        printf("%c", tolower(PLANET_NAME_PAIRS[x + 1]));
    }
}

/**
 * @brief Processes a template string and generates descriptive text for a planet.
 *
 * This function implements the "Goat Soup" algorithm from Elite, which generates
 * descriptive text by processing template strings with special codes:
 * - ASCII characters (< 0x80) are printed directly
 * - Codes between 0x81-0xA4 select random text options from descList
 * - Special codes 0xB0-0xB2 insert planet-specific content:
 *   - 0xB0: planet_tname (first letter capital, rest lowercase)
 *   - 0xB1: planet_tname in adjective form (e.g., "Martian")
 *   - 0xB2: Random name generated using character pairs
 *
 * The function recursively processes templates, allowing for nested text generation.
 *
 * @param source_string The template string to process
 * @param planet_system Pointer to the planet system data structure
 */
// NOLINTNEXTLINE(misc-no-recursion)
static inline void goat_soup(const char *source_string, const struct plan_sys_t *planet_system) {
    for (;;) {
        int c = (unsigned char)*(source_string++);
        if (c == '\0') {
            break;
        }
        if (c < 0x80) {
            printf("%c", c);
        } else if (c <= 0xA4) {
            int rnd = gen_rnd_number();
            goat_soup(g_desc_list[c - 0x81].options[(rnd >= 0x33) + (rnd >= 0x66) + (rnd >= 0x99) + (rnd >= 0xCC)],
                      planet_system);
        } else {
            switch (c) {
            case 0xB0: /* planet name */
                goat_soup_print_planet_name(planet_system);
                break;
            case 0xB1: /* <planet name>ian */
                goat_soup_print_adjective_name(planet_system);
                break;
            case 0xB2: /* random name */
                goat_soup_print_random_name();
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
 * The format of the output depends on the `use_compressed_output` parameter.
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
 * @param planet_system_info The planetary system information structure to be displayed
 * @param use_compressed_output If true, outputs in single-line format; if false, outputs in detailed format
 *
 * @note Relies on global arrays g_econ_names and g_gov_names from elite_state.h
 * @note When using detailed format, modifies the global RndSeed variable for goat_soup generation
 */
[[maybe_unused]] static inline void print_system_info(struct plan_sys_t planet_system_info, bool use_compressed_output) {
    if (use_compressed_output) {
        printf("%10s", planet_system_info.name);
        printf(" TL: %2i ", (planet_system_info.techLev) + 1);
        printf("%12s", g_econ_names[planet_system_info.economy]); // g_econ_names from elite_state.h
        printf(" %15s", g_gov_names[planet_system_info.govType]); // g_gov_names from elite_state.h
    } else {
        printf("\n\nSystem:  ");
        printf("%s", planet_system_info.name);
        printf("\nPosition (%i,", planet_system_info.x);
        printf("%i)", planet_system_info.y);
        printf("\nEconomy: (%i) ", planet_system_info.economy);
        printf("%s", g_econ_names[planet_system_info.economy]); // g_econ_names from elite_state.h
        printf("\nGovernment: (%i) ", planet_system_info.govType);
        printf("%s", g_gov_names[planet_system_info.govType]); // g_gov_names from elite_state.h
        printf("\nTech Level: %2i", (planet_system_info.techLev) + 1);
        printf("\nTurnover: %u", (planet_system_info.productivity));
        printf("\nRadius: %u", planet_system_info.radius);
        printf("\nPopulation: %u Billion", (planet_system_info.population) >> 3);

        g_state.RndSeed = planet_system_info.goatSoupSeed; // RndSeed is global
        printf("\n");
        goat_soup("\x8F is \x97.", &planet_system_info);
    }
}