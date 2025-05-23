#pragma once

#include "elite_state.h"      // For PlanSys and other related structures
#include "elite_navigation.h" // For NavigationState and CelestialType
#include "elite_market.h"     // For market-related functions
#include "elite_ship_types.h" // For PlayerShip structure
#include "elite_ship_maintenance.h" // For ConsumeFuel function

// Forward declarations
struct Star;
struct Planet;
struct Station;
struct StarSystem;

// Maximum number of planets per star system and stations per planet
#define MAX_PLANETS_PER_SYSTEM 8
#define MAX_STATIONS_PER_PLANET 5

// Structure for a star in a star system
typedef struct Star
{
    char name[MAX_LEN];    // Star name
    uint8_t spectralClass; // O, B, A, F, G, K, M classification
    double mass;           // Mass relative to Sol
    double luminosity;     // Luminosity relative to Sol
} Star;

// Structure for a planet in a star system
typedef struct Planet
{
    char name[MAX_LEN];                                // Planet name
    double orbitalDistance;                            // Distance from parent star in AU
    double radius;                                     // Radius in km
    uint8_t type;                                      // Planet type (gas giant, terrestrial, etc.)
    uint8_t numStations;                               // Number of stations orbiting this planet
    struct Station *stations[MAX_STATIONS_PER_PLANET]; // Pointers to station structures

    // Planetary market data for when landed on the planet
    struct
    {
        MarketType market;  // Market data for the planet's surface
        bool isInitialized; // Whether the market has been initialized
    } planetaryMarket;
    uint8_t marketFluctuation; // Planet-specific market fluctuation factor
    uint64_t lastMarketUpdate; // Last game time when market was updated
} Planet;

// Structure for a space station
typedef struct Station
{
    char name[MAX_LEN];        // Station name
    double orbitalDistance;    // Distance from parent planet in AU
    uint8_t type;              // Station type/class
    bool hasDockingComputer;   // Whether automated docking is available
    bool hasShipyard;          // Whether ship equipment can be purchased
    bool hasMarket;            // Whether market trading is available
    bool hasMissions;          // Whether missions are available
    MarketType market;         // Station-specific market data
    uint8_t marketFluctuation; // Station-specific market fluctuation factor (0-15)
    uint8_t specialization;    // Economic specialization (0: Balanced, 1: Industrial, 2: Agricultural, 3: Mining)
    uint64_t lastMarketUpdate; // Last game time when market was updated
} Station;

// Structure for a complete star system
typedef struct StarSystem
{
    struct PlanSys *planSys;                // Pointer to existing system info (economy, gov, etc.)
    Star centralStar;                       // The central star of the system
    uint8_t numPlanets;                     // Number of planets in the system
    Planet planets[MAX_PLANETS_PER_SYSTEM]; // Array of planets
    double navBeaconDistance;               // Distance of nav beacon from central star in AU
} StarSystem;

// Forward function declarations
static inline MarketType GenerateStationMarket(Station *station, Planet *planet, struct PlanSys *planSys);
static inline void UpdateStationMarket(Station *station, uint64_t currentTime, Planet *planet, struct PlanSys *planSys);
static inline void UseStationMarket(Station *station, Planet *planet, struct PlanSys *planSys);
static inline MarketType GeneratePlanetaryMarket(Planet *planet, struct PlanSys *planSys);
static inline void UpdatePlanetaryMarket(Planet *planet, uint64_t currentTime, struct PlanSys *planSys);
static inline void UsePlanetaryMarket(Planet *planet, struct PlanSys *planSys);

// Function to generate the market for a station
static inline MarketType GenerateStationMarket(Station *station, Planet *planet, struct PlanSys *planSys)
{
    if (!station || !planSys) // Planet can be NULL if station is not orbiting one (e.g. deep space station, though plan implies planet context)
    {
        // Handle error or return a default/empty market
        MarketType emptyMarket = {0}; // Initialize all members to zero
        // fprintf(stderr, "Warning: GenerateStationMarket called with NULL station or planSys.\\n");
        return emptyMarket;
    }    // Avoid unused parameter warning
    (void)planet;

    // 1. Generate a baseMarket using the existing generate_market function from elite_market.h
    MarketType baseMarket = generate_market(station->marketFluctuation, *planSys);

    // 2. Apply modifiers to baseMarket.price[i] and baseMarket.quantity[i] for each commodity based on station->specialization
    // Ensure station->specialization is a valid enum value
    StationSpecialization specialization = (StationSpecialization)station->specialization;
    if (specialization >= NUM_STATION_SPECIALIZATIONS || specialization < 0) 
    {
        // Optionally log this case:
        // fprintf(stderr, "Warning: Station '%s' has invalid specialization value %d. Defaulting to Balanced.\\n", station->name, station->specialization);
        specialization = STATION_SPECIALIZATION_BALANCED; // Default to balanced if out of bounds
    }

    for (int i = 0; i < NUM_STANDARD_COMMODITIES; i++)
    {
        // Get the modifier for the current commodity and station specialization
        MarketModifier modifier = stationSpecializationModifiers[specialization][i];

        // Apply price modifier
        float newPrice = (float)baseMarket.price[i] * modifier.priceFactor;
        if (newPrice < 0) newPrice = 0; // Price should not be negative
        // Potentially clamp to a max price if one is defined: if (newPrice > MAX_COMMODITY_PRICE) newPrice = MAX_COMMODITY_PRICE;
        baseMarket.price[i] = (uint16_t)roundf(newPrice); // Round to nearest integer for price

        // Apply quantity modifier
        float newQuantity = (float)baseMarket.quantity[i] * modifier.quantityFactor;
        if (newQuantity < 0) newQuantity = 0; // Quantity should not be negative
        // Clamp quantity to avoid overflow (uint16_t max is 65535).
        if (newQuantity > 0xFFFF) newQuantity = 0xFFFF; 
        baseMarket.quantity[i] = (uint16_t)roundf(newQuantity); // Round to nearest integer for quantity
    }

    // 3. Update station->lastMarketUpdate with the currentGameTimeSeconds.
    // As per the plan, this is handled by the caller (e.g., initialize_star_system or UpdateStationMarket).
    // station->lastMarketUpdate = currentGameTimeSeconds; // This line would be here if handled internally

    return baseMarket;
}

// Function to generate the market for a planet's surface
static inline MarketType GeneratePlanetaryMarket(Planet *planet, struct PlanSys *planSys)
{
    if (!planet || !planSys)
    {
        MarketType emptyMarket = {0}; // Initialize all members to zero
        // fprintf(stderr, "Warning: GeneratePlanetaryMarket called with NULL planet or planSys.\\n");
        return emptyMarket;
    }

    // 1. Generate a baseMarket using generate_market from elite_market.h
    MarketType baseMarket = generate_market(planet->marketFluctuation, *planSys);

    // 2. Apply modifiers based on planet->type
    PlanetMarketType planetType = (PlanetMarketType)planet->type; // planet->type is uint8_t

    // Check if planetType is within the valid range for the planetTypeModifiers array
    if (planetType >= NUM_PLANET_MARKET_TYPES || planetType < 0) // planet->type is uint8_t, so < 0 is only for robustness if type changes
    {
        // fprintf(stderr, "Warning: Planet '%s' (type %u) has invalid type for market modifiers. Using base market without type-specific changes.\\n", planet->name, planet->type);
        // If type is out of bounds, no type-specific modifiers are applied. The market remains the baseMarket.
    }
    else
    {
        for (int i = 0; i < NUM_STANDARD_COMMODITIES; i++)
        {
            MarketModifier modifier = planetTypeModifiers[planetType][i];

            // Apply price modifier
            float newPrice = (float)baseMarket.price[i] * modifier.priceFactor;
            if (newPrice < 0) newPrice = 0; // Price should not be negative
            // Consider clamping to a MAX_PRICE if defined
            baseMarket.price[i] = (uint16_t)roundf(newPrice); // Round to nearest integer

            // Apply quantity modifier
            float newQuantity = (float)baseMarket.quantity[i] * modifier.quantityFactor;
            if (newQuantity < 0) newQuantity = 0; // Quantity should not be negative
            if (newQuantity > 0xFFFF) newQuantity = 0xFFFF; // Clamp to uint16_t max (65535)
            baseMarket.quantity[i] = (uint16_t)roundf(newQuantity); // Round to nearest integer
        }
    }

    // 3. Update planet->lastMarketUpdate with currentGameTimeSeconds
    // Assuming game_time_get_seconds() is available globally or via included headers (e.g., elite_state.h)
    planet->lastMarketUpdate = game_time_get_seconds();

    // 4. Set planet->planetaryMarket.isInitialized = true
    // This is done here as per step III.4 of the plan.
    // The caller (e.g., initialize_star_system) will assign the returned market to planet->planetaryMarket.market.
    planet->planetaryMarket.isInitialized = true;

    // 5. Return the modified market
    return baseMarket;
}

// Function to update the market for a station if enough time has passed
static inline void UpdateStationMarket(Station *station, uint64_t currentTime, Planet *planet, struct PlanSys *planSys) // Added Planet* and PlanSys* params
{
    if (!station || !planSys) // Planet can be NULL for deep space stations, but planSys is essential
    {
        // fprintf(stderr, "Warning: UpdateStationMarket called with NULL station or planSys.\\n");
        return;
    }

    const uint64_t STATION_UPDATE_INTERVAL = 3600; // 1 hour in game seconds, as per existing findings

    // 1. Check if enough game time (UPDATE_INTERVAL) has passed since lastMarketUpdate.
    if (currentTime >= station->lastMarketUpdate && (currentTime - station->lastMarketUpdate >= STATION_UPDATE_INTERVAL))
    {
        // 2. If so, calculate updateCycles.
        uint64_t elapsedSeconds = currentTime - station->lastMarketUpdate;
        uint16_t updateCycles = (uint16_t)(elapsedSeconds / STATION_UPDATE_INTERVAL);

        if (updateCycles > 0)
        {
            // 3. Modify marketFluctuation based on updateCycles.
            // Simple cyclic increment for fluctuation. Max fluctuation is 15 (0-15 range).
            station->marketFluctuation = (station->marketFluctuation + updateCycles) % 16;

            // 4. Call GenerateStationMarket to regenerate the market.
            // The GenerateStationMarket function itself does not update lastMarketUpdate.
            station->market = GenerateStationMarket(station, planet, planSys);

            // 5. lastMarketUpdate is then set to the currentTime.
            // To prevent drift, set it to the time of the last completed interval, or current time.
            // Setting to currentTime is simpler as per plan.
            station->lastMarketUpdate = currentTime;
        }
    }
}

// Function to update the market for a planet if enough time has passed
static inline void UpdatePlanetaryMarket(Planet *planet, uint64_t currentTime, struct PlanSys *planSys) // Added PlanSys* param
{
    if (!planet || !planSys)
    {
        // fprintf(stderr, "Warning: UpdatePlanetaryMarket called with NULL planet or planSys.\\n");
        return;
    }

    const uint64_t PLANET_UPDATE_INTERVAL = 7200; // 2 hours in game seconds, as per existing findings

    // 1. Check if enough game time (UPDATE_INTERVAL) has passed since lastMarketUpdate.
    if (currentTime >= planet->lastMarketUpdate && (currentTime - planet->lastMarketUpdate >= PLANET_UPDATE_INTERVAL))
    {
        // 2. If so, calculate updateCycles.
        uint64_t elapsedSeconds = currentTime - planet->lastMarketUpdate;
        uint16_t updateCycles = (uint16_t)(elapsedSeconds / PLANET_UPDATE_INTERVAL);

        if (updateCycles > 0)
        {
            // 3. Modify marketFluctuation based on updateCycles.
            planet->marketFluctuation = (planet->marketFluctuation + updateCycles) % 16;

            // 4. Call GeneratePlanetaryMarket to regenerate the market.
            // GeneratePlanetaryMarket updates its own lastMarketUpdate and isInitialized fields.
            MarketType newMarket = GeneratePlanetaryMarket(planet, planSys);
            // The plan for GeneratePlanetaryMarket (Step III.4) says it updates planet->lastMarketUpdate and sets isInitialized.
            // However, the plan for UpdatePlanetaryMarket (Step IV.5) also says lastMarketUpdate is set to currentTime.
            // To adhere to Step IV.5, we will explicitly set it here. GeneratePlanetaryMarket already sets it, this will overwrite with the exact currentTime.
            
            // The market data itself needs to be stored in planet->planetaryMarket.market
            // The plan for V. says: call GeneratePlanetaryMarket(planet, planSysEntry) to populate planet->planetaryMarket.market
            // So, the GeneratePlanetaryMarket should ideally return the market to be assigned.
            // And indeed it does. We need to assign it to the correct place in the Planet struct.
            // The Planet struct has: struct { MarketType market; bool isInitialized; } planetaryMarket;
            planet->planetaryMarket.market = newMarket; 

            // 5. lastMarketUpdate is then set to the currentTime.
            planet->lastMarketUpdate = currentTime; 
            // planet->planetaryMarket.isInitialized is already set by GeneratePlanetaryMarket
        }
    }
}

// Function to initialize a star system from a PlanSys entry
static inline void initialize_star_system(StarSystem *system, struct PlanSys *planSysEntry)
{
    if (!system || !planSysEntry)
    {
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
    switch (nameVariant)
    {
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
    if (system->numPlanets > MAX_PLANETS_PER_SYSTEM)
    {
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
    for (int i = 0; i < system->numPlanets; i++)
    {
        Planet *planet = &system->planets[i];

        // Set planet name with more variety based on position and system characteristics
        if (i == 0)
        {
            // First planet often shares system name
            uint8_t nameVariant = (planSysEntry->goatSoupSeed.b % 2);
            if (nameVariant == 0)
            {
                snprintf(planet->name, MAX_LEN, "%s", planSysEntry->name);
            }
            else
            {
                snprintf(planet->name, MAX_LEN, "%s Prime", planSysEntry->name);
            }
        }
        else if (i == 1)
        {
            // Second planet often has "New" prefix
            uint8_t nameVariant = (planSysEntry->goatSoupSeed.c % 3);
            if (nameVariant == 0)
            {
                snprintf(planet->name, MAX_LEN, "New %s", planSysEntry->name);
            }
            else if (nameVariant == 1)
            {
                snprintf(planet->name, MAX_LEN, "%s II", planSysEntry->name);
            }
            else
            {
                snprintf(planet->name, MAX_LEN, "%s Beta", planSysEntry->name);
            }
        }
        else
        {
            // Other planets get variety in naming
            uint8_t nameVariant = ((planSysEntry->goatSoupSeed.d + i) % 4);
            if (nameVariant == 0)
            {
                snprintf(planet->name, MAX_LEN, "%s %c", planSysEntry->name, 'A' + i);
            }
            else if (nameVariant == 1)
            {
                snprintf(planet->name, MAX_LEN, "%s %d", planSysEntry->name, i + 1);
            }
            else if (nameVariant == 2)
            {
                const char *suffixes[] = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta"};
                snprintf(planet->name, MAX_LEN, "%s %s", planSysEntry->name, suffixes[i % 8]);
            }
            else
            {
                // Generate a slightly different name using seed
                char altName[MAX_LEN];
                uint32_t nameSeed = planSysEntry->goatSoupSeed.a + i * planSysEntry->goatSoupSeed.b;
                uint8_t nameLen = (nameSeed % 4) + 3; // 3-6 letter name

                for (int j = 0; j < nameLen; j++)
                {
                    nameSeed = (nameSeed * 2654435761U) % 4294967296U; // Knuth's multiplicative hash
                    char letter = 'a' + (nameSeed % 26);
                    if (j == 0)
                        letter = toupper(letter);
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

        switch (sizeClass)
        {
        case 0:
            baseRadius = 2000.0 + ((double)((planSysEntry->goatSoupSeed.b + i) % 3000));
            break; // Small
        case 1:
            baseRadius = 5000.0 + ((double)((planSysEntry->goatSoupSeed.c + i) % 5000));
            break; // Medium
        case 2:
            baseRadius = 10000.0 + ((double)((planSysEntry->goatSoupSeed.d + i) % 10000));
            break; // Large
        case 3:
            baseRadius = 30000.0 + ((double)((planSysEntry->goatSoupSeed.a + i) % 40000));
            break; // Giant
        }

        planet->radius = baseRadius;

        // Set planet type - more gas giants in outer system, more terrestrial in inner system
        double innerOuterThreshold = 2.5; // AU
        if (planet->orbitalDistance < innerOuterThreshold)
        {
            // Inner system - more likely to be rocky/terrestrial
            planet->type = (planSysEntry->goatSoupSeed.b + i) % 2; // 0-1 (Rocky or Terrestrial)
        }
        else
        {
            // Outer system - more likely to be gas/ice giant
            planet->type = 2 + ((planSysEntry->goatSoupSeed.c + i) % 2); // 2-3 (Gas Giant or Ice Giant)
        }

        // Initialize planetary market fluctuation factor
        planet->marketFluctuation = (planSysEntry->goatSoupSeed.b + i) % 16; // 0-15 fluctuation

        // Initialize the planetary market (this also sets lastMarketUpdate and isInitialized)
        planet->planetaryMarket.market = GeneratePlanetaryMarket(planet, planSysEntry);
        // Ensure lastMarketUpdate is set by GeneratePlanetaryMarket, or set it here if needed.
        // Per plan, GeneratePlanetaryMarket handles its own lastMarketUpdate.

        // Determine number of stations for this planet
        // More developed systems (higher tech) have more stations
        // Terrestrial planets are more likely to have stations than gas giants
        uint8_t maxStations;
        if (planet->type <= 1)
        { // Rocky or Terrestrial
            maxStations = (planSysEntry->techLev >= 8) ? MAX_STATIONS_PER_PLANET : 3;
        }
        else
        { // Gas or Ice Giant
            maxStations = (planSysEntry->techLev >= 10) ? 2 : 1;
        }

        planet->numStations = (planSysEntry->goatSoupSeed.d + i) % (maxStations + 1);

        // Initialize stations for this planet
        for (int j = 0; j < planet->numStations; j++)
        {
            // Allocate memory for station
            Station *station = (Station *)malloc(sizeof(Station));
            if (!station)
            {
                fprintf(stderr, "Error: Memory allocation failed for station.\n");
                continue; // Memory allocation failed
            }

            // Set station name with more variety
            uint8_t stationNameVariant = ((planSysEntry->goatSoupSeed.a + i + j) % 4);
            if (stationNameVariant == 0)
            {
                snprintf(station->name, MAX_LEN, "%s Station %d", planet->name, j + 1);
            }
            else if (stationNameVariant == 1)
            {
                const char *prefixes[] = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon"};
                snprintf(station->name, MAX_LEN, "%s %s", prefixes[j % 5], planet->name);
            }
            else if (stationNameVariant == 2)
            {
                const char *prefixes[] = {"Orbital", "Port", "Hub", "Gateway", "Outpost"};
                snprintf(station->name, MAX_LEN, "%s %s", prefixes[j % 5], planet->name);
            }
            else
            {
                const char *uniqueNames[] = {"Nexus", "StarPort", "Horizon", "Tranquility", "Zenith"};
                snprintf(station->name, MAX_LEN, "%s %s", uniqueNames[j % 5], planet->name);
            }

            // Set orbital distance from planet - closer for inner system
            if (planet->orbitalDistance < innerOuterThreshold)
            {
                station->orbitalDistance = 0.005 + ((double)((planSysEntry->goatSoupSeed.b + j) % 10) / 1000.0);
            }
            else
            {
                station->orbitalDistance = 0.01 + ((double)((planSysEntry->goatSoupSeed.c + j) % 20) / 1000.0);
            }
            // Set station type based on tech level and planet type
            if (planSysEntry->techLev >= 10)
            {
                station->type = (planSysEntry->goatSoupSeed.d + j) % 3; // 0-2 for high tech
            }
            else if (planSysEntry->techLev >= 5)
            {
                station->type = (planSysEntry->goatSoupSeed.a + j) % 2; // 0-1 for medium tech
            }
            else
            {
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

            // Set economic specialization based on planet type and random factors
            // Generate specialization: 0=Balanced, 1=Industrial, 2=Agricultural, 3=Mining
            if (planet->type <= 1)
            { // Rocky or Terrestrial
                // These planets are more likely to have agricultural or industrial stations
                uint8_t specRoll = (planSysEntry->goatSoupSeed.a + i + j) % 10;
                if (specRoll < 4)
                {
                    station->specialization = 2; // Agricultural
                }
                else if (specRoll < 8)
                {
                    station->specialization = 1; // Industrial
                }
                else
                {
                    station->specialization = 0; // Balanced
                }
            }
            else
            { // Gas or Ice Giants
                // These planets are more likely to have mining stations
                uint8_t specRoll = (planSysEntry->goatSoupSeed.b + i + j) % 10;
                if (specRoll < 6)
                {
                    station->specialization = 3; // Mining
                }
                else if (specRoll < 8)
                {
                    station->specialization = 1; // Industrial
                }
                else
                {
                    station->specialization = 0; // Balanced
                }
            }

            // Economy modifiers based on system's economy type
            if (planSysEntry->economy < 4)
            { // Industrial economies (0-3)
                if (station->specialization == 0)
                {
                    station->specialization = 1; // More likely to be industrial
                }
            }
            else
            { // Agricultural economies (4-7)
                if (station->specialization == 0)
                {
                    station->specialization = 2; // More likely to be agricultural
                }
            }

            // Initialize market fluctuation factor
            station->marketFluctuation = (planSysEntry->goatSoupSeed.c + i + j) % 16; // 0-15 fluctuation

            // Initialize the last market update time to current game time
            // This is important so that the first call to UpdateStationMarket doesn't immediately regenerate.
            station->lastMarketUpdate = game_time_get_seconds();

            // Generate the station's market
            station->market = GenerateStationMarket(station, planet, planSysEntry);

            // Link station to planet
            planet->stations[j] = station;
        }
    }
}

// Function to clean up allocated memory for a star system
static inline void cleanup_star_system(StarSystem *system)
{
    if (!system)
        return;

    // Free memory for each station
    for (int i = 0; i < system->numPlanets; i++)
    {
        Planet *planet = &system->planets[i];
        if (!planet)
            continue;

        for (int j = 0; j < planet->numStations; j++)
        {
            if (planet->stations[j])
            {
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
static inline Planet *get_planet_by_index(StarSystem *system, uint8_t index)
{
    if (!system || index >= system->numPlanets)
        return NULL;
    return &system->planets[index];
}

// Function to get station from a planet by index
static inline Station *get_station_by_index(Planet *planet, uint8_t index)
{
    if (!planet || index >= planet->numStations)
        return NULL;
    return planet->stations[index];
}

// Function to calculate travel time between two points in a system
// Returns time in seconds
static inline uint32_t calculate_travel_time(double startDistance, double endDistance)
{
    // Simple model: 1 AU = 20 minutes of travel
    const double TRAVEL_SPEED_AU_PER_MINUTE = 0.05; // 0.05 AU per minute
    const uint32_t SECONDS_PER_MINUTE = 60;

    double distanceDelta = fabs(endDistance - startDistance);
    double timeInMinutes = distanceDelta / TRAVEL_SPEED_AU_PER_MINUTE;

    return (uint32_t)(timeInMinutes * SECONDS_PER_MINUTE);
}

// Function to travel to a celestial body within a star system
// Updates navigation state and game time
static inline bool travel_to_celestial(StarSystem *system, NavigationState *navState,
                                       CelestialType targetType, void *targetBody)
{
    if (!system)
    {
        fprintf(stderr, "Error: Invalid star system data for travel.\n");
        return false;
    }

    if (!navState)
    {
        fprintf(stderr, "Error: Invalid navigation state for travel.\n");
        return false;
    }

    // For non-NavBeacon targets, we need a valid body pointer
    if (targetType != CELESTIAL_NAV_BEACON && !targetBody)
    {
        fprintf(stderr, "Error: Invalid target body for travel destination.\n");
        return false;
    }

    double startDistance = navState->distanceFromStar;
    double endDistance = 0.0;

    // Determine target distance based on type
    switch (targetType)
    {
    case CELESTIAL_STAR:
        endDistance = 0.0; // Star is at center
        break;

    case CELESTIAL_PLANET:
    {
        Planet *targetPlanet = (Planet *)targetBody;

        // Validate the planet is part of this system
        bool planetFound = false;
        for (int i = 0; i < system->numPlanets; i++)
        {
            if (&system->planets[i] == targetPlanet)
            {
                planetFound = true;
                break;
            }
        }

        if (!planetFound)
        {
            fprintf(stderr, "Error: Target planet is not part of the current star system.\n");
            return false;
        }

        endDistance = targetPlanet->orbitalDistance;
        break;
    }

    case CELESTIAL_STATION:
    {
        // Need to find parent planet
        Station *targetStation = (Station *)targetBody;
        Planet *parentPlanet = NULL;

        // Find which planet this station belongs to
        for (int i = 0; i < system->numPlanets && !parentPlanet; i++)
        {
            Planet *planet = &system->planets[i];
            if (!planet)
                continue;

            for (int j = 0; j < planet->numStations; j++)
            {
                if (planet->stations[j] == targetStation)
                {
                    parentPlanet = planet;
                    break;
                }
            }
        }

        if (!parentPlanet)
        {
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
    } // Calculate travel time
    uint32_t travelTime = calculate_travel_time(startDistance, endDistance); // Calculate energy requirement for travel
    double distanceDelta = fabs(endDistance - startDistance);
    double energyRequired = calculate_travel_energy_requirement(distanceDelta);
    double fuelRequired = calculate_travel_fuel_requirement(distanceDelta);

    // Check if player ship has enough energy and fuel
    extern struct PlayerShip *PlayerShipPtr;
    if (PlayerShipPtr != NULL)
    {
        // Check if there's enough energy
        if (PlayerShipPtr->attributes.energyBanks < energyRequired)
        {
            fprintf(stderr, "Error: Insufficient energy for travel.\n");
            printf("\nTravel aborted: Insufficient energy.\n");
            printf("Required: %.1f, Available: %.1f\n", energyRequired, PlayerShipPtr->attributes.energyBanks);
            return false;
        }

        // Check if there's enough fuel
        if (PlayerShipPtr->attributes.fuelLiters < fuelRequired)
        {
            fprintf(stderr, "Error: Insufficient fuel for travel.\n");
            printf("\nTravel aborted: Insufficient fuel.\n");
            printf("Required: %.3f liters, Available: %.1f liters\n", fuelRequired, PlayerShipPtr->attributes.fuelLiters);
            return false;
        }        // Consume energy
        PlayerShipPtr->attributes.energyBanks -= energyRequired;

        // Consume fuel using ConsumeFuel function
        if (!ConsumeFuel(fuelRequired, true)) {
            fprintf(stderr, "Error: Failed to consume fuel for travel.\n");
            printf("\nTravel aborted: Insufficient fuel for operation.\n");
            return false;
        }

        printf("\nTravel energy consumed: %.1f units", energyRequired);
        printf("\nTravel fuel consumed: %.3f liters (%.5f LY)", fuelRequired, fuelRequired / 100.0);
    }

    // Update game time
    game_time_advance(travelTime);

    // Update navigation state
    navState->currentLocationType = targetType;
    switch (targetType)
    {
    case CELESTIAL_STAR:
        navState->currentLocation.star = &system->centralStar;
        break;

    case CELESTIAL_PLANET:
        navState->currentLocation.planet = (Planet *)targetBody;
        break;
    case CELESTIAL_STATION:
        navState->currentLocation.station = (Station *)targetBody;

        // Update global location type to indicate we're at a station but not yet docked
        extern int PlayerLocationType;
        PlayerLocationType = 0; // We're at the station but not docked yet
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
static inline const char *celestial_type_to_string(CelestialType type)
{
    switch (type)
    {
    case CELESTIAL_STAR:
        return "Star";
    case CELESTIAL_PLANET:
        return "Planet";
    case CELESTIAL_STATION:
        return "Station";
    case CELESTIAL_NAV_BEACON:
        return "Nav Beacon";
    default:
        return "Unknown";
    }
}

// Function to get current location name with more context
static inline void get_current_location_name(NavigationState *navState, char *buffer, size_t bufferSize)
{
    if (!navState || !buffer || bufferSize == 0)
        return;

    switch (navState->currentLocationType)
    {
    case CELESTIAL_STAR:
        if (navState->currentLocation.star)
        {
            snprintf(buffer, bufferSize, "%s (Star)", navState->currentLocation.star->name);
        }
        else
        {
            snprintf(buffer, bufferSize, "Unknown Star");
        }
        break;

    case CELESTIAL_PLANET:
        if (navState->currentLocation.planet)
        {
            // Add planet type information for context
            const char *planetTypes[] = {
                "Rocky/Airless", "Terrestrial", "Gas Giant", "Ice Giant", "Unknown"};

            uint8_t type = navState->currentLocation.planet->type;
            if (type >= 4)
                type = 4; // Default to "Unknown" for invalid types

            snprintf(buffer, bufferSize, "%s (%s Planet)",
                     navState->currentLocation.planet->name,
                     planetTypes[type]);
        }
        else
        {
            snprintf(buffer, bufferSize, "Unknown Planet");
        }
        break;

    case CELESTIAL_STATION:
        if (navState->currentLocation.station)
        {
            // This requires a pointer to the current star system
            // If we don't have that in this context, we'll use a generic format
            snprintf(buffer, bufferSize, "%s (Orbital Station)",
                     navState->currentLocation.station->name);
        }
        else
        {
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

/**
 * Sets the current market to a station's market when docking
 *
 * @param station Pointer to the station being docked with
 * @param planet Pointer to the parent planet
 * @param planSys Pointer to the planet system data
 */
static inline void UseStationMarket(Station *station, Planet *planet, struct PlanSys *planSys)
{
    if (!station || !planSys) // Planet can be NULL for deep space stations
    {
        // fprintf(stderr, "Warning: UseStationMarket called with NULL station or planSys.\\n");
        // Optionally clear LocalMarket or set to a default empty state
        memset(&LocalMarket, 0, sizeof(MarketType));
        return;
    }

    // 1. Call UpdateStationMarket to ensure the market is up-to-date.
    // game_time_get_seconds() should be available from an included header like elite_state.h
    UpdateStationMarket(station, game_time_get_seconds(), planet, planSys);

    // 2. Copy the station's market data to the global LocalMarket.
    // Assuming LocalMarket is a global variable of type MarketType.
    LocalMarket = station->market;
}

// Forward declarations exist at the beginning of the file

/**
 * Sets the current market to a planet's market when landing
 *
 * @param planet Pointer to the planet being landed on
 * @param planSys Pointer to the planet system data
 */
static inline void UsePlanetaryMarket(Planet *planet, struct PlanSys *planSys)
{
    if (!planet || !planSys)
    {
        memset(&LocalMarket, 0, sizeof(MarketType));
        return;
    }
    
    // Ensure the planetary market is initialized if it hasn't been already
    if (!planet->planetaryMarket.isInitialized)
    {
        planet->planetaryMarket.market = GeneratePlanetaryMarket(planet, planSys);
    }
    else
    {
        // Ensure the market is up to date
        UpdatePlanetaryMarket(planet, game_time_get_seconds(), planSys);
    }

    // Set the global LocalMarket to this planet's market
    LocalMarket = planet->planetaryMarket.market;
}