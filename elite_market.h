#pragma once

#include "elite_state.h"      // Unified header for constants, structures, and globals
#include "elite_utils.h"      // For minimum_value
#include "elite_ship_types.h" // For PlayerShip structure
#include <math.h>             // For floor (used in execute_buy_order)
#include <string.h>           // For string operations (strcpy, strncpy)

// Define constants for market modifiers if not already defined elsewhere
// These represent a 25% change.
#define PRICE_DECREASE_FACTOR 0.75f
#define PRICE_INCREASE_FACTOR 1.25f
#define QUANTITY_DECREASE_FACTOR 0.75f
#define QUANTITY_INCREASE_FACTOR 1.25f
#define NO_CHANGE_FACTOR 1.0f

// Enum for Station Specializations
typedef enum StationSpecialization {
    STATION_SPECIALIZATION_BALANCED = 0,
    STATION_SPECIALIZATION_INDUSTRIAL = 1,
    STATION_SPECIALIZATION_AGRICULTURAL = 2,
    STATION_SPECIALIZATION_MINING = 3,
    NUM_STATION_SPECIALIZATIONS // Keep this last for array sizing
} StationSpecialization;

// Enum for Planet Market Types (using "MarketType" to distinguish from general planet properties if any)
typedef enum PlanetMarketType {
    PLANET_MARKET_TYPE_ROCKY_AIRLESS = 0,
    PLANET_MARKET_TYPE_TERRESTRIAL = 1,
    PLANET_MARKET_TYPE_GAS_GIANT = 2,
    PLANET_MARKET_TYPE_ICE_GIANT_WORLD = 3,
    NUM_PLANET_MARKET_TYPES // Keep this last for array sizing
} PlanetMarketType;

// Structure to hold market modifiers for a single commodity
typedef struct MarketModifier {
    float priceFactor; // Added priceFactor
    float quantityFactor;
} MarketModifier;

// This macro was originally with the Commodities array definition
#define POLITICALLY_CORRECT 0
/* Set to 1 for NES-sanitised trade goods */

// UnitNames array, static within this header
static char UnitNames[][5] = {"t", "kg", "g"};

// Commodities array, static within this header
// Defines NUM_STANDARD_COMMODITIES (10) items
static TradeGood Commodities[] = {
    {0x13, -0x02, 0x06, 0x01, 0, "Food        "},
    {0x14, -0x01, 0x0A, 0x03, 0, "Textiles    "},
    {0x41, -0x03, 0x02, 0x07, 0, "Radioactives"},
#if POLITICALLY_CORRECT
    {0x28, -0x05, 0xE2, 0x1F, 0, "Robot Slaves"},
    {0x53, -0x05, 0xFB, 0x0F, 0, "Beverages   "},
#else
    {0x28, -0x05, 0xE2, 0x1F, 0, "Slaves      "},
    {0x53, -0x05, 0xFB, 0x0F, 0, "Liquor/Wines"},
#endif
    {0xC4, +0x08, 0x36, 0x03, 0, "Luxuries    "},
#if POLITICALLY_CORRECT
    {0xEB, +0x1D, 0x08, 0x78, 0, "Rare Species"},
#else
    {0xEB, +0x1D, 0x08, 0x78, 0, "Narcotics   "},
#endif
    {0x9A, +0x0E, 0x38, 0x03, 0, "Computers   "},
    {0x75, +0x06, 0x28, 0x07, 0, "Machinery   "},
    {0x4E, +0x01, 0x11, 0x1F, 0, "Alloys      "},
};

// Market Modifiers for Station Specializations
// Indexed by [StationSpecialization][CommodityIndex]
// Commodity Indices: 0:Food, 1:Textiles, 2:Radioactives, 3:Slaves, 4:Liquor, 5:Luxuries, 6:Narcotics, 7:Computers, 8:Machinery, 9:Alloys
static MarketModifier stationSpecializationModifiers[NUM_STATION_SPECIALIZATIONS][NUM_STANDARD_COMMODITIES] = {
    // STATION_SPECIALIZATION_BALANCED (0) - Minor or no strong modifications
    {
        {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
        {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}
    },
    // STATION_SPECIALIZATION_INDUSTRIAL (1)
    // Produces: Machinery (8), Alloys (9)
    // Consumes: Food (0), Textiles (1), Radioactives (2)
    {
        {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
        {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}, {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}
    },
    // STATION_SPECIALIZATION_AGRICULTURAL (2)
    // Produces: Food (0), Textiles (1)
    // Consumes: Machinery (8), Luxuries (5)
    {
        {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}, {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
        {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}
    },
    // STATION_SPECIALIZATION_MINING (3)
    // Produces: Radioactives (2), Alloys (9)
    // Consumes: Food (0), Machinery (8)
    {
        {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
        {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}
    }
};

// Market Modifiers for Planet Types
// Indexed by [PlanetMarketType][CommodityIndex]
static MarketModifier planetTypeModifiers[NUM_PLANET_MARKET_TYPES][NUM_STANDARD_COMMODITIES] = {
    // PLANET_MARKET_TYPE_ROCKY_AIRLESS (0)
    // Produces: Radioactives (2), Alloys (9)
    // Consumes: Food (0), Textiles (1)
    {
        {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
        {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}
    },
    // PLANET_MARKET_TYPE_TERRESTRIAL (1)
    // Produces: Food (0), Textiles (1)
    // Consumes: Machinery (8), Computers (7)
    {
        {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}, {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
        {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}
    },
    // PLANET_MARKET_TYPE_GAS_GIANT (2)
    // Produces: Radioactives (2) (e.g. fuel components)
    // Consumes: Food (0), Machinery (8), Computers (7)
    {
        {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
        {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}
    },
    // PLANET_MARKET_TYPE_ICE_GIANT_WORLD (3)
    // Produces: Liquor/Wines (4)
    // Consumes: Food (0), Machinery (8), Luxuries (5)
    {
        {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR},
        {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}, {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR}, {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}
    }
};

// Initializes the global tradnames array.
// tradnames is char tradnames[LAST_TRADE + 1][MAX_LEN];
// Copies names for the first NUM_STANDARD_COMMODITIES.
// Clears remaining entries up to LAST_TRADE.
static inline void init_tradnames(void)
{
    uint16_t i;
    // Copy names from the Commodities array
    for (i = 0; i < NUM_STANDARD_COMMODITIES; i++)
    {
        if (i < (sizeof(Commodities) / sizeof(Commodities[0])))
        {
            strncpy(tradnames[i], Commodities[i].name, MAX_LEN - 1);
            tradnames[i][MAX_LEN - 1] = '\0'; // Ensure null termination
        }
        else
        {
            tradnames[i][0] = '\0';
        }
    }
    // Initialize the remaining part of tradnames up to LAST_TRADE.
    for (; i <= LAST_TRADE; i++)
    {
        if (i < (LAST_TRADE + 1))
        {
            tradnames[i][0] = '\0';
        }
    }
}

// Generates market data for a given planet system and fluctuation.
// MarketType arrays are sized COMMODITY_ARRAY_SIZE (ALIEN_ITEMS_IDX + 1).
static inline MarketType generate_market(uint16_t fluctuation, struct PlanSys planetSystem)
{
    MarketType market;
    uint16_t i;

    for (i = 0; i < NUM_STANDARD_COMMODITIES; i++)
    {
        int32_t q;
        int32_t product = (planetSystem.economy) * (Commodities[i].gradient);
        int32_t changing = fluctuation & (Commodities[i].maskByte);
        q = (Commodities[i].baseQuant) + changing - product;
        q = q & 0xFF;
        if (q & 0x80)
        {
            q = 0;
        }
        market.quantity[i] = (uint16_t)(q & 0x3F);

        q = (Commodities[i].basePrice) + changing + product;
        q = q & 0xFF;
        market.price[i] = (uint16_t)(q * 4);
    }

    for (i = NUM_STANDARD_COMMODITIES; i <= LAST_TRADE; i++)
    {
        market.quantity[i] = 0;
        market.price[i] = 0;
    }

    market.quantity[ALIEN_ITEMS_IDX] = 0;
    market.price[ALIEN_ITEMS_IDX] = 0;

    return market;
}

// Displays the market information.
static inline void display_market_info(MarketType marketData)
{
    uint16_t i;
    printf("ITEM          PRICE  QTY UNIT CARGO");

    // Only show the actual defined commodities (NUM_STANDARD_COMMODITIES)
    // This matches the original game behavior more closely
    for (i = 0; i < NUM_STANDARD_COMMODITIES; i++)
    {
        printf("\n");
        printf("%-12s", Commodities[i].name);
        printf("   %5.1f", ((float)(marketData.price[i]) / 10));
        printf("   %3u", marketData.quantity[i]);
        printf(" %-3s", UnitNames[Commodities[i].units]);
        printf("   %3u", ShipHold[i]);
    }
    printf("\n");
}

// Executes a buy order for a given item and amount.
static inline uint16_t execute_buy_order(uint16_t itemIndex, uint16_t amount)
{
    uint16_t t;
    if (Cash < 0)
    {
        t = 0;
    }
    else
    {
        if (itemIndex >= COMMODITY_ARRAY_SIZE)
            return 0;

        t = minimum_value(LocalMarket.quantity[itemIndex], amount);

        if (itemIndex < NUM_STANDARD_COMMODITIES)
        {
            if ((Commodities[itemIndex].units) == TONNES_UNIT)
            {
                t = minimum_value(HoldSpace, t);
            }
        }

        if (LocalMarket.price[itemIndex] > 0)
        {
            t = minimum_value(t, (uint16_t)floor((double)Cash / LocalMarket.price[itemIndex]));
        }
        else if (Cash > 0 && LocalMarket.quantity[itemIndex] > 0 && LocalMarket.price[itemIndex] == 0)
        {
            // Free item, t is already min(available, requested)
        }
        else
        {
            t = 0;
        }
    }

    if (itemIndex >= COMMODITY_ARRAY_SIZE)
        return 0;

    ShipHold[itemIndex] += t;
    LocalMarket.quantity[itemIndex] -= t;
    Cash -= (int32_t)t * (LocalMarket.price[itemIndex]);

    if (itemIndex < NUM_STANDARD_COMMODITIES)
    {
        if ((Commodities[itemIndex].units) == TONNES_UNIT)
        {
            HoldSpace -= t;
        }
    }
    return t;
}

// Executes a sell order for a given item and amount.
static inline uint16_t execute_sell_order(uint16_t itemIndex, uint16_t amount)
{
    if (itemIndex >= COMMODITY_ARRAY_SIZE)
        return 0;

    uint16_t t = minimum_value(ShipHold[itemIndex], amount);

    ShipHold[itemIndex] -= t;
    LocalMarket.quantity[itemIndex] += t;
    Cash += (int32_t)t * (LocalMarket.price[itemIndex]);

    if (itemIndex < NUM_STANDARD_COMMODITIES)
    {
        if ((Commodities[itemIndex].units) == TONNES_UNIT)
        {
            HoldSpace += t;
        }
    }
    return t;
}

/**
 * Synchronizes cargo between the global ShipHold array and the PlayerShip cargo system.
 * This ensures that both systems have consistent cargo information.
 *
 * @param playerShip Pointer to the PlayerShip structure to synchronize with
 * @return true if synchronization was successful, false otherwise
 */
static inline bool SynchronizeCargoSystems(struct PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Clear current cargo in the PlayerShip structure
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        playerShip->cargo[i].quantity = 0;
        strcpy(playerShip->cargo[i].name, "Empty");
        playerShip->cargo[i].purchasePrice = 0;
    }

    // Reset current cargo tons
    playerShip->attributes.currentCargoTons = 0;

    // Transfer cargo from ShipHold to PlayerShip
    for (uint16_t i = 0; i <= LAST_TRADE; ++i)
    {
        if (ShipHold[i] > 0)
        {
            // Find empty slot in PlayerShip
            int emptySlot = -1;
            for (int j = 0; j < MAX_CARGO_SLOTS; ++j)
            {
                if (playerShip->cargo[j].quantity == 0)
                {
                    emptySlot = j;
                    break;
                }
            }

            if (emptySlot >= 0)
            {
                // Copy the cargo into the player ship
                strncpy(playerShip->cargo[emptySlot].name, tradnames[i], MAX_SHIP_NAME_LENGTH - 1);
                playerShip->cargo[emptySlot].name[MAX_SHIP_NAME_LENGTH - 1] = '\0';
                playerShip->cargo[emptySlot].quantity = ShipHold[i];

                // For simplicity, use the current market price as the purchase price
                playerShip->cargo[emptySlot].purchasePrice = LocalMarket.price[i] / 10; // Convert from internal to display units

                // Update current cargo tons if it's measured in tons
                if (Commodities[i].units == TONNES_UNIT)
                {
                    playerShip->attributes.currentCargoTons += ShipHold[i];
                }
            }
            else
            {
                // This should not happen if MAX_CARGO_SLOTS is large enough
                printf("Warning: Not enough cargo slots to synchronize cargo.\n");
                return false;
            }
        }
    }

    return true;
}
