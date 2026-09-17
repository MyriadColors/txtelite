#pragma once

#include <stdint.h>
#include <stdio.h>  // For printf()
#include <stdlib.h> // For malloc() and free()
#include <string.h> // For memset()

#include "elite_equipment_constants.h" // For equipment mapping functions
#include "elite_galaxy.h"              // For build_galaxy_data()
#include "elite_market.h"              // For generate_market(), init_tradnames()
#include "elite_navigation.h"          // For NavigationState definition
#include "elite_navigation_types.h"
#include "elite_player_ship.h"
#include "elite_star_system.h"         // For StarSystem
#include "elite_state.h"               // Unified header for constants, structures, and globals
#include "elite_utils.h"               // For minimum_value

/**
 * @brief Initializes the player's state at the start of a new game.
 *
 * This function sets up all the initial player state for the game, including:
 * - Setting the random seed for Galaxy 1 generation
 * - Building the galaxy data based on the seed
 * - Setting the current planet to Lave (starting planet)
 * - Generating the local market for the starting planet
 * - Initializing player resources:
 *   - Maximum fuel (7.0 LY)
 *   - Starting credits (100.0 credits)
 *   - Initial cargo capacity (20 tons)
 *   - Empty cargo hold
 * - Initializing trade commodity names for command parsing
 * - Allocating and initializing the player's ship as a Cobra Mk III
 * - Synchronizing ship properties with global game state
 * - Setting up the star system for the current planet
 *
 * @note This function allocates memory for the PlayerShipPtr which must be freed
 *       when no longer needed to prevent memory leaks.
 *
 * @warning If memory allocation fails for the player ship, an error message is
 *          printed and the function returns without completing initialization.
 */
[[maybe_unused]] static inline void initialize_player_state(void) {
    // Set initial seed for Galaxy 1, but allow customization from my_rand seed
    // Use the current state of my_rand to derive galaxy seed values
    uint32_t rand_seed = (uint8_t)my_rand();

    // SeedType has a, b, c, d members as defined in elite_structs.h
    g_state.SEED.a = BASE_0 ^ (rand_seed & 0xFFFF); // XOR with random value to vary galaxy
    g_state.SEED.b = BASE_1 ^ ((rand_seed >> 8) & 0xFFFF);
    g_state.SEED.c = BASE_2 ^ ((rand_seed >> 16) & 0xFFFF);
    g_state.SEED.d = BASE_2 ^ ((rand_seed >> 24) & 0xFFFF);

    // Also initialize RndSeed structure for deterministic randomness within the galaxy
    g_state.RndSeed.a = (uint8_t)(rand_seed & 0xFF);
    g_state.RndSeed.b = (uint8_t)((rand_seed >> 8) & 0xFF);
    g_state.RndSeed.c = (uint8_t)((rand_seed >> 16) & 0xFF);
    g_state.RndSeed.d = (uint8_t)((rand_seed >> 24) & 0xFF);

    g_state.NativeRand = false; // Set to 0 as per original logic for predictable generation initially
    g_state.GalaxyNum = 1;  // Start in Galaxy 1

    // Populate Galaxy[] array for the current GalaxyNum using the Seed
    build_galaxy_data(g_state.SEED);

    // Set current planet to Lave (planet 7 in galaxy 1)
    g_state.CurrentPlanet = NUM_FOR_LAVE; // NUM_FOR_LAVE is defined in elite_state.h

    // Populate LocalMarket for the starting planet. Use random fluctuation instead of 0.
    // Galaxy[CurrentPlanet] is now valid after build_galaxy_data()
    g_state.LocalMarket = generate_market(random_byte(), g_state.Galaxy[g_state.CurrentPlanet]);

    // Initialize player ship
    if (g_state.PlayerShipPtr != nullptr) {
        free(g_state.PlayerShipPtr);
    }

    g_state.PlayerShipPtr = (player_ship_t*)malloc(sizeof(player_ship_t));
    if (g_state.PlayerShipPtr == nullptr) {
        printf("Error: Could not allocate memory for player ship!\n");
        return;
    }

    initialize_cobra_mk_iii(g_state.PlayerShipPtr);

    // Cobra Mk III maximum fuel is 7.0 light-years (stored in tenths).
    g_state.Fuel = 70;

    g_state.Cash = 1000; // Start with 100.0 credits (1000 internal units)

    // Initialize the tradenames array for command parsing
    init_tradnames();

    // Synchronize ship fuel with global state
    g_state.PlayerShipPtr->attributes.fuelLiters = g_state.Fuel * 10.0; // Convert game units to liters

    // Map equipment indices for quick status checks
    map_equipment_indices(g_state.PlayerShipPtr);

    // Initialize star system for the current planet
    initialize_star_system_for_current_planet();
}

/**
 * @brief Releases memory allocated for the player's ship.
 *
 * This function checks if PlayerShipPtr is not nullptr, frees the memory allocated
 * for the player's ship, and then sets the pointer to nullptr to avoid any dangling
 * pointer issues. Any additional resource cleanup related to the ship would be
 * performed within this function.
 *
 * @note This function is defined as inline to reduce function call overhead.
 */
[[maybe_unused]] static inline void cleanup_player_ship(void) {
    if (g_state.PlayerShipPtr != nullptr) {
        // Any additional cleanup for ship resources would go here

        free(g_state.PlayerShipPtr);
        g_state.PlayerShipPtr = nullptr;
    }
}

/**
 * @brief Initializes or reinitializes the current star system based on the player's current planet.
 *
 * This function performs the following operations:
 * 1. Cleans up and frees any existing star system
 * 2. Allocates memory for a new star system
 * 3. Initializes the star system with data from the current planet in the galaxy
 * 4. Sets up the player's navigation state to start at the main planet
 *
 * The function ensures proper memory management by freeing previously allocated
 * star system resources before allocating new ones. If memory allocation fails,
 * an error message is printed and the function returns without completing initialization.
 *
 * After successful initialization, the player's position is set to the main planet
 * of the system with appropriate distance values.
 *
 * @note This function assumes that CurrentPlanet and Galaxy are valid global variables.
 * @note PlayerNavState is reset completely before being initialized with new values.
 */
static inline void initialize_star_system_for_current_planet(void) {
    // Clean up any existing star system
    if (g_state.CurrentStarSystem != nullptr) {
        cleanup_star_system(g_state.CurrentStarSystem);
        free(g_state.CurrentStarSystem);
        g_state.CurrentStarSystem = nullptr;
    }

    // Allocate a new star system
    g_state.CurrentStarSystem = (struct StarSystem *)malloc(sizeof(struct StarSystem));
    if (g_state.CurrentStarSystem == nullptr) {
        printf("Error: Could not allocate memory for star system!\n");
        return;
    }

    // Initialize the star system with the current planet's data
    initialize_star_system(g_state.CurrentStarSystem, &g_state.Galaxy[g_state.CurrentPlanet]);

    // Initialize navigation state - default position at the main planet
    memset(&g_state.PlayerNavState, 0, sizeof(g_state.PlayerNavState));
    g_state.PlayerNavState.currentLocationType = CELESTIAL_PLANET;
    g_state.PlayerNavState.currentLocation.planet = get_planet_by_index(g_state.CurrentStarSystem, 0);
    g_state.PlayerNavState.distanceFromStar = g_state.PlayerNavState.currentLocation.planet->orbitalDistance;
}

/**
 * @brief Displays a brief summary of the player's ship status
 * * This function prints a concise status line containing:
 * - Ship name and class
 * - Hull integrity as percentage of base strength
 * - Fuel level in light years (converted from liters)
 * - Cargo hold utilization (current/maximum capacity)
 *
 * If the PlayerShipPtr is nullptr, an error message is displayed instead.
 *
 * @note Assumes COBRA_MK3_BASE_HULL_STRENGTH is defined
 * @note Assumes 100 liters of fuel equals 1 light year of range
 */
[[maybe_unused]] static inline void display_ship_status_brief(void) {
    if (g_state.PlayerShipPtr == nullptr) {
        printf("\nError: Ship data is not available.\n");
        return;
    }

    printf("\nShip: %s (%s) - ", g_state.PlayerShipPtr->shipName, g_state.PlayerShipPtr->shipClassName);

    // Calculate hull percentage
    int hull_percentage =
        (g_state.PlayerShipPtr->attributes.hullStrength * 100) / g_state.PlayerShipPtr->ship_type_t->baseHullStrength;
    printf("Hull: %d%% - ", hull_percentage);
    // Display fuel information including consumption rate
    double current_fuel_ly = g_state.PlayerShipPtr->attributes.fuelLiters / 100.0;
    double max_fuel_ly = g_state.PlayerShipPtr->ship_type_t->maxFuelLY;
    double fuel_percent = (current_fuel_ly / max_fuel_ly) * 100.0;

    printf("Fuel: %.1f/%.1f LY (%.0F%%) - ", current_fuel_ly, max_fuel_ly, fuel_percent);

    // Show cargo capacity
    printf("Cargo: %d/%d tons", g_state.PlayerShipPtr->attributes.currentCargoTons,
           g_state.PlayerShipPtr->attributes.cargoCapacityTons);
}

/**
 * @brief Calculates the maximum amount of fuel that can be purchased.
 *
 * This function determines how much fuel the player can actually purchase
 * based on their available cash and the desired amount. It takes into account
 * the current fuel cost and prevents purchasing more than the player can afford.
 *
 * @param desiredAmount The amount of fuel units the player wishes to purchase (in 0.1 LY units)
 * @return The actual number of fuel units that can be purchased:
 *         - Returns 0 if fuel cost is invalid (<=0) or player has no cash
 *         - Returns the desired amount if the player can afford it
 *         - Returns the maximum affordable amount otherwise
 */
[[maybe_unused]] static inline uint16_t calculate_fuel_purchase(uint16_t desired_amount) {
    int current_fuel_cost = get_fuel_cost();
    if (current_fuel_cost <= 0) {
        return 0; // Avoid division by zero if FuelCost is invalid
}
    if (g_state.Cash <= 0) {
        return 0; // No cash, no fuel
}

    // Calculate fuel units player can afford (1 unit = 0.1 LY)
    uint16_t affordable_fuel_units = (uint16_t)((double)g_state.Cash / current_fuel_cost);

    // Return the minimum of what's desired and what's affordable
    return (desired_amount < affordable_fuel_units) ? desired_amount : affordable_fuel_units;
}

/**
 * @brief Displays detailed fuel information for the player's ship.
 *
 * This function shows the current fuel level, maximum capacity, consumption rate,
 * and estimated range based on the ship's specifications.
 */
[[maybe_unused]] static inline void display_ship_fuel_status(void) {
    if (g_state.PlayerShipPtr == nullptr) {
        printf("\nError: Ship data is not available.\n");
        return;
    }

    double current_fuel_ly = g_state.PlayerShipPtr->attributes.fuelLiters / 100.0;
    double max_fuel_ly = g_state.PlayerShipPtr->ship_type_t->maxFuelLY;
    double fuel_percent = (current_fuel_ly / max_fuel_ly) * 100.0;
    double consumption_rate = g_state.PlayerShipPtr->ship_type_t->fuelConsumptionRate;

    printf("\n=== Fuel Status ===\n");
    printf("Current fuel:     %.1f LY (%.0F%%)\n", current_fuel_ly, fuel_percent);
    printf("Maximum capacity: %.1f LY\n", max_fuel_ly);
    printf("Consumption rate: %.1f credits per 0.1 LY\n", consumption_rate / 10.0);

    // Calculate estimated maximum range based on current fuel and consumption rate
    // The formula accounts for the ship's efficiency
    double efficiency = 2.0 / consumption_rate; // Higher efficiency = better fuel economy
    double estimated_range = current_fuel_ly * efficiency;

    printf("Estimated range:  %.1f LY at current efficiency\n", estimated_range);

    // Display cost to refill
    double fuel_needed = max_fuel_ly - current_fuel_ly;
    if (fuel_needed > 0) {
        int total_cost = (int)((fuel_needed * 10.0) * consumption_rate);
        printf("Cost to refill:   %d credits\n", total_cost);
    } else {
        printf("Fuel tanks are full\n");
    }
}
