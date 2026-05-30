#pragma once

#include "elite_player_ship.h"
#include "elite_market.h"  // For Commodities array
#include <string.h> // For string functions
#include <stdio.h>  // For printf
#include <ctype.h>  // For tolower
#include "platform_compat.h" // For StringCompareIgnoreCase

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
 * @return 1 if cargo was successfully added, 0 if there was no space
 */
static inline bool AddCargo(PlayerShip *playerShip, const char *cargoName, int quantity, int purchasePrice)
{
    if (playerShip == NULL || cargoName == NULL || quantity <= 0)
    {
        return 0;
    }

    // Check if there's enough cargo capacity
    if (playerShip->attributes.currentCargoTons + quantity > playerShip->attributes.cargoCapacityTons)
    {
        printf("Error: Not enough cargo space. Available: %d tonnes, Needed: %d tonnes\n",
               playerShip->attributes.cargoCapacityTons - playerShip->attributes.currentCargoTons,
               quantity);
        return 0;
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
            return 0;
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
    return 1;
}

/**
 * Remove cargo from the player's ship.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to remove
 * @param quantity Amount of cargo to remove (in tonnes)
 * @return 1 if cargo was successfully removed, 0 if the ship doesn't have that cargo
 */
static inline bool RemoveCargo(PlayerShip *playerShip, const char *cargoName, int quantity)
{
    if (playerShip == NULL || cargoName == NULL || quantity <= 0)
    {
        return 0;
    }
    // Find the cargo slot
    int cargoSlot = FindCargoSlot(playerShip, cargoName);

    if (cargoSlot < 0)
    {
        printf("\nError: %s not found in cargo hold.", cargoName);
        return 0;
    }

    // Check if we have enough of this cargo
    if (playerShip->cargo[cargoSlot].quantity < quantity)
    {
        printf("\nError: Not enough %s in cargo hold. Available: %d tonnes, Requested: %d tonnes",
               cargoName, playerShip->cargo[cargoSlot].quantity, quantity);
        return 0;
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
    return 1;
}

/**
 * Sell cargo from the player's ship at a trading port and earn credits.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to sell
 * @param quantity Amount of cargo to sell (in tonnes)
 * @param salePrice Price per tonne that will be paid
 * @param externalSync If 1, synchronize with the global state (Cash)
 * @return 1 if cargo was successfully sold, 0 if there was an error
 */
static inline bool SellCargo(PlayerShip *playerShip, const char *cargoName, int quantity, int salePrice, bool externalSync)
{
    if (playerShip == NULL || cargoName == NULL || quantity <= 0 || salePrice < 0)
    {
        return 0;
    }

    // Try to remove the cargo
    if (!RemoveCargo(playerShip, cargoName, quantity))
    {
        return 0;
    }

    // Calculate the total sale amount
    int totalSale = quantity * salePrice;

    // If we need to sync with the game's global state
    if (externalSync)
    {
        g_state.Cash += totalSale;
    }

    printf("Sold %d tonnes of %s for %d credits.\n", quantity, cargoName, totalSale);
    return 1;
}

/**
 * Buy cargo for the player's ship from a trading port.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to buy
 * @param quantity Amount of cargo to buy (in tonnes)
 * @param purchasePrice Price per tonne that must be paid
 * @param externalSync If 1, synchronize with the global state (Cash)
 * @return 1 if cargo was successfully purchased, 0 if there was an error
 */
static inline bool BuyCargo(PlayerShip *playerShip, const char *cargoName, int quantity, int purchasePrice, bool externalSync)
{
    if (playerShip == NULL || cargoName == NULL || quantity <= 0 || purchasePrice < 0)
    {
        return 0;
    }

    // Calculate the total cost
    int totalCost = quantity * purchasePrice;

    // If we need to sync with the game's global state, check if we have enough cash
    if (externalSync)
    {
        if (g_state.Cash < totalCost)
        {
            printf("Error: Not enough credits. Available: %d, Required: %d\n", g_state.Cash, totalCost);
            return 0;
        }
    }

    // Try to add the cargo
    if (!AddCargo(playerShip, cargoName, quantity, purchasePrice))
    {
        return 0;
    }

    // If everything successful and we need to sync, deduct the cash
    if (externalSync)
    {
        g_state.Cash -= totalCost;
    }

    printf("Purchased %d tonnes of %s for %d credits.\n", quantity, cargoName, totalCost);
    return 1;
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

    bool hasCargo = 0;
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity > 0)
        {
            hasCargo = 1;
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
 * @return 1 if cargo was successfully jettisoned, 0 if there was an error
 */
static inline bool JettisonCargo(PlayerShip *playerShip, const char *cargoName, int quantity)
{
    if (playerShip == NULL || cargoName == NULL || quantity <= 0)
    {
        return 0;
    }

    // This simply removes cargo but with a different message
    if (RemoveCargo(playerShip, cargoName, quantity))
    {
        printf("\nJettisoned %d tonnes of %s into space.", quantity, cargoName);
        return 1;
    }

    return 0;
}

/**
 * Jettison all cargo from the ship (remove all cargo without selling it).
 * Useful in emergencies such as being chased by authorities or in combat situations.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @return 1 if cargo was successfully jettisoned, 0 if there was an error
 */
static inline bool JettisonAllCargo(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    // Check if the ship has any cargo at all
    if (playerShip->attributes.currentCargoTons <= 0)
    {
        printf("\nNo cargo to jettison.");
        return 0;
    }

    int totalJettisoned = 0;

    // Iterate through all cargo slots
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity > 0)
        {
            int quantity = playerShip->cargo[i].quantity;
            totalJettisoned += quantity;

            printf("\nJettisoned %d tonnes of %s into space.",
                   quantity, playerShip->cargo[i].name);

            // Clear the cargo slot
            playerShip->cargo[i].quantity = 0;
            snprintf(playerShip->cargo[i].name, MAX_SHIP_NAME_LENGTH, "%s", "Empty");
            playerShip->cargo[i].purchasePrice = 0;
        }
    }

    // Reset current cargo weight
    playerShip->attributes.currentCargoTons = 0;

    // Print summary
    printf("\nAll cargo jettisoned: %d tonnes total.", totalJettisoned);

    return 1;
}
