#pragma once

#include "elite_ship_types.h"
#include "elite_market.h"  // For Commodities array
#include <string.h> // For string functions
#include <stdio.h>  // For printf
#include <ctype.h>  // For tolower

// Helper function for case-insensitive string comparison
// Also ignores trailing whitespace in either string
static inline int StringCompareIgnoreCase(const char *str1, const char *str2)
{
    if (str1 == NULL || str2 == NULL)
    {
        return -1;
    }

    // Skip trailing whitespace
    const char *end1 = str1 + strlen(str1);
    const char *end2 = str2 + strlen(str2);

    // Move end1 back to the last non-whitespace character
    while (end1 > str1 && isspace((unsigned char)*(end1 - 1)))
    {
        end1--;
    }

    // Move end2 back to the last non-whitespace character
    while (end2 > str2 && isspace((unsigned char)*(end2 - 1)))
    {
        end2--;
    }

    // Compare characters up to the non-whitespace ends
    const char *p1 = str1;
    const char *p2 = str2;

    while (p1 < end1 && p2 < end2)
    {
        int c1 = tolower((unsigned char)*p1);
        int c2 = tolower((unsigned char)*p2);

        if (c1 != c2)
        {
            return c1 - c2;
        }

        p1++;
        p2++;
    }

    // If we reached the end of both strings (up to non-whitespace),
    // they are equal; otherwise, the shorter one is "less"
    if (p1 == end1 && p2 == end2)
    {
        return 0; // Strings are equal (ignoring trailing whitespace)
    }
    else if (p1 == end1)
    {
        return -1; // str1 is shorter than str2
    }
    else
    {
        return 1; // str1 is longer than str2
    }
}

/**
 * @brief Finds the index of a cargo slot containing a specified cargo item in the player's ship.
 *
 * This function searches through the player's ship cargo slots to find a slot
 * where the cargo name matches the specified name and the quantity is greater than zero.
 * The comparison is done in a case-insensitive manner.
 *
 * @param playerShip Pointer to the PlayerShip structure to search within.
 * @param cargoName Name of the cargo item to search for.
 * @return The index of the cargo slot if found; -1 if not found or if input is invalid.
 */
static inline int FindCargoSlot(const PlayerShip *playerShip, const char *cargoName)
{
    if (playerShip == NULL || cargoName == NULL)
    {
        return -1;
    }

    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity > 0 && StringCompareIgnoreCase(playerShip->cargo[i].name, cargoName) == 0)
        {
            return i;
        }
    }

    return -1; // Not found
}

/**
 * @brief Finds the index of the first empty cargo slot in the player's ship.
 *
 * This function iterates through the cargo slots of the given PlayerShip and returns
 * the index of the first slot where the quantity is zero, indicating an empty slot.
 *
 * @param playerShip Pointer to the PlayerShip structure to search for an empty cargo slot.
 * @return The index of the first empty cargo slot, or -1 if no empty slot is found or if playerShip is NULL.
 */
static inline int FindEmptyCargoSlot(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return -1;
    }

    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity == 0)
        {
            return i;
        }
    }

    return -1; // No empty slots
}

/**
 * Add cargo to the player's ship. If the cargo already exists, it will increase the quantity.
 * If the cargo doesn't exist, it will be added to the first empty slot.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to add
 * @param quantity Amount of cargo to add (in tonnes)
 * @param purchasePrice Price per tonne (for player's reference)
 * @return true if cargo was successfully added, false if there was no space
 */
static inline bool AddCargo(PlayerShip *playerShip, const char *cargoName, int quantity, int purchasePrice)
{
    if (playerShip == NULL || cargoName == NULL || quantity <= 0)
    {
        return false;
    }

    // Check if there's enough cargo capacity
    if (playerShip->attributes.currentCargoTons + quantity > playerShip->attributes.cargoCapacityTons)
    {
        printf("Error: Not enough cargo space. Available: %d tonnes, Needed: %d tonnes\n",
               playerShip->attributes.cargoCapacityTons - playerShip->attributes.currentCargoTons,
               quantity);
        return false;
    }

    // Check if we already have this cargo type
    int cargoSlot = FindCargoSlot(playerShip, cargoName);

    if (cargoSlot >= 0)
    {
        // Cargo already exists, increase quantity
        playerShip->cargo[cargoSlot].quantity += quantity;

        // Update purchase price as the average of the previous and new price
        // This gives a weighted average of purchase prices
        playerShip->cargo[cargoSlot].purchasePrice =
            (playerShip->cargo[cargoSlot].purchasePrice * (playerShip->cargo[cargoSlot].quantity - quantity) +
             purchasePrice * quantity) /
            playerShip->cargo[cargoSlot].quantity;
    }
    else
    {
        // Need to find an empty slot for the new cargo type
        cargoSlot = FindEmptyCargoSlot(playerShip);

        if (cargoSlot < 0)
        {
            printf("Error: No available cargo slots. Maximum different cargo types reached.\n");
            return false;
        }

        // Add new cargo type
        snprintf(playerShip->cargo[cargoSlot].name, MAX_SHIP_NAME_LENGTH, "%s", cargoName);
        // playerShip->cargo[cargoSlot].name[MAX_SHIP_NAME_LENGTH - 1] = '\\0'; // snprintf handles null termination
        playerShip->cargo[cargoSlot].quantity = quantity;
        playerShip->cargo[cargoSlot].purchasePrice = purchasePrice;
    }

    // Update current cargo weight
    playerShip->attributes.currentCargoTons += quantity;

    printf("Added %d tonnes of %s to cargo hold.\n", quantity, cargoName);
    return true;
}

/**
 * Remove cargo from the player's ship.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to remove
 * @param quantity Amount of cargo to remove (in tonnes)
 * @return true if cargo was successfully removed, false if the ship doesn't have that cargo
 */
static inline bool RemoveCargo(PlayerShip *playerShip, const char *cargoName, int quantity)
{
    if (playerShip == NULL || cargoName == NULL || quantity <= 0)
    {
        return false;
    }
    // Find the cargo slot
    int cargoSlot = FindCargoSlot(playerShip, cargoName);

    if (cargoSlot < 0)
    {
        printf("\nError: %s not found in cargo hold.", cargoName);
        return false;
    }

    // Check if we have enough of this cargo
    if (playerShip->cargo[cargoSlot].quantity < quantity)
    {
        printf("\nError: Not enough %s in cargo hold. Available: %d tonnes, Requested: %d tonnes",
               cargoName, playerShip->cargo[cargoSlot].quantity, quantity);
        return false;
    }

    // Remove the cargo
    playerShip->cargo[cargoSlot].quantity -= quantity;

    // Update current cargo weight
    playerShip->attributes.currentCargoTons -= quantity;

    // If quantity is now 0, clear the slot
    if (playerShip->cargo[cargoSlot].quantity == 0)
    {
        // Clear the cargo slot after removing all quantity
        playerShip->cargo[cargoSlot].quantity = 0;
        snprintf(playerShip->cargo[cargoSlot].name, MAX_SHIP_NAME_LENGTH, "%s", "Empty");
        // playerShip->cargo[cargoSlot].name[MAX_SHIP_NAME_LENGTH - 1] = '\\0'; // snprintf handles null termination
        playerShip->cargo[cargoSlot].purchasePrice = 0;
    }

    printf("Removed %d tonnes of %s from cargo hold.\n", quantity, cargoName);
    return true;
}

/**
 * Sell cargo from the player's ship at a trading port and earn credits.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to sell
 * @param quantity Amount of cargo to sell (in tonnes)
 * @param salePrice Price per tonne that will be paid
 * @param externalSync If true, synchronize with the global state (Cash)
 * @return true if cargo was successfully sold, false if there was an error
 */
static inline bool SellCargo(PlayerShip *playerShip, const char *cargoName, int quantity, int salePrice, bool externalSync)
{
    if (playerShip == NULL || cargoName == NULL || quantity <= 0 || salePrice < 0)
    {
        return false;
    }

    // Try to remove the cargo
    if (!RemoveCargo(playerShip, cargoName, quantity))
    {
        return false;
    }

    // Calculate the total sale amount
    int totalSale = quantity * salePrice;

    // If we need to sync with the game's global state
    if (externalSync)
    {
        extern int32_t Cash;
        Cash += totalSale;
    }

    printf("Sold %d tonnes of %s for %d credits.\n", quantity, cargoName, totalSale);
    return true;
}

/**
 * Buy cargo for the player's ship from a trading port.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to buy
 * @param quantity Amount of cargo to buy (in tonnes)
 * @param purchasePrice Price per tonne that must be paid
 * @param externalSync If true, synchronize with the global state (Cash)
 * @return true if cargo was successfully purchased, false if there was an error
 */
static inline bool BuyCargo(PlayerShip *playerShip, const char *cargoName, int quantity, int purchasePrice, bool externalSync)
{
    if (playerShip == NULL || cargoName == NULL || quantity <= 0 || purchasePrice < 0)
    {
        return false;
    }

    // Calculate the total cost
    int totalCost = quantity * purchasePrice;

    // If we need to sync with the game's global state, check if we have enough cash
    if (externalSync)
    {
        extern int32_t Cash;

        if (Cash < totalCost)
        {
            printf("Error: Not enough credits. Available: %d, Required: %d\n", Cash, totalCost);
            return false;
        }
    }

    // Try to add the cargo
    if (!AddCargo(playerShip, cargoName, quantity, purchasePrice))
    {
        return false;
    }

    // If everything successful and we need to sync, deduct the cash
    if (externalSync)
    {
        extern int32_t Cash;
        Cash -= totalCost;
    }

    printf("Purchased %d tonnes of %s for %d credits.\n", quantity, cargoName, totalCost);
    return true;
}

/**
 * List all cargo items in the player's cargo hold.
 *
 * @param playerShip Pointer to the PlayerShip structure
 */
static inline void ListCargo(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return;
    }

    printf("\n--- Cargo Hold (%d/%d tonnes) ---\n",
           playerShip->attributes.currentCargoTons,
           playerShip->attributes.cargoCapacityTons);

    bool hasCargo = false;
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity > 0)
        {
            hasCargo = true;
            printf("- %s: %d tonnes (Purchased at: %d cr/tonne)\n",
                   playerShip->cargo[i].name,
                   playerShip->cargo[i].quantity,
                   playerShip->cargo[i].purchasePrice);
        }
    }

    if (!hasCargo)
    {
        printf("Cargo hold is empty.\n");
    }

    printf("---------------------------\n");
}

/**
 * Check if the ship has a specific cargo commodity.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to check for
 * @return The quantity of the specified cargo, or 0 if not found
 */
static inline int GetCargoQuantity(const PlayerShip *playerShip, const char *cargoName)
{
    if (playerShip == NULL || cargoName == NULL)
    {
        return 0;
    }

    int cargoSlot = FindCargoSlot(playerShip, cargoName);
    if (cargoSlot >= 0)
    {
        return playerShip->cargo[cargoSlot].quantity;
    }

    return 0;
}

/**
 * Jettison cargo from the ship (remove cargo without selling it).
 * Useful in emergencies or when cargo is illegal.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to jettison
 * @param quantity Amount of cargo to jettison (in tonnes)
 * @return true if cargo was successfully jettisoned, false if there was an error
 */
static inline bool JettisonCargo(PlayerShip *playerShip, const char *cargoName, int quantity)
{
    if (playerShip == NULL || cargoName == NULL || quantity <= 0)
    {
        return false;
    }

    // This simply removes cargo but with a different message
    if (RemoveCargo(playerShip, cargoName, quantity))
    {
        printf("\nJettisoned %d tonnes of %s into space.", quantity, cargoName);
        return true;
    }

    return false;
}

/**
 * Jettison all cargo from the ship (remove all cargo without selling it).
 * Useful in emergencies such as being chased by authorities or in combat situations.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if cargo was successfully jettisoned, false if there was an error
 */
static inline bool JettisonAllCargo(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Check if the ship has any cargo at all
    if (playerShip->attributes.currentCargoTons <= 0)
    {
        printf("\nNo cargo to jettison.");
        return false;
    }

    int totalJettisoned = 0;

    // Temporary array to store cargo data before jettisoning
    // (because we'll be modifying the cargo array while iterating)
    struct
    {
        char name[MAX_SHIP_NAME_LENGTH];
        int quantity;
    } cargoToJettison[MAX_CARGO_SLOTS];

    int numCargoTypes = 0;

    // First, collect all cargo items that need to be jettisoned
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity > 0)
        {
            // Store information about this cargo type for jettisoning
            snprintf(cargoToJettison[numCargoTypes].name, MAX_SHIP_NAME_LENGTH, "%s", playerShip->cargo[i].name);
            // cargoToJettison[numCargoTypes].name[MAX_SHIP_NAME_LENGTH - 1] = '\\0'; // snprintf handles null termination
            cargoToJettison[numCargoTypes].quantity = playerShip->cargo[i].quantity;
            numCargoTypes++;
            totalJettisoned += playerShip->cargo[i].quantity;
        }
    }

    // If no cargo found (should not happen since we checked currentCargoTons)
    if (numCargoTypes == 0)
    {
        printf("\nNo cargo to jettison.");
        return false;
    }

    // Now jettison each cargo item
    for (int i = 0; i < numCargoTypes; ++i)
    {
        // Find cargo index in global tradnames array for ShipHold update
        uint16_t cargoIndex = 0;
        bool cargoFound = false;

        // Find the cargo index in the global tradnames array
        for (uint16_t j = 0; j <= LAST_TRADE; j++)
        {
            if (StringCompareIgnoreCase(tradnames[j], cargoToJettison[i].name) == 0)
            {
                cargoIndex = j;
                cargoFound = true;
                break;
            }
        }

        if (cargoFound)
        {
            // Update the global ShipHold array
            if (ShipHold[cargoIndex] >= cargoToJettison[i].quantity)
            {
                ShipHold[cargoIndex] -= cargoToJettison[i].quantity;

                // Update HoldSpace if it's measured in tons
                if (Commodities[cargoIndex].units == TONNES_UNIT)
                {
                    HoldSpace += cargoToJettison[i].quantity;
                }

                // Remove the cargo using RemoveCargo (which will update playerShip->cargo)
                RemoveCargo(playerShip, cargoToJettison[i].name, cargoToJettison[i].quantity);

                printf("\nJettisoned %d tonnes of %s into space.",
                       cargoToJettison[i].quantity, cargoToJettison[i].name);
            }
        }
    }

    // Clear all remaining cargo in playerShip (in case RemoveCargo missed anything)
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        playerShip->cargo[i].quantity = 0;
        snprintf(playerShip->cargo[i].name, MAX_SHIP_NAME_LENGTH, "%s", "Empty");
        // playerShip->cargo[i].name[MAX_SHIP_NAME_LENGTH - 1] = '\\0'; // snprintf handles null termination
        playerShip->cargo[i].purchasePrice = 0;
    }

    // Reset current cargo tons
    playerShip->attributes.currentCargoTons = 0;

    // Print summary
    printf("\nAll cargo jettisoned: %d tonnes total.", totalJettisoned);

    return true;
}
