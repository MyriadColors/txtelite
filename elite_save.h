#pragma once

/**
 * ELITE SAVE HEADER
 *
 * This file provides functionality to save and load the game state.
 * The implementation is contained entirely within this header file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "platform_compat.h"      // For cross-platform compatibility
#include "elite_state.h"
#include "elite_player_state.h"
#include "elite_galaxy.h"
#include "elite_market.h"
#include "elite_star_system.h"

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
static inline bool GetSaveFilePath(const char *filename, char *fullPath, size_t size)
{
    // Create the save directory if it doesn't exist
    platform_stat_struct st = {0};
    if (platform_stat(SAVE_DIRECTORY, &st) == -1)
    {
        if (MKDIR(SAVE_DIRECTORY) != 0)
        {
            printf("Error: Could not create directory '%s'.\n", SAVE_DIRECTORY);
            return 0;
        }
    }

    // Check if filename already contains the directory
    if (strncmp(filename, SAVE_DIRECTORY, strlen(SAVE_DIRECTORY)) == 0) 
    {
        // Filename already includes the directory path
        snprintf(fullPath, size, "%s", filename);
    } 
    else 
    {
        // Construct the full path using cross-platform path separator
        platform_make_path(fullPath, size, SAVE_DIRECTORY, filename);
    }

    return 1;
}

// Structure for the save file header
typedef struct
{
    char signature[8];    // "TXTELITE"
    uint16_t version;     // Save format version
    time_t timestamp;     // When the save was created
    char description[64]; // Optional description
} SaveHeader;

// Structure for the game state
typedef struct
{
    // Galaxy and seed data
    struct SeedType seed;
    struct FastSeedType rndSeed;
    uint16_t galaxyNum;

    // Player state
    int currentPlanet;
    int32_t cash;
    uint16_t fuel;
    
    // Legacy cargo fields (kept in save format for now, but populated from PlayerShip)
    uint16_t holdSpace;
    uint16_t shipHold[COMMODITY_ARRAY_SIZE];

    // Market state
    MarketType localMarket;

    // Game Time
    uint64_t gameTimeSeconds; // Added to save the game time

    // Navigation state
    CelestialType currentLocationType;
    double distanceFromStar;
    uint8_t currentPlanetIndex;  // Index of planet in player location, if applicable
    uint8_t currentStationIndex; // Index of station in player location, if applicable

    // Ship data
    char shipClassName[MAX_SHIP_NAME_LENGTH];
    ShipCoreAttributes shipAttributes;
} SaveGameState;

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
static inline bool save_game(const char *filename, const char *description)
{
    char fullPath[256];
    
    // Get the full path with save directory
    if (!GetSaveFilePath(filename, fullPath, sizeof(fullPath)))
    {
        return 0;
    }
      FILE *file = safe_fopen(fullPath, "wb");
    if (!file)
    {
        printf("Error: Could not open file '%s' for writing.\n", fullPath);
        return 0;
    }
    // Prepare header
    SaveHeader header;
    memset(&header, 0, sizeof(header));
    memcpy(header.signature, SAVE_SIGNATURE, 7); 
    header.version = SAVE_VERSION;
    header.timestamp = time(NULL);    if (description)
    {
        snprintf(header.description, sizeof(header.description), "%s", description);
    }
    else
    {
        // Create a default description
        char timeStr[32];
        struct tm timeBuffer;
        if (safe_localtime(&header.timestamp, &timeBuffer) == 0)
        {
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeBuffer);
        }
        else
        {
            snprintf(timeStr, sizeof(timeStr), "Unknown time");
        }
        snprintf(header.description, sizeof(header.description),
                 "%s - %s (Galaxy %d)",
                 timeStr, g_state.Galaxy[g_state.CurrentPlanet].name, g_state.GalaxyNum);
    }

    // Write header
    if (fwrite(&header, sizeof(header), 1, file) != 1)
    {
        printf("Error: Failed to write save header.\n");
        fclose(file);
        return 0;
    }

    // Prepare game state
    SaveGameState state;
    memset(&state, 0, sizeof(state));
    
    state.seed = g_state.SEED;
    state.rndSeed = g_state.RndSeed;
    state.galaxyNum = g_state.GalaxyNum;
    state.currentPlanet = g_state.CurrentPlanet;
    state.cash = g_state.Cash;
    state.fuel = g_state.Fuel;
    
    // Sync legacy cargo fields from PlayerShipPtr
    if (g_state.PlayerShipPtr) {
        state.holdSpace = g_state.PlayerShipPtr->attributes.cargoCapacityTons - g_state.PlayerShipPtr->attributes.currentCargoTons;
        snprintf(state.shipClassName, MAX_SHIP_NAME_LENGTH, "%s", g_state.PlayerShipPtr->shipClassName);
        state.shipAttributes = g_state.PlayerShipPtr->attributes;
        
        // Populate shipHold array from PlayerShipPtr->cargo
        for (int i = 0; i <= LAST_TRADE; i++) {
            for (int j = 0; j < MAX_CARGO_SLOTS; j++) {
                if (StringCompareIgnoreCase(g_state.PlayerShipPtr->cargo[j].name, g_state.tradnames[i]) == 0) {
                    state.shipHold[i] = g_state.PlayerShipPtr->cargo[j].quantity;
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

    if (g_state.PlayerNavState.currentLocationType == CELESTIAL_PLANET)
    {
        for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets; i++)
        {
            if (g_state.PlayerNavState.currentLocation.planet == &g_state.CurrentStarSystem->planets[i])
            {
                state.currentPlanetIndex = i;
                break;
            }
        }
    }
    else if (g_state.PlayerNavState.currentLocationType == CELESTIAL_STATION)
    {
        for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets; i++)
        {
            Planet *planet = &g_state.CurrentStarSystem->planets[i];
            for (uint8_t j = 0; j < planet->numStations; j++)
            {
                if (g_state.PlayerNavState.currentLocation.station == planet->stations[j])
                {
                    state.currentPlanetIndex = i;
                    state.currentStationIndex = j;
                    break;
                }
            }
        }
    }

    // Write game state
    if (fwrite(&state, sizeof(state), 1, file) != 1)
    {
        printf("Error: Failed to write game state.\n");
        fclose(file);
        return 0;
    }

    fclose(file);
    printf("Game saved to '%s'.\n", filename);
    return 1;
}

/**
 * @brief Loads game state from a saved file
 */
static inline bool load_game(const char *filename)
{
    char fullPath[256];
    
    if (!GetSaveFilePath(filename, fullPath, sizeof(fullPath)))
    {
        return 0;
    }
    
    FILE *file = safe_fopen(fullPath, "rb");
    if (!file)
    {
        printf("Error: Could not open file '%s' for reading.\n", fullPath);
        return 0;
    }
    // Read header
    SaveHeader header;
    if (fread(&header, sizeof(header), 1, file) != 1)
    {
        printf("Error: Failed to read save header.\n");
        fclose(file);
        return 0;
    }

    // Verify signature
    if (strncmp(header.signature, SAVE_SIGNATURE, 7) != 0)
    {
        printf("Error: Invalid save file format. Expected '%s', found '%.7s'.\n",
               SAVE_SIGNATURE, header.signature);
        fclose(file);
        return 0;
    }

    // Verify version
    if (header.version != SAVE_VERSION)
    {
        printf("Error: Incompatible save file version %d (expected %d).\n",
               header.version, SAVE_VERSION);
        fclose(file);
        return 0;
    }

    // Read game state
    SaveGameState state;
    if (fread(&state, sizeof(state), 1, file) != 1)
    {
        printf("Error: Failed to read game state.\n");
        fclose(file);
        return 0;
    }

    fclose(file);
    // Apply loaded state to game
    g_state.SEED = state.seed;
    g_state.RndSeed = state.rndSeed;
    g_state.GalaxyNum = state.galaxyNum;

    // Rebuild galaxy data if needed
    build_galaxy_data(g_state.SEED);

    g_state.CurrentPlanet = state.currentPlanet;
    g_state.Cash = state.cash;
    g_state.Fuel = state.fuel;
    
    // PlayerShip restoration
    if (g_state.PlayerShipPtr) {
        g_state.PlayerShipPtr->attributes = state.shipAttributes;
        // In a more complete implementation, we'd lookup shipType by shipClassName
        
        // Restore cargo from saved shipHold array
        for (int i = 0; i < MAX_CARGO_SLOTS; i++) {
            g_state.PlayerShipPtr->cargo[i].quantity = 0;
            snprintf(g_state.PlayerShipPtr->cargo[i].name, MAX_SHIP_NAME_LENGTH, "Empty");
        }
        
        int cargoSlot = 0;
        for (int i = 0; i <= LAST_TRADE; i++) {
            if (state.shipHold[i] > 0 && cargoSlot < MAX_CARGO_SLOTS) {
                snprintf(g_state.PlayerShipPtr->cargo[cargoSlot].name, MAX_SHIP_NAME_LENGTH, "%s", g_state.tradnames[i]);
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
    if (state.currentLocationType == CELESTIAL_STAR)
    {
        g_state.PlayerNavState.currentLocation.star = &g_state.CurrentStarSystem->centralStar;
    }
    else if (state.currentLocationType == CELESTIAL_PLANET)
    {
        if (state.currentPlanetIndex < g_state.CurrentStarSystem->numPlanets)
        {
            g_state.PlayerNavState.currentLocation.planet = &g_state.CurrentStarSystem->planets[state.currentPlanetIndex];
        }
        else
        {
            g_state.PlayerNavState.currentLocation.planet = &g_state.CurrentStarSystem->planets[0];
            g_state.PlayerNavState.distanceFromStar = g_state.PlayerNavState.currentLocation.planet->orbitalDistance;
        }
    }
    else if (state.currentLocationType == CELESTIAL_STATION)
    {
        if (state.currentPlanetIndex < g_state.CurrentStarSystem->numPlanets)
        {
            Planet *planet = &g_state.CurrentStarSystem->planets[state.currentPlanetIndex];
            if (state.currentStationIndex < planet->numStations)
            {
                g_state.PlayerNavState.currentLocation.station = planet->stations[state.currentStationIndex];
            }
            else
            {
                g_state.PlayerNavState.currentLocationType = CELESTIAL_PLANET;
                g_state.PlayerNavState.currentLocation.planet = planet;
                g_state.PlayerNavState.distanceFromStar = planet->orbitalDistance;
            }
        }
        else
        {
            g_state.PlayerNavState.currentLocationType = CELESTIAL_PLANET;
            g_state.PlayerNavState.currentLocation.planet = &g_state.CurrentStarSystem->planets[0];
            g_state.PlayerNavState.distanceFromStar = g_state.PlayerNavState.currentLocation.planet->orbitalDistance;
        }
    }
    else if (state.currentLocationType == CELESTIAL_NAV_BEACON)
    {
        g_state.PlayerNavState.distanceFromStar = g_state.CurrentStarSystem->navBeaconDistance;
    }    
    
    // Show load information
    char timeStr[32];
    struct tm timeBuffer;
    if (safe_localtime(&header.timestamp, &timeBuffer) == 0)
    {
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeBuffer);
    }
    else
    {
        snprintf(timeStr, sizeof(timeStr), "Unknown time");
    }
    printf("Game loaded from '%s'.\n", filename);
    printf("Save info: %s\n", header.description);
    printf("Created: %s\n", timeStr);
    printf("Current planet: %s (Galaxy %d)\n", g_state.Galaxy[g_state.CurrentPlanet].name, g_state.GalaxyNum);

    return 1;
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
static inline bool show_save_info(const char *filename)
{
    char fullPath[256];
    
    // Get the full path with save directory
    if (!GetSaveFilePath(filename, fullPath, sizeof(fullPath)))
    {
        return 0;
    }
    
    FILE *file = safe_fopen(fullPath, "rb");
    if (!file)
    {
        printf("Error: Could not open file '%s' for reading.\n", fullPath);
        return 0;
    }
    // Read header
    SaveHeader header;
    if (fread(&header, sizeof(header), 1, file) != 1)
    {
        printf("Error: Failed to read save header.\n");
        fclose(file);
        return 0;
    }

    // Verify signature
    if (strncmp(header.signature, SAVE_SIGNATURE, 7) != 0)
    {
        printf("Error: Invalid save file format. Expected '%s', found '%.7s'.\n",
               SAVE_SIGNATURE, header.signature);
        fclose(file);
        return 0;
    }    // Display information
    char timeStr[32];
    struct tm timeBuffer;
    if (safe_localtime(&header.timestamp, &timeBuffer) == 0)
    {
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeBuffer);
    }
    else
    {
        snprintf(timeStr, sizeof(timeStr), "Unknown time");
    }
    printf("Save file: %s\n", filename);
    printf("Version: %d\n", header.version);
    printf("Created: %s\n", timeStr);
    printf("Description: %s\n", header.description);

    fclose(file);
    return 1;
}

// GetSaveFilePath function has been moved to the top of the file

/**
 * Helper function to get a default save filename based on the current planet.
 *
 * @param buffer Buffer to write the filename to.
 * @param size Size of the buffer.
 */
static inline void get_default_save_filename(char *buffer, size_t size)
{
    char filename[MAX_PATH];
    snprintf(filename, sizeof(filename), "txtelite_save_%s_g%d.sav", g_state.Galaxy[g_state.CurrentPlanet].name, g_state.GalaxyNum);
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
static inline bool create_save_directory()
{
    platform_stat_struct st = {0};
    if (platform_stat(SAVE_DIRECTORY, &st) == -1)
    {
        // Directory does not exist, attempt to create it
        if (MKDIR(SAVE_DIRECTORY) != 0)
        {
            printf("Error: Failed to create save directory '%s'.\n", SAVE_DIRECTORY);
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Gets the full path for a save file in the designated save directory.
 *
 * This function constructs the full file path for a save file by combining the save directory,
 * the base filename, and the file extension. It ensures that the directory separator is correct
 * for the current platform.
 *
 * @param buffer Buffer to write the file path to.
 * @param size Size of the buffer.
 * @param filename Base filename without path or extension.
 */
static inline void get_save_file_path(char *buffer, size_t size, const char *filename)
{
    char fullFilename[MAX_PATH];
    snprintf(fullFilename, sizeof(fullFilename), "%s.sav", filename);
    platform_make_path(buffer, size, SAVE_DIRECTORY, fullFilename);
}
