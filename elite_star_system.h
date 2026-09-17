#pragma once

#include "elite_market.h"     // For market-related functions
#include "elite_navigation.h" // For NavigationState and CelestialType
#include "elite_navigation_types.h"
#include "elite_ship_maintenance.h"
#include "elite_state.h" // For plan_sys_t and other related structures
#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maximum number of planets per star system and stations per planet
#define MAX_PLANETS_PER_SYSTEM 8
#define MAX_STATIONS_PER_PLANET 5

// Realistic astronomical constants
#define SOLAR_MASS_KG 1.989e30
#define SOLAR_LUMINOSITY_WATTS 3.828e26
#define AU_TO_KM 149597870.7
#define EARTH_RADIUS_KM 6371.0

// Stellar classification data
typedef struct stellar_data_t {
    double minMass;     // Minimum mass for this class (solar masses)
    double maxMass;     // Maximum mass for this class (solar masses)
    double temperature; // Surface temperature (Kelvin)
    double lifetimeGyr; // Main sequence lifetime (billion years)
    double frequency;   // Relative frequency in galaxy (0.0-1.0)
} stellar_data_t;

// Structure for a star in a star system
typedef struct star_t {
    char name[MAX_LEN];        // Star name
    uint8_t spectralClass;     // O, B, A, F, G, K, M classification (0-6)
    double mass;               // Mass relative to Sol
    double luminosity;         // Luminosity relative to Sol
    double temperature;        // Surface temperature in Kelvin
    double age;                // Age in billion years
    double habitableZoneInner; // Inner edge of habitable zone (AU)
    double habitableZoneOuter; // Outer edge of habitable zone (AU)
} star_t;

// Structure for a planet in a star system
typedef struct planet_t {
    char name[MAX_LEN];                                  // planet_tname
    double orbitalDistance;                              // Distance from parent star in AU
    double radius;                                       // Radius in km
    double surfaceTemperature;                           // Average surface temperature in Kelvin
    uint8_t type;                                        // planet_ttype (gas giant, terrestrial, etc.)
    bool isInHabitableZone;                              // Whether planet is in habitable zone
    uint8_t numStations;                                 // Number of stations orbiting this planet
    struct station_t *stations[MAX_STATIONS_PER_PLANET]; // Pointers to station structures

    // Planetary market data for when landed on the planet
    struct {
        market_type_t market; // Market data for the planet's surface
        bool isInitialized;   // Whether the market has been initialized
    } planetaryMarket;
    uint8_t marketFluctuation; // Planet-specific market fluctuation factor
    uint64_t lastMarketUpdate; // Last game time when market was updated
} planet_t;

// Structure for a space station
typedef struct station_t {
    char name[MAX_LEN];        // station_t name
    double orbitalDistance;    // Distance from parent planet in AU
    uint8_t type;              // station_t type/class
    uint8_t size;              // station_t size (0=Small, 1=Medium, 2=Large)
    uint8_t services;          // Bitmask of available services
    bool hasDockingComputer;   // Whether automated docking is available
    bool hasShipyard;          // Whether ship equipment can be purchased
    bool hasMarket;            // Whether market trading is available
    bool hasMissions;          // Whether missions are available
    market_type_t market;      // station_t-specific market data
    uint8_t marketFluctuation; // station_t-specific market fluctuation factor (0-15)
    uint8_t specialization;    // Economic specialization (0: Balanced, 1:
                               // Industrial, 2: Agricultural, 3: Mining)
    uint64_t lastMarketUpdate; // Last game time when market was updated
} station_t;

// Structure for a complete star system
typedef struct StarSystem {
    struct plan_sys_t *plan_sys_t;            // Pointer to existing system info (economy, gov, etc.)
    star_t centralStar;                       // The central star of the system
    uint8_t numPlanets;                       // Number of planets in the system
    planet_t planets[MAX_PLANETS_PER_SYSTEM]; // Array of planets
    double navBeaconDistance;                 // Distance of nav beacon from central star in AU
} star_system_t;

// Forward function declarations
static inline market_type_t generate_station_market(station_t *station, planet_t *planet,
                                                    struct plan_sys_t *plan_sys_t);
static inline void update_station_market(station_t *station, uint64_t current_time, planet_t *planet,
                                         struct plan_sys_t *plan_sys_t);
static inline void use_station_market(station_t *station, planet_t *planet, struct plan_sys_t *plan_sys_t);
static inline market_type_t generate_planetary_market(planet_t *planet, struct plan_sys_t *plan_sys_t);
static inline void update_planetary_market(planet_t *planet, uint64_t current_time, struct plan_sys_t *plan_sys_t);
static inline void use_planetary_market(planet_t *planet, struct plan_sys_t *plan_sys_t);

// Realistic stellar classification data for main sequence stars
static const stellar_data_t STELLAR_CLASSES[7] = {
    // O-class: Hot, massive, short-lived blue giants (ultra-rare)
    {15.0, 90.0, 30000.0, 0.01, 0.000003},
    // B-class: Hot, blue-white stars (very rare)
    {2.1, 16.0, 20000.0, 0.4, 0.0001},
    // A-class: White stars (increased frequency to ensure they appear)
    {1.4, 2.1, 8500.0, 2.5, 0.006},
    // F-class: Yellow-white stars
    {1.04, 1.4, 6500.0, 7.0, 0.030},
    // G-class: Yellow stars like our Sun
    {0.8, 1.04, 5500.0, 10.0, 0.076},
    // K-class: Orange dwarf stars
    {0.45, 0.8, 4000.0, 50.0, 0.121},
    // M-class: Red dwarf stars (most common, adjusted to maintain total = 1.0)
    {0.08, 0.45, 3000.0, 1000.0, 0.766897}};

// Helper function to calculate luminosity from mass (L proportional to M^3.5
// for main sequence)
static inline double calculate_luminosity_from_mass(double mass) {
    if (mass <= 0.43) {
        // Very low mass stars have different scaling
        return 0.23 * pow(mass, 2.3);
    }
    if (mass < 2.0) {
        // Main sequence scaling
        return pow(mass, 4.0);
    }
    if (mass < 20.0) {
        // Massive stars have different scaling
        return 1.4 * pow(mass, 3.5);
    } // Very massive stars
    return pow(mass, 3.0);
}

// Helper function to calculate habitable zone boundaries
static inline void calculate_habitable_zone(double luminosity, double *inner_edge, double *outer_edge) {
    // Habitable zone based on liquid water temperatures (273-373K)
    // Using simplified calculations based on solar flux
    double luminosity_square_root = sqrt(luminosity);
    // Optimistic habitable zone (liquid water with greenhouse effects)
    *inner_edge = 0.85 * luminosity_square_root; // Inner edge (runaway greenhouse)
    *outer_edge = 1.7 * luminosity_square_root;  // Outer edge (maximum greenhouse effect)

    // Ensure minimum distances
    if (*inner_edge < 0.1) {
        *inner_edge = 0.1;
    }
    if (*outer_edge < *inner_edge + 0.2) {
        *outer_edge = *inner_edge + 0.2;
    }
}

// Helper function to calculate planet surface temperature
static inline double
calculate_planet_temperature(double stellar_luminosity, double orbital_distance,
                             double albedo) { // Stefan-Boltzmann law for planetary equilibrium temperature
    // T = (L * (1-A) / (16 * pi * sigma * d^2))^0.25 * T_sun
    // Simplified: T ~= 278.5 * (L/d^2)^0.25 * (1-A)^0.25

    double flux = stellar_luminosity / (orbital_distance * orbital_distance);
    double temperature = 278.5 * pow(flux, 0.25) * pow(1.0 - albedo, 0.25);

    return temperature;
}

// Function to generate the market for a station
static inline market_type_t generate_station_market(station_t *station, planet_t *planet,
                                                    struct plan_sys_t *plan_sys_t) {
    if (!station || !plan_sys_t) // planet_tcan be nullptr if station is not orbiting one (e.g. deep
                                 // space station, though plan implies planet context)
    {
        // Handle error or return a default/empty market
        market_type_t empty_market = {0}; // Initialize all members to zero
        // fprintf(stderr, "Warning: generate_station_market called with nullptr station
        // or plan_sys_t.\\n");
        return empty_market;
    } // Avoid unused parameter warning
    (void)planet;

    // 1. Generate a baseMarket using the existing generate_market function from
    // elite_market.h
    market_type_t base_market = generate_market(station->marketFluctuation, *plan_sys_t);

    // 2. Apply modifiers to baseMarket.price[i] and baseMarket.quantity[i] for
    // each commodity based on station->specialization Ensure
    // station->specialization is a valid enum value
    station_specialization_t specialization = (station_specialization_t)station->specialization;
    if (specialization >= NUM_STATION_SPECIALIZATIONS || specialization < 0) {
        // Optionally log this case:
        // fprintf(stderr, "Warning: station_t '%s' has invalid specialization value
        // %d. Defaulting to Balanced.\\n", station->name, station->specialization);
        specialization = STATION_SPECIALIZATION_BALANCED; // Default to balanced if out of bounds
    }

    for (int i = 0; i < NUM_STANDARD_COMMODITIES; i++) {
        // Get the modifier for the current commodity and station specialization
        market_modifier_t modifier = g_station_specialization_modifiers[specialization][i];

        // Apply price modifier
        float new_price = (float)base_market.price[i] * modifier.priceFactor;
        if (new_price < 0) {
            new_price = 0; // Price should not be negative
        }
        // Potentially clamp to a max price if one is defined: if (newPrice >
        // MAX_COMMODITY_PRICE) newPrice = MAX_COMMODITY_PRICE;
        float rounded_price = roundf(new_price);
        base_market.price[i] = (uint16_t)rounded_price; // Round to nearest integer for price

        // Apply quantity modifier
        float new_quantity = (float)base_market.quantity[i] * modifier.quantityFactor;
        if (new_quantity < 0) {
            new_quantity = 0; // Quantity should not be negative
        }
        // Clamp quantity to avoid overflow (uint16_t max is 65535).
        if (new_quantity > 0xFFFF) {
            new_quantity = 0xFFFF;
        }
        float rounded_quantity = roundf(new_quantity);
        base_market.quantity[i] = (uint16_t)rounded_quantity; // Round to nearest integer for quantity
    }

    // 3. Update station->lastMarketUpdate with the currentGameTimeSeconds.
    // As per the plan, this is handled by the caller (e.g.,
    // initialize_star_system or update_station_market). station->lastMarketUpdate =
    // currentGameTimeSeconds; // This line would be here if handled internally

    return base_market;
}

// Function to generate the market for a planet's surface
static inline market_type_t generate_planetary_market(planet_t *planet, struct plan_sys_t *plan_sys_t) {
    if (!planet || !plan_sys_t) {
        market_type_t empty_market = {0}; // Initialize all members to zero
        // fprintf(stderr, "Warning: GeneratePlanetaryMarket called with nullptr planet
        // or plan_sys_t.\\n");
        return empty_market;
    }

    // 1. Generate a baseMarket using generate_market from elite_market.h
    market_type_t base_market = generate_market(planet->marketFluctuation, *plan_sys_t);

    // 2. Apply modifiers based on planet->type
    planet_market_type_t planet_type = (planet_market_type_t)planet->type; // planet->type is uint8_t

    // Check if planetType is within the valid range for the planetTypeModifiers
    // array
    if (planet_type >= NUM_PLANET_MARKET_TYPES || planet_type < 0) // planet->type is uint8_t, so < 0 is only for
                                                                   // robustness if type changes
    {
        // fprintf(stderr, "Warning: planet_t'%s' (type %u) has invalid type for
        // market modifiers. Using base market without type-specific changes.\\n",
        // planet->name, planet->type); If type is out of bounds, no type-specific
        // modifiers are applied. The market remains the baseMarket.
    } else {
        for (int i = 0; i < NUM_STANDARD_COMMODITIES; i++) {
            market_modifier_t modifier = g_planet_type_modifiers[planet_type][i];

            // Apply price modifier
            float new_price = (float)base_market.price[i] * modifier.priceFactor;
            if (new_price < 0) {
                new_price = 0; // Price should not be negative
            }
            // Consider clamping to a MAX_PRICE if defined
            float rounded_price = roundf(new_price);
            base_market.price[i] = (uint16_t)rounded_price; // Round to nearest integer

            // Apply quantity modifier
            float new_quantity = (float)base_market.quantity[i] * modifier.quantityFactor;
            if (new_quantity < 0) {
                new_quantity = 0; // Quantity should not be negative
            }
            if (new_quantity > 0xFFFF) {
                new_quantity = 0xFFFF; // Clamp to uint16_t max (65535)
            }
            float rounded_quantity = roundf(new_quantity);
            base_market.quantity[i] = (uint16_t)rounded_quantity; // Round to nearest integer
        }
    }

    // 3. Update planet->lastMarketUpdate with currentGameTimeSeconds
    // Assuming game_time_get_seconds() is available globally or via included
    // headers (e.g., elite_state.h)
    planet->lastMarketUpdate = game_time_get_seconds();

    // 4. Set planet->planetaryMarket.isInitialized = 1
    // This is done here as per step III.4 of the plan.
    // The caller (e.g., initialize_star_system) will assign the returned market
    // to planet->planetaryMarket.market.
    planet->planetaryMarket.isInitialized = true;

    // 5. Return the modified market
    return base_market;
}

// Function to update the market for a station if enough time has passed
static inline void update_station_market(station_t *station, uint64_t current_time, planet_t *planet,
                                         struct plan_sys_t *plan_sys_t) // Added Planet* and plan_sys_t* params
{
    if (!station || !plan_sys_t) // planet_tcan be nullptr for deep space stations, but
                                 // plan_sys_t is essential
    {
        // fprintf(stderr, "Warning: update_station_market called with nullptr station or
        // plan_sys_t.\\n");
        return;
    }

    const uint64_t STATION_UPDATE_INTERVAL = 3600; // 1 hour in game seconds, as per existing findings

    // 1. Check if enough game time (UPDATE_INTERVAL) has passed since
    // lastMarketUpdate.
    if (current_time >= station->lastMarketUpdate &&
        (current_time - station->lastMarketUpdate >= STATION_UPDATE_INTERVAL)) {
        // 2. If so, calculate updateCycles.
        uint64_t elapsed_seconds = current_time - station->lastMarketUpdate;
        uint16_t update_cycles = (uint16_t)(elapsed_seconds / STATION_UPDATE_INTERVAL);

        if (update_cycles > 0) {
            // 3. Modify marketFluctuation based on updateCycles.
            // Simple cyclic increment for fluctuation. Max fluctuation is 15 (0-15
            // range).
            station->marketFluctuation = (station->marketFluctuation + update_cycles) % 16;

            // 4. Call generate_station_market to regenerate the market.
            // The generate_station_market function itself does not update
            // lastMarketUpdate.
            station->market = generate_station_market(station, planet, plan_sys_t);

            // 5. lastMarketUpdate is then set to the currentTime.
            // To prevent drift, set it to the time of the last completed interval, or
            // current time. Setting to currentTime is simpler as per plan.
            station->lastMarketUpdate = current_time;
        }
    }
}

// Function to update the market for a planet if enough time has passed
static inline void update_planetary_market(planet_t *planet, uint64_t current_time,
                                           struct plan_sys_t *plan_sys_t) // Added plan_sys_t* param
{
    if (!planet || !plan_sys_t) {
        (void)fprintf(stderr, "Warning: UpdatePlanetaryMarket called with nullptr planetor plan_sys_t.\\n");
        return;
    }

    const uint64_t PLANET_UPDATE_INTERVAL = 7200; // 2 hours in game seconds, as per existing findings

    // 1. Check if enough game time (UPDATE_INTERVAL) has passed since
    // lastMarketUpdate.
    if (current_time >= planet->lastMarketUpdate &&
        (current_time - planet->lastMarketUpdate >= PLANET_UPDATE_INTERVAL)) {
        // 2. If so, calculate updateCycles.
        uint64_t elapsed_seconds = current_time - planet->lastMarketUpdate;
        uint16_t update_cycles = (uint16_t)(elapsed_seconds / PLANET_UPDATE_INTERVAL);

        if (update_cycles > 0) {
            // 3. Modify marketFluctuation based on updateCycles.
            planet->marketFluctuation = (planet->marketFluctuation + update_cycles) % 16;

            // 4. Call GeneratePlanetaryMarket to regenerate the market.
            // GeneratePlanetaryMarket updates its own lastMarketUpdate and
            // isInitialized fields.
            market_type_t new_market = generate_planetary_market(planet, plan_sys_t);
            // The plan for GeneratePlanetaryMarket (Step III.4) says it updates
            // planet->lastMarketUpdate and sets isInitialized. However, the plan for
            // UpdatePlanetaryMarket (Step IV.5) also says lastMarketUpdate is set to
            // currentTime. To adhere to Step IV.5, we will explicitly set it here.
            // GeneratePlanetaryMarket already sets it, this will overwrite with the
            // exact currentTime.

            // The market data itself needs to be stored in
            // planet->planetaryMarket.market The plan for V. says: call
            // GeneratePlanetaryMarket(planet, plan_sys_tEntry) to populate
            // planet->planetaryMarket.market So, the GeneratePlanetaryMarket should
            // ideally return the market to be assigned. And indeed it does. We need
            // to assign it to the correct place in the planet_tstruct. The Planet
            // struct has: struct { MarketType market; bool isInitialized; }
            // planetaryMarket;
            planet->planetaryMarket.market = new_market;

            // 5. lastMarketUpdate is then set to the currentTime.
            planet->lastMarketUpdate = current_time;
            // planet->planetaryMarket.isInitialized is already set by
            // GeneratePlanetaryMarket
        }
    }
}

// Function to initialize a star system from a plan_sys_t entry
[[maybe_unused]] static inline void initialize_star_system(star_system_t *system, struct plan_sys_t *plan_sys_tEntry) {
    if (!system || !plan_sys_tEntry) {
        fprintf(stderr, "Error: Invalid parameters for star system initialization.\n");
        return;
    }

    // Link the existing plan_sys_t information
    system->plan_sys_t = plan_sys_tEntry; // ------------------------------------
    // Initialize the central star
    // ------------------------------------

    // Generate star name with variety based on system characteristics
    uint8_t nameVariant = (plan_sys_tEntry->goatSoupSeed.a % 3);
    switch (nameVariant) {
    case 0:
        snprintf(system->centralStar.name, MAX_LEN, "%s Prime", plan_sys_tEntry->name);
        break;
    case 1:
        snprintf(system->centralStar.name, MAX_LEN, "%s Star", plan_sys_tEntry->name);
        break;
    case 2:
        snprintf(system->centralStar.name, MAX_LEN, "%s Alpha", plan_sys_tEntry->name);
        break;
    } // Realistic spectral class distribution (M-class stars are most common)
    // Use cumulative probability distribution based on seed
    // Combine multiple seed components to get better distribution
    uint32_t seedCombined = ((uint32_t)plan_sys_tEntry->goatSoupSeed.a << 16) | plan_sys_tEntry->goatSoupSeed.b;
    double classRoll = ((double)(seedCombined % 1000000)) / 1000000.0;
    double cumulative = 0.0;
    system->centralStar.spectralClass = 6; // Default to M-class (most common)

    for (int i = 0; i < 7; i++) {
        cumulative += STELLAR_CLASSES[i].frequency;
        if (classRoll <= cumulative) {
            system->centralStar.spectralClass = i;
            break;
        }
    }

    // Generate realistic mass within spectral class range
    const stellar_data_t *star_data = &STELLAR_CLASSES[system->centralStar.spectralClass];
    double mass_range = star_data->maxMass - star_data->minMass;
    double mass_roll = ((double)(plan_sys_tEntry->goatSoupSeed.b % 1000)) / 1000.0;
    system->centralStar.mass = star_data->minMass + (mass_range * mass_roll);

    // Calculate realistic luminosity from mass
    system->centralStar.luminosity = calculate_luminosity_from_mass(system->centralStar.mass);

    // Set temperature based on spectral class with some variation
    double temp_variation = ((double)(plan_sys_tEntry->goatSoupSeed.c % 1000)) / 1000.0 - 0.5;
    system->centralStar.temperature =
        star_data->temperature * (1.0 + temp_variation * 0.1); // Generate stellar age (more realistic distribution)
    // Use a weighted distribution that favors older stars for more realistic
    // galactic population
    double universe_age = 13.8; // Age of universe in billion years
    double max_reasonable_age = (star_data->lifetimeGyr < universe_age) ? star_data->lifetimeGyr * 0.9 : universe_age;

    // Use a power-law distribution to favor older stars (typical galactic age
    // ~6-8 billion years)
    double age_roll = ((double)(plan_sys_tEntry->goatSoupSeed.d % 1000)) / 1000.0;

    // Apply power function to bias toward older ages (power of 0.5 makes
    // distribution more realistic) This gives average age around 5-7 billion
    // years instead of 1-2 billion
    double biased_age_roll = pow(age_roll, 0.5);

    // Add minimum age for stellar evolution (at least 0.1 billion years)
    double min_age = 0.1;
    system->centralStar.age = min_age + ((max_reasonable_age - min_age) * biased_age_roll);

    // Calculate habitable zone
    calculate_habitable_zone(system->centralStar.luminosity, &system->centralStar.habitableZoneInner,
                             &system->centralStar.habitableZoneOuter); // ------------------------------------
    // Determine number of planets
    // ------------------------------------

    // Number of planets based on stellar mass and age (more massive/older stars
    // tend to have more planets) Also factor in tech level as a proxy for
    // exploration thoroughness
    int base_planets = 2; // Minimum planets

    // Stellar mass factor (more massive stars can capture more material)
    if (system->centralStar.mass > 1.5) {
        base_planets += 2;
    } else if (system->centralStar.mass > 1.0) {
        base_planets += 1;
    } else if (system->centralStar.mass < 0.5) {
        base_planets -= 1;
    }

    // Age factor (older systems have had more time for planet formation)
    if (system->centralStar.age > 5.0) {
        base_planets += 1;
    }

    // Tech level factor (better detection of distant planets)
    base_planets += (plan_sys_tEntry->techLev / 3);

    // Random variation
    int planet_variation = (plan_sys_tEntry->goatSoupSeed.c % 3) - 1; // -1, 0, or 1
    system->numPlanets = (uint8_t)(base_planets + planet_variation);

    // Clamp to valid range
    if (system->numPlanets < 1) {
        system->numPlanets = 1;
    }
    if (system->numPlanets > MAX_PLANETS_PER_SYSTEM) {
        system->numPlanets = MAX_PLANETS_PER_SYSTEM; // ------------------------------------
    }
    // Set Nav Beacon position
    // ------------------------------------

    // Nav beacon distance varies based on system size and habitable zone
    // Place it beyond the outermost likely planet orbit
    double system_radius = system->centralStar.habitableZoneOuter * 3.0; // 3x habitable zone outer edge
    system->navBeaconDistance = system_radius + ((double)(plan_sys_tEntry->techLev) * 0.5);

    // ------------------------------------
    // Initialize planets
    // ------------------------------------
    for (int i = 0; i < system->numPlanets; i++) {
        planet_t *planet = &system->planets[i];

        // Set planet name with more variety based on position and system
        // characteristics
        if (i == 0) {
            // First planet often shares system name
            uint8_t name_variant = (plan_sys_tEntry->goatSoupSeed.b % 2);
            if (name_variant == 0) {
                snprintf(planet->name, MAX_LEN, "%s", plan_sys_tEntry->name);
            } else {
                snprintf(planet->name, MAX_LEN, "%s Prime", plan_sys_tEntry->name);
            }
        } else if (i == 1) {
            // Second planet often has "New" prefix
            uint8_t name_variant = (plan_sys_tEntry->goatSoupSeed.c % 3);
            if (name_variant == 0) {
                int written = snprintf(planet->name, MAX_LEN, "New %s", plan_sys_tEntry->name);
                if (written < 0 || written >= MAX_LEN) {
                    planet->name[MAX_LEN - 1] = '\0';
                }
            } else if (name_variant == 1) {
                int written = snprintf(planet->name, MAX_LEN, "%s II", plan_sys_tEntry->name);
                if (written < 0 || written >= MAX_LEN) {
                    planet->name[MAX_LEN - 1] = '\0';
                }
            } else {
                int written = snprintf(planet->name, MAX_LEN, "%s Beta", plan_sys_tEntry->name);
                if (written < 0 || written >= MAX_LEN) {
                    planet->name[MAX_LEN - 1] = '\0';
                }
            }
        } else {
            // Other planets get variety in naming
            uint8_t name_variant = (uint8_t)((plan_sys_tEntry->goatSoupSeed.d + i) % 4);
            if (name_variant == 0) {
                int written = snprintf(planet->name, MAX_LEN, "%s %c", plan_sys_tEntry->name, 'A' + i);
                if (written < 0 || written >= MAX_LEN) {
                    planet->name[MAX_LEN - 1] = '\0';
                }
            } else if (name_variant == 1) {
                int written = snprintf(planet->name, MAX_LEN, "%s %d", plan_sys_tEntry->name, i + 1);
                if (written < 0 || written >= MAX_LEN) {
                    planet->name[MAX_LEN - 1] = '\0';
                }
            } else if (name_variant == 2) {
                const char *suffixes[] = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta"};
                int written = snprintf(planet->name, MAX_LEN, "%s %s", plan_sys_tEntry->name, suffixes[i % 8]);
                if (written < 0 || written >= MAX_LEN) {
                    planet->name[MAX_LEN - 1] = '\0';
                }
            } else {
                // Generate a slightly different name using seed
                char alt_name[MAX_LEN];
                uint32_t name_seed = plan_sys_tEntry->goatSoupSeed.a + ((uint32_t)i * plan_sys_tEntry->goatSoupSeed.b);
                uint8_t name_len = (name_seed % 4) + 3; // 3-6 letter name

                for (int j = 0; j < name_len; j++) {
                    name_seed *=
                        UINT32_C(2654435761); // Knuth's multiplicative hash; uint32_t arithmetic wraps modulo 2^32
                    static const char ALPHABET[] = "abcdefghijklmnopqrstuvwxyz";
                    char letter = ALPHABET[name_seed % 26];
                    if (j == 0)
                        letter = (char)toupper((unsigned char)letter);
                    alt_name[j] = letter;
                }
                alt_name[name_len] = '\0';

                int written = snprintf(planet->name, MAX_LEN, "%s", alt_name);
                if (written < 0 || written >= MAX_LEN) {
                    planet->name[MAX_LEN - 1] = '\0';
                }
            }
        } // Set orbital distance with physics-based constraints and enhanced stability
          // Use modified Titius-Bode law with Hill sphere and resonane considerations
        double base_distance = 0.0;

        if (i == 0) {
            // First planet: more conservative inner placement to avoid instability
            uint32_t inner_seed = plan_sys_tEntry->goatSoupSeed.a + plan_sys_tEntry->goatSoupSeed.b;
            if ((inner_seed % 10) < 2) {                                    // Reduced from 3 to 2 (20% vs 30%)
                base_distance = 0.25 + ((double)(inner_seed % 20) / 100.0); // 0.25-0.45 AU (slightly farther)
            } else {
                base_distance = 0.5 + ((double)(inner_seed % 30) / 100.0); // 0.5-0.8 AU (more conservative)
            }
        } else {
            // Subsequent planets use enhanced spacing for stability
            double previous_distance = system->planets[i - 1].orbitalDistance;

            // Calculate minimum separation based on Hill sphere approximation
            double stellar_mass = system->centralStar.mass;
            double hill_sphere_radius =
                previous_distance * pow(stellar_mass / 3.0, 1.0 / 3.0) * 2.5; // Enhanced Hill sphere
            double minimum_separation = hill_sphere_radius * 3.0;             // 3x Hill sphere for stability

            // Use spacing multiplier that respects physical constraints
            double spacing_multiplier =
                1.6 + ((double)((plan_sys_tEntry->goatSoupSeed.d + i) % 60) / 100.0); // 1.6-2.2 (more conservative)
            base_distance = previous_distance * spacing_multiplier;

            // Ensure minimum separation is respected
            if (base_distance - previous_distance < minimum_separation) {
                base_distance = previous_distance + minimum_separation;
            }

            // Avoid unstable resonances (2:1, 3:1, 3:2)
            double ratio = base_distance / previous_distance;
            if ((ratio > 1.9 && ratio < 2.1) || (ratio > 2.9 && ratio < 3.1) || (ratio > 1.45 && ratio < 1.55)) {
                // Adjust to avoid resonance
                base_distance *= 1.15; // Push out of resonance zone
            }
        }

        // Reduced randomization for better stability
        double variability =
            ((double)((plan_sys_tEntry->goatSoupSeed.d + i * 17) % 100) / 200.0) - 0.25; // -0.25 to 0.25
        planet->orbitalDistance = base_distance * (1.0 + (variability * 0.4));           // Reduced from 0.8 to 0.4
        // Further reduced habitable zone bias to minimize super-habitable planets
        uint32_t habitable_bias = (plan_sys_tEntry->goatSoupSeed.c + i) % 100;
        if (habitable_bias < 8 && i < system->numPlanets - 1) { // Reduced from 15% to 8% chance
            double habitable_zone_center =
                (system->centralStar.habitableZoneInner + system->centralStar.habitableZoneOuter) / 2.0;
            double bias_strength = 0.15; // Reduced from 0.25 to 0.15 (much weaker bias)
            planet->orbitalDistance =
                (planet->orbitalDistance * (1.0 - bias_strength)) + (habitable_zone_center * bias_strength);
        }

        // Enhanced minimum distance check based on stellar type and planet size
        double minimum_distance = 0.1 + ((system->centralStar.mass - 1.0) * 0.05); // Scales with stellar mass
        if (planet->orbitalDistance < minimum_distance) {
            planet->orbitalDistance = minimum_distance;
        }

        // Check if planet is in habitable zone
        planet->isInHabitableZone = (planet->orbitalDistance >= system->centralStar.habitableZoneInner &&
                                     planet->orbitalDistance <= system->centralStar.habitableZoneOuter);

        // Calculate surface temperature (assuming Earth-like albedo of 0.3)
        planet->surfaceTemperature =
            calculate_planet_temperature(system->centralStar.luminosity, planet->orbitalDistance,
                                         0.3); // Determine planet type based on distance from star, temperature,
                                               // and probabilistic factors
        uint32_t type_seed = plan_sys_tEntry->goatSoupSeed.a + ((uint32_t)(i)*plan_sys_tEntry->goatSoupSeed.b);
        uint8_t type_roll = type_seed % 100;

        if (planet->orbitalDistance < system->centralStar.habitableZoneInner * 0.4) {
            // Very close to star - always rocky/airless world
            planet->type = 0; // Rocky/Airless        } else if (planet->isInHabitableZone) {
            // In habitable zone - more conservative terrestrial world generation
            if (type_roll < 75) {        // Reduced from 85%
                planet->type = 1;        // Terrestrial (75% chance)
            } else if (type_roll < 90) { // Increased rocky/airless chance
                planet->type = 0;        // Rocky/Airless (15% chance)
            } else {
                planet->type = 2; // Gas Giant (10% chance - slightly higher for variety)
            }
        } else if (planet->orbitalDistance < system->centralStar.habitableZoneOuter * 2.0) {
            // Near habitable zone - mix of terrestrial and gas giants
            if (type_roll < 50) {
                planet->type = 1; // Terrestrial (50% chance)
            } else if (type_roll < 80) {
                planet->type = 2; // Gas Giant (30% chance)
            } else {
                planet->type = 0; // Rocky/Airless (20% chance)
            }
        } else if (planet->orbitalDistance < system->centralStar.habitableZoneOuter * 8.0) {
            // Outer system - favor gas giants but include some terrestrial
            if (type_roll < 60) {
                planet->type = 2; // Gas Giant (60% chance)
            } else if (type_roll < 85) {
                planet->type = 3; // Ice Giant (25% chance)
            } else {
                planet->type = 1; // Terrestrial (15% chance - cold super-Earths)
            }
        } else {
            // Very far from star - ice giants and some gas giants
            if (type_roll < 70) {
                planet->type = 3; // Ice Giant (70% chance)
            } else {
                planet->type = 2; // Gas Giant (30% chance)
            }
        }

        // Adjust planet type based on stellar mass (massive stars can have gas
        // giants closer in)
        if (system->centralStar.mass > 2.0 && planet->orbitalDistance > 1.0) {
            // Massive stars can have gas giants closer in
            if (planet->type == 1 && (plan_sys_tEntry->goatSoupSeed.a + i) % 3 == 0) {
                planet->type = 2; // Convert some terrestrial to gas giant
            }
        }

        // Set realistic planet radius based on type and formation conditions
        double base_radius = 0.0;
        uint32_t radius_seed = plan_sys_tEntry->goatSoupSeed.b + i * 1009; // Use different seed offset

        switch (planet->type) {
        case 0:                                                    // Rocky/Airless (Mercury-like to Mars-like)
            base_radius = 2400.0 + ((double)(radius_seed % 3600)); // 2,400-6,000 km
            break;
        case 1: // Terrestrial (Mars-like to super-Earth, more conservative)
            base_radius = 3400.0 + ((double)(radius_seed % 5600)); // 3,400-9,000 km (reduced max)
            // Planets in habitable zone are more Earth-like, less super-Earth
            if (planet->isInHabitableZone) {
                base_radius = 5800.0 + ((double)(radius_seed % 2400)); // 5,800-8,200 km (narrower, Earth-like range)
            }
            break;
        case 2:                                                      // Gas Giant (Neptune to Jupiter and beyond)
            base_radius = 24000.0 + ((double)(radius_seed % 46000)); // 24,000-70,000 km
            break;
        case 3:                                                      // Ice Giant (Uranus/Neptune-like)
            base_radius = 20000.0 + ((double)(radius_seed % 30000)); // 20,000-50,000 km
            break;
        }

        planet->radius = base_radius;

        // Initialize planetary market fluctuation factor
        planet->marketFluctuation = (plan_sys_tEntry->goatSoupSeed.b + i) % 16; // 0-15 fluctuation

        // Initialize the planetary market (this also sets lastMarketUpdate and
        // isInitialized)
        planet->planetaryMarket.market = generate_planetary_market(planet, plan_sys_tEntry);
        // Ensure lastMarketUpdate is set by GeneratePlanetaryMarket, or set it here
        // if needed. Per plan, GeneratePlanetaryMarket handles its own
        // lastMarketUpdate.        // Determine number of stations for this planet
        // More developed systems (higher tech) have more stations
        // Habitable planets and terrestrial worlds are more likely to have stations
        uint8_t max_stations = 0;

        if (planet->isInHabitableZone) {
            // Habitable zone planets get the most stations
            max_stations = (plan_sys_tEntry->techLev >= 8) ? MAX_STATIONS_PER_PLANET : 4;
        } else if (planet->type <= 1) {
            // Rocky/Terrestrial planets
            max_stations = (plan_sys_tEntry->techLev >= 8) ? 3 : 2;
        } else if (planet->type == 2) {
            // Gas giants (good for fuel and mining)
            max_stations = (plan_sys_tEntry->techLev >= 10) ? 2 : 1;
        } else {
            // Ice giants (least attractive)
            max_stations = (plan_sys_tEntry->techLev >= 12) ? 1 : 0;
        }

        // Reduce stations for very hot or very cold planets
        if (planet->surfaceTemperature > 400.0 || planet->surfaceTemperature < 200.0) {
            if (max_stations > 0) {
                max_stations--; // Harsh environments get fewer stations
            }
        }
        planet->numStations = (plan_sys_tEntry->goatSoupSeed.d + i) % (max_stations + 1);

        // Initialize stations for this planet
        for (int j = 0; j < planet->numStations; j++) {
            // Allocate memory for station
            station_t *station = (station_t *)malloc(sizeof(station_t));
            if (!station) {
                int diagnostic_result = fprintf(stderr, "Error: Memory allocation failed for station.\n");
                if (diagnostic_result < 0) {
                    clearerr(stderr);
                }
                continue; // Memory allocation failed
            }

            // Set station name with more variety
            int station_name_variant = (plan_sys_tEntry->goatSoupSeed.a + i + j) % 4;
            if (station_name_variant == 0) {
                snprintf(station->name, MAX_LEN, "%s station_t %d", planet->name, j + 1);
            } else if (station_name_variant == 1) {
                const char *prefixes[] = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon"};
                snprintf(station->name, MAX_LEN, "%s %s", prefixes[j % 5], planet->name);
            } else if (station_name_variant == 2) {
                const char *prefixes[] = {"Orbital", "Port", "Hub", "Gateway", "Outpost"};
                snprintf(station->name, MAX_LEN, "%s %s", prefixes[j % 5], planet->name);
            } else {
                const char *uniqueNames[] = {"Nexus", "StarPort", "Horizon", "Tranquility", "Zenith"};
                snprintf(station->name, MAX_LEN, "%s %s", uniqueNames[j % 5], planet->name);
            } // Set orbital distance from planet - realistic based on planet type and
              // safety
            double base_orbit_distance = 0.0;
            if (planet->type <= 1) {
                // Rocky/Terrestrial planets - closer orbits for easier access
                base_orbit_distance =
                    0.002 + ((double)((plan_sys_tEntry->goatSoupSeed.b + j) % 8) / 1000.0); // 0.002-0.010 AU
            } else if (planet->type == 2) {
                // Gas giants - farther orbits to avoid radiation and gravitational
                // stress
                base_orbit_distance =
                    0.01 + ((double)((plan_sys_tEntry->goatSoupSeed.c + j) % 15) / 1000.0); // 0.010-0.025 AU
            } else {
                // Ice giants - moderate orbits
                base_orbit_distance =
                    0.005 + ((double)((plan_sys_tEntry->goatSoupSeed.d + j) % 10) / 1000.0); // 0.005-0.015 AU
            }

            // For habitable zone planets, keep stations close for easy access
            if (planet->isInHabitableZone) {
                base_orbit_distance *= 0.7; // 30% closer for habitable worlds
            }

            station->orbitalDistance = base_orbit_distance;

            // Set station type based on tech level, planet type, and environmental
            // conditions
            if (plan_sys_tEntry->techLev >= 10) {
                station->type = (plan_sys_tEntry->goatSoupSeed.d + j) % 3; // 0-2 for high tech
            } else if (plan_sys_tEntry->techLev >= 5) {
                station->type = (plan_sys_tEntry->goatSoupSeed.a + j) % 2; // 0-1 for medium tech
            } else {
                station->type = 0; // Only basic stations for low tech
            }

            // Set station services based on tech level, planet type, and conditions
            station->hasDockingComputer =
                (((plan_sys_tEntry->techLev >= 8) || ((plan_sys_tEntry->goatSoupSeed.b + j) % 5 == 0)) != 0);

            // Shipyards more common around habitable and terrestrial worlds
            bool shipyard_bonus = (planet->isInHabitableZone || (planet->type <= 1)) != 0;
            station->hasShipyard = (((plan_sys_tEntry->techLev >= 5) ||
                                     (shipyard_bonus && (plan_sys_tEntry->goatSoupSeed.c + j) % 3 == 0) ||
                                     ((plan_sys_tEntry->goatSoupSeed.c + j) % 4 == 0)) != 0);

            station->hasMarket = true; // All stations have markets

            // Missions more common in populated (habitable) systems
            bool mission_bonus = planet->isInHabitableZone;
            station->hasMissions = (((plan_sys_tEntry->techLev >= 3) ||
                                     (mission_bonus && (plan_sys_tEntry->goatSoupSeed.d + j) % 2 == 0) ||
                                     ((plan_sys_tEntry->goatSoupSeed.d + j) % 3 == 0)) != 0);

            // Set economic specialization based on planet type, habitability, and
            // environmental conditions Generate specialization: 0=Balanced,
            // 1=Industrial, 2=Agricultural, 3=Mining
            if (planet->isInHabitableZone) {
                // Habitable zone planets favor agricultural and balanced economies
                uint8_t spec_roll = (uint8_t)((plan_sys_tEntry->goatSoupSeed.a + i + j) % 10);
                if (spec_roll < 5) {
                    station->specialization = 2; // Agricultural
                } else if (spec_roll < 7) {
                    station->specialization = 1; // Industrial
                } else {
                    station->specialization = 0; // Balanced
                }
            } else if (planet->type <= 1) {
                // Rocky/Terrestrial planets outside habitable zone - mining and
                // industrial
                uint8_t spec_roll = (uint8_t)((plan_sys_tEntry->goatSoupSeed.b + i + j) % 10);
                if (spec_roll < 4) {
                    station->specialization = 3; // Mining
                } else if (spec_roll < 8) {
                    station->specialization = 1; // Industrial
                } else {
                    station->specialization = 0; // Balanced
                }
            } else {
                // Gas and Ice Giants - primarily mining and industrial
                uint8_t spec_roll = (uint8_t)((plan_sys_tEntry->goatSoupSeed.c + i + j) % 10);
                if (spec_roll < 7) {
                    station->specialization = 3; // Mining (fuel processing, etc.)
                } else if (spec_roll < 9) {
                    station->specialization = 1; // Industrial
                } else {
                    station->specialization = 0; // Balanced
                }
            }

            // Economy modifiers based on system's economy type
            if (plan_sys_tEntry->economy < 4) { // Industrial economies (0-3)
                if (station->specialization == 0) {
                    station->specialization = 1; // More likely to be industrial
                }
            } else { // Agricultural economies (4-7)
                if (station->specialization == 0) {
                    station->specialization = 2; // More likely to be agricultural
                }
            }

            // Initialize market fluctuation factor
            station->marketFluctuation = (uint8_t)((plan_sys_tEntry->goatSoupSeed.c + i + j) % 16); // 0-15 fluctuation

            // Initialize the last market update time to current game time
            // This is important so that the first call to update_station_market doesn't
            // immediately regenerate.
            station->lastMarketUpdate = game_time_get_seconds();

            // Generate the station's market
            station->market = generate_station_market(station, planet, plan_sys_tEntry); // Link station to planet
            planet->stations[j] = station;
        }
    }
    // Post-generation check: Ensure populated systems have adequate station
    // infrastructure This addresses the logical inconsistency where systems claim
    // billions of inhabitants but have no stations (infrastructure) for them to
    // live in
    uint64_t total_system_population = (plan_sys_tEntry->population >> 3); // Convert to billions like in display
    int total_stations = 0;

    // Count existing stations
    for (int i = 0; i < system->numPlanets; i++) {
        total_stations += system->planets[i].numStations;
    }

    // Determine minimum stations required based on population
    int minStationsRequired = 0;
    if (total_system_population >= 4) {        // 4+ billion people
        minStationsRequired = 3;               // Major population centers need multiple stations
    } else if (total_system_population >= 2) { // 2+ billion people
        minStationsRequired = 2;               // Large populations need at least 2 stations
    } else if (total_system_population >= 1) { // 1+ billion people
        minStationsRequired = 1;               // Moderate populations need at least 1 station
    }

    // If we don't have enough stations for the population, add them
    if (total_stations < minStationsRequired) {
        int stations_to_add = minStationsRequired - total_stations; // Find the most suitable planets to add stations to
        // Priority: Rock/Earth-like planets first, then others
        for (int add_count = 0; add_count < stations_to_add; add_count++) {
            planet_t *best_planet = nullptr;
            int best_priority = -1;

            // Find the best planet that can accommodate another station
            for (int i = 0; i < system->numPlanets; i++) {
                planet_t *planet = &system->planets[i];

                // Skip planets that already have maximum reasonable stations
                if (planet->numStations >= 4) {
                    continue;
                }

                int priority = 0;
                // Rock and Earth-like planets are best for habitation
                // planet_ttypes: 0=Rocky, 1=Terrestrial, 2=Gas Giant, 3=Ice Giant
                if (planet->type == 0 || planet->type == 1) {
                    priority = 3; // Rocky/Terrestrial planets best for habitation
                } else if (planet->type == 2) {
                    priority = 2; // Gas giants for fuel processing
                } else {
                    priority = 1; // Ice giants as last resort
                }

                // Prefer planets with fewer existing stations (spread them out)
                priority = (priority * 10) - planet->numStations;

                if (priority > best_priority) {
                    best_priority = priority;
                    best_planet = planet;
                }
            }
            // Add a station to the best planet found
            if (best_planet && best_planet->numStations < MAX_STATIONS_PER_PLANET) {
                station_t *new_station = (station_t *)malloc(sizeof(station_t));
                if (new_station) {
                    // Initialize the station structure
                    memset(new_station, 0, sizeof(station_t));

                    // Set up the emergency station with basic properties
                    int name_length = snprintf(new_station->name, sizeof(new_station->name), "Orbital Hab %c",
                                               'A' + best_planet->numStations);
                    if (name_length < 0 || (size_t)name_length >= sizeof(new_station->name)) {
                        free(new_station);
                        new_station = NULL;
                    }
                    new_station->type = 0;        // Coriolis (most common)
                    new_station->size = 1;        // Medium size
                    new_station->services = 0xFF; // All services available for populated areas

                    // Set orbital distance from planet
                    if (best_planet->type <= 1) {
                        // Rocky/Terrestrial planets - closer orbits for easier access
                        new_station->orbitalDistance =
                            0.002 + ((double)((plan_sys_tEntry->goatSoupSeed.b + best_planet->numStations) % 8) /
                                     1000.0); // 0.002-0.010 AU
                    } else if (best_planet->type == 2) {
                        // Gas giants - farther orbits to avoid radiation and gravitational
                        // stress
                        new_station->orbitalDistance =
                            0.01 + ((double)((plan_sys_tEntry->goatSoupSeed.c + best_planet->numStations) % 15) /
                                    1000.0); // 0.010-0.025 AU
                    } else {
                        // Ice giants - moderate orbits
                        new_station->orbitalDistance =
                            0.005 + ((double)((plan_sys_tEntry->goatSoupSeed.d + best_planet->numStations) % 10) /
                                     1000.0); // 0.005-0.015 AU
                    }

                    // Initialize all station services for populated systems
                    new_station->hasDockingComputer = true; // High population areas need docking computers
                    new_station->hasShipyard = true;        // Major population centers have shipyards
                    new_station->hasMarket = true;          // All stations have markets
                    new_station->hasMissions = true;        // Populated areas have missions

                    // Set specialization based on planet type
                    // planet_ttypes: 0=Rocky, 1=Terrestrial, 2=Gas Giant, 3=Ice Giant
                    if (best_planet->type == 0 || best_planet->type == 1) {
                        new_station->specialization = 0; // Balanced for habitation
                    } else if (best_planet->type == 2) {
                        new_station->specialization = 3; // Mining (gas giant fuel processing)
                    } else {
                        new_station->specialization = 1; // Industrial for ice giants
                    }

                    // Initialize market
                    new_station->marketFluctuation = (plan_sys_tEntry->goatSoupSeed.a + best_planet->numStations) % 16;
                    new_station->lastMarketUpdate = game_time_get_seconds();
                    new_station->market = generate_station_market(new_station, best_planet, plan_sys_tEntry);

                    // Link to planet
                    best_planet->stations[best_planet->numStations] = new_station;
                    best_planet->numStations++;
                }
            }
        }
    }
}

// Function to clean up allocated memory for a star system
[[maybe_unused]] static inline void cleanup_star_system(star_system_t *system) {
    if (!system) {
        return;
    }

    // Free memory for each station
    for (int i = 0; i < system->numPlanets; i++) {
        planet_t *planet = &system->planets[i];
        if (!planet) {
            continue;
        }

        for (int j = 0; j < planet->numStations; j++) {
            if (planet->stations[j]) {
                free(planet->stations[j]);
                planet->stations[j] = nullptr;
            }
        }

        // Reset station count to avoid accessing freed memory
        planet->numStations = 0;
    }

    // Reset planet count to avoid accessing invalid data
    system->numPlanets = 0;

    // Don't free system->plan_sys_t as it's managed elsewhere
    system->plan_sys_t = nullptr;
}

// Function to get planet from a star system by index
[[maybe_unused]] static inline planet_t *get_planet_by_index(star_system_t *system, uint8_t index) {
    if (!system || index >= system->numPlanets) {
        return nullptr;
    }
    return &system->planets[index];
}

// Function to get station from a planet by index
[[maybe_unused]] static inline struct station_t *get_station_by_index(planet_t *planet, uint8_t index) {
    if (!planet || index >= planet->numStations) {
        return nullptr;
    }
    return planet->stations[index];
}

// Function to calculate travel time between two points in a system
// Returns time in seconds
static inline uint32_t calculate_travel_time(double start_distance, double end_distance) {
    // Simple model: 1 AU = 20 minutes of travel
    const double TRAVEL_SPEED_AU_PER_MINUTE = 0.05; // 0.05 AU per minute
    const uint32_t SECONDS_PER_MINUTE = 60;

    double distance_delta = fabs(end_distance - start_distance);
    double time_in_minutes = distance_delta / TRAVEL_SPEED_AU_PER_MINUTE;

    return (uint32_t)(time_in_minutes * SECONDS_PER_MINUTE);
}

// Function to travel to a celestial body within a star system
// Updates navigation state and game time
[[maybe_unused]] static inline bool travel_to_celestial(star_system_t *system, navigation_state_t *nav_state,
                                                        celestial_type_t target_type, void *target_body) {
    if (!system) {
        fprintf(stderr, "Error: Invalid star system data for travel.\n");
        return false;
    }

    if (!nav_state) {
        fprintf(stderr, "Error: Invalid navigation state for travel.\n");
        return false;
    }

    // For non-NavBeacon targets, we need a valid body pointer
    if (target_type != CELESTIAL_NAV_BEACON && !target_body) {
        fprintf(stderr, "Error: Invalid target body for travel destination.\n");
        return false;
    }

    double start_distance = nav_state->distanceFromStar;
    double end_distance = 0.0;

    // Determine target distance based on type
    switch (target_type) {
    case CELESTIAL_STAR:
        end_distance = 0.0; // Star is at center
        break;

    case CELESTIAL_PLANET: {
        planet_t *target_planet = (planet_t *)target_body;

        // Validate the planet is part of this system
        bool planet_found = false;
        for (int i = 0; i < system->numPlanets; i++) {
            if (&system->planets[i] == target_planet) {
                planet_found = true;
                break;
            }
        }

        if (!planet_found) {
            fprintf(stderr, "Error: Target planet is not part of the current star system.\n");
            return false;
        }

        end_distance = target_planet->orbitalDistance;
        break;
    }

    case CELESTIAL_STATION: {
        // Need to find parent planet
        station_t *target_station = (station_t *)target_body;
        planet_t *parent_planet = nullptr;

        // Find which planet this station belongs to
        for (int i = 0; i < system->numPlanets && !parent_planet; i++) {
            planet_t *planet = &system->planets[i];
            if (!planet) {
                continue;
            }

            for (int j = 0; j < planet->numStations; j++) {
                if (planet->stations[j] == target_station) {
                    parent_planet = planet;
                    break;
                }
            }
        }

        if (!parent_planet) {
            fprintf(stderr, "Error: Could not find parent planet for target station.\n");
            return false;
        }

        // station_t distance is planet distance plus orbital offset
        end_distance = parent_planet->orbitalDistance + target_station->orbitalDistance;
        break;
    }

    case CELESTIAL_NAV_BEACON:
        end_distance = system->navBeaconDistance;
        break;

    default:
        fprintf(stderr, "Error: Unknown celestial type for travel destination.\n");
        return false;
    } // Calculate travel time
    uint32_t travel_time = calculate_travel_time(start_distance, end_distance); // Calculate fuel requirement for travel
    double distance_delta = fabs(end_distance - start_distance);
    double fuel_required = calculate_travel_fuel_requirement(distance_delta);

    // Check if player ship has enough fuel
    if (g_state.PlayerShipPtr != nullptr) {
        // Check if there's enough fuel
        if (g_state.PlayerShipPtr->attributes.fuelLiters < fuel_required) {
            fprintf(stderr, "Error: Insufficient fuel for travel.\n");
            printf("\nTravel aborted: Insufficient fuel.\n");
            printf("Required: %.3f liters, Available: %.1f liters\n", fuel_required,
                   g_state.PlayerShipPtr->attributes.fuelLiters);
            return false;
        }

        // Consume fuel using ConsumeFuel function
        if (!consume_fuel(fuel_required, 1)) {
            fprintf(stderr, "Error: Failed to consume fuel for travel.\n");
            printf("\nTravel aborted: Insufficient fuel for operation.\n");
            return false;
        }

        printf("\nTravel fuel consumed: %.3f liters (%.5f LY)", fuel_required, fuel_required / 100.0);
    }

    // Update game time
    game_time_advance(travel_time);

    // Update navigation state
    nav_state->currentLocationType = target_type;
    switch (target_type) {
    case CELESTIAL_STAR:
        nav_state->currentLocation.star = &system->centralStar;
        break;

    case CELESTIAL_PLANET:
        nav_state->currentLocation.planet = (planet_t *)target_body;
        break;
    case CELESTIAL_STATION:
        nav_state->currentLocation.station = (station_t *)target_body;

        // Update global location type to indicate we're at a station but not yet
        // docked
        g_state.PlayerLocationType = 0; // We're at the station but not docked yet
        break;

    case CELESTIAL_NAV_BEACON:
        // Nav beacon doesn't need a specific structure reference
        // Just ensure the currentLocation union doesn't contain garbage
        memset(&nav_state->currentLocation, 0, sizeof(nav_state->currentLocation));
        break;
    }

    nav_state->distanceFromStar = end_distance;

    return true;
}

// Function to convert celestial type to string for display
[[maybe_unused]] static inline const char *celestial_type_to_string(celestial_type_t type) {
    switch (type) {
    case CELESTIAL_STAR:
        return "Star";
    case CELESTIAL_PLANET:
        return "Planet";
    case CELESTIAL_STATION:
        return "station_t";
    case CELESTIAL_NAV_BEACON:
        return "Nav Beacon";
    default:
        return "Unknown";
    }
}

// Function to get current location name with more context
[[maybe_unused]] static inline void get_current_location_name(navigation_state_t *navState, char *buffer,
                                                              size_t bufferSize) {
    if (!navState || !buffer || bufferSize == 0) {
        return;
    }

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
            const char *planetTypes[] = {"Rocky/Airless", "Terrestrial", "Gas Giant", "Ice Giant", "Unknown"};

            uint8_t type = navState->currentLocation.planet->type;
            if (type >= 4) {
                type = 4; // Default to "Unknown" for invalid types
            }

            snprintf(buffer, bufferSize, "%s (%s Planet)", navState->currentLocation.planet->name, planetTypes[type]);
        } else {
            snprintf(buffer, bufferSize, "Unknown Planet");
        }
        break;

    case CELESTIAL_STATION:
        if (navState->currentLocation.station) {
            // This requires a pointer to the current star system
            // If we don't have that in this context, we'll use a generic format
            snprintf(buffer, bufferSize, "%s (Orbital station_t)", navState->currentLocation.station->name);
        } else {
            snprintf(buffer, bufferSize, "Unknown station_t");
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
 * @param plan_sys_t Pointer to the planet system data
 */
[[maybe_unused]] static inline void use_station_market(station_t *station, planet_t *planet,
                                                       struct plan_sys_t *plan_sys_t) {
    if (!station || !plan_sys_t) // planet_tcan be nullptr for deep space stations
    {
        // fprintf(stderr, "Warning: use_station_market called with nullptr station or
        // plan_sys_t.\\n"); Optionally clear LocalMarket or set to a default empty
        // state
        memset(&g_state.LocalMarket, 0, sizeof(market_type_t));
        return;
    }

    // 1. Call update_station_market to ensure the market is up-to-date.
    // game_time_get_seconds() should be available from an included header like
    // elite_state.h
    update_station_market(station, game_time_get_seconds(), planet, plan_sys_t);

    // 2. Copy the station's market data to the global LocalMarket.
    // Assuming LocalMarket is a global variable of type MarketType.
    g_state.LocalMarket = station->market;
}

// Forward declarations exist at the beginning of the file

/**
 * Sets the current market to a planet's market when landing
 *
 * @param planet Pointer to the planet being landed on
 * @param plan_sys_t Pointer to the planet system data
 */
[[maybe_unused]] static inline void use_planetary_market(planet_t *planet, struct plan_sys_t *plan_sys_t) {
    if (!planet || !plan_sys_t) {
        memset(&g_state.LocalMarket, 0, sizeof(market_type_t));
        return;
    }

    // Ensure the planetary market is initialized if it hasn't been already
    if (!planet->planetaryMarket.isInitialized) {
        planet->planetaryMarket.market = generate_planetary_market(planet, plan_sys_t);
    } else {
        // Ensure the market is up to date
        update_planetary_market(planet, game_time_get_seconds(), plan_sys_t);
    }

    // Set the global LocalMarket to this planet's market
    g_state.LocalMarket = planet->planetaryMarket.market;
}

// =============================================================================
// HABITABILITY ANALYSIS FUNCTIONS
// =============================================================================

/**
 * Calculate radiation exposure at planet's orbit relative to Earth
 */
static inline double calculate_radiation_exposure(planet_t *planet, star_t *star) {
    // Calculate radiation exposure relative to Earth
    double distance = planet->orbitalDistance;
    double stellar_luminosity = star->luminosity;

    // Flux at planet's orbit relative to Earth's solar flux
    double flux = stellar_luminosity / (distance * distance);

    // Additional radiation from high-energy stellar types
    double stellar_radiation_factor = 1.0;
    if (star->spectralClass <= 2) {                      // O, B, A stars
        stellar_radiation_factor = pow(star->mass, 2.0); // Much higher UV and X-ray emission
    } else if (star->spectralClass == 3) {               // F stars
        stellar_radiation_factor = 1.5;
    }

    return flux * stellar_radiation_factor;
}

/**
 * Check if a planet is likely to be tidally locked to its star
 */
static inline bool check_tidal_locking(planet_t *planet, star_t *star) {
    // Improved tidal locking check based on stellar type and planetary distance
    // Tidal locking is more common around:
    // 1. Close-orbiting planets around any star
    // 2. Planets around M-dwarf stars (which have closer habitable zones)
    // 3. Smaller planets (lower moment of inertia)

    // Base locking distance varies by stellar class
    double base_locking_distance = 0.0;

    if (star->spectralClass == 6) {
        // M-dwarfs: habitable zone is very close, so larger locking zone
        base_locking_distance = 0.5 * sqrt(star->luminosity); // More realistic for M-dwarfs
    } else if (star->spectralClass >= 4) {
        // K and G stars: moderate locking zones
        base_locking_distance = 0.2 * sqrt(star->luminosity);
    } else {
        // Hotter stars (O, B, A, F): smaller locking zones relative to habitability
        base_locking_distance = 0.15 * sqrt(star->luminosity);
    }

    // Adjust for planetary size (smaller planets lock more easily)
    double size_multiplier = 1.0;
    if (planet->radius < 4000) {
        size_multiplier = 1.3; // Smaller planets more likely to be locked
    } else if (planet->radius > 8000) {
        size_multiplier = 0.8; // Larger planets resist locking
    }

    double effective_locking_distance = base_locking_distance * size_multiplier;

    return planet->orbitalDistance < effective_locking_distance;
}

/**
 * Check if a planet has potential for retaining an atmosphere
 */
[[maybe_unused]] static inline bool check_planetary_atmosphere_potential(planet_t *planet, star_t *star) {
    // Planets need sufficient mass/size to retain atmosphere
    if (planet->radius < 2000) {
        return false; // Too small to retain significant atmosphere
    }

    // Check if planet is not too close to star (atmosphere stripped)
    double escape_distance = 0.1 * sqrt(star->luminosity); // Simplified calculation
    if (planet->orbitalDistance < escape_distance) {
        return false; // Too close, atmosphere likely stripped
    }

    // Gas giants and ice giants have atmospheres by definition
    if (planet->type >= 2) {
        return true;
    }

    // Terrestrial and rocky planets depend on size and distance
    return planet->radius >= 3000; // Earth-like or larger
}

/**
 * Get human-readable temperature category for a planet
 */
[[maybe_unused]] static inline const char *get_temperature_category(double temperature) {
    if (temperature < 200) {
        return "Very Cold";
    }
    if (temperature < 250) {
        return "Cold";
    }
    if (temperature < 290) {
        return "Cool";
    }
    if (temperature < 323) {
        return "Warm";
    }
    if (temperature < 373) {
        return "Hot";
    }
    return "Very Hot";
}

/**
 * Get human-readable habitability rating based on score
 */
[[maybe_unused]] static inline const char *get_habitability_rating(double score) {
    if (score >= 80) {
        return "Excellent";
    }
    if (score >= 60) {
        return "Good";
    }
    if (score >= 40) {
        return "Marginal";
    }
    if (score >= 20) {
        return "Poor";
    }
    return "Hostile";
}

/**
 * Calculate comprehensive habitability score for a planet
 * Returns a score from 0-100 based on multiple factors including temperature,
 * orbital position, planet type, stellar characteristics, and hazards
 */
[[maybe_unused]] static inline double calculate_habitability_score(planet_t *planet, star_t *star) {
    double score = 0.0;

    // Generate pseudo-random variations based on planet properties for
    // consistency Use planet radius and orbital distance as seeds for
    // reproducible "randomness"
    unsigned int seed = (unsigned int)((planet->radius * 1000) + (planet->orbitalDistance * 10000));
    double variation_factor = (double)(seed % 1000) / 1000.0; // 0.0 to 1.0

    // Temperature scoring (0-25 points, more conservative)
    double temp_celsius = planet->surfaceTemperature - 273.15;
    if (temp_celsius >= -10 && temp_celsius <= 40) {
        score += 25.0; // Optimal temperature range (narrower for realism)
    } else if (temp_celsius >= -40 && temp_celsius <= 70) {
        score += 20.0; // Good temperature range
    } else if (temp_celsius >= -80 && temp_celsius <= 100) {
        score += 15.0; // Acceptable temperature range
    } else if (temp_celsius >= -150 && temp_celsius <= 150) {
        score += 8.0; // Marginal temperature range
    }

    // Orbital distance scoring (0-20 points)
    if (planet->isInHabitableZone) {
        double hz_center = (star->habitableZoneInner + star->habitableZoneOuter) / 2.0;
        double distance_from_center = fabs(planet->orbitalDistance - hz_center);
        double hz_width = star->habitableZoneOuter - star->habitableZoneInner;

        if (distance_from_center < hz_width * 0.2) {
            score += 20.0; // In center of habitable zone (narrower sweet spot)
        } else if (distance_from_center < hz_width * 0.4) {
            score += 15.0; // In habitable zone
        } else {
            score += 8.0; // Edge of habitable zone
        }
    } else {
        // Penalty for being outside habitable zone
        score -= 5.0;
    }

    // planet_ttype scoring (0-25 points, more selective) with individual
    // variations
    switch (planet->type) {
    case 1: // Terrestrial
        if (planet->radius >= 5500 && planet->radius <= 7500) {
            score += 25.0; // Very Earth-like size (narrower range)
        } else if (planet->radius >= 4500 && planet->radius <= 8500) {
            score += 20.0; // Earth-like size
        } else if (planet->radius >= 3500 && planet->radius <= 10000) {
            score += 15.0; // Reasonable terrestrial size
        } else {
            score += 8.0; // Marginal terrestrial
        }
        // Add variation based on planetary density (affects magnetic field,
        // tectonics)
        double densityVariation = (variation_factor - 0.5) * 8.0; // ±4 points
        score += densityVariation;
        break;
    case 0: // Rocky/Airless
        if (planet->radius >= 4000) {
            score += 12.0; // Large enough for atmosphere retention
        } else if (planet->radius >= 2500) {
            score += 8.0; // Marginal atmosphere retention
        } else {
            score += 3.0; // Too small for significant atmosphere
        }
        // Variation for surface composition and potential volatiles
        double rockVariation = (variation_factor - 0.5) * 6.0; // ±3 points
        score += rockVariation;
        break;
    case 2: // Gas Giant
        // Gas giants can vary significantly in habitability potential based on:
        // - Moon systems potential
        // - Atmospheric composition
        // - Magnetic field strength
        // - Distance from star (affects moon habitability)
        score += 8.0; // Base score for potential habitable moons

        // Size factor - larger gas giants more likely to have interesting moons
        if (planet->radius > 50000) {
            score += 6.0; // Jupiter-size or larger
        } else if (planet->radius > 35000) {
            score += 4.0; // Saturn-size
        } else {
            score += 2.0; // Smaller gas giant
        }

        // Distance factor - gas giants in outer system better for moon habitability
        if (planet->orbitalDistance > star->habitableZoneOuter) {
            score += 5.0; // Good position for moon systems
        } else if (planet->orbitalDistance > star->habitableZoneInner) {
            score += 3.0; // Marginal position
        }

        // Individual variation for magnetic field strength, composition
        double gasVariation = (variation_factor - 0.5) * 12.0; // ±6 points
        score += gasVariation;
        break;
    case 3: // Ice Giant
        // Ice giants have different potential than gas giants
        score += 5.0; // Base score - less potential than gas giants

        // Size and composition factors
        if (planet->radius > 25000) {
            score += 4.0; // Larger ice giants better for moons
        } else {
            score += 2.0; // Smaller ice giants
        }

        // Ice giants closer to habitable zone might have liquid subsurface oceans
        if (planet->orbitalDistance < star->habitableZoneOuter * 2.0) {
            score += 4.0; // Potential for subsurface ocean heating
        }

        // Individual variation for ice/rock ratio, internal heating
        double ice_variation = (variation_factor - 0.5) * 10.0; // ±5 points
        score += ice_variation;
        break;
    } // Stellar factors (0-15 points, more conservative)
    if (star->spectralClass == 4) {                                    // G stars (like our Sun)
        score += 15.0;                                                 // Best stellar type for habitability
    } else if (star->spectralClass == 3 || star->spectralClass == 5) { // F, K stars
        score += 12.0;                                                 // Good stellar types for habitability
    } else if (star->spectralClass == 6) {                             // M stars
        score += 8.0;                                                  // M stars can support life but with challenges
    } else {
        score += 0.0; // Other stellar types less suitable
    }

    // Add orbital characteristics variation
    // Simulate orbital eccentricity effects (planets closer to star have more
    // circular orbits)
    double eccentricity_penalty = 0.0;
    if (planet->orbitalDistance < 1.0) {
        // Close planets likely more circular
        eccentricity_penalty = variation_factor * 3.0; // 0-3 point penalty
    } else if (planet->orbitalDistance < 3.0) {
        // Mid-distance planets moderate eccentricity
        eccentricity_penalty = variation_factor * 6.0; // 0-6 point penalty
    } else {
        // Outer planets can be quite eccentric
        eccentricity_penalty = variation_factor * 10.0; // 0-10 point penalty
    }
    score -= eccentricity_penalty;

    // Add planetary formation history variation
    // Some planets may have formed with better/worse initial conditions
    double formation_bonus = (sin(variation_factor * 6.28) + 1.0) * 4.0; // ±4 points sinusoidal
    score += formation_bonus;

    // Stellar age factor (0-10 points, more restrictive)
    if (star->age >= 2.0 && star->age <= 8.0) {
        score += 10.0; // Optimal age for life development
    } else if (star->age >= 1.0 && star->age <= 12.0) {
        score += 7.0; // Acceptable age
    } else if (star->age >= 0.5 && star->age <= 15.0) {
        score += 4.0; // Marginal age
    } else {
        score += 1.0; // Too young or too old
    } // Radiation exposure penalty (enhanced with individual variation)
    double radiation_exposure = calculate_radiation_exposure(planet, star);

    // Add planet-specific magnetic field variation
    // Larger planets and gas giants typically have stronger magnetic fields
    double magnetic_field_strength = 1.0;
    if (planet->type >= 2) {                                    // Gas/Ice giants
        magnetic_field_strength = 1.5 + (variation_factor * 1.0); // 1.5-2.5x Earth's field
    } else if (planet->radius > 6000) {
        magnetic_field_strength = 0.8 + (variation_factor * 0.8); // 0.8-1.6x Earth's field
    } else {
        magnetic_field_strength = 0.2 + (variation_factor * 0.6); // 0.2-0.8x Earth's field
    }

    // Adjust radiation exposure based on magnetic field
    double effective_radiation = radiation_exposure / magnetic_field_strength;

    if (effective_radiation > 20.0) {
        score -= 30.0; // Extreme radiation penalty
    } else if (effective_radiation > 10.0) {
        score -= 20.0; // High radiation penalty
    } else if (effective_radiation > 5.0) {
        score -= 12.0; // Moderate radiation penalty
    } else if (effective_radiation > 2.0) {
        score -= 6.0; // Minor radiation penalty
    }

    // Tidal locking penalty for close planets (enhanced)
    if (check_tidal_locking(planet, star)) {
        score -= 20.0; // Tidal locking significantly reduces habitability
    }

    // Additional realism penalties

    // Penalty for systems with extreme stellar masses
    if (star->mass > 1.5) {
        score -= 8.0; // Massive stars have shorter lifespans and more radiation
    } else if (star->mass < 0.3) {
        score -= 5.0; // Very low mass stars have issues with flares
    }

    // Penalty for very young or very old systems
    if (star->age < 0.8) {
        score -= 10.0; // Too young for complex life
    } else if (star->age > 12.0) {
        score -= 8.0; // Star may be evolving off main sequence
    }

    // Ensure score is between 0 and 100
    if (score < 0) {
        score = 0;
    }
    if (score > 100) {
        score = 100;
    }

    return score;
}