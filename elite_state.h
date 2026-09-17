#pragma once

#include "platform_compat.h"
#define MAX_CARGO_ITEMS 50
#define MAX_EQUIPMENT_SLOTS 10
/**
 * ELITE STATE HEADER
 *
 * This file consolidates the global state, constants, and data structures
 * used throughout the Text Elite game. It combines the functionality previously
 * spread across elite_includes.h, elite_structs.h, and elite_globals.h.
 */

// =====================================
// Standard Library Includes
// =====================================
#include <assert.h> // For static_assert
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =====================================
// Type Definitions
// =====================================
typedef uint16_t planet_num_t; // For planet/system indexing

// =====================================
// Game Constants
// =====================================
// Core Game Defines
#define MAX_LEN 30   // General string length
#define GAL_SIZE 256 // Galaxy size
#define TONNES_UNIT 0
#define KILOGRAM_UNIT 1
#define GRAM_UNIT 2

// Commodity-related constants
#define NUM_STANDARD_COMMODITIES 10                // Number of defined standard commodities (indices 0-9)
#define LAST_TRADE 16                              // Max index for normal trade goods display/loops (0-16)
#define ALIEN_ITEMS_IDX 17                         // Specific index for Alien Items, follows after LAST_TRADE items
#define COMMODITY_ARRAY_SIZE (ALIEN_ITEMS_IDX + 1) // Total size for commodity-related arrays

// Other game constants
#define MAX_CARGO_ITEMS 50
#define MAX_EQUIPMENT_SLOTS 10
#define NUM_COMMANDS 35  // Number of commands in the commands array
#define GOV_MAX_COUNT 8  // Number of government types
#define ECON_MAX_COUNT 8 // Number of economy types
#define MAX_MISSIONS 10

// planet_tsystem constants
#define NUM_FOR_LAVE 7 // Lave is 7th generated planet in galaxy one
#define NUM_FOR_ZAONCE 129
#define NUM_FOR_DISO 147
#define NUM_FOR_RIED 46

// Game balance constants
#define FUEL_COST 2 // Credits per 0.1 LY
#define MAX_FUEL 70 // 7.0 LY maximum fuel capacity

// Static assertions for critical sizes
static_assert(GAL_SIZE == 256, "Galaxy size must be 256");

// =====================================
// Data Structures
// =====================================
// Forward declarations
struct fast_seed_type_t;

// Navigation State is defined in elite_navigation_types.h to avoid circular dependencies
// The actual definition includes CelestialType enum and location union
// for complex in-system navigation functionality

// Struct definitions
struct fast_seed_type_t {
    uint8_t a, b, c, d;
};

struct seed_type_t {
    uint16_t a;
    uint16_t b;
    uint16_t c;
    uint16_t d;
};

struct plan_sys_t {
    uint16_t x;
    uint16_t y;
    uint16_t economy;
    uint16_t govType;
    uint16_t techLev;
    uint16_t population;
    uint16_t productivity;
    uint16_t radius;
    struct fast_seed_type_t goatSoupSeed;
    char name[12];
};

typedef struct {
    uint16_t basePrice;
    int16_t gradient;
    uint16_t baseQuant;
    uint16_t maskByte;
    uint16_t units;
    char name[20];
} trade_good_t;

typedef struct {
    uint16_t quantity[COMMODITY_ARRAY_SIZE];
    uint16_t price[COMMODITY_ARRAY_SIZE];
} market_type_t;

// =====================================
// Global Variables
// =====================================

#include "elite_navigation_types.h"

/**
 * Encapsulates the global game state to improve organization and modularity.
 */
typedef struct {
    int ExitStatus;
    bool NativeRand;

    struct plan_sys_t Galaxy[GAL_SIZE];
    struct seed_type_t SEED;
    struct fast_seed_type_t RndSeed;

    int CurrentPlanet;
    uint16_t GalaxyNum;
    int32_t Cash;
    uint16_t Fuel;
    market_type_t LocalMarket;

    uint64_t currentGameTimeSeconds;

    struct StarSystem *CurrentStarSystem;
    struct navigation_state_t PlayerNavState;

    struct player_ship_t *PlayerShipPtr;

    char CurrentSystemName[20];
    int CurrentSystemEconomy;
    int PlayerLocationType;
    bool InCombat;

    char tradnames[LAST_TRADE + 1][MAX_LEN];
} game_state_t;

extern game_state_t g_state;

// Base seeds for galaxy generation (constant)
extern const uint16_t BASE_0;
extern const uint16_t BASE_1;
extern const uint16_t BASE_2;

// Names and descriptors (read-only lookup tables)
extern char g_gov_names[GOV_MAX_COUNT][MAX_LEN];
extern char g_econ_names[ECON_MAX_COUNT][MAX_LEN];

// Function declarations (implementations are later in this file or in txtelite.c)
int get_fuel_cost(void);
int get_max_fuel(void);

// --- Game Time Functions ---

/**
 * Initializes the game time to zero.
 */
[[maybe_unused]] static inline void game_time_initialize(void) { g_state.currentGameTimeSeconds = 0; }

/**
 * Advances the game time by a specified number of seconds.
 * @param seconds_to_add The number of seconds to add to the current game time.
 */
[[maybe_unused]] static inline void game_time_advance(uint32_t seconds_to_add) {
    if (seconds_to_add > 0) {
        g_state.currentGameTimeSeconds += seconds_to_add;
    }
}

/**
 * Gets the current total game time in seconds.
 * @return The current game time in seconds.
 */
[[maybe_unused]] static inline uint64_t game_time_get_seconds(void) { return g_state.currentGameTimeSeconds; }

/**
 * Formats the current game time into a human-readable string.
 * Format: "Year: Y, Day: D, HH:MM:SS"
 * @param buffer The character buffer to write the formatted time string to.
 * @param buffer_size The size of the buffer.
 */
[[maybe_unused]] static inline void game_time_get_formatted(char *buffer, size_t buffer_size) {
    if (buffer == nullptr || buffer_size == 0) {
        return;
    }

    uint64_t time_val = g_state.currentGameTimeSeconds;

    const uint64_t SECS_IN_MINUTE = 60;
    const uint64_t SECS_IN_HOUR = 60 * SECS_IN_MINUTE;
    const uint64_t SECS_IN_DAY = 24 * SECS_IN_HOUR;
    // Using a simplified year for game purposes.
    const uint64_t SECS_IN_YEAR = 365 * SECS_IN_DAY;

    uint64_t years = time_val / SECS_IN_YEAR;
    time_val %= SECS_IN_YEAR;

    uint64_t days = time_val / SECS_IN_DAY;
    time_val %= SECS_IN_DAY;

    uint64_t hours = time_val / SECS_IN_HOUR;
    time_val %= SECS_IN_HOUR;

    uint64_t minutes = time_val / SECS_IN_MINUTE;
    time_val %= SECS_IN_MINUTE;

    uint64_t current_seconds = time_val;

    safe_snprintf(buffer, buffer_size, "Year: %llu, Day: %llu, %02llu:%02llu:%02llu", (unsigned long long)years,
                  (unsigned long long)days, (unsigned long long)hours, (unsigned long long)minutes,
                  (unsigned long long)current_seconds);
}
