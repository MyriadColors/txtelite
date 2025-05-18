#pragma once

#include "elite_navigation_types.h" // For NavigationState definition
#include "elite_state.h"            // Unified header for constants, structures, and globals
#include "elite_utils.h"            // For float_to_int_round, random_byte, string_begins_with
#include "elite_market.h"           // For generate_market
#include "elite_galaxy.h"           // For Galaxy array functions
#include "elite_player_state.h"     // For initialize_star_system_for_current_planet
#include <math.h>                   // For sqrt


/**
 * @brief Calculates the distance between two planetary systems.
 * 
 * This function calculates the Euclidean distance between two planetary systems
 * with a scaling factor of 4. It uses double precision for intermediate calculations
 * to maintain accuracy, following the approach from the original Elite game.
 * 
 * The y-coordinate is given half the weight of the x-coordinate during distance
 * calculation, which creates an elliptical rather than circular distance metric.
 * 
 * @param systemA The first planetary system.
 * @param systemB The second planetary system.
 * 
 * @return The scaled distance between the two systems as a 16-bit unsigned integer.
 */
static inline uint16_t distance(struct PlanSys systemA, struct PlanSys systemB)
{
    // Using doubles for intermediate calculations for precision, as in original.
    double dx = (double)systemA.x - systemB.x;
    double dy = (double)systemA.y - systemB.y;
    return (uint16_t)float_to_int_round(4 * sqrt(dx * dx + (dy * dy) / 4.0));
}


/**
 * @brief Finds a planet with a name matching the search string
 * 
 * This function searches the galaxy for planets whose names begin with the 
 * provided search string. If multiple planets match, it returns the one
 * closest to the current planet.
 * 
 * @param searchName Pointer to the string to search for in planet names
 * @return PlanetNum The index of the matching planet in the Galaxy array
 *                   (returns CurrentPlanet if no match is found)
 * 
 * @note The function uses the global variables Galaxy and CurrentPlanet
 * @see distance - Function used to calculate distances between planets
 * @see string_begins_with - Function that checks if a string begins with another string
 */
static inline PlanetNum find_matching_system_name(char *searchName)
{
    PlanetNum syscount;
    PlanetNum p = CurrentPlanet; // Global variable
    uint16_t d = 0xFFFF;         // Initialize with max uint16_t value

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

/**
 * @brief Executes a jump to a specified planet and sets up the new environment.
 *
 * This function performs the following operations when a jump is made to a new planet:
 * 1. Updates the CurrentPlanet global variable to the new planet index
 * 2. Generates a new local market for the destination planet
 * 3. Initializes the star system and navigation state for the new planet
 *
 * @param planetIndex The index of the destination planet in the Galaxy array
 *
 * @note This function relies on several global variables:
 *       - CurrentPlanet: Updated to the new planet index
 *       - LocalMarket: Updated with newly generated market data
 *       - Galaxy: Used to access planet information
 * 
 * @see random_byte(), generate_market(), initialize_star_system_for_current_planet()
 */
static inline void execute_jump_to_planet(PlanetNum planetIndex)
{
    CurrentPlanet = planetIndex; // Global variable
    // Galaxy is a global variable, random_byte from elite_utils, generate_market from elite_market
    LocalMarket = generate_market(random_byte(), Galaxy[planetIndex]); // Global variable

    // Update the star system and navigation state for the new planet
    initialize_star_system_for_current_planet();
}
