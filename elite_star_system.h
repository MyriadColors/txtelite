#pragma once

#include "elite_market.h"           // For market-related functions
#include "elite_navigation.h"       // For NavigationState and CelestialType
#include "elite_ship_maintenance.h" // For ConsumeFuel function
#include "elite_ship_types.h"       // For PlayerShip structure
#include "elite_state.h"            // For PlanSys and other related structures

// Forward declarations
struct Star;
struct Planet;
struct Station;
struct StarSystem;

// Maximum number of planets per star system and stations per planet
#define MAX_PLANETS_PER_SYSTEM 8
#define MAX_STATIONS_PER_PLANET 5

// Realistic astronomical constants
#define SOLAR_MASS_KG 1.989e30
#define SOLAR_LUMINOSITY_WATTS 3.828e26
#define AU_TO_KM 149597870.7
#define EARTH_RADIUS_KM 6371.0

// Stellar classification data
typedef struct StellarData {
  double minMass;     // Minimum mass for this class (solar masses)
  double maxMass;     // Maximum mass for this class (solar masses)
  double temperature; // Surface temperature (Kelvin)
  double lifetimeGyr; // Main sequence lifetime (billion years)
  double frequency;   // Relative frequency in galaxy (0.0-1.0)
} StellarData;

// Structure for a star in a star system
typedef struct Star {
  char name[MAX_LEN];        // Star name
  uint8_t spectralClass;     // O, B, A, F, G, K, M classification (0-6)
  double mass;               // Mass relative to Sol
  double luminosity;         // Luminosity relative to Sol
  double temperature;        // Surface temperature in Kelvin
  double age;                // Age in billion years
  double habitableZoneInner; // Inner edge of habitable zone (AU)
  double habitableZoneOuter; // Outer edge of habitable zone (AU)
} Star;

// Structure for a planet in a star system
typedef struct Planet {
  char name[MAX_LEN];        // Planet name
  double orbitalDistance;    // Distance from parent star in AU
  double radius;             // Radius in km
  double surfaceTemperature; // Average surface temperature in Kelvin
  uint8_t type;              // Planet type (gas giant, terrestrial, etc.)
  bool isInHabitableZone;    // Whether planet is in habitable zone
  uint8_t numStations;       // Number of stations orbiting this planet
  struct Station
      *stations[MAX_STATIONS_PER_PLANET]; // Pointers to station structures

  // Planetary market data for when landed on the planet
  struct {
    MarketType market;  // Market data for the planet's surface
    bool isInitialized; // Whether the market has been initialized
  } planetaryMarket;
  uint8_t marketFluctuation; // Planet-specific market fluctuation factor
  uint64_t lastMarketUpdate; // Last game time when market was updated
} Planet;

// Structure for a space station
typedef struct Station {
  char name[MAX_LEN];      // Station name
  double orbitalDistance;  // Distance from parent planet in AU
  uint8_t type;            // Station type/class
  uint8_t size;            // Station size (0=Small, 1=Medium, 2=Large)
  uint8_t services;        // Bitmask of available services
  bool hasDockingComputer; // Whether automated docking is available
  bool hasShipyard;        // Whether ship equipment can be purchased
  bool hasMarket;          // Whether market trading is available
  bool hasMissions;        // Whether missions are available
  MarketType market;       // Station-specific market data
  uint8_t
      marketFluctuation;  // Station-specific market fluctuation factor (0-15)
  uint8_t specialization; // Economic specialization (0: Balanced, 1:
                          // Industrial, 2: Agricultural, 3: Mining)
  uint64_t lastMarketUpdate; // Last game time when market was updated
} Station;

// Structure for a complete star system
typedef struct StarSystem {
  struct PlanSys
      *planSys;       // Pointer to existing system info (economy, gov, etc.)
  Star centralStar;   // The central star of the system
  uint8_t numPlanets; // Number of planets in the system
  Planet planets[MAX_PLANETS_PER_SYSTEM]; // Array of planets
  double navBeaconDistance; // Distance of nav beacon from central star in AU
} StarSystem;

// Forward function declarations
static inline MarketType GenerateStationMarket(Station *station, Planet *planet,
                                               struct PlanSys *planSys);
static inline void UpdateStationMarket(Station *station, uint64_t currentTime,
                                       Planet *planet, struct PlanSys *planSys);
static inline void UseStationMarket(Station *station, Planet *planet,
                                    struct PlanSys *planSys);
static inline MarketType GeneratePlanetaryMarket(Planet *planet,
                                                 struct PlanSys *planSys);
static inline void UpdatePlanetaryMarket(Planet *planet, uint64_t currentTime,
                                         struct PlanSys *planSys);
static inline void UsePlanetaryMarket(Planet *planet, struct PlanSys *planSys);

// Realistic stellar classification data for main sequence stars
static const StellarData stellarClasses[7] = {
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
  } else if (mass < 2.0) {
    // Main sequence scaling
    return pow(mass, 4.0);
  } else if (mass < 20.0) {
    // Massive stars have different scaling
    return 1.4 * pow(mass, 3.5);
  } else {
    // Very massive stars
    return pow(mass, 3.0);
  }
}

// Helper function to calculate habitable zone boundaries
static inline void calculate_habitable_zone(double luminosity,
                                            double *innerEdge,
                                            double *outerEdge) {
  // Habitable zone based on liquid water temperatures (273-373K)
  // Using simplified calculations based on solar flux
  double sqrtLum = sqrt(luminosity);
  // Optimistic habitable zone (liquid water with greenhouse effects)
  *innerEdge = 0.85 * sqrtLum; // Inner edge (runaway greenhouse)
  *outerEdge = 1.7 * sqrtLum;  // Outer edge (maximum greenhouse effect)

  // Ensure minimum distances
  if (*innerEdge < 0.1)
    *innerEdge = 0.1;
  if (*outerEdge < *innerEdge + 0.2)
    *outerEdge = *innerEdge + 0.2;
}

// Helper function to calculate planet surface temperature
static inline double calculate_planet_temperature(
    double stellarLuminosity, double orbitalDistance,
    double
        albedo) { // Stefan-Boltzmann law for planetary equilibrium temperature
  // T = (L * (1-A) / (16 * pi * sigma * d^2))^0.25 * T_sun
  // Simplified: T ~= 278.5 * (L/d^2)^0.25 * (1-A)^0.25

  double flux = stellarLuminosity / (orbitalDistance * orbitalDistance);
  double temperature = 278.5 * pow(flux, 0.25) * pow(1.0 - albedo, 0.25);

  return temperature;
}

// Function to generate the market for a station
static inline MarketType GenerateStationMarket(Station *station, Planet *planet,
                                               struct PlanSys *planSys) {
  if (!station ||
      !planSys) // Planet can be NULL if station is not orbiting one (e.g. deep
                // space station, though plan implies planet context)
  {
    // Handle error or return a default/empty market
    MarketType emptyMarket = {0}; // Initialize all members to zero
    // fprintf(stderr, "Warning: GenerateStationMarket called with NULL station
    // or planSys.\\n");
    return emptyMarket;
  } // Avoid unused parameter warning
  (void)planet;

  // 1. Generate a baseMarket using the existing generate_market function from
  // elite_market.h
  MarketType baseMarket = generate_market(station->marketFluctuation, *planSys);

  // 2. Apply modifiers to baseMarket.price[i] and baseMarket.quantity[i] for
  // each commodity based on station->specialization Ensure
  // station->specialization is a valid enum value
  StationSpecialization specialization =
      (StationSpecialization)station->specialization;
  if (specialization >= NUM_STATION_SPECIALIZATIONS || specialization < 0) {
    // Optionally log this case:
    // fprintf(stderr, "Warning: Station '%s' has invalid specialization value
    // %d. Defaulting to Balanced.\\n", station->name, station->specialization);
    specialization =
        STATION_SPECIALIZATION_BALANCED; // Default to balanced if out of bounds
  }

  for (int i = 0; i < NUM_STANDARD_COMMODITIES; i++) {
    // Get the modifier for the current commodity and station specialization
    MarketModifier modifier = stationSpecializationModifiers[specialization][i];

    // Apply price modifier
    float newPrice = (float)baseMarket.price[i] * modifier.priceFactor;
    if (newPrice < 0)
      newPrice = 0; // Price should not be negative
    // Potentially clamp to a max price if one is defined: if (newPrice >
    // MAX_COMMODITY_PRICE) newPrice = MAX_COMMODITY_PRICE;
    baseMarket.price[i] =
        (uint16_t)roundf(newPrice); // Round to nearest integer for price

    // Apply quantity modifier
    float newQuantity = (float)baseMarket.quantity[i] * modifier.quantityFactor;
    if (newQuantity < 0)
      newQuantity = 0; // Quantity should not be negative
    // Clamp quantity to avoid overflow (uint16_t max is 65535).
    if (newQuantity > 0xFFFF)
      newQuantity = 0xFFFF;
    baseMarket.quantity[i] =
        (uint16_t)roundf(newQuantity); // Round to nearest integer for quantity
  }

  // 3. Update station->lastMarketUpdate with the currentGameTimeSeconds.
  // As per the plan, this is handled by the caller (e.g.,
  // initialize_star_system or UpdateStationMarket). station->lastMarketUpdate =
  // currentGameTimeSeconds; // This line would be here if handled internally

  return baseMarket;
}

// Function to generate the market for a planet's surface
static inline MarketType GeneratePlanetaryMarket(Planet *planet,
                                                 struct PlanSys *planSys) {
  if (!planet || !planSys) {
    MarketType emptyMarket = {0}; // Initialize all members to zero
    // fprintf(stderr, "Warning: GeneratePlanetaryMarket called with NULL planet
    // or planSys.\\n");
    return emptyMarket;
  }

  // 1. Generate a baseMarket using generate_market from elite_market.h
  MarketType baseMarket = generate_market(planet->marketFluctuation, *planSys);

  // 2. Apply modifiers based on planet->type
  PlanetMarketType planetType =
      (PlanetMarketType)planet->type; // planet->type is uint8_t

  // Check if planetType is within the valid range for the planetTypeModifiers
  // array
  if (planetType >= NUM_PLANET_MARKET_TYPES ||
      planetType < 0) // planet->type is uint8_t, so < 0 is only for robustness
                      // if type changes
  {
    // fprintf(stderr, "Warning: Planet '%s' (type %u) has invalid type for
    // market modifiers. Using base market without type-specific changes.\\n",
    // planet->name, planet->type); If type is out of bounds, no type-specific
    // modifiers are applied. The market remains the baseMarket.
  } else {
    for (int i = 0; i < NUM_STANDARD_COMMODITIES; i++) {
      MarketModifier modifier = planetTypeModifiers[planetType][i];

      // Apply price modifier
      float newPrice = (float)baseMarket.price[i] * modifier.priceFactor;
      if (newPrice < 0)
        newPrice = 0; // Price should not be negative
      // Consider clamping to a MAX_PRICE if defined
      baseMarket.price[i] =
          (uint16_t)roundf(newPrice); // Round to nearest integer

      // Apply quantity modifier
      float newQuantity =
          (float)baseMarket.quantity[i] * modifier.quantityFactor;
      if (newQuantity < 0)
        newQuantity = 0; // Quantity should not be negative
      if (newQuantity > 0xFFFF)
        newQuantity = 0xFFFF; // Clamp to uint16_t max (65535)
      baseMarket.quantity[i] =
          (uint16_t)roundf(newQuantity); // Round to nearest integer
    }
  }

  // 3. Update planet->lastMarketUpdate with currentGameTimeSeconds
  // Assuming game_time_get_seconds() is available globally or via included
  // headers (e.g., elite_state.h)
  planet->lastMarketUpdate = game_time_get_seconds();

  // 4. Set planet->planetaryMarket.isInitialized = true
  // This is done here as per step III.4 of the plan.
  // The caller (e.g., initialize_star_system) will assign the returned market
  // to planet->planetaryMarket.market.
  planet->planetaryMarket.isInitialized = true;

  // 5. Return the modified market
  return baseMarket;
}

// Function to update the market for a station if enough time has passed
static inline void UpdateStationMarket(
    Station *station, uint64_t currentTime, Planet *planet,
    struct PlanSys *planSys) // Added Planet* and PlanSys* params
{
  if (!station || !planSys) // Planet can be NULL for deep space stations, but
                            // planSys is essential
  {
    // fprintf(stderr, "Warning: UpdateStationMarket called with NULL station or
    // planSys.\\n");
    return;
  }

  const uint64_t STATION_UPDATE_INTERVAL =
      3600; // 1 hour in game seconds, as per existing findings

  // 1. Check if enough game time (UPDATE_INTERVAL) has passed since
  // lastMarketUpdate.
  if (currentTime >= station->lastMarketUpdate &&
      (currentTime - station->lastMarketUpdate >= STATION_UPDATE_INTERVAL)) {
    // 2. If so, calculate updateCycles.
    uint64_t elapsedSeconds = currentTime - station->lastMarketUpdate;
    uint16_t updateCycles =
        (uint16_t)(elapsedSeconds / STATION_UPDATE_INTERVAL);

    if (updateCycles > 0) {
      // 3. Modify marketFluctuation based on updateCycles.
      // Simple cyclic increment for fluctuation. Max fluctuation is 15 (0-15
      // range).
      station->marketFluctuation =
          (station->marketFluctuation + updateCycles) % 16;

      // 4. Call GenerateStationMarket to regenerate the market.
      // The GenerateStationMarket function itself does not update
      // lastMarketUpdate.
      station->market = GenerateStationMarket(station, planet, planSys);

      // 5. lastMarketUpdate is then set to the currentTime.
      // To prevent drift, set it to the time of the last completed interval, or
      // current time. Setting to currentTime is simpler as per plan.
      station->lastMarketUpdate = currentTime;
    }
  }
}

// Function to update the market for a planet if enough time has passed
static inline void
UpdatePlanetaryMarket(Planet *planet, uint64_t currentTime,
                      struct PlanSys *planSys) // Added PlanSys* param
{
  if (!planet || !planSys) {
    // fprintf(stderr, "Warning: UpdatePlanetaryMarket called with NULL planet
    // or planSys.\\n");
    return;
  }

  const uint64_t PLANET_UPDATE_INTERVAL =
      7200; // 2 hours in game seconds, as per existing findings

  // 1. Check if enough game time (UPDATE_INTERVAL) has passed since
  // lastMarketUpdate.
  if (currentTime >= planet->lastMarketUpdate &&
      (currentTime - planet->lastMarketUpdate >= PLANET_UPDATE_INTERVAL)) {
    // 2. If so, calculate updateCycles.
    uint64_t elapsedSeconds = currentTime - planet->lastMarketUpdate;
    uint16_t updateCycles = (uint16_t)(elapsedSeconds / PLANET_UPDATE_INTERVAL);

    if (updateCycles > 0) {
      // 3. Modify marketFluctuation based on updateCycles.
      planet->marketFluctuation =
          (planet->marketFluctuation + updateCycles) % 16;

      // 4. Call GeneratePlanetaryMarket to regenerate the market.
      // GeneratePlanetaryMarket updates its own lastMarketUpdate and
      // isInitialized fields.
      MarketType newMarket = GeneratePlanetaryMarket(planet, planSys);
      // The plan for GeneratePlanetaryMarket (Step III.4) says it updates
      // planet->lastMarketUpdate and sets isInitialized. However, the plan for
      // UpdatePlanetaryMarket (Step IV.5) also says lastMarketUpdate is set to
      // currentTime. To adhere to Step IV.5, we will explicitly set it here.
      // GeneratePlanetaryMarket already sets it, this will overwrite with the
      // exact currentTime.

      // The market data itself needs to be stored in
      // planet->planetaryMarket.market The plan for V. says: call
      // GeneratePlanetaryMarket(planet, planSysEntry) to populate
      // planet->planetaryMarket.market So, the GeneratePlanetaryMarket should
      // ideally return the market to be assigned. And indeed it does. We need
      // to assign it to the correct place in the Planet struct. The Planet
      // struct has: struct { MarketType market; bool isInitialized; }
      // planetaryMarket;
      planet->planetaryMarket.market = newMarket;

      // 5. lastMarketUpdate is then set to the currentTime.
      planet->lastMarketUpdate = currentTime;
      // planet->planetaryMarket.isInitialized is already set by
      // GeneratePlanetaryMarket
    }
  }
}

// Function to initialize a star system from a PlanSys entry
static inline void initialize_star_system(StarSystem *system,
                                          struct PlanSys *planSysEntry) {
  if (!system || !planSysEntry) {
    fprintf(stderr,
            "Error: Invalid parameters for star system initialization.\n");
    return;
  }

  // Link the existing PlanSys information
  system->planSys = planSysEntry; // ------------------------------------
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
  } // Realistic spectral class distribution (M-class stars are most common)
  // Use cumulative probability distribution based on seed
  // Combine multiple seed components to get better distribution
  uint32_t seedCombined = ((uint32_t)planSysEntry->goatSoupSeed.a << 16) |
                          planSysEntry->goatSoupSeed.b;
  double classRoll = ((double)(seedCombined % 1000000)) / 1000000.0;
  double cumulative = 0.0;
  system->centralStar.spectralClass = 6; // Default to M-class (most common)

  for (int i = 0; i < 7; i++) {
    cumulative += stellarClasses[i].frequency;
    if (classRoll <= cumulative) {
      system->centralStar.spectralClass = i;
      break;
    }
  }

  // Generate realistic mass within spectral class range
  const StellarData *starData =
      &stellarClasses[system->centralStar.spectralClass];
  double massRange = starData->maxMass - starData->minMass;
  double massRoll = ((double)(planSysEntry->goatSoupSeed.b % 1000)) / 1000.0;
  system->centralStar.mass = starData->minMass + (massRange * massRoll);

  // Calculate realistic luminosity from mass
  system->centralStar.luminosity =
      calculate_luminosity_from_mass(system->centralStar.mass);

  // Set temperature based on spectral class with some variation
  double tempVariation =
      ((double)(planSysEntry->goatSoupSeed.c % 1000)) / 1000.0 - 0.5;
  system->centralStar.temperature =
      starData->temperature *
      (1.0 + tempVariation *
                 0.1); // Generate stellar age (more realistic distribution)
  // Use a weighted distribution that favors older stars for more realistic
  // galactic population
  double universeAge = 13.8; // Age of universe in billion years
  double maxReasonableAge = (starData->lifetimeGyr < universeAge)
                                ? starData->lifetimeGyr * 0.9
                                : universeAge;

  // Use a power-law distribution to favor older stars (typical galactic age
  // ~6-8 billion years)
  double ageRoll = ((double)(planSysEntry->goatSoupSeed.d % 1000)) / 1000.0;

  // Apply power function to bias toward older ages (power of 0.5 makes
  // distribution more realistic) This gives average age around 5-7 billion
  // years instead of 1-2 billion
  double biasedAgeRoll = pow(ageRoll, 0.5);

  // Add minimum age for stellar evolution (at least 0.1 billion years)
  double minAge = 0.1;
  system->centralStar.age =
      minAge + (maxReasonableAge - minAge) * biasedAgeRoll;

  // Calculate habitable zone
  calculate_habitable_zone(
      system->centralStar.luminosity, &system->centralStar.habitableZoneInner,
      &system->centralStar
           .habitableZoneOuter); // ------------------------------------
  // Determine number of planets
  // ------------------------------------

  // Number of planets based on stellar mass and age (more massive/older stars
  // tend to have more planets) Also factor in tech level as a proxy for
  // exploration thoroughness
  int basePlanets = 2; // Minimum planets

  // Stellar mass factor (more massive stars can capture more material)
  if (system->centralStar.mass > 1.5)
    basePlanets += 2;
  else if (system->centralStar.mass > 1.0)
    basePlanets += 1;
  else if (system->centralStar.mass < 0.5)
    basePlanets -= 1;

  // Age factor (older systems have had more time for planet formation)
  if (system->centralStar.age > 5.0)
    basePlanets += 1;

  // Tech level factor (better detection of distant planets)
  basePlanets += (planSysEntry->techLev / 3);

  // Random variation
  int planetVariation = (planSysEntry->goatSoupSeed.c % 3) - 1; // -1, 0, or 1
  system->numPlanets = basePlanets + planetVariation;

  // Clamp to valid range
  if (system->numPlanets < 1)
    system->numPlanets = 1;
  if (system->numPlanets > MAX_PLANETS_PER_SYSTEM)
    system->numPlanets =
        MAX_PLANETS_PER_SYSTEM; // ------------------------------------
  // Set Nav Beacon position
  // ------------------------------------

  // Nav beacon distance varies based on system size and habitable zone
  // Place it beyond the outermost likely planet orbit
  double systemRadius = system->centralStar.habitableZoneOuter *
                        3.0; // 3x habitable zone outer edge
  system->navBeaconDistance =
      systemRadius + ((double)(planSysEntry->techLev) * 0.5);

  // ------------------------------------
  // Initialize planets
  // ------------------------------------
  for (int i = 0; i < system->numPlanets; i++) {
    Planet *planet = &system->planets[i];

    // Set planet name with more variety based on position and system
    // characteristics
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
        const char *suffixes[] = {"Alpha",   "Beta", "Gamma", "Delta",
                                  "Epsilon", "Zeta", "Eta",   "Theta"};
        snprintf(planet->name, MAX_LEN, "%s %s", planSysEntry->name,
                 suffixes[i % 8]);
      } else {
        // Generate a slightly different name using seed
        char altName[MAX_LEN];
        uint32_t nameSeed =
            planSysEntry->goatSoupSeed.a + i * planSysEntry->goatSoupSeed.b;
        uint8_t nameLen = (nameSeed % 4) + 3; // 3-6 letter name

        for (int j = 0; j < nameLen; j++) {
          nameSeed = (nameSeed * 2654435761U) %
                     4294967296U; // Knuth's multiplicative hash
          char letter = 'a' + (nameSeed % 26);
          if (j == 0)
            letter = toupper(letter);
          altName[j] = letter;
        }
        altName[nameLen] = '\0';

        snprintf(planet->name, MAX_LEN, "%s", altName);
      }
    } // Set orbital distance with physics-based constraints and enhanced
      // stability
    // Use modified Titius-Bode law with Hill sphere and resonance
    // considerations
    double baseDistance = 0.0;

    if (i == 0) {
      // First planet: more conservative inner placement to avoid instability
      uint32_t innerSeed =
          planSysEntry->goatSoupSeed.a + planSysEntry->goatSoupSeed.b;
      if ((innerSeed % 10) < 2) { // Reduced from 3 to 2 (20% vs 30%)
        baseDistance = 0.25 + ((double)(innerSeed % 20) /
                               100.0); // 0.25-0.45 AU (slightly farther)
      } else {
        baseDistance = 0.5 + ((double)(innerSeed % 30) /
                              100.0); // 0.5-0.8 AU (more conservative)
      }
    } else {
      // Subsequent planets use enhanced spacing for stability
      double previousDistance = system->planets[i - 1].orbitalDistance;

      // Calculate minimum separation based on Hill sphere approximation
      double stellarMass = system->centralStar.mass;
      double hillSphereRadius = previousDistance *
                                pow(stellarMass / 3.0, 1.0 / 3.0) *
                                2.5; // Enhanced Hill sphere
      double minimumSeparation =
          hillSphereRadius * 3.0; // 3x Hill sphere for stability

      // Use spacing multiplier that respects physical constraints
      double spacingMultiplier =
          1.6 + ((double)((planSysEntry->goatSoupSeed.d + i) % 60) /
                 100.0); // 1.6-2.2 (more conservative)
      baseDistance = previousDistance * spacingMultiplier;

      // Ensure minimum separation is respected
      if (baseDistance - previousDistance < minimumSeparation) {
        baseDistance = previousDistance + minimumSeparation;
      }

      // Avoid unstable resonances (2:1, 3:1, 3:2)
      double ratio = baseDistance / previousDistance;
      if ((ratio > 1.9 && ratio < 2.1) || (ratio > 2.9 && ratio < 3.1) ||
          (ratio > 1.45 && ratio < 1.55)) {
        // Adjust to avoid resonance
        baseDistance *= 1.15; // Push out of resonance zone
      }
    }

    // Reduced randomization for better stability
    double variability =
        ((double)((planSysEntry->goatSoupSeed.d + i * 17) % 100) / 200.0) -
        0.25; // -0.25 to 0.25
    planet->orbitalDistance =
        baseDistance * (1.0 + (variability * 0.4)); // Reduced from 0.8 to 0.4
    // Further reduced habitable zone bias to minimize super-habitable planets
    uint32_t habitableBias = (planSysEntry->goatSoupSeed.c + i) % 100;
    if (habitableBias < 8 &&
        i < system->numPlanets - 1) { // Reduced from 15% to 8% chance
      double habitableZoneCenter = (system->centralStar.habitableZoneInner +
                                    system->centralStar.habitableZoneOuter) /
                                   2.0;
      double biasStrength =
          0.15; // Reduced from 0.25 to 0.15 (much weaker bias)
      planet->orbitalDistance = planet->orbitalDistance * (1.0 - biasStrength) +
                                habitableZoneCenter * biasStrength;
    }

    // Enhanced minimum distance check based on stellar type and planet size
    double minimumDistance = 0.1 + (system->centralStar.mass - 1.0) *
                                       0.05; // Scales with stellar mass
    if (planet->orbitalDistance < minimumDistance) {
      planet->orbitalDistance = minimumDistance;
    }

    // Check if planet is in habitable zone
    planet->isInHabitableZone =
        (planet->orbitalDistance >= system->centralStar.habitableZoneInner &&
         planet->orbitalDistance <= system->centralStar.habitableZoneOuter);

    // Calculate surface temperature (assuming Earth-like albedo of 0.3)
    planet->surfaceTemperature = calculate_planet_temperature(
        system->centralStar.luminosity, planet->orbitalDistance,
        0.3); // Determine planet type based on distance from star, temperature,
              // and probabilistic factors
    uint32_t typeSeed =
        planSysEntry->goatSoupSeed.a + i * planSysEntry->goatSoupSeed.b;
    uint8_t typeRoll = typeSeed % 100;

    if (planet->orbitalDistance <
        system->centralStar.habitableZoneInner * 0.4) {
      // Very close to star - always rocky/airless world
      planet->type =
          0; // Rocky/Airless        } else if (planet->isInHabitableZone) {
      // In habitable zone - more conservative terrestrial world generation
      if (typeRoll < 75) {        // Reduced from 85%
        planet->type = 1;         // Terrestrial (75% chance)
      } else if (typeRoll < 90) { // Increased rocky/airless chance
        planet->type = 0;         // Rocky/Airless (15% chance)
      } else {
        planet->type =
            2; // Gas Giant (10% chance - slightly higher for variety)
      }
    } else if (planet->orbitalDistance <
               system->centralStar.habitableZoneOuter * 2.0) {
      // Near habitable zone - mix of terrestrial and gas giants
      if (typeRoll < 50) {
        planet->type = 1; // Terrestrial (50% chance)
      } else if (typeRoll < 80) {
        planet->type = 2; // Gas Giant (30% chance)
      } else {
        planet->type = 0; // Rocky/Airless (20% chance)
      }
    } else if (planet->orbitalDistance <
               system->centralStar.habitableZoneOuter * 8.0) {
      // Outer system - favor gas giants but include some terrestrial
      if (typeRoll < 60) {
        planet->type = 2; // Gas Giant (60% chance)
      } else if (typeRoll < 85) {
        planet->type = 3; // Ice Giant (25% chance)
      } else {
        planet->type = 1; // Terrestrial (15% chance - cold super-Earths)
      }
    } else {
      // Very far from star - ice giants and some gas giants
      if (typeRoll < 70) {
        planet->type = 3; // Ice Giant (70% chance)
      } else {
        planet->type = 2; // Gas Giant (30% chance)
      }
    }

    // Adjust planet type based on stellar mass (massive stars can have gas
    // giants closer in)
    if (system->centralStar.mass > 2.0 && planet->orbitalDistance > 1.0) {
      // Massive stars can have gas giants closer in
      if (planet->type == 1 && (planSysEntry->goatSoupSeed.a + i) % 3 == 0) {
        planet->type = 2; // Convert some terrestrial to gas giant
      }
    }

    // Set realistic planet radius based on type and formation conditions
    double baseRadius = 0.0;
    uint32_t radiusSeed =
        planSysEntry->goatSoupSeed.b + i * 1009; // Use different seed offset

    switch (planet->type) {
    case 0: // Rocky/Airless (Mercury-like to Mars-like)
      baseRadius = 2400.0 + ((double)(radiusSeed % 3600)); // 2,400-6,000 km
      break;
    case 1: // Terrestrial (Mars-like to super-Earth, more conservative)
      baseRadius =
          3400.0 +
          ((double)(radiusSeed % 5600)); // 3,400-9,000 km (reduced max)
      // Planets in habitable zone are more Earth-like, less super-Earth
      if (planet->isInHabitableZone) {
        baseRadius =
            5800.0 +
            ((double)(radiusSeed %
                      2400)); // 5,800-8,200 km (narrower, Earth-like range)
      }
      break;
    case 2: // Gas Giant (Neptune to Jupiter and beyond)
      baseRadius = 24000.0 + ((double)(radiusSeed % 46000)); // 24,000-70,000 km
      break;
    case 3: // Ice Giant (Uranus/Neptune-like)
      baseRadius = 20000.0 + ((double)(radiusSeed % 30000)); // 20,000-50,000 km
      break;
    }

    planet->radius = baseRadius;

    // Initialize planetary market fluctuation factor
    planet->marketFluctuation =
        (planSysEntry->goatSoupSeed.b + i) % 16; // 0-15 fluctuation

    // Initialize the planetary market (this also sets lastMarketUpdate and
    // isInitialized)
    planet->planetaryMarket.market =
        GeneratePlanetaryMarket(planet, planSysEntry);
    // Ensure lastMarketUpdate is set by GeneratePlanetaryMarket, or set it here
    // if needed. Per plan, GeneratePlanetaryMarket handles its own
    // lastMarketUpdate.        // Determine number of stations for this planet
    // More developed systems (higher tech) have more stations
    // Habitable planets and terrestrial worlds are more likely to have stations
    uint8_t maxStations = 0;

    if (planet->isInHabitableZone) {
      // Habitable zone planets get the most stations
      maxStations = (planSysEntry->techLev >= 8) ? MAX_STATIONS_PER_PLANET : 4;
    } else if (planet->type <= 1) {
      // Rocky/Terrestrial planets
      maxStations = (planSysEntry->techLev >= 8) ? 3 : 2;
    } else if (planet->type == 2) {
      // Gas giants (good for fuel and mining)
      maxStations = (planSysEntry->techLev >= 10) ? 2 : 1;
    } else {
      // Ice giants (least attractive)
      maxStations = (planSysEntry->techLev >= 12) ? 1 : 0;
    }

    // Reduce stations for very hot or very cold planets
    if (planet->surfaceTemperature > 400.0 ||
        planet->surfaceTemperature < 200.0) {
      if (maxStations > 0)
        maxStations--; // Harsh environments get fewer stations
    }
    planet->numStations =
        (planSysEntry->goatSoupSeed.d + i) % (maxStations + 1);

    // Initialize stations for this planet
    for (int j = 0; j < planet->numStations; j++) {
      // Allocate memory for station
      Station *station = (Station *)malloc(sizeof(Station));
      if (!station) {
        fprintf(stderr, "Error: Memory allocation failed for station.\n");
        continue; // Memory allocation failed
      }

      // Set station name with more variety
      uint8_t stationNameVariant = ((planSysEntry->goatSoupSeed.a + i + j) % 4);
      if (stationNameVariant == 0) {
        snprintf(station->name, MAX_LEN, "%s Station %d", planet->name, j + 1);
      } else if (stationNameVariant == 1) {
        const char *prefixes[] = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon"};
        snprintf(station->name, MAX_LEN, "%s %s", prefixes[j % 5],
                 planet->name);
      } else if (stationNameVariant == 2) {
        const char *prefixes[] = {"Orbital", "Port", "Hub", "Gateway",
                                  "Outpost"};
        snprintf(station->name, MAX_LEN, "%s %s", prefixes[j % 5],
                 planet->name);
      } else {
        const char *uniqueNames[] = {"Nexus", "StarPort", "Horizon",
                                     "Tranquility", "Zenith"};
        snprintf(station->name, MAX_LEN, "%s %s", uniqueNames[j % 5],
                 planet->name);
      } // Set orbital distance from planet - realistic based on planet type and
        // safety
      double baseOrbitDistance = 0.0;
      if (planet->type <= 1) {
        // Rocky/Terrestrial planets - closer orbits for easier access
        baseOrbitDistance =
            0.002 + ((double)((planSysEntry->goatSoupSeed.b + j) % 8) /
                     1000.0); // 0.002-0.010 AU
      } else if (planet->type == 2) {
        // Gas giants - farther orbits to avoid radiation and gravitational
        // stress
        baseOrbitDistance =
            0.01 + ((double)((planSysEntry->goatSoupSeed.c + j) % 15) /
                    1000.0); // 0.010-0.025 AU
      } else {
        // Ice giants - moderate orbits
        baseOrbitDistance =
            0.005 + ((double)((planSysEntry->goatSoupSeed.d + j) % 10) /
                     1000.0); // 0.005-0.015 AU
      }

      // For habitable zone planets, keep stations close for easy access
      if (planet->isInHabitableZone) {
        baseOrbitDistance *= 0.7; // 30% closer for habitable worlds
      }

      station->orbitalDistance = baseOrbitDistance;

      // Set station type based on tech level, planet type, and environmental
      // conditions
      if (planSysEntry->techLev >= 10) {
        station->type =
            (planSysEntry->goatSoupSeed.d + j) % 3; // 0-2 for high tech
      } else if (planSysEntry->techLev >= 5) {
        station->type =
            (planSysEntry->goatSoupSeed.a + j) % 2; // 0-1 for medium tech
      } else {
        station->type = 0; // Only basic stations for low tech
      }

      // Set station services based on tech level, planet type, and conditions
      station->hasDockingComputer =
          (planSysEntry->techLev >= 8) ||
          ((planSysEntry->goatSoupSeed.b + j) % 5 == 0);

      // Shipyards more common around habitable and terrestrial worlds
      bool shipyardBonus = planet->isInHabitableZone || (planet->type <= 1);
      station->hasShipyard =
          (planSysEntry->techLev >= 5) ||
          (shipyardBonus && (planSysEntry->goatSoupSeed.c + j) % 3 == 0) ||
          ((planSysEntry->goatSoupSeed.c + j) % 4 == 0);

      station->hasMarket = true; // All stations have markets

      // Missions more common in populated (habitable) systems
      bool missionBonus = planet->isInHabitableZone;
      station->hasMissions =
          (planSysEntry->techLev >= 3) ||
          (missionBonus && (planSysEntry->goatSoupSeed.d + j) % 2 == 0) ||
          ((planSysEntry->goatSoupSeed.d + j) % 3 == 0);

      // Set economic specialization based on planet type, habitability, and
      // environmental conditions Generate specialization: 0=Balanced,
      // 1=Industrial, 2=Agricultural, 3=Mining
      if (planet->isInHabitableZone) {
        // Habitable zone planets favor agricultural and balanced economies
        uint8_t specRoll = (planSysEntry->goatSoupSeed.a + i + j) % 10;
        if (specRoll < 5) {
          station->specialization = 2; // Agricultural
        } else if (specRoll < 7) {
          station->specialization = 1; // Industrial
        } else {
          station->specialization = 0; // Balanced
        }
      } else if (planet->type <= 1) {
        // Rocky/Terrestrial planets outside habitable zone - mining and
        // industrial
        uint8_t specRoll = (planSysEntry->goatSoupSeed.b + i + j) % 10;
        if (specRoll < 4) {
          station->specialization = 3; // Mining
        } else if (specRoll < 8) {
          station->specialization = 1; // Industrial
        } else {
          station->specialization = 0; // Balanced
        }
      } else {
        // Gas and Ice Giants - primarily mining and industrial
        uint8_t specRoll = (planSysEntry->goatSoupSeed.c + i + j) % 10;
        if (specRoll < 7) {
          station->specialization = 3; // Mining (fuel processing, etc.)
        } else if (specRoll < 9) {
          station->specialization = 1; // Industrial
        } else {
          station->specialization = 0; // Balanced
        }
      }

      // Economy modifiers based on system's economy type
      if (planSysEntry->economy < 4) { // Industrial economies (0-3)
        if (station->specialization == 0) {
          station->specialization = 1; // More likely to be industrial
        }
      } else { // Agricultural economies (4-7)
        if (station->specialization == 0) {
          station->specialization = 2; // More likely to be agricultural
        }
      }

      // Initialize market fluctuation factor
      station->marketFluctuation =
          (planSysEntry->goatSoupSeed.c + i + j) % 16; // 0-15 fluctuation

      // Initialize the last market update time to current game time
      // This is important so that the first call to UpdateStationMarket doesn't
      // immediately regenerate.
      station->lastMarketUpdate = game_time_get_seconds();

      // Generate the station's market
      station->market = GenerateStationMarket(
          station, planet, planSysEntry); // Link station to planet
      planet->stations[j] = station;
    }
  }
  // Post-generation check: Ensure populated systems have adequate station
  // infrastructure This addresses the logical inconsistency where systems claim
  // billions of inhabitants but have no stations (infrastructure) for them to
  // live in
  uint64_t totalSystemPopulation =
      (planSysEntry->population >> 3); // Convert to billions like in display
  int totalStations = 0;

  // Count existing stations
  for (int i = 0; i < system->numPlanets; i++) {
    totalStations += system->planets[i].numStations;
  }

  // Determine minimum stations required based on population
  int minStationsRequired = 0;
  if (totalSystemPopulation >= 4) { // 4+ billion people
    minStationsRequired = 3; // Major population centers need multiple stations
  } else if (totalSystemPopulation >= 2) { // 2+ billion people
    minStationsRequired = 2; // Large populations need at least 2 stations
  } else if (totalSystemPopulation >= 1) { // 1+ billion people
    minStationsRequired = 1; // Moderate populations need at least 1 station
  }

  // If we don't have enough stations for the population, add them
  if (totalStations < minStationsRequired) {
    int stationsToAdd =
        minStationsRequired -
        totalStations; // Find the most suitable planets to add stations to
    // Priority: Rock/Earth-like planets first, then others
    for (int addCount = 0; addCount < stationsToAdd; addCount++) {
      Planet *bestPlanet = NULL;
      int bestPriority = -1;

      // Find the best planet that can accommodate another station
      for (int i = 0; i < system->numPlanets; i++) {
        Planet *planet = &system->planets[i];

        // Skip planets that already have maximum reasonable stations
        if (planet->numStations >= 4)
          continue;

        int priority = 0;
        // Rock and Earth-like planets are best for habitation
        // Planet types: 0=Rocky, 1=Terrestrial, 2=Gas Giant, 3=Ice Giant
        if (planet->type == 0 || planet->type == 1) {
          priority = 3; // Rocky/Terrestrial planets best for habitation
        } else if (planet->type == 2) {
          priority = 2; // Gas giants for fuel processing
        } else {
          priority = 1; // Ice giants as last resort
        }

        // Prefer planets with fewer existing stations (spread them out)
        priority = priority * 10 - planet->numStations;

        if (priority > bestPriority) {
          bestPriority = priority;
          bestPlanet = planet;
        }
      }
      // Add a station to the best planet found
      if (bestPlanet && bestPlanet->numStations < MAX_STATIONS_PER_PLANET) {
        Station *newStation = (Station *)malloc(sizeof(Station));
        if (newStation) {
          // Initialize the station structure
          memset(newStation, 0, sizeof(Station));

          // Set up the emergency station with basic properties
          snprintf(newStation->name, sizeof(newStation->name), "Orbital Hab %c",
                   'A' + bestPlanet->numStations);
          newStation->type = 0; // Coriolis (most common)
          newStation->size = 1; // Medium size
          newStation->services =
              0xFF; // All services available for populated areas

          // Set orbital distance from planet
          if (bestPlanet->type <= 1) {
            // Rocky/Terrestrial planets - closer orbits for easier access
            newStation->orbitalDistance =
                0.002 + ((double)((planSysEntry->goatSoupSeed.b +
                                   bestPlanet->numStations) %
                                  8) /
                         1000.0); // 0.002-0.010 AU
          } else if (bestPlanet->type == 2) {
            // Gas giants - farther orbits to avoid radiation and gravitational
            // stress
            newStation->orbitalDistance =
                0.01 + ((double)((planSysEntry->goatSoupSeed.c +
                                  bestPlanet->numStations) %
                                 15) /
                        1000.0); // 0.010-0.025 AU
          } else {
            // Ice giants - moderate orbits
            newStation->orbitalDistance =
                0.005 + ((double)((planSysEntry->goatSoupSeed.d +
                                   bestPlanet->numStations) %
                                  10) /
                         1000.0); // 0.005-0.015 AU
          }

          // Initialize all station services for populated systems
          newStation->hasDockingComputer =
              true; // High population areas need docking computers
          newStation->hasShipyard =
              true; // Major population centers have shipyards
          newStation->hasMarket = true;   // All stations have markets
          newStation->hasMissions = true; // Populated areas have missions

          // Set specialization based on planet type
          // Planet types: 0=Rocky, 1=Terrestrial, 2=Gas Giant, 3=Ice Giant
          if (bestPlanet->type == 0 || bestPlanet->type == 1) {
            newStation->specialization = 0; // Balanced for habitation
          } else if (bestPlanet->type == 2) {
            newStation->specialization =
                3; // Mining (gas giant fuel processing)
          } else {
            newStation->specialization = 1; // Industrial for ice giants
          }

          // Initialize market
          newStation->marketFluctuation =
              (planSysEntry->goatSoupSeed.a + bestPlanet->numStations) % 16;
          newStation->lastMarketUpdate = game_time_get_seconds();
          newStation->market =
              GenerateStationMarket(newStation, bestPlanet, planSysEntry);

          // Link to planet
          bestPlanet->stations[bestPlanet->numStations] = newStation;
          bestPlanet->numStations++;
        }
      }
    }
  }
}

// Function to clean up allocated memory for a star system
static inline void cleanup_star_system(StarSystem *system) {
  if (!system)
    return;

  // Free memory for each station
  for (int i = 0; i < system->numPlanets; i++) {
    Planet *planet = &system->planets[i];
    if (!planet)
      continue;

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
static inline Planet *get_planet_by_index(StarSystem *system, uint8_t index) {
  if (!system || index >= system->numPlanets)
    return NULL;
  return &system->planets[index];
}

// Function to get station from a planet by index
static inline Station *get_station_by_index(Planet *planet, uint8_t index) {
  if (!planet || index >= planet->numStations)
    return NULL;
  return planet->stations[index];
}

// Function to calculate travel time between two points in a system
// Returns time in seconds
static inline uint32_t calculate_travel_time(double startDistance,
                                             double endDistance) {
  // Simple model: 1 AU = 20 minutes of travel
  const double TRAVEL_SPEED_AU_PER_MINUTE = 0.05; // 0.05 AU per minute
  const uint32_t SECONDS_PER_MINUTE = 60;

  double distanceDelta = fabs(endDistance - startDistance);
  double timeInMinutes = distanceDelta / TRAVEL_SPEED_AU_PER_MINUTE;

  return (uint32_t)(timeInMinutes * SECONDS_PER_MINUTE);
}

// Function to travel to a celestial body within a star system
// Updates navigation state and game time
static inline bool travel_to_celestial(StarSystem *system,
                                       NavigationState *navState,
                                       CelestialType targetType,
                                       void *targetBody) {
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
    Planet *targetPlanet = (Planet *)targetBody;

    // Validate the planet is part of this system
    bool planetFound = false;
    for (int i = 0; i < system->numPlanets; i++) {
      if (&system->planets[i] == targetPlanet) {
        planetFound = true;
        break;
      }
    }

    if (!planetFound) {
      fprintf(stderr,
              "Error: Target planet is not part of the current star system.\n");
      return false;
    }

    endDistance = targetPlanet->orbitalDistance;
    break;
  }

  case CELESTIAL_STATION: {
    // Need to find parent planet
    Station *targetStation = (Station *)targetBody;
    Planet *parentPlanet = NULL;

    // Find which planet this station belongs to
    for (int i = 0; i < system->numPlanets && !parentPlanet; i++) {
      Planet *planet = &system->planets[i];
      if (!planet)
        continue;

      for (int j = 0; j < planet->numStations; j++) {
        if (planet->stations[j] == targetStation) {
          parentPlanet = planet;
          break;
        }
      }
    }

    if (!parentPlanet) {
      fprintf(stderr,
              "Error: Could not find parent planet for target station.\n");
      return false;
    }

    // Station distance is planet distance plus orbital offset
    endDistance =
        parentPlanet->orbitalDistance + targetStation->orbitalDistance;
    break;
  }

  case CELESTIAL_NAV_BEACON:
    endDistance = system->navBeaconDistance;
    break;

  default:
    fprintf(stderr, "Error: Unknown celestial type for travel destination.\n");
    return false;  } // Calculate travel time
  uint32_t travelTime = calculate_travel_time(
      startDistance, endDistance); // Calculate fuel requirement for travel
  double distanceDelta = fabs(endDistance - startDistance);
  double fuelRequired = calculate_travel_fuel_requirement(distanceDelta);

  // Check if player ship has enough fuel
  extern struct PlayerShip *PlayerShipPtr;
  if (PlayerShipPtr != NULL) {
    // Check if there's enough fuel
    if (PlayerShipPtr->attributes.fuelLiters < fuelRequired) {
      fprintf(stderr, "Error: Insufficient fuel for travel.\n");
      printf("\nTravel aborted: Insufficient fuel.\n");
      printf("Required: %.3f liters, Available: %.1f liters\n", fuelRequired,
             PlayerShipPtr->attributes.fuelLiters);
      return false;
    }

    // Consume fuel using ConsumeFuel function
    if (!ConsumeFuel(fuelRequired, true)) {
      fprintf(stderr, "Error: Failed to consume fuel for travel.\n");
      printf("\nTravel aborted: Insufficient fuel for operation.\n");
      return false;
    }

    printf("\nTravel fuel consumed: %.3f liters (%.5f LY)", fuelRequired,
           fuelRequired / 100.0);
  }

  // Update game time
  game_time_advance(travelTime);

  // Update navigation state
  navState->currentLocationType = targetType;
  switch (targetType) {
  case CELESTIAL_STAR:
    navState->currentLocation.star = &system->centralStar;
    break;

  case CELESTIAL_PLANET:
    navState->currentLocation.planet = (Planet *)targetBody;
    break;
  case CELESTIAL_STATION:
    navState->currentLocation.station = (Station *)targetBody;

    // Update global location type to indicate we're at a station but not yet
    // docked
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
static inline const char *celestial_type_to_string(CelestialType type) {
  switch (type) {
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
static inline void get_current_location_name(NavigationState *navState,
                                             char *buffer, size_t bufferSize) {
  if (!navState || !buffer || bufferSize == 0)
    return;

  switch (navState->currentLocationType) {
  case CELESTIAL_STAR:
    if (navState->currentLocation.star) {
      snprintf(buffer, bufferSize, "%s (Star)",
               navState->currentLocation.star->name);
    } else {
      snprintf(buffer, bufferSize, "Unknown Star");
    }
    break;

  case CELESTIAL_PLANET:
    if (navState->currentLocation.planet) {
      // Add planet type information for context
      const char *planetTypes[] = {"Rocky/Airless", "Terrestrial", "Gas Giant",
                                   "Ice Giant", "Unknown"};

      uint8_t type = navState->currentLocation.planet->type;
      if (type >= 4)
        type = 4; // Default to "Unknown" for invalid types

      snprintf(buffer, bufferSize, "%s (%s Planet)",
               navState->currentLocation.planet->name, planetTypes[type]);
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

/**
 * Sets the current market to a station's market when docking
 *
 * @param station Pointer to the station being docked with
 * @param planet Pointer to the parent planet
 * @param planSys Pointer to the planet system data
 */
static inline void UseStationMarket(Station *station, Planet *planet,
                                    struct PlanSys *planSys) {
  if (!station || !planSys) // Planet can be NULL for deep space stations
  {
    // fprintf(stderr, "Warning: UseStationMarket called with NULL station or
    // planSys.\\n"); Optionally clear LocalMarket or set to a default empty
    // state
    memset(&LocalMarket, 0, sizeof(MarketType));
    return;
  }

  // 1. Call UpdateStationMarket to ensure the market is up-to-date.
  // game_time_get_seconds() should be available from an included header like
  // elite_state.h
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
static inline void UsePlanetaryMarket(Planet *planet, struct PlanSys *planSys) {
  if (!planet || !planSys) {
    memset(&LocalMarket, 0, sizeof(MarketType));
    return;
  }

  // Ensure the planetary market is initialized if it hasn't been already
  if (!planet->planetaryMarket.isInitialized) {
    planet->planetaryMarket.market = GeneratePlanetaryMarket(planet, planSys);
  } else {
    // Ensure the market is up to date
    UpdatePlanetaryMarket(planet, game_time_get_seconds(), planSys);
  }

  // Set the global LocalMarket to this planet's market
  LocalMarket = planet->planetaryMarket.market;
}

// =============================================================================
// HABITABILITY ANALYSIS FUNCTIONS
// =============================================================================

/**
 * Calculate radiation exposure at planet's orbit relative to Earth
 */
static inline double calculate_radiation_exposure(Planet *planet, Star *star) {
  // Calculate radiation exposure relative to Earth
  double distance = planet->orbitalDistance;
  double stellarLuminosity = star->luminosity;

  // Flux at planet's orbit relative to Earth's solar flux
  double flux = stellarLuminosity / (distance * distance);

  // Additional radiation from high-energy stellar types
  double stellarRadiationFactor = 1.0;
  if (star->spectralClass <= 2) { // O, B, A stars
    stellarRadiationFactor =
        pow(star->mass, 2.0);            // Much higher UV and X-ray emission
  } else if (star->spectralClass == 3) { // F stars
    stellarRadiationFactor = 1.5;
  }

  return flux * stellarRadiationFactor;
}

/**
 * Check if a planet is likely to be tidally locked to its star
 */
static inline bool check_tidal_locking(Planet *planet, Star *star) {
  // Improved tidal locking check based on stellar type and planetary distance
  // Tidal locking is more common around:
  // 1. Close-orbiting planets around any star
  // 2. Planets around M-dwarf stars (which have closer habitable zones)
  // 3. Smaller planets (lower moment of inertia)

  // Base locking distance varies by stellar class
  double baseLockingDistance = 0.0;

  if (star->spectralClass == 6) {
    // M-dwarfs: habitable zone is very close, so larger locking zone
    baseLockingDistance =
        0.5 * sqrt(star->luminosity); // More realistic for M-dwarfs
  } else if (star->spectralClass >= 4) {
    // K and G stars: moderate locking zones
    baseLockingDistance = 0.2 * sqrt(star->luminosity);
  } else {
    // Hotter stars (O, B, A, F): smaller locking zones relative to habitability
    baseLockingDistance = 0.15 * sqrt(star->luminosity);
  }

  // Adjust for planetary size (smaller planets lock more easily)
  double sizeMultiplier = 1.0;
  if (planet->radius < 4000) {
    sizeMultiplier = 1.3; // Smaller planets more likely to be locked
  } else if (planet->radius > 8000) {
    sizeMultiplier = 0.8; // Larger planets resist locking
  }

  double effectiveLockingDistance = baseLockingDistance * sizeMultiplier;

  return planet->orbitalDistance < effectiveLockingDistance;
}

/**
 * Check if a planet has potential for retaining an atmosphere
 */
static inline bool check_planetary_atmosphere_potential(Planet *planet,
                                                        Star *star) {
  // Planets need sufficient mass/size to retain atmosphere
  if (planet->radius < 2000) {
    return false; // Too small to retain significant atmosphere
  }

  // Check if planet is not too close to star (atmosphere stripped)
  double escapeDistance =
      0.1 * sqrt(star->luminosity); // Simplified calculation
  if (planet->orbitalDistance < escapeDistance) {
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
static inline const char *get_temperature_category(double temperature) {
  if (temperature < 200)
    return "Very Cold";
  else if (temperature < 250)
    return "Cold";
  else if (temperature < 290)
    return "Cool";
  else if (temperature < 323)
    return "Warm";
  else if (temperature < 373)
    return "Hot";
  else
    return "Very Hot";
}

/**
 * Get human-readable habitability rating based on score
 */
static inline const char *get_habitability_rating(double score) {
  if (score >= 80)
    return "Excellent";
  else if (score >= 60)
    return "Good";
  else if (score >= 40)
    return "Marginal";
  else if (score >= 20)
    return "Poor";
  else
    return "Hostile";
}

/**
 * Calculate comprehensive habitability score for a planet
 * Returns a score from 0-100 based on multiple factors including temperature,
 * orbital position, planet type, stellar characteristics, and hazards
 */
static inline double calculate_habitability_score(Planet *planet, Star *star) {
  double score = 0.0;

  // Generate pseudo-random variations based on planet properties for
  // consistency Use planet radius and orbital distance as seeds for
  // reproducible "randomness"
  unsigned int seed =
      (unsigned int)(planet->radius * 1000 + planet->orbitalDistance * 10000);
  double variationFactor = (double)(seed % 1000) / 1000.0; // 0.0 to 1.0

  // Temperature scoring (0-25 points, more conservative)
  double tempCelsius = planet->surfaceTemperature - 273.15;
  if (tempCelsius >= -10 && tempCelsius <= 40) {
    score += 25.0; // Optimal temperature range (narrower for realism)
  } else if (tempCelsius >= -40 && tempCelsius <= 70) {
    score += 20.0; // Good temperature range
  } else if (tempCelsius >= -80 && tempCelsius <= 100) {
    score += 15.0; // Acceptable temperature range
  } else if (tempCelsius >= -150 && tempCelsius <= 150) {
    score += 8.0; // Marginal temperature range
  }

  // Orbital distance scoring (0-20 points)
  if (planet->isInHabitableZone) {
    double hzCenter =
        (star->habitableZoneInner + star->habitableZoneOuter) / 2.0;
    double distanceFromCenter = fabs(planet->orbitalDistance - hzCenter);
    double hzWidth = star->habitableZoneOuter - star->habitableZoneInner;

    if (distanceFromCenter < hzWidth * 0.2) {
      score += 20.0; // In center of habitable zone (narrower sweet spot)
    } else if (distanceFromCenter < hzWidth * 0.4) {
      score += 15.0; // In habitable zone
    } else {
      score += 8.0; // Edge of habitable zone
    }
  } else {
    // Penalty for being outside habitable zone
    score -= 5.0;
  }

  // Planet type scoring (0-25 points, more selective) with individual
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
    double densityVariation = (variationFactor - 0.5) * 8.0; // ±4 points
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
    double rockVariation = (variationFactor - 0.5) * 6.0; // ±3 points
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
    double gasVariation = (variationFactor - 0.5) * 12.0; // ±6 points
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
    double iceVariation = (variationFactor - 0.5) * 10.0; // ±5 points
    score += iceVariation;
    break;
  } // Stellar factors (0-15 points, more conservative)
  if (star->spectralClass == 4) { // G stars (like our Sun)
    score += 15.0;                // Best stellar type for habitability
  } else if (star->spectralClass == 3 ||
             star->spectralClass == 5) { // F, K stars
    score += 12.0;                       // Good stellar types for habitability
  } else if (star->spectralClass == 6) { // M stars
    score += 8.0; // M stars can support life but with challenges
  } else {
    score += 0.0; // Other stellar types less suitable
  }

  // Add orbital characteristics variation
  // Simulate orbital eccentricity effects (planets closer to star have more
  // circular orbits)
  double eccentricityPenalty = 0.0;
  if (planet->orbitalDistance < 1.0) {
    // Close planets likely more circular
    eccentricityPenalty = variationFactor * 3.0; // 0-3 point penalty
  } else if (planet->orbitalDistance < 3.0) {
    // Mid-distance planets moderate eccentricity
    eccentricityPenalty = variationFactor * 6.0; // 0-6 point penalty
  } else {
    // Outer planets can be quite eccentric
    eccentricityPenalty = variationFactor * 10.0; // 0-10 point penalty
  }
  score -= eccentricityPenalty;

  // Add planetary formation history variation
  // Some planets may have formed with better/worse initial conditions
  double formationBonus =
      (sin(variationFactor * 6.28) + 1.0) * 4.0; // ±4 points sinusoidal
  score += formationBonus;

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
  double radiationExposure = calculate_radiation_exposure(planet, star);

  // Add planet-specific magnetic field variation
  // Larger planets and gas giants typically have stronger magnetic fields
  double magneticFieldStrength = 1.0;
  if (planet->type >= 2) { // Gas/Ice giants
    magneticFieldStrength =
        1.5 + variationFactor * 1.0; // 1.5-2.5x Earth's field
  } else if (planet->radius > 6000) {
    magneticFieldStrength =
        0.8 + variationFactor * 0.8; // 0.8-1.6x Earth's field
  } else {
    magneticFieldStrength =
        0.2 + variationFactor * 0.6; // 0.2-0.8x Earth's field
  }

  // Adjust radiation exposure based on magnetic field
  double effectiveRadiation = radiationExposure / magneticFieldStrength;

  if (effectiveRadiation > 20.0) {
    score -= 30.0; // Extreme radiation penalty
  } else if (effectiveRadiation > 10.0) {
    score -= 20.0; // High radiation penalty
  } else if (effectiveRadiation > 5.0) {
    score -= 12.0; // Moderate radiation penalty
  } else if (effectiveRadiation > 2.0) {
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
  if (score < 0)
    score = 0;
  if (score > 100)
    score = 100;

  return score;
}