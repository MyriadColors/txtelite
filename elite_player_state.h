#pragma once

#include "elite_navigation_types.h" // For NavigationState definition
#include "elite_state.h"            // Unified header for constants, structures, and globals
#include "elite_market.h"           // For generate_market(), init_tradnames()
#include "elite_galaxy.h"           // For build_galaxy_data()
#include "elite_utils.h"            // For minimum_value
#include "elite_star_system.h"      // For StarSystem
#include "elite_ship_types.h"       // For PlayerShip structure and functions

extern struct NavigationState PlayerNavState;
extern struct StarSystem *CurrentStarSystem;

// Forward declarations
static inline void initialize_star_system_for_current_planet(void);

// Initializes the player's state at the beginning of the game.
static inline void initialize_player_state(void)
{
    // Set initial seed for Galaxy 1
    // SeedType has a, b, c, d members as defined in elite_structs.h
    Seed.a = BASE_0; // 0x5A4A
    Seed.b = BASE_1; // 0x0248
    Seed.c = BASE_2; // 0xB753
    Seed.d = BASE_2; // Match original seed for Galaxy 1 (d was same as c)

    NativeRand = false; // Set to false as per original logic for predictable generation initially
    GalaxyNum = 1;      // Start in Galaxy 1

    // Populate Galaxy[] array for the current GalaxyNum using the Seed
    build_galaxy_data(Seed);      // Set current planet to Lave (planet 7 in galaxy 1)    CurrentPlanet = NUM_FOR_LAVE; // NUM_FOR_LAVE is defined in elite_state.h

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

    // Initialize star system for the current planet
    initialize_star_system_for_current_planet();
}

// Cleanup player ship resources
static inline void cleanup_player_ship(void)
{
    if (PlayerShipPtr != NULL)
    {
        // Any additional cleanup for ship resources would go here
        
        free(PlayerShipPtr);
        PlayerShipPtr = NULL;
    }
}

// Initializes the star system for the current planet.
// Call this when switching planets (e.g., after hyperspace jumps).
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

// Displays a brief summary of the ship status
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
    int hullPercentage = (PlayerShipPtr->attributes.hullStrength * 100) / COBRA_MK3_BASE_HULL_STRENGTH;
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

// Calculates how much fuel can be bought with the available cash.
// Moved from elite_commands.h as it's player-state related.
// Relies on global Cash and FuelCost.
// Parameter desiredAmount: How much fuel the player wants to buy (in 0.1 LY units)
// Returns the actual amount that can be purchased (limited by cash)
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
