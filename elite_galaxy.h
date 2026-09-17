#pragma once

#include "elite_state.h" // Unified header for constants, structures, and globals
#include "elite_utils.h" // For tweak_seed
#include <assert.h>      // For debug assertions
#include <stdint.h>

// Cleaner, dot-free planet naming pairs.
static const char PLANET_NAME_PAIRS[] = "LEXEGEZACEBISO"
                                        "USESARMAINDIREA"
                                        "ERATENBERALAVETI"
                                        "EDORQUANTEISRION";

/**
 * @brief Rotates an 8-bit number leftwards, mimicking 6502 ROL instruction.
 * @param value The byte to rotate.
 * @return The rotated byte.
 */
static inline uint8_t rotate_left(uint8_t value) {
    // A 6502 ROL instruction shifts all bits left, moving bit 7 to the Carry
    // flag and the Carry flag into bit 0. This implementation simplifies it
    // by rotating bit 7 directly into bit 0.
    return (uint8_t)(((uint16_t)value << 1) | (value >> 7));
}

/**
 * @brief "Twists" a 16-bit number by rotating each byte leftwards.
 * This is a key part of the galaxy generation algorithm.
 * @param value The 16-bit value to twist.
 * @return The twisted value.
 */
static inline uint16_t twist(uint16_t value) {
    uint8_t high_byte = (uint8_t)(value >> 8);
    uint8_t low_byte = (uint8_t)(value & 0xFF);
    return (uint16_t)(rotate_left(high_byte) << 8) | rotate_left(low_byte);
}

/**
 * @brief Advances the seed to generate the next galaxy.
 * Each call to this function transitions to the next galaxy in a cycle of 8.
 * @param seed A pointer to the seed to be modified.
 */
[[maybe_unused]] static inline void next_galaxy(struct seed_type_t *seed) {
    assert(seed != nullptr);
    seed->a = twist(seed->a);
    seed->b = twist(seed->b);
    seed->c = twist(seed->c);
    seed->d = twist(seed->d);
}

/**
 * @brief Generates a planetary system from a seed.
 * This function is pure; it does not modify the input seed. It creates a local
 * copy to ensure the generation process is predictable and free of side effects.
 * @param seed A constant pointer to the initial seed.
 * @return A fully generated plan_sys_t structure.
 */
static inline struct plan_sys_t make_system(const struct seed_type_t *seed) {
    assert(seed != nullptr);
    struct plan_sys_t system;
    struct seed_type_t local_seed = *seed; // Use a local copy for generation

    // --- System Coordinates and Basic Attributes ---
    const bool LONG_NAME_FLAG = (local_seed.a & 0x40) != 0; // Bit 6 of seed.a
    system.x = local_seed.b >> 8;
    system.y = local_seed.a >> 8;

    // --- Government and Economy ---
    // Gov type is bits 3-5 of seed.b
    system.govType = (local_seed.b >> 3) & 0x07;
    // Economy is bits 8-10 (high byte) of seed.a
    system.economy = (local_seed.a >> 8) & 0x07;
    // Anarchic systems (govType 0 or 1) have at least a poor industrial base
    if (system.govType <= 1) {
        system.economy |= 0x02;
    }

    // --- Tech Level ---
    // Base tech level from seed.b and economy
    system.techLev = ((local_seed.b >> 8) & 0x03) + (system.economy ^ 0x07);
    // Modified by government type
    system.techLev += (system.govType >> 1) + (system.govType & 0x01);

    // --- Population, Productivity, and Radius ---
    system.population = (4 * system.techLev) + system.economy + system.govType + 1;
    system.productivity = ((system.economy ^ 0x07) + 3) * (system.govType + 4) * system.population * 8;
    system.radius = (256 * (((local_seed.c >> 8) & 0x0F) + 11)) + system.x;

    // --- "Goat Soup" Special Seed ---
    // This is a special seed passed to the planet description generator.
    system.goatSoupSeed.a = local_seed.b & 0xFF;
    system.goatSoupSeed.b = local_seed.b >> 8;
    system.goatSoupSeed.c = local_seed.c & 0xFF;
    system.goatSoupSeed.d = local_seed.c >> 8;

    // --- planet_tName Generation ---
    char *name_ptr = system.name;
    const int NUM_PAIRS = (int)LONG_NAME_FLAG ? 4 : 3;
    for (int i = 0; i < NUM_PAIRS; ++i) {
        uint8_t pair_index = 2 * ((local_seed.c >> 8) & 0x1F); // Get a value from 0-62
        *name_ptr++ = PLANET_NAME_PAIRS[pair_index];
        *name_ptr++ = PLANET_NAME_PAIRS[pair_index + 1];
        tweak_seed(&local_seed); // "Stir" the seed for the next pair
    }
    *name_ptr = '\0'; // nullptr-terminate the string

    return system;
}

/**
 * @brief Populates the global Galaxy array with systems generated from a seed.
 * @param seed The initial seed for the galaxy.
 */
[[maybe_unused]] static inline void build_galaxy_data(struct seed_type_t seed) {
    g_state.SEED = seed;
    for (uint16_t i = 0; i < GAL_SIZE; ++i) {
        g_state.Galaxy[i] = make_system(&g_state.SEED);
        tweak_seed(&g_state.SEED); // Advance the global seed for the next system
    }
}
