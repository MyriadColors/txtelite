#pragma once

/**
 * ELITE SAVE HEADER
 *
 * This file provides functionality to save and load the game state.
 * The implementation is contained entirely within this header file.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "elite_galaxy.h"
#include "elite_navigation_types.h"
#include "elite_player_state.h"
#include "elite_ship_components.h"
#include "elite_star_system.h"
#include "elite_state.h"
#include "platform_compat.h" // For cross-platform compatibility

// Version identifier for the save file format
#define SAVE_VERSION 2

// Signature to identify valid save files
#define SAVE_SIGNATURE "TXTELITE"

// Directory where save files are stored
#define SAVE_DIRECTORY "saves"

/**
 * Helper function to ensure the saves directory exists and construct the full path.
 *
 * @param filename The base filename for the save
 * @param fullPath Buffer to store the full path
 * @param size Size of the fullPath buffer
 * @return 1 if the directory exists or was created successfully, 0 otherwise
 */
static inline bool get_save_file_path(const char *filename, char *full_path, size_t size) {
    // Create the save directory if it doesn't exist
    PLATFORM_STAT_STRUCT st = {0};
    if (PLATFORM_STAT(SAVE_DIRECTORY, &st) == -1) {
        if (MKDIR(SAVE_DIRECTORY) != 0) {
            printf("Error: Could not create directory '%s'.\n", SAVE_DIRECTORY);
            return false;
        }
    }

    // Check if filename already contains the directory
    if (strncmp(filename, SAVE_DIRECTORY, strlen(SAVE_DIRECTORY)) == 0) {
        // Filename already includes the directory path
        safe_snprintf(full_path, size, "%s", filename);
    } else {
        // Construct the full path using cross-platform path separator
        platform_make_path(full_path, size, SAVE_DIRECTORY, filename);
    }

    return true;
}

// Structure for the save file header
typedef struct {
    char signature[8];    // "TXTELITE"
    uint16_t version;     // Save format version
    time_t timestamp;     // When the save was created
    char description[64]; // Optional description
} save_header_t;

// Structure for the game state
typedef struct {
    // Galaxy and seed data
    struct seed_type_t seed;
    struct fast_seed_type_t rndSeed;
    uint16_t galaxyNum;

    // Player state
    int currentPlanet;
    int32_t cash;
    uint16_t fuel;

    // Legacy cargo fields (kept in save format for now, but populated from PlayerShip)
    uint16_t holdSpace;
    uint16_t shipHold[COMMODITY_ARRAY_SIZE];

    // Market state
    market_type_t localMarket;

    // Game Time
    uint64_t gameTimeSeconds; // Added to save the game time

    // Navigation state
    celestial_type_t currentLocationType;
    double distanceFromStar;
    uint8_t currentPlanetIndex;  // Index of planet in player location, if applicable
    uint8_t currentStationIndex; // Index of station in player location, if applicable

    // Ship data
    char shipClassName[MAX_SHIP_NAME_LENGTH];
    ship_core_attributes_t shipAttributes;
} save_game_state_t;

/**
 * @brief Saves the current game state to a file
 *
 * This function writes the current game state to a binary file.
 *
 * @param filename The path to the save file to be created
 * @param description Optional custom description for the save
 *
 * @return 1 if the save operation succeeded, 0 if any error occurred
 */
// This header exposes optional save functionality; not every translation unit uses it.
// The implementation is intentionally kept together to preserve the save transaction order.
[[maybe_unused]] static inline bool
save_game(const char *filename,
          const char *description) { // NOLINT(readability-function-size, readability-function-cognitive-complexity)
    char full_path[256];

    // Get the full path with save directory
    if (!get_save_file_path(filename, full_path, sizeof(full_path))) {
        return false;
    }
    FILE *file = safe_fopen(full_path, "wb");
    if (!file) {
        printf("Error: Could not open file '%s' for writing.\n", full_path);
        return false;
    }
    // Prepare header
    save_header_t header;
    memset(&header, 0, sizeof(header));
    memcpy(header.signature, SAVE_SIGNATURE, 7);
    header.version = SAVE_VERSION;
    header.timestamp = time(nullptr);
    if (description) {
        safe_snprintf(header.description, sizeof(header.description), "%s", description);
    } else {
        // Create a default description
        char time_str[32];
        struct tm timeBuffer;
        if (safe_localtime(&header.timestamp, &timeBuffer) == 0) {
            if (strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &timeBuffer) == 0) {
                safe_snprintf(time_str, sizeof(time_str), "Unknown time");
            }
        } else {
            safe_snprintf(time_str, sizeof(time_str), "Unknown time");
        }
        safe_snprintf(header.description, sizeof(header.description), "%s - %s (Galaxy %d)", time_str,
                      g_state.Galaxy[g_state.CurrentPlanet].name, g_state.GalaxyNum);
    }

    // Write header
    if (fwrite(&header, sizeof(header), 1, file) != 1) {
        printf("Error: Failed to write save header.\n");
        if (fclose(file) != 0) {
            printf("Error: Failed to close save file.\n");
        }
        return false;
    }

    // Prepare game state
    save_game_state_t state;
    memset(&state, 0, sizeof(state));

    state.seed = g_state.SEED;
    state.rndSeed = g_state.RndSeed;
    state.galaxyNum = g_state.GalaxyNum;
    state.currentPlanet = g_state.CurrentPlanet;
    state.cash = g_state.Cash;
    state.fuel = g_state.Fuel;

    // Sync legacy cargo fields from PlayerShipPtr
    if (g_state.PlayerShipPtr) {
        state.holdSpace = (uint16_t)(g_state.PlayerShipPtr->attributes.cargoCapacityTons -
                                     g_state.PlayerShipPtr->attributes.currentCargoTons);
        safe_snprintf(state.shipClassName, MAX_SHIP_NAME_LENGTH, "%s", g_state.PlayerShipPtr->shipClassName);
        state.shipAttributes = g_state.PlayerShipPtr->attributes;

        // Populate shipHold array from PlayerShipPtr->cargo
        for (int i = 0; i <= LAST_TRADE; i++) {
            for (int j = 0; j < MAX_CARGO_SLOTS; j++) {
                if (StringCompareIgnoreCase(g_state.PlayerShipPtr->cargo[j].name, g_state.tradnames[i]) == 0) {
                    state.shipHold[i] = (uint16_t)(g_state.PlayerShipPtr->cargo[j].quantity);
                    break;
                }
            }
        }
    }

    state.localMarket = g_state.LocalMarket;
    state.gameTimeSeconds = g_state.currentGameTimeSeconds;

    state.currentLocationType = g_state.PlayerNavState.currentLocationType;
    state.distanceFromStar = g_state.PlayerNavState.distanceFromStar;

    state.currentPlanetIndex = 0;
    state.currentStationIndex = 0;

    if (g_state.PlayerNavState.currentLocationType == CELESTIAL_PLANET) {
        for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets; i++) {
            if (g_state.PlayerNavState.currentLocation.planet == &g_state.CurrentStarSystem->planets[i]) {
                state.currentPlanetIndex = i;
                break;
            }
        }
    } else if (g_state.PlayerNavState.currentLocationType == CELESTIAL_STATION) {
        for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets; i++) {
            planet_t *planet = &g_state.CurrentStarSystem->planets[i];
            for (uint8_t j = 0; j < planet->numStations; j++) {
                if (g_state.PlayerNavState.currentLocation.station == planet->stations[j]) {
                    state.currentPlanetIndex = i;
                    state.currentStationIndex = j;
                    break;
                }
            }
        }
    }

    // Write game state
    if (fwrite(&state, sizeof(state), 1, file) != 1) {
        printf("Error: Failed to write game state.\n");
        if (fclose(file) != 0) {
            printf("Error: Failed to close save file after write failure.\n");
        }
        return false;
    }

    if (fclose(file) != 0) {
        printf("Error: Failed to close save file.\n");
        return false;
    }
    printf("Game saved to '%s'.\n", filename);
    return true;
}

/**
 * @brief Loads game state from a saved file
 */
[[maybe_unused]] static inline bool load_game(const char *filename) {
    char fullPath[256];

    if (!get_save_file_path(filename, fullPath, sizeof(fullPath))) {
        return false;
    }

    FILE *file = safe_fopen(fullPath, "rb");
    if (!file) {
        printf("Error: Could not open file '%s' for reading.\n", fullPath);
        return false;
    }
    // Read header
    save_header_t header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        printf("Error: Failed to read save header.\n");
        if (fclose(file) != 0) {
            printf("Error: Failed to close save file after write failure.\n");
        }
        return false;
    }

    // Verify signature
    if (strncmp(header.signature, SAVE_SIGNATURE, 7) != 0) {
        printf("Error: Invalid save file format. Expected '%s', found '%.7s'.\n", SAVE_SIGNATURE, header.signature);
        if (fclose(file) != 0) {
            printf("Error: Failed to close save file after write failure.\n");
        }
        return false;
    }

    // Verify version
    if (header.version != SAVE_VERSION) {
        printf("Error: Incompatible save file version %d (expected %d).\n", header.version, SAVE_VERSION);
        if (fclose(file) != 0) {
            printf("Error: Failed to close save file after write failure.\n");
        }
        return false;
    }

    // Read game state
    save_game_state_t state;
    if (fread(&state, sizeof(state), 1, file) != 1) {
        printf("Error: Failed to read game state.\n");
        if (fclose(file) != 0) {
            printf("Error: Failed to close save file after write failure.\n");
        }
        return false;
    }

    if (fclose(file) != 0) {
        printf("Error: Failed to close save file after write failure.\n");
    }
    // Apply loaded state to game
    g_state.SEED = state.seed;
    g_state.RndSeed = state.rndSeed;
    g_state.GalaxyNum = state.galaxyNum;

    // Rebuild galaxy data if needed
    build_galaxy_data(g_state.SEED);

    g_state.CurrentPlanet = state.currentPlanet;
    g_state.Cash = state.cash;
    g_state.Fuel = state.fuel;

    // player_ship_trestoration
    if (g_state.PlayerShipPtr) {
        g_state.PlayerShipPtr->attributes = state.shipAttributes;
        // In a more complete implementation, we'd lookup ship_type_t by shipClassName

        // Restore cargo from saved shipHold array
        for (int i = 0; i < MAX_CARGO_SLOTS; i++) {
            g_state.PlayerShipPtr->cargo[i].quantity = 0;
            safe_snprintf(g_state.PlayerShipPtr->cargo[i].name, MAX_SHIP_NAME_LENGTH, "Empty");
        }

        int cargoSlot = 0;
        for (int i = 0; i <= LAST_TRADE; i++) {
            if (state.shipHold[i] > 0 && cargoSlot < MAX_CARGO_SLOTS) {
                safe_snprintf(g_state.PlayerShipPtr->cargo[cargoSlot].name, MAX_SHIP_NAME_LENGTH, "%s",
                              g_state.tradnames[i]);
                g_state.PlayerShipPtr->cargo[cargoSlot].quantity = state.shipHold[i];
                cargoSlot++;
            }
        }
    }

    g_state.LocalMarket = state.localMarket;
    g_state.currentGameTimeSeconds = state.gameTimeSeconds;

    // Initialize star system for the current planet
    initialize_star_system_for_current_planet();

    // Restore navigation state
    g_state.PlayerNavState.currentLocationType = state.currentLocationType;
    g_state.PlayerNavState.distanceFromStar = state.distanceFromStar;

    // Reconstruct the pointers based on saved indices
    if (state.currentLocationType == CELESTIAL_STAR) {
        g_state.PlayerNavState.currentLocation.star = &g_state.CurrentStarSystem->centralStar;
    } else if (state.currentLocationType == CELESTIAL_PLANET) {
        if (state.currentPlanetIndex < g_state.CurrentStarSystem->numPlanets) {
            g_state.PlayerNavState.currentLocation.planet =
                &g_state.CurrentStarSystem->planets[state.currentPlanetIndex];
        } else {
            g_state.PlayerNavState.currentLocation.planet = &g_state.CurrentStarSystem->planets[0];
            g_state.PlayerNavState.distanceFromStar = g_state.PlayerNavState.currentLocation.planet->orbitalDistance;
        }
    } else if (state.currentLocationType == CELESTIAL_STATION) {
        if (state.currentPlanetIndex < g_state.CurrentStarSystem->numPlanets) {
            planet_t *planet = &g_state.CurrentStarSystem->planets[state.currentPlanetIndex];
            if (state.currentStationIndex < planet->numStations) {
                g_state.PlayerNavState.currentLocation.station = planet->stations[state.currentStationIndex];
            } else {
                g_state.PlayerNavState.currentLocationType = CELESTIAL_PLANET;
                g_state.PlayerNavState.currentLocation.planet = planet;
                g_state.PlayerNavState.distanceFromStar = planet->orbitalDistance;
            }
        } else {
            g_state.PlayerNavState.currentLocationType = CELESTIAL_PLANET;
            g_state.PlayerNavState.currentLocation.planet = &g_state.CurrentStarSystem->planets[0];
            g_state.PlayerNavState.distanceFromStar = g_state.PlayerNavState.currentLocation.planet->orbitalDistance;
        }
    } else if (state.currentLocationType == CELESTIAL_NAV_BEACON) {
        g_state.PlayerNavState.distanceFromStar = g_state.CurrentStarSystem->navBeaconDistance;
    }

    // Show load information
    char timeStr[32];
    struct tm timeBuffer;
    if (safe_localtime(&header.timestamp, &timeBuffer) == 0) {
        if (strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeBuffer) == 0) {
            safe_snprintf(timeStr, sizeof(timeStr), "Unknown time");
        }
    } else {
        safe_snprintf(timeStr, sizeof(timeStr), "Unknown time");
    }
    printf("Game loaded from '%s'.\n", filename);
    printf("Save info: %s\n", header.description);
    printf("Created: %s\n", timeStr);
    printf("Current planet: %s (Galaxy %d)\n", g_state.Galaxy[g_state.CurrentPlanet].name, g_state.GalaxyNum);

    return true;
}

/**
 * @brief Displays information about a saved game file.
 *
 * This function opens a saved game file, verifies its format by checking the signature,
 * and displays information including the version, creation timestamp, and description.
 *
 * The function performs the following steps:
 * 1. Opens the specified file for reading in binary mode
 * 2. Reads the save header
 * 3. Verifies the signature matches the expected format
 * 4. Formats and displays the save information
 *
 * @param filename Path to the save file to display information about
 * @return 1 if the save information was successfully displayed, 0 if an error occurred
 *
 * @note The function handles its own error messages, printing them to stdout
 */
[[maybe_unused]] static inline bool show_save_info(const char *filename) {
    char full_path[256];

    // Get the full path with save directory
    if (!get_save_file_path(filename, full_path, sizeof(full_path))) {
        return false;
    }

    FILE *file = safe_fopen(full_path, "rb");
    if (!file) {
        printf("Error: Could not open file '%s' for reading.\n", full_path);
        return false;
    }
    // Read header
    save_header_t header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        printf("Error: Failed to read save header.\n");
        if (fclose(file) != 0) {
            printf("Error: Failed to close save file after write failure.\n");
        }
        return false;
    }

    // Verify signature
    if (strncmp(header.signature, SAVE_SIGNATURE, 7) != 0) {
        printf("Error: Invalid save file format. Expected '%s', found '%.7s'.\n", SAVE_SIGNATURE, header.signature);
        if (fclose(file) != 0) {
            printf("Error: Failed to close save file after write failure.\n");
        }
        return false;
    } // Display information
    char timeStr[32];
    struct tm timeBuffer;
    if (safe_localtime(&header.timestamp, &timeBuffer) == 0) {
        if (strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeBuffer) == 0) {
            safe_snprintf(timeStr, sizeof(timeStr), "Unknown time");
        }
    } else {
        safe_snprintf(timeStr, sizeof(timeStr), "Unknown time");
    }
    printf("Save file: %s\n", filename);
    printf("Version: %d\n", header.version);
    printf("Created: %s\n", timeStr);
    printf("Description: %s\n", header.description);

    if (fclose(file) != 0) {
        printf("Error: Failed to close save file '%s'.\n", full_path);
        return false;
    }
    return true;
}

// GetSaveFilePath function has been moved to the top of the file

/**
 * Helper function to get a default save filename based on the current planet.
 *
 * @param buffer Buffer to write the filename to.
 * @param size Size of the buffer.
 */
[[maybe_unused]] static inline void get_default_save_filename(char *buffer, size_t size) {
    char filename[MAX_PATH];
    safe_snprintf(filename, sizeof(filename), "txtelite_save_%s_g%d.sav", g_state.Galaxy[g_state.CurrentPlanet].name,
                  g_state.GalaxyNum);
    platform_make_path(buffer, size, SAVE_DIRECTORY, filename);
}

/**
 * @brief Creates the save directory if it doesn't exist.
 *
 * This function checks if the directory for saving files exists, and creates it if necessary.
 * It is used to ensure that the save directory is available before attempting to save a game.
 *
 * @return 1 if the directory exists or was created successfully, 0 if an error occurred
 */
[[maybe_unused]] static inline bool create_save_directory() {
    PLATFORM_STAT_STRUCT st = {0};
    if (PLATFORM_STAT(SAVE_DIRECTORY, &st) == -1) {
        // Directory does not exist, attempt to create it
        if (MKDIR(SAVE_DIRECTORY) != 0) {
            printf("Error: Failed to create save directory '%s'.\n", SAVE_DIRECTORY);
            return false;
        }
    }
    return true;
}
