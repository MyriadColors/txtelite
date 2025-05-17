#ifndef ELITE_NAVIGATION_H
#define ELITE_NAVIGATION_H

#include "elite_state.h" // Unified header for constants, structures, and globals
#include "elite_utils.h" // For float_to_int_round, random_byte, string_begins_with
#include "elite_market.h" // For generate_market
#include "elite_galaxy.h" // For Galaxy array functions
#include "elite_player_state.h" // For initialize_star_system_for_current_planet
#include <math.h>         // For sqrt

// Calculates the distance between two planetary systems.
// Depends on sqrt from math.h and float_to_int_round from elite_utils.h
static inline uint16_t distance(struct PlanSys systemA, struct PlanSys systemB)
{
    // Using doubles for intermediate calculations for precision, as in original.
    double dx = (double)systemA.x - systemB.x;
    double dy = (double)systemA.y - systemB.y;
    return (uint16_t)float_to_int_round(4 * sqrt(dx * dx + (dy * dy) / 4.0));
}

// Finds the planet index that best matches the searchName, closest to the CurrentPlanet.
// Returns CurrentPlanet if no match is found.
// Depends on string_begins_with from elite_utils.h and distance (defined above).
// Relies on global variables: CurrentPlanet, Galaxy from elite_state.h
static inline PlanetNum find_matching_system_name(char *searchName)
{
    PlanetNum syscount;
    PlanetNum p = CurrentPlanet; // Global variable
    uint16_t d = 0xFFFF; // Initialize with max uint16_t value

    for (syscount = 0; syscount < GAL_SIZE; ++syscount)
    {
        if (string_begins_with(searchName, Galaxy[syscount].name)) // Galaxy is global
        {
            uint16_t dist_to_current = distance(Galaxy[syscount], Galaxy[CurrentPlanet]);
            if (dist_to_current < d)
            {
                d = dist_to_current;
                p = syscount;
            }
        }
    }
    return p;
}

// Executes a jump to the specified planetIndex.
// Updates CurrentPlanet and LocalMarket globals.
// Also updates CurrentStarSystem and PlayerNavState.
// Depends on random_byte from elite_utils.h and generate_market from elite_market.h.
// Relies on global variables: CurrentPlanet, LocalMarket, Galaxy from elite_state.h
static inline void execute_jump_to_planet(PlanetNum planetIndex)
{
    CurrentPlanet = planetIndex; // Global variable
    // Galaxy is a global variable, random_byte from elite_utils, generate_market from elite_market
    LocalMarket = generate_market(random_byte(), Galaxy[planetIndex]); // Global variable
    
    // Update the star system and navigation state for the new planet
    initialize_star_system_for_current_planet();
}

#endif // ELITE_NAVIGATION_H
