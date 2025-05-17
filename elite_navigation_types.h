#ifndef ELITE_NAVIGATION_TYPES_H
#define ELITE_NAVIGATION_TYPES_H

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

#endif // ELITE_NAVIGATION_TYPES_H
