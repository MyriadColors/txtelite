#pragma once

#include "elite_navigation.h"    // For NavigationState definition
#include "elite_state.h"            // Unified header for constants, structures, and globals
#include "elite_market.h"           // For generate_market(), init_tradnames()
#include "elite_galaxy.h"           // For build_galaxy_data()
#include "elite_utils.h"            // For minimum_value
#include "elite_star_system.h"      // For StarSystem
#include "elite_ship_types.h"       // For PlayerShip structure and functions
#include "elite_equipment_constants.h" // For equipment mapping functions

extern struct NavigationState PlayerNavState;
extern struct StarSystem *CurrentStarSystem;

// Forward declarations
static inline void initialize_star_system_for_current_planet(void);


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
static inline void initialize_player_state(void)
{
    // Set initial seed for Galaxy 1
    // SeedType has a, b, c, d members as defined in elite_structs.h
    SEED.a = BASE_0; // 0x5A4A
    SEED.b = BASE_1; // 0x0248
    SEED.c = BASE_2; // 0xB753
    SEED.d = BASE_2; // Match original seed for Galaxy 1 (d was same as c)

    NativeRand = false; // Set to false as per original logic for predictable generation initially
    GalaxyNum = 1;      // Start in Galaxy 1

    // Populate Galaxy[] array for the current GalaxyNum using the Seed
    build_galaxy_data(SEED);      // Set current planet to Lave (planet 7 in galaxy 1)    CurrentPlanet = NUM_FOR_LAVE; // NUM_FOR_LAVE is defined in elite_state.h

    // Populate LocalMarket for the starting planet. Fluctuation is 0 for initial state.
    // Galaxy[CurrentPlanet] is now valid after build_galaxy_data()
    LocalMarket = generate_market(0, Galaxy[CurrentPlanet]);

    Fuel = MaxFuel; // Start with maximum fuel (e.g., 7.0 LY)
    Cash = 1000;    // Start with 100.0 credits (1000 internal units)
    HoldSpace = 20; // Start with 20t hold space    // Zero out the player's ship hold
    // COMMODITY_ARRAY_SIZE is from elite_state.h
    for (int i = 0; i < COMMODITY_ARRAY_SIZE; i++)
    {
        ShipHold[i] = 0;
    }

    // Initialize the tradenames array for command parsing
    init_tradnames();
    
    // Initialize player ship
    if (PlayerShipPtr != NULL) {
        free(PlayerShipPtr);
    }
    
    PlayerShipPtr = (PlayerShip *)malloc(sizeof(PlayerShip));
    if (PlayerShipPtr == NULL) {
        printf("Error: Could not allocate memory for player ship!\n");
        return;
    }
    
    InitializeCobraMkIII(PlayerShipPtr);
    
    // Synchronize ship fuel with global state
    PlayerShipPtr->attributes.fuelLiters = Fuel * 10.0; // Convert game units to liters
    
    // Synchronize cargo capacity with HoldSpace
    PlayerShipPtr->attributes.cargoCapacityTons = HoldSpace;
    
    // Map equipment indices for quick status checks
    MapEquipmentIndices(PlayerShipPtr);

    // Initialize star system for the current planet
    initialize_star_system_for_current_planet();
}

/**
 * @brief Releases memory allocated for the player's ship.
 * 
 * This function checks if PlayerShipPtr is not NULL, frees the memory allocated
 * for the player's ship, and then sets the pointer to NULL to avoid any dangling
 * pointer issues. Any additional resource cleanup related to the ship would be
 * performed within this function.
 * 
 * @note This function is defined as inline to reduce function call overhead.
 */
static inline void cleanup_player_ship(void)
{
    if (PlayerShipPtr != NULL)
    {
        // Any additional cleanup for ship resources would go here
        
        free(PlayerShipPtr);
        PlayerShipPtr = NULL;
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
static inline void initialize_star_system_for_current_planet(void)
{
    // Clean up any existing star system
    if (CurrentStarSystem != NULL)
    {
        cleanup_star_system(CurrentStarSystem);
        free(CurrentStarSystem);
        CurrentStarSystem = NULL;
    }

    // Allocate a new star system
    CurrentStarSystem = (struct StarSystem *)malloc(sizeof(struct StarSystem));
    if (CurrentStarSystem == NULL)
    {
        printf("Error: Could not allocate memory for star system!\n");
        return;
    }

    // Initialize the star system with the current planet's data
    initialize_star_system(CurrentStarSystem, &Galaxy[CurrentPlanet]);

    // Initialize navigation state - default position at the main planet
    memset(&PlayerNavState, 0, sizeof(PlayerNavState));
    PlayerNavState.currentLocationType = CELESTIAL_PLANET;
    PlayerNavState.currentLocation.planet = get_planet_by_index(CurrentStarSystem, 0);
    PlayerNavState.distanceFromStar = PlayerNavState.currentLocation.planet->orbitalDistance;
}

/**
 * @brief Displays a brief summary of the player's ship status
 * 
 * This function prints a concise status line containing:
 * - Ship name and class
 * - Hull integrity as percentage of base strength
 * - Energy bank level (with one decimal place)
 * - Fuel level in light years (converted from liters)
 * - Cargo hold utilization (current/maximum capacity)
 * 
 * If the PlayerShipPtr is NULL, an error message is displayed instead.
 * 
 * @note Assumes COBRA_MK3_BASE_HULL_STRENGTH is defined
 * @note Assumes 100 liters of fuel equals 1 light year of range
 */

static inline void display_ship_status_brief(void)
{
    if (PlayerShipPtr == NULL)
    {
        printf("\nError: Ship data is not available.\n");
        return;
    }

    printf("\nShip: %s (%s) - ", 
           PlayerShipPtr->shipName, 
           PlayerShipPtr->shipClassName);
    
    // Calculate hull percentage
    int hullPercentage = (PlayerShipPtr->attributes.hullStrength * 100) / PlayerShipPtr->shipType->baseHullStrength;
    printf("Hull: %d%% - ", hullPercentage);
    
    // Round energy to one decimal place
    printf("Energy: %.1f - ", PlayerShipPtr->attributes.energyBanks);
    
    // Convert fuel liters to LY (assuming 100L = 1LY)
    printf("Fuel: %.1f LY - ", PlayerShipPtr->attributes.fuelLiters / 100.0);
    
    // Show cargo capacity
    printf("Cargo: %d/%d tons", 
           PlayerShipPtr->attributes.currentCargoTons,
           PlayerShipPtr->attributes.cargoCapacityTons);
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
static inline uint16_t calculate_fuel_purchase(uint16_t desiredAmount)
{
    if (FuelCost <= 0)
        return 0; // Avoid division by zero if FuelCost is invalid
    if (Cash <= 0)
        return 0; // No cash, no fuel

    // Calculate fuel units player can afford (1 unit = 0.1 LY)
    uint16_t affordable_fuel_units = (uint16_t)((double)Cash / FuelCost);

    // Return the minimum of what's desired and what's affordable
    return (desiredAmount < affordable_fuel_units) ? desiredAmount : affordable_fuel_units;
}
