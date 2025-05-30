#pragma once

#include "elite_ship_types.h"
#include "elite_star_system.h"
#include "elite_player_state.h"
#include <ctype.h> // For isdigit()

// --- Constants for Ship Trading ---
#define MAX_SHIPS_AT_SHIPYARD 5       // Maximum number of ships available at a shipyard
#define SHIP_DEPRECIATION_RATE 0.05   // 5% depreciation per game month
#define MIN_TRADE_IN_VALUE_PERCENT 40 // Ship won't go below 40% of its new value

// --- Ship Availability by System Type ---
typedef struct ShipAvailability
{
    char shipClassName[MAX_SHIP_NAME_LENGTH]; // Ship class name
    bool availableInIndustrialSystems;
    bool availableInAgriculturalSystems;
    bool availableInMilitarySystems;
    double priceMultiplier; // Price multiplier based on system type
} ShipAvailability;

// --- Temporary Ship Storage ---
typedef struct TempShipStorage
{
    PlayerShip ship;
    bool isActive; // Whether there's a ship in storage
} TempShipStorage;

// Global temporary storage for ship during trade-in
static TempShipStorage tradeInStorage = {.isActive = false};

// --- Ship Availability Database ---
static const ShipAvailability shipAvailabilityDB[] = {
    {"Cobra Mk III", true, true, true, 1.0}, // Available everywhere at standard price
    {"Viper", false, false, true, 0.9},      // Available in military systems, slightly cheaper
    {"Asp Mk II", true, false, true, 1.1},   // Available in industrial and military, slightly more expensive
    // Additional ships can be added here
};

#define NUM_SHIP_AVAILABILITY (sizeof(shipAvailabilityDB) / sizeof(ShipAvailability))

// --- Function Prototypes ---

/**
 * Determines if a ship type is available in the current system
 *
 * @param shipClassName Name of the ship class to check
 * @param systemEconomy Economy of the current system
 * @return true if ship is available, false otherwise
 */
static inline bool IsShipAvailableInSystem(const char *shipClassName, int systemEconomy)
{
    // Find the ship in the availability database
    for (size_t i = 0; i < NUM_SHIP_AVAILABILITY; i++)
    {
        if (strcmp(shipClassName, shipAvailabilityDB[i].shipClassName) == 0)
        {
            // Check availability based on system economy
            switch (systemEconomy)
            {
            case 0: // Agricultural
                return shipAvailabilityDB[i].availableInAgriculturalSystems;
            case 1: // Industrial
                return shipAvailabilityDB[i].availableInIndustrialSystems;
            case 7: // Military
                return shipAvailabilityDB[i].availableInMilitarySystems;
            default:
                // For other economies, available if industrial or agricultural
                return shipAvailabilityDB[i].availableInIndustrialSystems ||
                       shipAvailabilityDB[i].availableInAgriculturalSystems;
            }
        }
    }

    // If not found, assume not available
    return false;
}

/**
 * Gets the price multiplier for a ship in the current system
 *
 * @param shipClassName Name of the ship class
 * @param systemEconomy Economy of the current system
 * @return Price multiplier (1.0 is standard price)
 */
static inline double GetShipPriceMultiplier(const char *shipClassName, int systemEconomy)
{
    // Find the ship in the availability database
    for (size_t i = 0; i < NUM_SHIP_AVAILABILITY; i++)
    {
        if (strcmp(shipClassName, shipAvailabilityDB[i].shipClassName) == 0)
        {
            // Apply system-specific adjustments
            double baseMultiplier = shipAvailabilityDB[i].priceMultiplier;

            // Additional adjustments based on system economy
            switch (systemEconomy)
            {
            case 0:                           // Agricultural
                return baseMultiplier * 1.05; // 5% more expensive
            case 1:                           // Industrial
                return baseMultiplier * 0.95; // 5% cheaper
            case 7:                           // Military
                return baseMultiplier * 1.1;  // 10% more expensive
            default:
                return baseMultiplier;
            }
        }
    }

    // If not found, assume standard price
    return 1.0;
}

/**
 * Gets a list of ships available for purchase in the current system
 *
 * @param systemName Name of the current star system
 * @param systemEconomy Economy of the current system
 * @param availableShips Array to store available ship types (must be at least MAX_SHIPS_AT_SHIPYARD)
 * @param shipPrices Array to store ship prices (must be at least MAX_SHIPS_AT_SHIPYARD)
 * @return Number of ships available
 */
static inline int GetAvailableShips(const char *systemName, int systemEconomy,
                                    const ShipType **availableShips, double *shipPrices)
{
    // Initialize ship registry if needed
    InitializeShipRegistry();

    int shipCount = 0;

    // Iterate through all registered ship types
    for (int i = 0; i < shipRegistry.registeredShipCount && shipCount < MAX_SHIPS_AT_SHIPYARD; i++)
    {
        const ShipType *shipType = &shipRegistry.shipTypes[i];

        // Check if ship is available in this system
        if (IsShipAvailableInSystem(shipType->className, systemEconomy))
        {
            // Add to the available ships list
            availableShips[shipCount] = shipType;

            // Calculate price with system-specific multiplier
            double multiplier = GetShipPriceMultiplier(shipType->className, systemEconomy);
            shipPrices[shipCount] = shipType->baseCost * multiplier;

            shipCount++;
        }
    }

    // Log availability for debugging (uses systemName parameter)
    if (shipCount == 0 && systemName != NULL)
    {
        printf("Debug: No ships available at %s shipyard (economy type: %d)\n", systemName, systemEconomy);
    }

    return shipCount;
}

/**
 * Calculates the trade-in value of the player's current ship
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param gameTime Current game time in seconds
 * @return Trade-in value in credits
 */
static inline double CalculateTradeInValue(const PlayerShip *playerShip, uint64_t gameTime)
{
    if (playerShip == NULL || playerShip->shipType == NULL)
    {
        return 0.0;
    }

    // Start with base cost of the ship
    double baseValue = playerShip->shipType->baseCost;

    // Apply condition-based depreciation
    int hullPercentage = (playerShip->attributes.hullStrength * 100) /
                         playerShip->shipType->baseHullStrength;
    double conditionFactor = (double)hullPercentage / 100.0;

    // Apply time-based depreciation (for future expansion)
    // Currently we don't track the ship purchase time, so this is simplified
    double gameMonths = gameTime / (30.0 * 24.0 * 60.0 * 60.0); // Rough estimate of game months
    double timeFactor = 1.0 - (gameMonths * SHIP_DEPRECIATION_RATE);
    if (timeFactor < (MIN_TRADE_IN_VALUE_PERCENT / 100.0))
    {
        timeFactor = (MIN_TRADE_IN_VALUE_PERCENT / 100.0);
    }

    // Calculate value of installed equipment (not including standard equipment)
    double equipmentValue = 0.0;
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++)
    {
        if (playerShip->equipment[i].isActive)
        {
            // For simplicity, we're assuming a fixed value per equipment item
            // This could be expanded to actual equipment costs
            equipmentValue += 200.0; // Arbitrary value per equipment item
        }
    }

    // Calculate trade-in value
    double tradeInValue = (baseValue * conditionFactor * timeFactor) + (equipmentValue * 0.75);

    return tradeInValue;
}

/**
 * Displays a list of ships available for purchase
 *
 * @param systemName Name of the current star system
 * @param systemEconomy Economy of the current system
 * @param playerShip Pointer to the PlayerShip structure
 * @param gameTime Current game time in seconds
 */
static inline void DisplayShipyard(const char *systemName, int systemEconomy,
                                   const PlayerShip *playerShip, uint64_t gameTime)
{
    // Get list of available ships
    const ShipType *availableShips[MAX_SHIPS_AT_SHIPYARD];
    double shipPrices[MAX_SHIPS_AT_SHIPYARD];
    int shipCount = GetAvailableShips(systemName, systemEconomy, availableShips, shipPrices);

    // Calculate trade-in value of current ship
    double tradeInValue = CalculateTradeInValue(playerShip, gameTime);

    // Display shipyard header
    printf("\n=== %s Shipyard ===\n", systemName);
    printf("Your current ship: %s (%s)\n", playerShip->shipName, playerShip->shipClassName);
    printf("Trade-in value: %.1f CR\n\n", tradeInValue);
    // Display available ships
    printf("Available Ships:\n");
    printf("%-4s %-15s %-8s %-8s %-6s %-7s %-8s %-10s\n",
           "ID", "Ship Class", "Hull", "Energy", "Cargo", "Cost", "Net Cost", "Status");
    printf("%-4s %-15s %-8s %-8s %-6s %-7s %-8s %-10s\n",
           "--", "----------", "----", "------", "-----", "----", "--------", "------");

    for (int i = 0; i < shipCount; i++)
    {
        const ShipType *ship = availableShips[i];
        double price = shipPrices[i];
        double netCost = price - tradeInValue;

        // Determine if the player can afford this ship        extern int32_t Cash; // Access to player's cash from elite_state.h
        // Cash is stored internally as a value 10x the displayed value
        bool canAfford = (netCost * 10.0 <= Cash);

        printf("[%d] %-15s %-8d %-8.1f %-6d %-7.1f %-8.1f %s\n",
               i + 1, // Use 1-based indexing for user-friendliness
               ship->className,
               ship->baseHullStrength,
               ship->baseEnergyBanks,
               ship->baseCargoCapacityTons,
               price,
               netCost,
               canAfford ? "AVAILABLE" : "TOO EXPENSIVE");
    }

    printf("\nUse 'buyship <ID>' to purchase a new ship (e.g., 'buyship 1').\n");
    printf("Use 'buyship <ID> notrade' to buy without trading in your current ship.\n");
    printf("Use 'compareship <shipname>' to compare with your current ship.\n");
}

/**
 * Compares the player's current ship with a ship available for purchase
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param compareShipName Name of the ship to compare with
 */
static inline void CompareShips(const PlayerShip *playerShip, const char *compareShipName)
{
    if (playerShip == NULL || compareShipName == NULL)
    {
        printf("Error: Invalid ship data.\n");
        return;
    }

    // Find the ship type to compare with
    InitializeShipRegistry();
    const ShipType *compareShip = GetShipTypeByName(compareShipName);

    if (compareShip == NULL)
    {
        printf("Error: Ship '%s' not found.\n", compareShipName);
        return;
    }

    // Display comparison
    printf("\n=== Ship Comparison: %s vs %s ===\n",
           playerShip->shipClassName, compareShip->className);

    printf("%-20s %-15s %-15s %-15s\n", "Specification",
           playerShip->shipClassName, compareShip->className, "Difference");
    printf("%-20s %-15s %-15s %-15s\n", "-------------",
           "---------------", "---------------", "----------");

    // Compare hull strength
    printf("%-20s %-15d %-15d %+d\n", "Hull Strength",
           playerShip->shipType->baseHullStrength,
           compareShip->baseHullStrength,
           compareShip->baseHullStrength - playerShip->shipType->baseHullStrength);

    // Compare energy banks
    printf("%-20s %-15.1f %-15.1f %+.1f\n", "Energy Banks",
           playerShip->shipType->baseEnergyBanks,
           compareShip->baseEnergyBanks,
           compareShip->baseEnergyBanks - playerShip->shipType->baseEnergyBanks);

    // Compare shield strength
    printf("%-20s %-15.1f %-15.1f %+.1f\n", "Shield (Front)",
           playerShip->shipType->baseShieldStrengthFront,
           compareShip->baseShieldStrengthFront,
           compareShip->baseShieldStrengthFront - playerShip->shipType->baseShieldStrengthFront);
    printf("%-20s %-15.1f %-15.1f %+.1f\n", "Shield (Aft)",
           playerShip->shipType->baseShieldStrengthAft,
           compareShip->baseShieldStrengthAft,
           compareShip->baseShieldStrengthAft - playerShip->shipType->baseShieldStrengthAft);

    // Compare fuel capacity
    printf("%-20s %-15.1f %-15.1f %+.1f\n", "Fuel Capacity (LY)",
           playerShip->shipType->maxFuelLY,
           compareShip->maxFuelLY,
           compareShip->maxFuelLY - playerShip->shipType->maxFuelLY);

    // Compare cargo capacity
    printf("%-20s %-15d %-15d %+d\n", "Cargo Capacity (T)",
           playerShip->shipType->baseCargoCapacityTons,
           compareShip->baseCargoCapacityTons,
           compareShip->baseCargoCapacityTons - playerShip->shipType->baseCargoCapacityTons);

    // Compare missile pylons
    printf("%-20s %-15d %-15d %+d\n", "Missile Pylons",
           playerShip->shipType->initialMissilePylons,
           compareShip->initialMissilePylons,
           compareShip->initialMissilePylons - playerShip->shipType->initialMissilePylons);

    // Compare speed
    printf("%-20s %-15d %-15d %+d\n", "Speed",
           playerShip->shipType->baseSpeed,
           compareShip->baseSpeed,
           compareShip->baseSpeed - playerShip->shipType->baseSpeed);

    // Compare maneuverability
    printf("%-20s %-15d %-15d %+d\n", "Maneuverability",
           playerShip->shipType->baseManeuverability,
           compareShip->baseManeuverability,
           compareShip->baseManeuverability - playerShip->shipType->baseManeuverability);

    // Compare equipment slots
    printf("%-20s %-15d %-15d %+d\n", "Weapon Slots",
           playerShip->shipType->defaultWeaponSlots,
           compareShip->defaultWeaponSlots,
           compareShip->defaultWeaponSlots - playerShip->shipType->defaultWeaponSlots);
    printf("%-20s %-15d %-15d %+d\n", "Defensive Slots",
           playerShip->shipType->defaultDefensiveSlots,
           compareShip->defaultDefensiveSlots,
           compareShip->defaultDefensiveSlots - playerShip->shipType->defaultDefensiveSlots);
    printf("%-20s %-15d %-15d %+d\n", "Utility Slots",
           playerShip->shipType->defaultUtilitySlots,
           compareShip->defaultUtilitySlots,
           compareShip->defaultUtilitySlots - playerShip->shipType->defaultUtilitySlots); // Compare cost
    printf("%-20s %-15.1f %-15.1f %+.1f\n", "Base Cost (CR)",
           playerShip->shipType->baseCost, compareShip->baseCost,
           compareShip->baseCost - playerShip->shipType->baseCost);
}

/**
 * Transfer equipment from one ship to another
 *
 * @param sourceShip Source ship to transfer from
 * @param targetShip Target ship to transfer to
 * @return Number of equipment items transferred
 */
static inline int TransferEquipment(PlayerShip *sourceShip, PlayerShip *targetShip)
{
    if (sourceShip == NULL || targetShip == NULL)
    {
        return 0;
    }

    int transferCount = 0;

    // Check each equipment slot in the source ship
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++)
    {
        if (sourceShip->equipment[i].isActive)
        {
            // Check if the target ship has this slot type
            EquipmentSlotType slotType = sourceShip->equipment[i].slotType;

            // Skip standard pulse laser if the target ship already includes one
            if (slotType == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON &&
                sourceShip->equipment[i].typeSpecific.weaponType == WEAPON_TYPE_PULSE_LASER &&
                targetShip->shipType->includesPulseLaser)
            {
                continue;
            }

            // Check if slot is valid for the target ship
            bool validSlot = false;
            switch (slotType)
            {
            case EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON:
            case EQUIPMENT_SLOT_TYPE_AFT_WEAPON:
                validSlot = (i < targetShip->shipType->defaultWeaponSlots);
                break;

            case EQUIPMENT_SLOT_TYPE_DEFENSIVE_1:
            case EQUIPMENT_SLOT_TYPE_DEFENSIVE_2:
                validSlot = (i < targetShip->shipType->defaultDefensiveSlots);
                break;

            case UTILITY_SYSTEM_1:
            case UTILITY_SYSTEM_2:
            case UTILITY_SYSTEM_3:
            case UTILITY_SYSTEM_4:
                validSlot = (i < targetShip->shipType->defaultUtilitySlots);
                break;

            default:
                validSlot = false;
                break;
            }

            if (validSlot && !targetShip->equipment[i].isActive)
            {
                // Copy equipment from source to target
                targetShip->equipment[i] = sourceShip->equipment[i];
                transferCount++;

                // Clear the slot in the source ship
                sourceShip->equipment[i].isActive = 0;
                snprintf(sourceShip->equipment[i].name, MAX_SHIP_NAME_LENGTH, "Empty");
            }
            else
            {
                // Store in inventory if slot not available
                bool stored = false;
                for (int j = 0; j < MAX_EQUIPMENT_INVENTORY; j++)
                {
                    if (!targetShip->equipmentInventory[j].isActive)
                    {
                        targetShip->equipmentInventory[j] = sourceShip->equipment[i];
                        stored = true;
                        transferCount++;
                        break;
                    }
                }

                if (stored)
                {
                    // Clear the slot in the source ship
                    sourceShip->equipment[i].isActive = 0;
                    snprintf(sourceShip->equipment[i].name, MAX_SHIP_NAME_LENGTH, "Empty");
                }
                else
                {
                    printf("Warning: Could not transfer %s - no free slots or inventory space.\n",
                           sourceShip->equipment[i].name);
                }
            }
        }
    }

    // Transfer inventory items
    for (int i = 0; i < MAX_EQUIPMENT_INVENTORY; i++)
    {
        if (sourceShip->equipmentInventory[i].isActive)
        {
            bool stored = false;
            for (int j = 0; j < MAX_EQUIPMENT_INVENTORY; j++)
            {
                if (!targetShip->equipmentInventory[j].isActive)
                {
                    targetShip->equipmentInventory[j] = sourceShip->equipmentInventory[i];
                    stored = true;
                    transferCount++;
                    break;
                }
            }

            if (stored)
            {
                // Clear the slot in the source ship
                sourceShip->equipmentInventory[i].isActive = 0;
                snprintf(sourceShip->equipmentInventory[i].name, MAX_SHIP_NAME_LENGTH, "Empty");
            }
            else
            {
                printf("Warning: Could not transfer inventory item %s - no free inventory space.\n",
                       sourceShip->equipmentInventory[i].name);
            }
        }
    }

    return transferCount;
}

/**
 * Transfer cargo from one ship to another
 *
 * @param sourceShip Source ship to transfer from
 * @param targetShip Target ship to transfer to
 * @return Amount of cargo (in tons) that could not be transferred due to space limitations
 */
static inline int TransferCargo(PlayerShip *sourceShip, PlayerShip *targetShip)
{
    if (sourceShip == NULL || targetShip == NULL)
    {
        return 0;
    }

    int unableToTransfer = 0;

    // Check each cargo slot in the source ship
    for (int i = 0; i < MAX_CARGO_SLOTS; i++)
    {
        if (sourceShip->cargo[i].quantity > 0)
        {
            // Check if we have enough space in the target ship
            int availableSpace = targetShip->attributes.cargoCapacityTons - targetShip->attributes.currentCargoTons;

            if (availableSpace >= sourceShip->cargo[i].quantity)
            {
                // We have enough space to transfer all
                // Look for the same cargo type in the target ship
                int targetSlot = -1;
                for (int j = 0; j < MAX_CARGO_SLOTS; j++)
                {
                    if (targetShip->cargo[j].quantity > 0 &&
                        strcmp(targetShip->cargo[j].name, sourceShip->cargo[i].name) == 0)
                    {
                        targetSlot = j;
                        break;
                    }
                }

                // If not found, find an empty slot
                if (targetSlot == -1)
                {
                    for (int j = 0; j < MAX_CARGO_SLOTS; j++)
                    {
                        if (targetShip->cargo[j].quantity == 0)
                        {
                            targetSlot = j;
                            break;
                        }
                    }
                }

                // If we found a slot, transfer the cargo
                if (targetSlot != -1)
                {
                    // If it's an empty slot, copy the cargo details
                    if (targetShip->cargo[targetSlot].quantity == 0)
                    {
                        // Copy cargo details
                        targetShip->cargo[targetSlot].quantity = sourceShip->cargo[i].quantity;
                        targetShip->cargo[targetSlot].purchasePrice = sourceShip->cargo[i].purchasePrice;
                        snprintf(targetShip->cargo[targetSlot].name, MAX_SHIP_NAME_LENGTH, "%s", sourceShip->cargo[i].name);
                        targetShip->cargo[targetSlot].name[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
                    }

                    // Update quantities
                    targetShip->cargo[targetSlot].quantity += sourceShip->cargo[i].quantity;
                    targetShip->attributes.currentCargoTons += sourceShip->cargo[i].quantity;
                    sourceShip->attributes.currentCargoTons -= sourceShip->cargo[i].quantity;
                    sourceShip->cargo[i].quantity = 0;
                }
                else
                {
                    // No free slots in target ship, shouldn't happen with MAX_CARGO_SLOTS
                    unableToTransfer += sourceShip->cargo[i].quantity;
                    printf("Warning: Could not transfer %d tons of %s - no free cargo slots.\n",
                           sourceShip->cargo[i].quantity, sourceShip->cargo[i].name);
                }
            }
            else if (availableSpace > 0)
            {
                // We can transfer some but not all
                // Look for the same cargo type in the target ship
                int targetSlot = -1;
                for (int j = 0; j < MAX_CARGO_SLOTS; j++)
                {
                    if (targetShip->cargo[j].quantity > 0 &&
                        strcmp(targetShip->cargo[j].name, sourceShip->cargo[i].name) == 0)
                    {
                        targetSlot = j;
                        break;
                    }
                }

                // If not found, find an empty slot
                if (targetSlot == -1)
                {
                    for (int j = 0; j < MAX_CARGO_SLOTS; j++)
                    {
                        if (targetShip->cargo[j].quantity == 0)
                        {
                            targetSlot = j;
                            break;
                        }
                    }
                }

                // If we found a slot, transfer as much cargo as possible
                if (targetSlot != -1)
                {
                    // If it's an empty slot, copy the cargo details
                    if (targetShip->cargo[targetSlot].quantity == 0)
                    {
                        // Copy cargo details
                        targetShip->cargo[targetSlot].quantity = sourceShip->cargo[i].quantity;
                        targetShip->cargo[targetSlot].purchasePrice = sourceShip->cargo[i].purchasePrice;
                        snprintf(targetShip->cargo[targetSlot].name, MAX_SHIP_NAME_LENGTH, "%s", sourceShip->cargo[i].name);
                        targetShip->cargo[targetSlot].name[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
                    }

                    // Update quantities
                    int amountToTransfer = availableSpace;
                    targetShip->cargo[targetSlot].quantity += amountToTransfer;
                    targetShip->attributes.currentCargoTons += amountToTransfer;
                    sourceShip->attributes.currentCargoTons -= amountToTransfer;
                    sourceShip->cargo[i].quantity -= amountToTransfer;

                    unableToTransfer += sourceShip->cargo[i].quantity;
                    printf("Warning: Only transferred %d of %d tons of %s due to space limitations.\n",
                           amountToTransfer,
                           amountToTransfer + sourceShip->cargo[i].quantity,
                           sourceShip->cargo[i].name);
                }
                else
                {
                    // No free slots in target ship, shouldn't happen with MAX_CARGO_SLOTS
                    unableToTransfer += sourceShip->cargo[i].quantity;
                    printf("Warning: Could not transfer %d tons of %s - no free cargo slots.\n",
                           sourceShip->cargo[i].quantity, sourceShip->cargo[i].name);
                }
            }
            else
            {
                // No space at all in target ship
                unableToTransfer += sourceShip->cargo[i].quantity;
                printf("Warning: Could not transfer %d tons of %s - no cargo space available.\n",
                       sourceShip->cargo[i].quantity, sourceShip->cargo[i].name);
            }
        }
    }

    return unableToTransfer;
}

/**
 * Buy a new ship, optionally trading in the current ship
 *
 * @param systemName Name of the current star system
 * @param systemEconomy Economy of the current system
 * @param playerShip Pointer to the PlayerShip structure
 * @param newShipName Name of the ship to buy
 * @param gameTime Current game time in seconds
 * @param tradeIn Whether to trade in the current ship
 * @return true if purchase successful, false otherwise
 */
static inline bool BuyNewShip(const char *systemName, int systemEconomy,
                              PlayerShip *playerShip, const char *newShipName, uint64_t gameTime, bool tradeIn)
{
    (void)systemName; // Unused parameter
    if (playerShip == NULL || newShipName == NULL)
    {
        printf("Error: Invalid ship data.\n");
        return false;
    }

    // Access to player's cash from elite_state.h
    extern int32_t Cash;

    // Find the new ship type
    InitializeShipRegistry();
    const ShipType *newShipType = GetShipTypeByName(newShipName);    if (newShipType == NULL)
    {
        printf("Error: Ship '%s' not found.\n", newShipName);
        return false;
    }    // Check if the ship is available in this system
    if (!IsShipAvailableInSystem(newShipType->className, systemEconomy))
    {
        printf("Error: Ship '%s' is not available in this star system.\n", newShipType->className);
        return false;
    }

    // Calculate the price with system-specific multiplier
    double multiplier = GetShipPriceMultiplier(newShipType->className, systemEconomy);
    double price = newShipType->baseCost * multiplier;

    // Calculate trade-in value of current ship if trading in
    double tradeInValue = 0.0;
    if (tradeIn)
    {
        tradeInValue = CalculateTradeInValue(playerShip, gameTime);
    }    // Calculate net cost
    double netCost = price - tradeInValue;
    // Check if player can afford the ship
    // Cash is stored internally as a value 10x the displayed value
    if (netCost * 10.0 > Cash)
    {
        printf("Error: Insufficient funds to purchase %s.\n", newShipType->className);
        printf("Ship price: %.1f CR, Trade-in value: %.1f CR, Net cost: %.1f CR, Your cash: %.1f CR\n",
               price, tradeInValue, netCost, (double)Cash / 10.0);
        return false;
    }

    // If trading in, store the current ship temporarily
    if (tradeIn)
    {
        // Store the current ship
        tradeInStorage.ship = *playerShip;
        tradeInStorage.isActive = true;
    }

    // Create the new ship
    const char *customName = NULL; // Use default name

    // Reset player ship and initialize with the new ship type
    memset(playerShip, 0, sizeof(PlayerShip));
    if (!InitializeShip(playerShip, newShipType, customName))
    {
        printf("Error: Failed to initialize new ship.\n");

        if (tradeIn)
        {
            // Restore the old ship
            *playerShip = tradeInStorage.ship;
            tradeInStorage.isActive = false;
        }

        return false;
    }
    // Transfer equipment if trading in
    int equipmentTransferred = 0;
    int cargoLost = 0;
    if (tradeIn)
    {
        equipmentTransferred = TransferEquipment(&tradeInStorage.ship, playerShip);
        cargoLost = TransferCargo(&tradeInStorage.ship, playerShip);

        // Clear the trade-in storage
        tradeInStorage.isActive = false;
    }
    // Deduct the cost from player's cash
    Cash -= (int32_t)netCost;

    // Display purchase information
    printf("\nCongratulations on your new ship purchase!\n");
    printf("You are now the proud owner of a %s.\n", newShipType->className);
    printf("Purchase price: %.1f CR\n", price);

    if (tradeIn)
    {
        printf("Trade-in value: %.1f CR\n", tradeInValue);
        printf("Equipment transferred: %d items\n", equipmentTransferred);

        if (cargoLost > 0)
        {
            printf("Warning: %d tons of cargo could not be transferred due to space limitations.\n", cargoLost);
        }
    }

    printf("Net cost: %.1f CR\n", netCost);
    printf("Remaining cash: %.1f CR\n", (double)Cash / 10.0);

    return true;
}

// Command handler functions to be included in elite_commands.h

/**
 * Command handler for the 'shipyard' command
 * Shows ships available for purchase at the current station
 *
 * @param arguments Command arguments (unused)
 * @return true if command handled successfully
 */
static inline bool ShipyardCommand(const char *arguments)
{
    (void)arguments; // Unused parameter
    // Check if player is docked at a station
    extern struct NavigationState PlayerNavState;
    extern int PlayerLocationType;

    // Need to be both at a station AND docked
    if (PlayerNavState.currentLocationType != CELESTIAL_STATION || PlayerLocationType != 10)
    {
        printf("Error: You must be docked at a station to access the shipyard.\n");
        return false;
    }

    // Get current system info
    extern char CurrentSystemName[20];      // From elite_player_state.h
    extern int CurrentSystemEconomy;        // From elite_player_state.h
    extern uint64_t currentGameTimeSeconds; // From elite_state.h
    extern PlayerShip *PlayerShipPtr;       // From elite_player_state.h

    // Display the shipyard
    DisplayShipyard(CurrentSystemName, CurrentSystemEconomy, PlayerShipPtr, currentGameTimeSeconds);

    return true;
}

/**
 * Command handler for the 'compare' command
 * Compares the player's current ship with a ship available for purchase
 *
 * @param arguments Name of the ship to compare with
 * @return true if command handled successfully
 */
static inline bool CompareShipCommand(const char *arguments)
{
    // Check if arguments are provided
    if (arguments == NULL || arguments[0] == '\0')
    {
        printf("Error: Please specify a ship to compare with.\n");
        printf("Usage: compare <shipname>\n");
        return false;
    }

    // Get player ship
    extern PlayerShip *PlayerShipPtr; // From elite_player_state.h

    // Compare ships
    CompareShips(PlayerShipPtr, arguments);

    return true;
}

/**
 * Gets the name of a ship based on its shipyard ID
 *
 * @param systemName Name of the current star system
 * @param systemEconomy Economy of the current system
 * @param shipID ID of the ship (1-based index as shown in the shipyard)
 * @param shipName Buffer to store the ship name (must be pre-allocated)
 * @param shipNameSize Size of the shipName buffer
 * @return true if the ship was found, false otherwise
 */
static inline bool GetShipNameByID(const char *systemName, int systemEconomy, int shipID,
                                   char *shipName, size_t shipNameSize)
{
    if (shipID < 1 || shipName == NULL || shipNameSize < 1)
    {
        return false;
    }

    // Get list of available ships
    const ShipType *availableShips[MAX_SHIPS_AT_SHIPYARD];
    double shipPrices[MAX_SHIPS_AT_SHIPYARD];
    int shipCount = GetAvailableShips(systemName, systemEconomy, availableShips, shipPrices);

    // Adjust shipID to 0-based index
    int shipIndex = shipID - 1;

    // Check if the shipID is valid
    if (shipIndex < 0 || shipIndex >= shipCount)
    {
        return false;
    }

    // Copy the ship name to the buffer
    snprintf(shipName, shipNameSize, "%s", availableShips[shipIndex]->className);
    shipName[shipNameSize - 1] = '\0'; // Ensure null termination

    return true;
}

/**
 * Command handler for the 'buyship' command
 * Buys a new ship, optionally trading in the current ship
 *
 * @param arguments Ship ID or name to buy, with optional 'notrade' flag
 * @return true if command handled successfully
 */
static inline bool BuyShipCommand(const char *arguments)
{
    // Check if player is docked at a station
    extern struct NavigationState PlayerNavState;
    extern int PlayerLocationType;

    // Need to be both at a station AND docked
    if (PlayerNavState.currentLocationType != CELESTIAL_STATION || PlayerLocationType != 10)
    {
        printf("Error: You must be docked at a station to purchase a ship.\n");
        return false;
    }

    // Check if arguments are provided
    if (arguments == NULL || arguments[0] == '\0')
    {
        printf("Error: Please specify a ship to buy.\n");
        printf("Usage: buyship <ID or shipname> [notrade]\n");
        printf("Example: buyship 1  or  buyship \"Cobra Mk III\"\n");
        return false;
    }

    // Get current system info and player ship
    extern char CurrentSystemName[20];      // From elite_player_state.h
    extern int CurrentSystemEconomy;        // From elite_player_state.h
    extern uint64_t currentGameTimeSeconds; // From elite_state.h
    extern PlayerShip *PlayerShipPtr;       // From elite_player_state.h

    // Parse arguments
    char shipNameOrID[64] = {0};
    bool tradeIn = true;

    // Copy the first part of the arguments (up to the first space)
    const char *space = strchr(arguments, ' ');
    if (space != NULL)
    {
        size_t nameLen = space - arguments;
        snprintf(shipNameOrID, sizeof(shipNameOrID), "%.*s", (int)nameLen, arguments);
        shipNameOrID[nameLen < 63 ? nameLen : 63] = '\0';

        // Check for 'notrade' flag in the remaining part
        if (strstr(space + 1, "notrade") != NULL)
        {
            tradeIn = false;
        }
    }
    else
    {
        // No space, just copy the entire argument
        snprintf(shipNameOrID, sizeof(shipNameOrID), "%s", arguments);
        // shipNameOrID[63] = \'\\0\'; // snprintf handles null termination
    }

    // Check if the argument is a number (ID) or a string (ship name)
    char actualShipName[MAX_SHIP_NAME_LENGTH] = {0};
    bool isID = true;

    // Check if shipNameOrID is a number
    for (size_t i = 0; i < strlen(shipNameOrID); i++)
    {
        if (!isdigit(shipNameOrID[i]))
        {
            isID = false;
            break;
        }
    }

    if (isID)
    {
        // Convert the ID to an integer
        int shipID = atoi(shipNameOrID);

        // Get the ship name by ID
        if (!GetShipNameByID(CurrentSystemName, CurrentSystemEconomy, shipID, actualShipName, MAX_SHIP_NAME_LENGTH))
        {
            printf("Error: Invalid ship ID: %d\n", shipID);
            return false;
        }
    }
    else
    {
        // The argument is a ship name, just copy it
        snprintf(actualShipName, sizeof(actualShipName), "%.63s", shipNameOrID);
        actualShipName[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null-termination
    }

    // Buy the new ship
    return BuyNewShip(
        CurrentSystemName,
        CurrentSystemEconomy,
        PlayerShipPtr,
        actualShipName,
        currentGameTimeSeconds,
        tradeIn);
}
