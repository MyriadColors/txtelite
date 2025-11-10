#pragma once

// Minimal includes to avoid circular dependencies
#include <stdint.h>
#include <stdbool.h>

// Forward declarations for structures used in NavigationState
// These will be resolved by the including file
typedef struct Star Star;
typedef struct Planet Planet;
typedef struct Station Station;

// Enum for celestial body types
typedef enum
{
    CELESTIAL_STAR,
    CELESTIAL_PLANET,
    CELESTIAL_STATION,
    CELESTIAL_NAV_BEACON
} CelestialType;

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