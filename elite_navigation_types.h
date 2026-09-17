#pragma once

// Minimal includes to avoid circular dependencies
#include <stdbool.h>
#include <stdint.h>


// Enum for celestial body types
typedef enum { CELESTIAL_STAR, CELESTIAL_PLANET, CELESTIAL_STATION, CELESTIAL_NAV_BEACON } celestial_type_t;

// Navigation helper structure for travel within system
typedef struct navigation_state_t {
    celestial_type_t currentLocationType; // Type of current location
    union {
        struct star_t *star;
        struct planet_t *planet;
        struct station_t *station;
    } currentLocation;       // Pointer to current location
    double distanceFromStar; // Current distance from system's star in AU
} navigation_state_t;