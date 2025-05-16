#ifndef ELITE_SAVE_H
#define ELITE_SAVE_H

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
#include "elite_state.h"
#include "elite_player_state.h"
#include "elite_galaxy.h"
#include "elite_market.h"

// Version identifier for the save file format
#define SAVE_VERSION 1

// Signature to identify valid save files
#define SAVE_SIGNATURE "TXTELITE"

// Structure for the save file header
typedef struct {
    char signature[8];    // "TXTELITE"
    uint16_t version;     // Save format version
    time_t timestamp;     // When the save was created
    char description[64]; // Optional description
} SaveHeader;

// Structure for the game state
typedef struct {
    // Galaxy and seed data
    struct SeedType seed;
    struct FastSeedType rndSeed;
    uint16_t galaxyNum;
    
    // Player state
    int currentPlanet;
    int32_t cash;
    uint16_t fuel;
    uint16_t holdSpace;
    uint16_t shipHold[COMMODITY_ARRAY_SIZE];
    
    // Market state
    MarketType localMarket;
} SaveGameState;

/**
 * Saves the current game state to a file.
 * 
 * @param filename The name of the file to save to.
 * @param description Optional description of the save (can be NULL).
 * @return True if the save was successful, false otherwise.
 */
static inline bool save_game(const char* filename, const char* description) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Could not open file '%s' for writing.\n", filename);
        return false;
    }
      // Prepare header
    SaveHeader header;
    memset(&header, 0, sizeof(header));
    memcpy(header.signature, SAVE_SIGNATURE, 7);  // Exactly copy the 7 characters without null terminator
    header.version = SAVE_VERSION;
    header.timestamp = time(NULL);
    
    if (description) {
        strncpy(header.description, description, sizeof(header.description) - 1);
    } else {
        // Create a default description with timestamp and planet name
        char timeStr[32];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&header.timestamp));
        snprintf(header.description, sizeof(header.description), 
                 "%s - %s (Galaxy %d)", 
                 timeStr, Galaxy[CurrentPlanet].name, GalaxyNum);
    }
    
    // Write header
    if (fwrite(&header, sizeof(header), 1, file) != 1) {
        printf("Error: Failed to write save header.\n");
        fclose(file);
        return false;
    }
    
    // Prepare game state
    SaveGameState state;
    memset(&state, 0, sizeof(state));
    
    // Copy current game state into save structure
    state.seed = Seed;
    state.rndSeed = RndSeed;
    state.galaxyNum = GalaxyNum;
    state.currentPlanet = CurrentPlanet;
    state.cash = Cash;
    state.fuel = Fuel;
    state.holdSpace = HoldSpace;
    memcpy(state.shipHold, ShipHold, sizeof(ShipHold));
    state.localMarket = LocalMarket;
    
    // Write game state
    if (fwrite(&state, sizeof(state), 1, file) != 1) {
        printf("Error: Failed to write game state.\n");
        fclose(file);
        return false;
    }
    
    fclose(file);
    printf("Game saved to '%s'.\n", filename);
    return true;
}

/**
 * Loads a game state from a file.
 * 
 * @param filename The name of the file to load from.
 * @return True if the load was successful, false otherwise.
 */
static inline bool load_game(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open file '%s' for reading.\n", filename);
        return false;
    }
      // Read header
    SaveHeader header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        printf("Error: Failed to read save header.\n");
        fclose(file);
        return false;
    }
    
    // Verify signature
    if (strncmp(header.signature, SAVE_SIGNATURE, 7) != 0) {
        printf("Error: Invalid save file format. Expected '%s', found '%.7s'.\n", 
               SAVE_SIGNATURE, header.signature);
        fclose(file);
        return false;
    }
    
    // Verify version
    if (header.version != SAVE_VERSION) {
        printf("Error: Incompatible save file version %d (expected %d).\n", 
               header.version, SAVE_VERSION);
        fclose(file);
        return false;
    }
    
    // Read game state
    SaveGameState state;
    if (fread(&state, sizeof(state), 1, file) != 1) {
        printf("Error: Failed to read game state.\n");
        fclose(file);
        return false;
    }
    
    fclose(file);
    
    // Apply loaded state to game
    Seed = state.seed;
    RndSeed = state.rndSeed;
    GalaxyNum = state.galaxyNum;
    
    // Rebuild galaxy data if needed
    build_galaxy_data(Seed);
    
    CurrentPlanet = state.currentPlanet;
    Cash = state.cash;
    Fuel = state.fuel;
    HoldSpace = state.holdSpace;
    memcpy(ShipHold, state.shipHold, sizeof(ShipHold));
    LocalMarket = state.localMarket;
    
    // Show load information
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&header.timestamp));
    printf("Game loaded from '%s'.\n", filename);
    printf("Save info: %s\n", header.description);
    printf("Created: %s\n", timeStr);
    printf("Current planet: %s (Galaxy %d)\n", Galaxy[CurrentPlanet].name, GalaxyNum);
    
    return true;
}

/**
 * Displays information about a save file without loading it.
 * 
 * @param filename The name of the file to examine.
 * @return True if the information was successfully retrieved, false otherwise.
 */
static inline bool show_save_info(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open file '%s' for reading.\n", filename);
        return false;
    }
      // Read header
    SaveHeader header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        printf("Error: Failed to read save header.\n");
        fclose(file);
        return false;
    }
    
    // Verify signature
    if (strncmp(header.signature, SAVE_SIGNATURE, 7) != 0) {
        printf("Error: Invalid save file format. Expected '%s', found '%.7s'.\n", 
               SAVE_SIGNATURE, header.signature);
        fclose(file);
        return false;
    }
    
    // Display information
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&header.timestamp));
    printf("Save file: %s\n", filename);
    printf("Version: %d\n", header.version);
    printf("Created: %s\n", timeStr);
    printf("Description: %s\n", header.description);
    
    fclose(file);
    return true;
}

/**
 * Helper function to get a default save filename based on the current planet.
 * 
 * @param buffer Buffer to write the filename to.
 * @param size Size of the buffer.
 */
static inline void get_default_save_filename(char* buffer, size_t size) {
    snprintf(buffer, size, "txtelite_save_%s_g%d.sav", 
            Galaxy[CurrentPlanet].name, GalaxyNum);
}

#endif // ELITE_SAVE_H
