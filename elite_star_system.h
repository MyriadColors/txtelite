#ifndef ELITE_STAR_SYSTEM_H
#define ELITE_STAR_SYSTEM_H

#include "elite_state.h" // For PlanSys and other related structures
#include "elite_navigation_types.h" // For NavigationState and CelestialType

// Forward declarations
struct Star;
struct Planet;
struct Station;
struct StarSystem;

// Maximum number of planets per star system and stations per planet
#define MAX_PLANETS_PER_SYSTEM 8
#define MAX_STATIONS_PER_PLANET 5

// Structure for a star in a star system
typedef struct Star {
    char name[MAX_LEN];         // Star name
    uint8_t spectralClass;      // O, B, A, F, G, K, M classification
    double mass;                // Mass relative to Sol
    double luminosity;          // Luminosity relative to Sol
} Star;

// Structure for a planet in a star system
typedef struct Planet {
    char name[MAX_LEN];         // Planet name
    double orbitalDistance;     // Distance from parent star in AU
    double radius;              // Radius in km
    uint8_t type;               // Planet type (gas giant, terrestrial, etc.)
    uint8_t numStations;        // Number of stations orbiting this planet
    struct Station* stations[MAX_STATIONS_PER_PLANET]; // Pointers to station structures
} Planet;

// Structure for a space station
typedef struct Station {
    char name[MAX_LEN];         // Station name
    double orbitalDistance;     // Distance from parent planet in AU
    uint8_t type;               // Station type/class
    bool hasDockingComputer;    // Whether automated docking is available
    bool hasShipyard;           // Whether ship equipment can be purchased
    bool hasMarket;             // Whether market trading is available
    bool hasMissions;           // Whether missions are available
} Station;

// Structure for a complete star system
typedef struct StarSystem {
    struct PlanSys* planSys;    // Pointer to existing system info (economy, gov, etc.)
    Star centralStar;           // The central star of the system
    uint8_t numPlanets;         // Number of planets in the system
    Planet planets[MAX_PLANETS_PER_SYSTEM];  // Array of planets
    double navBeaconDistance;   // Distance of nav beacon from central star in AU
} StarSystem;

// Function to initialize a star system from a PlanSys entry
static inline void initialize_star_system(StarSystem* system, struct PlanSys* planSysEntry) {
    if (!system || !planSysEntry) {
        fprintf(stderr, "Error: Invalid parameters for star system initialization.\n");
        return;
    }
    
    // Link the existing PlanSys information
    system->planSys = planSysEntry;
    
    // ------------------------------------
    // Initialize the central star
    // ------------------------------------
    
    // Generate star name with variety based on system characteristics
    uint8_t nameVariant = (planSysEntry->goatSoupSeed.a % 3);
    switch (nameVariant) {
        case 0:
            snprintf(system->centralStar.name, MAX_LEN, "%s Prime", planSysEntry->name);
            break;
        case 1:
            snprintf(system->centralStar.name, MAX_LEN, "%s Star", planSysEntry->name);
            break;
        case 2:
            snprintf(system->centralStar.name, MAX_LEN, "%s Alpha", planSysEntry->name);
            break;
    }
    
    // The spectral class, mass, and luminosity derived from the system's seed
    system->centralStar.spectralClass = (planSysEntry->goatSoupSeed.a % 7); // 0-6 mapping to O-M
    
    // Mass and luminosity correlated with spectral class for realism
    // O and B stars are massive and luminous, M stars are smaller and dimmer
    double massFactor = 0.5 + ((6 - system->centralStar.spectralClass) / 3.0);
    double lumFactor = 0.4 + ((6 - system->centralStar.spectralClass) / 2.0);
    
    // Add some randomization based on seed
    system->centralStar.mass = massFactor + ((double)(planSysEntry->goatSoupSeed.b % 100) / 100.0);
    system->centralStar.luminosity = lumFactor + ((double)(planSysEntry->goatSoupSeed.c % 100) / 100.0);
    
    // ------------------------------------
    // Determine number of planets
    // ------------------------------------
    
    // More advanced systems tend to have more explored planets
    system->numPlanets = 3 + (planSysEntry->techLev % 6); // 3-8 planets
    if (system->numPlanets > MAX_PLANETS_PER_SYSTEM) {
        system->numPlanets = MAX_PLANETS_PER_SYSTEM;
    }
    
    // ------------------------------------
    // Set Nav Beacon position
    // ------------------------------------
    
    // Nav beacon distance varies based on system characteristics
    // Higher tech systems have nav beacons farther out to cover more space
    system->navBeaconDistance = 35.0 + ((double)(planSysEntry->techLev) * 1.5);
    
    // ------------------------------------
    // Initialize planets
    // ------------------------------------
    for (int i = 0; i < system->numPlanets; i++) {
        Planet* planet = &system->planets[i];
        
        // Set planet name with more variety based on position and system characteristics
        if (i == 0) {
            // First planet often shares system name
            uint8_t nameVariant = (planSysEntry->goatSoupSeed.b % 2);
            if (nameVariant == 0) {
                snprintf(planet->name, MAX_LEN, "%s", planSysEntry->name);
            } else {
                snprintf(planet->name, MAX_LEN, "%s Prime", planSysEntry->name);
            }
        } else if (i == 1) {
            // Second planet often has "New" prefix
            uint8_t nameVariant = (planSysEntry->goatSoupSeed.c % 3);
            if (nameVariant == 0) {
                snprintf(planet->name, MAX_LEN, "New %s", planSysEntry->name);
            } else if (nameVariant == 1) {
                snprintf(planet->name, MAX_LEN, "%s II", planSysEntry->name);
            } else {
                snprintf(planet->name, MAX_LEN, "%s Beta", planSysEntry->name);
            }
        } else {
            // Other planets get variety in naming
            uint8_t nameVariant = ((planSysEntry->goatSoupSeed.d + i) % 4);
            if (nameVariant == 0) {
                snprintf(planet->name, MAX_LEN, "%s %c", planSysEntry->name, 'A' + i);
            } else if (nameVariant == 1) {
                snprintf(planet->name, MAX_LEN, "%s %d", planSysEntry->name, i + 1);
            } else if (nameVariant == 2) {
                const char* suffixes[] = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta"};
                snprintf(planet->name, MAX_LEN, "%s %s", planSysEntry->name, suffixes[i % 8]);
            } else {
                // Generate a slightly different name using seed
                char altName[MAX_LEN];
                uint32_t nameSeed = planSysEntry->goatSoupSeed.a + i * planSysEntry->goatSoupSeed.b;
                uint8_t nameLen = (nameSeed % 4) + 3; // 3-6 letter name
                
                for (int j = 0; j < nameLen; j++) {
                    nameSeed = (nameSeed * 2654435761U) % 4294967296U; // Knuth's multiplicative hash
                    char letter = 'a' + (nameSeed % 26);
                    if (j == 0) letter = toupper(letter);
                    altName[j] = letter;
                }
                altName[nameLen] = '\0';
                
                snprintf(planet->name, MAX_LEN, "%s", altName);
            }
        }
        
        // Set orbital distance with realistic spacing and variability
        // Bode's Law-inspired: a = 0.4 + 0.3 * 2^n
        double baseDistance = 0.4 + (0.3 * pow(1.7, i));
        double variability = ((double)((planSysEntry->goatSoupSeed.d + i) % 100) / 100.0) - 0.5;
        planet->orbitalDistance = baseDistance * (1.0 + (variability * 0.2));
        
        // Set radius based on planet type
        uint8_t sizeClass = (planSysEntry->goatSoupSeed.a + i) % 4; // 0=small, 1=medium, 2=large, 3=giant
        double baseRadius = 0.0;
        
        switch (sizeClass) {
            case 0: baseRadius = 2000.0 + ((double)((planSysEntry->goatSoupSeed.b + i) % 3000)); break; // Small
            case 1: baseRadius = 5000.0 + ((double)((planSysEntry->goatSoupSeed.c + i) % 5000)); break; // Medium
            case 2: baseRadius = 10000.0 + ((double)((planSysEntry->goatSoupSeed.d + i) % 10000)); break; // Large
            case 3: baseRadius = 30000.0 + ((double)((planSysEntry->goatSoupSeed.a + i) % 40000)); break; // Giant
        }
        
        planet->radius = baseRadius;
        
        // Set planet type - more gas giants in outer system, more terrestrial in inner system
        double innerOuterThreshold = 2.5; // AU
        if (planet->orbitalDistance < innerOuterThreshold) {
            // Inner system - more likely to be rocky/terrestrial
            planet->type = (planSysEntry->goatSoupSeed.b + i) % 2; // 0-1 (Rocky or Terrestrial)
        } else {
            // Outer system - more likely to be gas/ice giant
            planet->type = 2 + ((planSysEntry->goatSoupSeed.c + i) % 2); // 2-3 (Gas Giant or Ice Giant)
        }
        
        // Determine number of stations for this planet
        // More developed systems (higher tech) have more stations
        // Terrestrial planets are more likely to have stations than gas giants
        uint8_t maxStations;
        if (planet->type <= 1) { // Rocky or Terrestrial
            maxStations = (planSysEntry->techLev >= 8) ? MAX_STATIONS_PER_PLANET : 3;
        } else { // Gas or Ice Giant
            maxStations = (planSysEntry->techLev >= 10) ? 2 : 1;
        }
        
        planet->numStations = (planSysEntry->goatSoupSeed.d + i) % (maxStations + 1);
        
        // Initialize stations for this planet
        for (int j = 0; j < planet->numStations; j++) {
            // Allocate memory for station
            Station* station = (Station*)malloc(sizeof(Station));
            if (!station) {
                fprintf(stderr, "Error: Memory allocation failed for station.\n");
                continue; // Memory allocation failed
            }
            
            // Set station name with more variety
            uint8_t stationNameVariant = ((planSysEntry->goatSoupSeed.a + i + j) % 4);
            if (stationNameVariant == 0) {
                snprintf(station->name, MAX_LEN, "%s Station %d", planet->name, j+1);
            } else if (stationNameVariant == 1) {
                const char* prefixes[] = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon"};
                snprintf(station->name, MAX_LEN, "%s %s", prefixes[j % 5], planet->name);
            } else if (stationNameVariant == 2) {
                const char* prefixes[] = {"Orbital", "Port", "Hub", "Gateway", "Outpost"};
                snprintf(station->name, MAX_LEN, "%s %s", prefixes[j % 5], planet->name);
            } else {
                const char* uniqueNames[] = {"Nexus", "StarPort", "Horizon", "Tranquility", "Zenith"};
                snprintf(station->name, MAX_LEN, "%s %s", uniqueNames[j % 5], planet->name);
            }
            
            // Set orbital distance from planet - closer for inner system
            if (planet->orbitalDistance < innerOuterThreshold) {
                station->orbitalDistance = 0.005 + ((double)((planSysEntry->goatSoupSeed.b + j) % 10) / 1000.0);
            } else {
                station->orbitalDistance = 0.01 + ((double)((planSysEntry->goatSoupSeed.c + j) % 20) / 1000.0);
            }
            
            // Set station type based on tech level and planet type
            if (planSysEntry->techLev >= 10) {
                station->type = (planSysEntry->goatSoupSeed.d + j) % 3; // 0-2 for high tech
            } else if (planSysEntry->techLev >= 5) {
                station->type = (planSysEntry->goatSoupSeed.a + j) % 2; // 0-1 for medium tech
            } else {
                station->type = 0; // Only basic stations for low tech
            }
            
            // Set station services based on tech level and random factors
            station->hasDockingComputer = (planSysEntry->techLev >= 8) || 
                                         ((planSysEntry->goatSoupSeed.b + j) % 5 == 0);
            
            station->hasShipyard = (planSysEntry->techLev >= 5) ||
                                  ((planSysEntry->goatSoupSeed.c + j) % 4 == 0);
            
            station->hasMarket = true; // All stations have markets
            
            station->hasMissions = (planSysEntry->techLev >= 3) ||
                                   ((planSysEntry->goatSoupSeed.d + j) % 3 == 0);
            
            // Link station to planet
            planet->stations[j] = station;
        }
    }
}

// Function to clean up allocated memory for a star system
static inline void cleanup_star_system(StarSystem* system) {
    if (!system) return;
    
    // Free memory for each station
    for (int i = 0; i < system->numPlanets; i++) {
        Planet* planet = &system->planets[i];
        if (!planet) continue;
        
        for (int j = 0; j < planet->numStations; j++) {
            if (planet->stations[j]) {
                free(planet->stations[j]);
                planet->stations[j] = NULL;
            }
        }
        
        // Reset station count to avoid accessing freed memory
        planet->numStations = 0;
    }
    
    // Reset planet count to avoid accessing invalid data
    system->numPlanets = 0;
    
    // Don't free system->planSys as it's managed elsewhere
    system->planSys = NULL;
}

// Function to get planet from a star system by index
static inline Planet* get_planet_by_index(StarSystem* system, uint8_t index) {
    if (!system || index >= system->numPlanets) return NULL;
    return &system->planets[index];
}

// Function to get station from a planet by index
static inline Station* get_station_by_index(Planet* planet, uint8_t index) {
    if (!planet || index >= planet->numStations) return NULL;
    return planet->stations[index];
}

// Function to calculate travel time between two points in a system
// Returns time in seconds
static inline uint32_t calculate_travel_time(double startDistance, double endDistance) {
    // Simple model: 1 AU = 20 minutes of travel
    const double TRAVEL_SPEED_AU_PER_MINUTE = 0.05; // 0.05 AU per minute
    const uint32_t SECONDS_PER_MINUTE = 60;
    
    double distanceDelta = fabs(endDistance - startDistance);
    double timeInMinutes = distanceDelta / TRAVEL_SPEED_AU_PER_MINUTE;
    
    return (uint32_t)(timeInMinutes * SECONDS_PER_MINUTE);
}

// Function to travel to a celestial body within a star system
// Updates navigation state and game time
static inline bool travel_to_celestial(StarSystem* system, NavigationState* navState, 
                                      CelestialType targetType, void* targetBody) {
    if (!system) {
        fprintf(stderr, "Error: Invalid star system data for travel.\n");
        return false;
    }
    
    if (!navState) {
        fprintf(stderr, "Error: Invalid navigation state for travel.\n");
        return false;
    }
    
    // For non-NavBeacon targets, we need a valid body pointer
    if (targetType != CELESTIAL_NAV_BEACON && !targetBody) {
        fprintf(stderr, "Error: Invalid target body for travel destination.\n");
        return false;
    }
    
    double startDistance = navState->distanceFromStar;
    double endDistance = 0.0;
    
    // Determine target distance based on type
    switch (targetType) {
        case CELESTIAL_STAR:
            endDistance = 0.0; // Star is at center
            break;
            
        case CELESTIAL_PLANET: {
            Planet* targetPlanet = (Planet*)targetBody;
            
            // Validate the planet is part of this system
            bool planetFound = false;
            for (int i = 0; i < system->numPlanets; i++) {
                if (&system->planets[i] == targetPlanet) {
                    planetFound = true;
                    break;
                }
            }
            
            if (!planetFound) {
                fprintf(stderr, "Error: Target planet is not part of the current star system.\n");
                return false;
            }
            
            endDistance = targetPlanet->orbitalDistance;
            break;
        }
            
        case CELESTIAL_STATION: {
            // Need to find parent planet
            Station* targetStation = (Station*)targetBody;
            Planet* parentPlanet = NULL;
            
            // Find which planet this station belongs to
            for (int i = 0; i < system->numPlanets && !parentPlanet; i++) {
                Planet* planet = &system->planets[i];
                if (!planet) continue;
                
                for (int j = 0; j < planet->numStations; j++) {
                    if (planet->stations[j] == targetStation) {
                        parentPlanet = planet;
                        break;
                    }
                }
            }
            
            if (!parentPlanet) {
                fprintf(stderr, "Error: Could not find parent planet for target station.\n");
                return false;
            }
            
            // Station distance is planet distance plus orbital offset
            endDistance = parentPlanet->orbitalDistance + targetStation->orbitalDistance;
            break;
        }
            
        case CELESTIAL_NAV_BEACON:
            endDistance = system->navBeaconDistance;
            break;
            
        default:
            fprintf(stderr, "Error: Unknown celestial type for travel destination.\n");
            return false;
    }
    
    // Calculate travel time
    uint32_t travelTime = calculate_travel_time(startDistance, endDistance);
    
    // Update game time
    game_time_advance(travelTime);
    
    // Update navigation state
    navState->currentLocationType = targetType;
    switch (targetType) {
        case CELESTIAL_STAR:
            navState->currentLocation.star = &system->centralStar;
            break;
            
        case CELESTIAL_PLANET:
            navState->currentLocation.planet = (Planet*)targetBody;
            break;
            
        case CELESTIAL_STATION:
            navState->currentLocation.station = (Station*)targetBody;
            break;
            
        case CELESTIAL_NAV_BEACON:
            // Nav beacon doesn't need a specific structure reference
            // Just ensure the currentLocation union doesn't contain garbage
            memset(&navState->currentLocation, 0, sizeof(navState->currentLocation));
            break;
    }
    
    navState->distanceFromStar = endDistance;
    
    return true;
}

// Function to convert celestial type to string for display
static inline const char* celestial_type_to_string(CelestialType type) {
    switch (type) {
        case CELESTIAL_STAR: return "Star";
        case CELESTIAL_PLANET: return "Planet";
        case CELESTIAL_STATION: return "Station";
        case CELESTIAL_NAV_BEACON: return "Nav Beacon";
        default: return "Unknown";
    }
}

// Function to get current location name with more context
static inline void get_current_location_name(NavigationState* navState, char* buffer, size_t bufferSize) {
    if (!navState || !buffer || bufferSize == 0) return;
    
    switch (navState->currentLocationType) {
        case CELESTIAL_STAR:
            if (navState->currentLocation.star) {
                snprintf(buffer, bufferSize, "%s (Star)", navState->currentLocation.star->name);
            } else {
                snprintf(buffer, bufferSize, "Unknown Star");
            }
            break;
            
        case CELESTIAL_PLANET:
            if (navState->currentLocation.planet) {
                // Add planet type information for context
                const char* planetTypes[] = {
                    "Rocky/Airless", "Terrestrial", "Gas Giant", "Ice Giant", "Unknown"
                };
                
                uint8_t type = navState->currentLocation.planet->type;
                if (type >= 4) type = 4; // Default to "Unknown" for invalid types
                
                snprintf(buffer, bufferSize, "%s (%s Planet)", 
                         navState->currentLocation.planet->name,
                         planetTypes[type]);
            } else {
                snprintf(buffer, bufferSize, "Unknown Planet");
            }
            break;
            
        case CELESTIAL_STATION:
            if (navState->currentLocation.station) {
                // This requires a pointer to the current star system
                // If we don't have that in this context, we'll use a generic format
                snprintf(buffer, bufferSize, "%s (Orbital Station)", 
                         navState->currentLocation.station->name);
            } else {
                snprintf(buffer, bufferSize, "Unknown Station");
            }
            break;
            
        case CELESTIAL_NAV_BEACON:
            snprintf(buffer, bufferSize, "Navigation Beacon");
            break;
            
        default:
            snprintf(buffer, bufferSize, "Unknown Location");
    }
}

#endif // ELITE_STAR_SYSTEM_H