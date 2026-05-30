#pragma once

#include "elite_ship_registry.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct PlayerShip
{
    char shipName[MAX_SHIP_NAME_LENGTH];
    char shipClassName[MAX_SHIP_NAME_LENGTH]; // e.g., "Cobra Mk III"
    const ShipType *shipType;                 // Pointer to the ship type definition
    ShipCoreAttributes attributes;
    ShipEquipmentItem equipment[MAX_EQUIPMENT_SLOTS];              // Currently equipped items
    ShipEquipmentItem equipmentInventory[MAX_EQUIPMENT_INVENTORY]; // Inventory of stored equipment
    CargoItem cargo[MAX_CARGO_SLOTS];
} PlayerShip;

// --- Forward declarations for circular dependencies ---
// These functions might be defined in elite_ship_upgrades.h or elite_ship_inventory.h
static inline bool RemoveEquipmentToInventory(PlayerShip *playerShip, EquipmentSlotType slotType);

// --- Ship Operations ---

/**
 * Initializes a PlayerShip with the given ship type
 *
 * @return 1 if successful, 0 otherwise
 */
static inline bool InitializeShip(PlayerShip *playerShip, const ShipType *shipType, const char *customName)
{
    if (playerShip == NULL || shipType == NULL)
    {
        return 0;
    }

    // Set ship name (custom or default)
    if (customName != NULL && customName[0] != '\0')
    {
        snprintf(playerShip->shipName, MAX_SHIP_NAME_LENGTH, "%s", customName);
        playerShip->shipName[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
    }
    else
    {
        // Construct default name if no custom name is provided
        char defaultName[MAX_SHIP_NAME_LENGTH];
        snprintf(defaultName, sizeof(defaultName), "%s Class", shipType->className);
        snprintf(playerShip->shipName, MAX_SHIP_NAME_LENGTH, "%s", defaultName);
        playerShip->shipName[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
    }
    snprintf(playerShip->shipClassName, MAX_SHIP_NAME_LENGTH, "%s", shipType->className);
    playerShip->shipClassName[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination

    // Set the ship type pointer
    playerShip->shipType = shipType;

    // Initialize core attributes based on ship type
    playerShip->attributes.hullStrength = shipType->baseHullStrength;
    playerShip->attributes.shieldStrengthFront = shipType->baseShieldStrengthFront;
    playerShip->attributes.shieldStrengthAft = shipType->baseShieldStrengthAft;
    playerShip->attributes.fuelLiters = shipType->maxFuelLY * 100.0; // Assuming 100 liters per LY
    playerShip->attributes.cargoCapacityTons = shipType->baseCargoCapacityTons;
    playerShip->attributes.currentCargoTons = 0;
    playerShip->attributes.missilePylons = shipType->initialMissilePylons;
    playerShip->attributes.missilesLoadedHoming = 0;
    playerShip->attributes.missilesLoadedDumbfire = 0;

    // Initialize equipment slots to Empty
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        playerShip->equipment[i].isActive = 0;
        snprintf(playerShip->equipment[i].name, MAX_SHIP_NAME_LENGTH, "Empty");
        playerShip->equipment[i].name[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
        playerShip->equipment[i].typeSpecific.utilityType = UTILITY_SYSTEM_TYPE_NONE; // Example default
    }

    // Initialize inventory slots to empty
    for (int i = 0; i < MAX_EQUIPMENT_INVENTORY; ++i)
    {
        playerShip->equipmentInventory[i].isActive = 0;
        snprintf(playerShip->equipmentInventory[i].name, MAX_SHIP_NAME_LENGTH, "Empty");
        playerShip->equipmentInventory[i].slotType = EQUIPMENT_SLOT_TYPE_NONE;
        playerShip->equipmentInventory[i].damageOutput = 0.0;
    }

    // Initialize cargo holds to Empty
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        snprintf(playerShip->cargo[i].name, MAX_SHIP_NAME_LENGTH, "Empty");
        playerShip->cargo[i].name[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
        playerShip->cargo[i].quantity = 0;
    }

    // Add pulse laser if the ship type includes one
    if (shipType->includesPulseLaser)
    {
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].isActive = 1;
        snprintf(playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].name, MAX_SHIP_NAME_LENGTH, "Pulse Laser");
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].name[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].slotType = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].typeSpecific.weaponType = WEAPON_TYPE_PULSE_LASER;
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].damageOutput = 5.0; // Example
    }

    return 1;
}

/**
 * Initialize the ship registry and then initialize a PlayerShip to Cobra Mk III default specifications.
 */
static inline void InitializeCobraMkIII(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return;
    }

    // Ensure the ship registry is initialized
    InitializeShipRegistry();

    // Get the Cobra Mk III ship type from the registry
    const ShipType *cobraMkIII = GetShipTypeByName("Cobra Mk III");
    if (cobraMkIII == NULL)
    {
        printf("Error: Could not find Cobra Mk III ship type in registry.\n");
        return;
    }

    InitializeShip(playerShip, cobraMkIII, NULL);
}

/**
 * Displays the current status of the player's ship.
 */
static inline void DisplayShipStatus(const PlayerShip *playerShip)
{
    bool ecmFound = 0; 
    bool escapePodFound = 0;
    bool fuelScoopsFound = 0;
    bool dockingComputerFound = 0;
    bool scannerUpgradeFound = 0;
    bool rearLaserFound = 0;
    bool forwardPulseLaserFound = 0;

    if (playerShip == NULL)
    {
        printf("Error: Ship data is NULL.\n");
        return;
    }

    printf("\n--- %s (%s) Status ---\n", playerShip->shipName, playerShip->shipClassName);
    printf("Hull Strength: %d / %d\n", playerShip->attributes.hullStrength, playerShip->shipType->baseHullStrength);
    printf("Shields (F/A): %.2f / %.2f\n", playerShip->attributes.shieldStrengthFront, playerShip->attributes.shieldStrengthAft);
    // Convert Liters to LY for display, assuming 1 LY = 100 Liters (example factor)
    printf("Fuel: %.2f LY (%.0f Liters)\n", playerShip->attributes.fuelLiters / 100.0, playerShip->attributes.fuelLiters);
    printf("Cargo: %dT / %dT\n", playerShip->attributes.currentCargoTons, playerShip->attributes.cargoCapacityTons);
    printf("Missile Pylons: %d (Homing: %d, Dumbfire: %d)\n",
           playerShip->attributes.missilePylons,
           playerShip->attributes.missilesLoadedHoming,
           playerShip->attributes.missilesLoadedDumbfire);

    int hasEquipment = 0;
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            strlen(playerShip->equipment[i].name) > 0 &&
            strcmp(playerShip->equipment[i].name, "Empty") != 0)
        {
            hasEquipment = 1;
            printf("- %s", playerShip->equipment[i].name);

            // Only print slot info if it's useful
            if (playerShip->equipment[i].slotType != EQUIPMENT_SLOT_TYPE_NONE)
            {
                printf(" (Slot: %d", playerShip->equipment[i].slotType);

                // Print the type info based on slot type
                if (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON ||
                    playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_AFT_WEAPON)
                {
                    printf(", Type: Weapon - %d", playerShip->equipment[i].typeSpecific.weaponType);
                }
                else if (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
                         playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2)
                {
                    printf(", Type: Defensive - %d", playerShip->equipment[i].typeSpecific.defensiveType);
                }
                else if (playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
                         playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4)
                {
                    printf(", Type: Utility - %d", playerShip->equipment[i].typeSpecific.utilityType);
                }
                printf(")");
            }
            printf("\n");
        }
    }

    if (!hasEquipment)
    {
        printf("No active equipment.\n");
    }

    printf("\n--- Key Systems & Upgrades ---\n");

    bool isCobraMkIII = (strcmp(playerShip->shipClassName, "Cobra Mk III") == 0);
    if (isCobraMkIII)
    {
        printf("- Basic Shields System\n"); 
    }

    // Display fuel-related information for all ships
    printf("- %s Hyperspace Drive (%.1f LY Max, %.1f CR per 0.1 LY)\n",
           playerShip->shipType->hasStandardHyperdrive ? "Standard" : "Enhanced",
           playerShip->shipType->maxFuelLY,
           playerShip->shipType->fuelConsumptionRate / 10.0);

    // Standard Cargo Bay is reflected in attributes.cargoCapacityTons
    printf("- Standard Cargo Bay (%dT)\n", playerShip->shipType->baseCargoCapacityTons);

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive)
        {
            // Forward Pulse Laser
            if (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON &&
                playerShip->equipment[i].typeSpecific.weaponType == WEAPON_TYPE_PULSE_LASER)
            {
                if (!forwardPulseLaserFound)
                {
                    if (isCobraMkIII)
                        printf("- Standard Forward Pulse Laser\n");
                    else
                        printf("- Forward Pulse Laser\n");
                    forwardPulseLaserFound = 1;
                }
            }
            // Rear-mounted Laser
            if (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_AFT_WEAPON &&
                ((playerShip->equipment[i].typeSpecific.weaponType >= WEAPON_TYPE_PULSE_LASER &&
                  playerShip->equipment[i].typeSpecific.weaponType <= WEAPON_TYPE_MINING_LASER) ||
                 playerShip->equipment[i].typeSpecific.weaponType == WEAPON_TYPE_REAR_LASER))
            {
                if (!rearLaserFound)
                {
                    printf("- Rear-mounted Laser\n");
                    rearLaserFound = 1;
                }
            }
            // Defensive Systems
            if (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
                playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2)
            {
                if (playerShip->equipment[i].typeSpecific.defensiveType == DEFENSIVE_SYSTEM_TYPE_ECM && !ecmFound)
                {
                    printf("- ECM Unit\n");
                    ecmFound = 1;
                }
            }

            // Utility Systems
            if (playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
                playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4)
            {
                switch (playerShip->equipment[i].typeSpecific.utilityType)
                {
                case UTILITY_SYSTEM_TYPE_ESCAPE_POD:
                    if (!escapePodFound)
                    {
                        printf("- Escape Pod\n");
                        escapePodFound = 1;
                    }
                    break;
                case UTILITY_SYSTEM_TYPE_FUEL_SCOOPS:
                    if (!fuelScoopsFound)
                    {
                        printf("- Fuel Scoops\n");
                        fuelScoopsFound = 1;
                    }
                    break;
                case UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER:
                    if (!dockingComputerFound)
                    {
                        printf("- Docking Computer\n");
                        dockingComputerFound = 1;
                    }
                    break;
                case UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE:
                    if (!scannerUpgradeFound)
                    {
                        printf("- Scanner Upgrade\n");
                        scannerUpgradeFound = 1;
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }

    printf("\n--- Cargo Hold (%dT used / %dT capacity) ---\n",
           playerShip->attributes.currentCargoTons,
           playerShip->attributes.cargoCapacityTons);
    int hasCargo = 0;
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity > 0)
        {
            hasCargo = 1;
            printf("- %s: %d units (Bought at: %dcr each)\n",
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
    printf("\nEquipment inventory commands: 'inv', 'store <slot>', 'use <inv_idx> <slot>'\n");
}

// Include ship upgrade functionality (maintaining original inclusion sequence)
#include "elite_ship_upgrades.h"

/**
 * Checks if the ship has fuel scoops installed
 */
static inline bool HasFuelScoops(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_FUEL_SCOOPS)
        {
            return 1;
        }
    }

    return 0;
}

/**
 * Refuels the player's ship.
 */
static inline float RefuelShip(PlayerShip *playerShip, float fuelAmountLY, bool useFuelScoops, bool externalSync)
{
    if (playerShip == NULL)
    {
        return 0.0f;
    }

    const float maxFuelLY = playerShip->shipType->maxFuelLY;
    float currentFuelLY = playerShip->attributes.fuelLiters / 100.0f; 

    if (currentFuelLY >= maxFuelLY)
    {
        printf("Fuel tanks already full (%.1f LY).\n", maxFuelLY);
        return 0.0f;
    }

    float availableSpace = maxFuelLY - currentFuelLY;
    float effectiveRequestLY = (fuelAmountLY > availableSpace) ? availableSpace : fuelAmountLY;

    if (useFuelScoops)
    {
        if (!HasFuelScoops(playerShip))
        {
            printf("Error: Your ship is not equipped with fuel scoops.\n");
            return 0.0f;
        }

        playerShip->attributes.fuelLiters += (effectiveRequestLY * 100.0f);
        printf("Successfully scooped %.1f LY of fuel from the star.\n", effectiveRequestLY);

        if (externalSync)
        {
            int currentMaxFuel = GetMaxFuel();
            uint16_t fuelToAdd = (uint16_t)(effectiveRequestLY * 10.0f);

            if (g_state.Fuel + fuelToAdd > (uint16_t)currentMaxFuel)
            {
                g_state.Fuel = (uint16_t)currentMaxFuel;
            }
            else
            {
                g_state.Fuel += fuelToAdd;
            }
        }


        return effectiveRequestLY;
    }
    else
    {
        int currentFuelCost = GetFuelCost();
        uint16_t fuelUnits = (uint16_t)(effectiveRequestLY * 10.0f);
        int totalCost = fuelUnits * currentFuelCost;

        if (externalSync && totalCost > g_state.Cash)
        {
            uint16_t affordableUnits = (uint16_t)(g_state.Cash / currentFuelCost);
            fuelUnits = affordableUnits;
            totalCost = fuelUnits * currentFuelCost;
            effectiveRequestLY = (float)affordableUnits / 10.0f;

            if (fuelUnits == 0)
            {
                printf("Insufficient credits to purchase fuel.\n");
                return 0.0f;
            }
        }

        if (externalSync)
        {
            g_state.Cash -= totalCost;
        }

        playerShip->attributes.fuelLiters += (effectiveRequestLY * 100.0f);
        
        if (externalSync)
        {
            int currentMaxFuel = GetMaxFuel();
            if (g_state.Fuel + fuelUnits > (uint16_t)currentMaxFuel)
            {
                g_state.Fuel = (uint16_t)currentMaxFuel;
            }
            else
            {
                g_state.Fuel += fuelUnits;
            }
        }


        printf("Purchased %.1f LY of fuel for %d credits.\n", effectiveRequestLY, totalCost);
        return effectiveRequestLY;
    }
}

/**
 * Activates ECM to destroy incoming enemy missiles.
 */
static inline bool ActivateECM(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    bool hasECM = 0;
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
             playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2) &&
            playerShip->equipment[i].typeSpecific.defensiveType == DEFENSIVE_SYSTEM_TYPE_ECM)
        {
            hasECM = 1;
            break;
        }
    }

    if (!hasECM)
    {
        printf("Error: Your ship is not equipped with ECM System.\n");
        return 0;
    }

    printf("ECM System activated! All incoming missiles have been destroyed.\n");
    return 1;
}

/**
 * Activates the docking computer.
 */
static inline bool ActivateDockingComputer(PlayerShip *playerShip, double distance)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    bool hasDockingComputer = 0;
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER)
        {
            hasDockingComputer = 1;
            break;
        }
    }

    if (!hasDockingComputer)
    {
        printf("Error: Your ship is not equipped with a Docking Computer.\n");
        return 0;
    }

    int dockingTimeSeconds = (int)(distance * 5.0);
    printf("Docking Computer activated. Auto-docking sequence initiated.\n");
    printf("Estimated time to complete docking: %d seconds.\n", dockingTimeSeconds);

    return 1;
}

/**
 * Uses the ship's scanner.
 */
static inline bool UseScanner(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    bool hasUpgradedScanner = 0;
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE)
        {
            hasUpgradedScanner = 1;
            break;
        }
    }

    if (hasUpgradedScanner)
    {
        printf("Advanced scanner activated. Extended range and detailed scan initiated.\n");
    }
    else
    {
        printf("Basic scanner activated. Standard scan initiated.\n");
    }

    return 1;
}

/**
 * Attempts to deploy the escape pod.
 */
static inline bool DeployEscapePod(PlayerShip *playerShip, bool criticalDamage)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    bool hasEscapePod = 0;
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_ESCAPE_POD)
        {
            hasEscapePod = 1;
            break;
        }
    }

    if (!hasEscapePod)
    {
        printf("Error: Your ship is not equipped with an Escape Pod.\n");
        return 0;
    }

    if (!criticalDamage)
    {
        printf("Escape pod can only be deployed in case of critical ship damage.\n");
        return 0;
    }

    printf("EMERGENCY: Escape pod deployed! You have been safely ejected from your ship.\n");
    printf("Your ship and cargo have been lost, but you have survived.\n");

    return 1;
}

/**
 * Gets the damage output of a specific weapon.
 */
static inline double GetWeaponDamage(const PlayerShip *playerShip, EquipmentSlotType slotType)
{
    if (playerShip == NULL ||
        (slotType != EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON && slotType != EQUIPMENT_SLOT_TYPE_AFT_WEAPON))
    {
        return 0.0;
    }

    if (playerShip->equipment[slotType].isActive)
    {
        return playerShip->equipment[slotType].damageOutput;
    }

    return 0.0;
}

/**
 * Checks if the ship has a specific type of equipment installed.
 */
static inline bool HasEquipment(const PlayerShip *playerShip, EquipmentSlotType slotType, EquipmentTypeSpecifics specificType)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (!playerShip->equipment[i].isActive)
        {
            continue;
        }

        if (playerShip->equipment[i].slotType == slotType)
        {
            switch (slotType)
            {
            case EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON:
            case EQUIPMENT_SLOT_TYPE_AFT_WEAPON:
                if (playerShip->equipment[i].typeSpecific.weaponType == specificType.weaponType)
                {
                    return 1;
                }
                break;

            case EQUIPMENT_SLOT_TYPE_DEFENSIVE_1:
            case EQUIPMENT_SLOT_TYPE_DEFENSIVE_2:
                if (playerShip->equipment[i].typeSpecific.defensiveType == specificType.defensiveType)
                {
                    return 1;
                }
                break;

            case UTILITY_SYSTEM_1:
            case UTILITY_SYSTEM_2:
            case UTILITY_SYSTEM_3:
            case UTILITY_SYSTEM_4:
                if (playerShip->equipment[i].typeSpecific.utilityType == specificType.utilityType)
                {
                    return 1;
                }
                break;

            default:
                break;
            }
        }
    }

    return 0;
}

/**
 * Repairs the hull of the player's ship.
 */
static inline int RepairHull(PlayerShip *playerShip, int repairAmount, int costPerPoint, bool externalSync)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    if (playerShip->attributes.hullStrength >= playerShip->shipType->baseHullStrength)
    {
        printf("Hull already at maximum strength.\n");
        return 0;
    }

    int maxRepair = playerShip->shipType->baseHullStrength - playerShip->attributes.hullStrength;
    int effectiveRepair = (repairAmount > maxRepair) ? maxRepair : repairAmount;
    int totalCost = effectiveRepair * costPerPoint;

    if (externalSync)
    {
        if (totalCost > g_state.Cash)
        {
            effectiveRepair = (int)(g_state.Cash / costPerPoint);
            totalCost = effectiveRepair * costPerPoint;

            if (effectiveRepair == 0)
            {
                printf("Insufficient credits for hull repairs.\n");
                return 0;
            }
        }
        g_state.Cash -= totalCost;
    }

    playerShip->attributes.hullStrength += effectiveRepair;
    printf("Repaired %d hull strength points for %d credits.\n", effectiveRepair, totalCost);
    return effectiveRepair;
}

/**
 * Adds equipment to the player's ship.
 */
static inline bool AddEquipment(PlayerShip *playerShip,
                         EquipmentSlotType slotType,
                         const char *equipmentName,
                         EquipmentTypeSpecifics specificType,
                         double damageOutput)
{
    if (playerShip == NULL || equipmentName == NULL)
    {
        return 0;
    }

    if (slotType < 0 || slotType >= MAX_EQUIPMENT_SLOTS)
    {
        printf("Error: Invalid equipment slot type %d.\n", slotType);
        return 0;
    }

    if (playerShip->equipment[slotType].isActive)
    {
        char oldEquipName[MAX_SHIP_NAME_LENGTH];
        snprintf(oldEquipName, MAX_SHIP_NAME_LENGTH, "%s", playerShip->equipment[slotType].name);
        oldEquipName[MAX_SHIP_NAME_LENGTH - 1] = '\0'; 

        if (RemoveEquipmentToInventory(playerShip, slotType))
        {
            // Successfully moved to inventory
        }
        else
        {
            printf("Warning: Replacing existing equipment '%s' in slot %d without storing it (inventory may be full).\n",
                   oldEquipName, slotType);
            playerShip->equipment[slotType].isActive = 0;
            snprintf(playerShip->equipment[slotType].name, MAX_SHIP_NAME_LENGTH, "Empty");
        }
    }

    playerShip->equipment[slotType].isActive = 1;
    snprintf(playerShip->equipment[slotType].name, MAX_SHIP_NAME_LENGTH, "%s", equipmentName);
    playerShip->equipment[slotType].name[MAX_SHIP_NAME_LENGTH - 1] = '\0';
    playerShip->equipment[slotType].typeSpecific = specificType;
    playerShip->equipment[slotType].damageOutput = damageOutput;

    printf("%s added to slot %d.\n", equipmentName, slotType);
    return 1;
}

/**
 * Removes equipment from the player's ship.
 */
static inline bool RemoveEquipment(PlayerShip *playerShip, EquipmentSlotType slotType)
{
    if (playerShip == NULL || slotType >= MAX_EQUIPMENT_SLOTS)
    {
        return 0;
    }

    if (!playerShip->equipment[slotType].isActive)
    {
        printf("Error: No equipment installed in slot %d.\n", slotType);
        return 0;
    }

    char equipmentName[MAX_SHIP_NAME_LENGTH];
    snprintf(equipmentName, MAX_SHIP_NAME_LENGTH, "%s", playerShip->equipment[slotType].name);
    equipmentName[MAX_SHIP_NAME_LENGTH - 1] = '\0';

    if (playerShip->equipment[slotType].slotType >= UTILITY_SYSTEM_1 &&
        playerShip->equipment[slotType].slotType <= UTILITY_SYSTEM_4)
    {
        if (playerShip->equipment[slotType].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION)
        {
            if (playerShip->attributes.cargoCapacityTons - 5 < playerShip->attributes.currentCargoTons)
            {
                printf("Error: Can't remove cargo bay extension while cargo hold contains more than %d tons.\n",
                       playerShip->attributes.cargoCapacityTons - 5);
                return 0;
            }
            playerShip->attributes.cargoCapacityTons -= 5;
        }
    }

    playerShip->equipment[slotType].isActive = 0;
    snprintf(playerShip->equipment[slotType].name, MAX_SHIP_NAME_LENGTH, "Empty");

    printf("Successfully removed %s from slot %d.\n", equipmentName, slotType);
    return 1;
}

/**
 * Finds cargo by name.
 */
static inline int FindCargoByName(const PlayerShip *playerShip, const char *cargoName)
{
    if (playerShip == NULL || cargoName == NULL)
    {
        return -1;
    }

    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity > 0 &&
            strcmp(playerShip->cargo[i].name, cargoName) == 0)
        {
            return i;
        }
    }

    return -1;
}

/**
 * Gets the available cargo space.
 */
static inline int GetAvailableCargoSpace(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    return playerShip->attributes.cargoCapacityTons - playerShip->attributes.currentCargoTons;
}

/**
 * Gets the total number of cargo items.
 */
static inline int GetCargoItemCount(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity > 0)
        {
            count++;
        }
    }

    return count;
}

/**
 * Gets the cargo item at a specific index.
 */
static inline bool GetCargoItemAtIndex(const PlayerShip *playerShip, int index,
                                char *outCargoName, int *outQuantity, int *outPurchasePrice)
{
    if (playerShip == NULL || outCargoName == NULL || outQuantity == NULL || outPurchasePrice == NULL)
    {
        return 0;
    }

    if (index < 0 || index >= MAX_CARGO_SLOTS)
    {
        return 0;
    }

    if (playerShip->cargo[index].quantity <= 0)
    {
        return 0;
    }

    snprintf(outCargoName, MAX_SHIP_NAME_LENGTH, "%s", playerShip->cargo[index].name);
    *outQuantity = playerShip->cargo[index].quantity;
    *outPurchasePrice = playerShip->cargo[index].purchasePrice;

    return 1;
}

/**
 * Displays detailed information about the cargo.
 */
static inline void DisplayCargoDetails(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        printf("Error: Ship data is NULL.\n");
        return;
    }

    printf("\n=== Cargo Hold (%d/%d tons) ===\n",
           playerShip->attributes.currentCargoTons,
           playerShip->attributes.cargoCapacityTons);

    if (playerShip->attributes.currentCargoTons == 0)
    {
        printf("Cargo hold is empty.\n");
        return;
    }

    printf("%-20s %-10s %-15s %-15s\n", "Commodity", "Quantity", "Purchase Price", "Total Value");
    printf("%-20s %-10s %-15s %-15s\n", "----------", "--------", "--------------", "-----------");

    int totalItems = 0;
    int totalValue = 0;

    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity > 0)
        {
            int itemTotalValue = playerShip->cargo[i].quantity * playerShip->cargo[i].purchasePrice;
            totalItems += playerShip->cargo[i].quantity;
            totalValue += itemTotalValue;

            printf("%-20s %-10d %-15d %-15d\n",
                   playerShip->cargo[i].name,
                   playerShip->cargo[i].quantity,
                   playerShip->cargo[i].purchasePrice,
                   itemTotalValue);
        }
    }

    printf("%-20s %-10s %-15s %-15s\n", "----------", "--------", "--------------", "-----------");
    printf("%-20s %-10d %-15s %-15d\n", "TOTAL", totalItems, "", totalValue);
    printf("\nAvailable space: %d tons\n", GetAvailableCargoSpace(playerShip));
}
