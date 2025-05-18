#pragma once

// Enum for celestial body types
typedef enum
{
    CELESTIAL_STAR,
    CELESTIAL_PLANET,
    CELESTIAL_STATION,
    CELESTIAL_NAV_BEACON
} CelestialType;

// Forward declarations with typedefs for types needed by NavigationState
// Using typedefs to match the type definitions in elite_star_system.h
typedef struct Star Star;
typedef struct Planet Planet;
typedef struct Station Station;

// Navigation helper structure for travel within system
typedef struct NavigationState
{
    CelestialType currentLocationType; // Type of current location
    union
    {
        Star *star;
        Planet *planet;
        Station *station;
    } currentLocation;       // Pointer to current location
    double distanceFromStar; // Current distance from system's star in AU
} NavigationState;

// Calculates energy requirement for in-system travel based on distance
static inline double calculate_travel_energy_requirement(double distanceInAU)
{
    // Base energy requirement: 1 energy unit per 0.1 AU
    // This is an example formula that can be adjusted for game balance
    return distanceInAU * 10.0;
}

// Calculates fuel requirement for in-system travel based on distance
static inline double calculate_travel_fuel_requirement(double distanceInAU)
{
    // Base fuel requirement: 0.025 liters per AU
    // This is much less than hyperspace travel which consumes ~10 liters per 0.1 LY
    return distanceInAU * 0.025;
}
