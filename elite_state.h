#pragma once

// Other game constants
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h> // For static_assert

// =====================================
// Type Definitions
// =====================================
typedef uint16_t PlanetNum; // For planet/system indexing

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
#define NUM_COMMANDS 29  // Number of commands in the commands array
#define GOV_MAX_COUNT 8  // Number of government types
#define ECON_MAX_COUNT 8 // Number of economy types
#define MAX_MISSIONS 10

// Planet system constants
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
struct FastSeedType;
struct StarSystem;
struct NavigationState;
struct PlayerShip;

// Struct definitions
struct FastSeedType
{
    uint8_t a, b, c, d;
};

struct SeedType
{
    uint16_t a;
    uint16_t b;
    uint16_t c;
    uint16_t d;
};

struct PlanSys
{
    uint16_t x;
    uint16_t y;
    uint16_t economy;
    uint16_t govType;
    uint16_t techLev;
    uint16_t population;
    uint16_t productivity;
    uint16_t radius;
    struct FastSeedType goatSoupSeed;
    char name[12];
};

typedef struct
{
    uint16_t basePrice;
    int16_t gradient;
    uint16_t baseQuant;
    uint16_t maskByte;
    uint16_t units;
    char name[20];
} TradeGood;

typedef struct
{
    uint16_t quantity[COMMODITY_ARRAY_SIZE];
    uint16_t price[COMMODITY_ARRAY_SIZE];
} MarketType;

// =====================================
// Global Variables
// =====================================
// System state
extern int ExitStatus;
extern bool NativeRand;

// Galaxy and seed data
extern struct PlanSys Galaxy[GAL_SIZE];
extern struct SeedType Seed;
extern struct FastSeedType RndSeed;

// Base seeds for galaxy generation
extern const uint16_t BASE_0;
extern const uint16_t BASE_1;
extern const uint16_t BASE_2;

// Player state
extern uint16_t ShipHold[COMMODITY_ARRAY_SIZE];
extern int CurrentPlanet;
extern uint16_t GalaxyNum;
extern int32_t Cash;
extern uint16_t Fuel;
extern MarketType LocalMarket;
extern uint16_t HoldSpace;
extern int FuelCost;
extern int MaxFuel;

// Star System Navigation State
extern struct StarSystem *CurrentStarSystem;
extern struct NavigationState PlayerNavState;

// Names and descriptors
extern char GovNames[GOV_MAX_COUNT][MAX_LEN];
extern char EconNames[ECON_MAX_COUNT][MAX_LEN];
extern char tradnames[LAST_TRADE + 1][MAX_LEN];

// Declaration of the global game time variable (in seconds)
extern uint64_t currentGameTimeSeconds;

// --- Game Time Functions ---

/**
 * Initializes the game time to zero.
 * Should be called when starting a new game.
 */
static inline void game_time_initialize(void)
{
    currentGameTimeSeconds = 0;
}

/**
 * Advances the game time by a specified number of seconds.
 * @param seconds_to_add The number of seconds to add to the current game time.
 */
static inline void game_time_advance(uint32_t seconds_to_add)
{
    if (seconds_to_add > 0)
    { // Basic check
        currentGameTimeSeconds += seconds_to_add;
    }
}

/**
 * Gets the current total game time in seconds.
 * @return The current game time in seconds.
 */
static inline uint64_t game_time_get_seconds(void)
{
    return currentGameTimeSeconds;
}

/**
 * Formats the current game time into a human-readable string.
 * Format: "Year: Y, Day: D, HH:MM:SS"
 * @param buffer The character buffer to write the formatted time string to.
 * @param buffer_size The size of the buffer.
 */
static inline void game_time_get_formatted(char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0)
    {
        return;
    }

    uint64_t time_val = currentGameTimeSeconds;

    const uint64_t secs_in_minute = 60;
    const uint64_t secs_in_hour = 60 * secs_in_minute;
    const uint64_t secs_in_day = 24 * secs_in_hour;
    // Using a simplified year for game purposes.
    const uint64_t secs_in_year = 365 * secs_in_day;

    uint64_t years = time_val / secs_in_year;
    time_val %= secs_in_year;

    uint64_t days = time_val / secs_in_day;
    time_val %= secs_in_day;

    uint64_t hours = time_val / secs_in_hour;
    time_val %= secs_in_hour;

    uint64_t minutes = time_val / secs_in_minute;
    time_val %= secs_in_minute;

    uint64_t current_seconds = time_val;

    snprintf(buffer, buffer_size, "Year: %llu, Day: %llu, %02llu:%02llu:%02llu",
             (unsigned long long)years, (unsigned long long)days,
             (unsigned long long)hours, (unsigned long long)minutes, (unsigned long long)current_seconds);
}

// =====================================
// Global Variable Definitions
// =====================================
// These definitions are included here rather than in a separate .c file
// to maintain the header-only architecture of the codebase

int ExitStatus = EXIT_SUCCESS;
bool NativeRand;

struct PlanSys Galaxy[GAL_SIZE];
struct SeedType Seed;
struct FastSeedType RndSeed;

const uint16_t BASE_0 = 0x5A4A;
const uint16_t BASE_1 = 0x0248;
const uint16_t BASE_2 = 0xB753; /* Base seed for galaxy 1 */

uint16_t ShipHold[COMMODITY_ARRAY_SIZE];
int CurrentPlanet;
uint16_t GalaxyNum;
int32_t Cash;
uint16_t Fuel;
MarketType LocalMarket;
uint16_t HoldSpace;

int FuelCost = 2;
int MaxFuel = 70;

char GovNames[GOV_MAX_COUNT][MAX_LEN] = {
    "Anarchy", "Feudal", "Multi-gov", "Dictatorship",
    "Communist", "Confederacy", "Democracy", "Corporate State"};

char EconNames[ECON_MAX_COUNT][MAX_LEN] = {
    "Rich Ind", "Average Ind", "Poor Ind", "Mainly Ind",
    "Mainly Agri", "Rich Agri", "Average Agri", "Poor Agri"};

char tradnames[LAST_TRADE + 1][MAX_LEN];

uint64_t currentGameTimeSeconds;

// Star System Navigation State
struct StarSystem *CurrentStarSystem = NULL;
struct NavigationState PlayerNavState;

// Player Ship
struct PlayerShip *PlayerShipPtr = NULL;

// Combat state
bool InCombat = false;
