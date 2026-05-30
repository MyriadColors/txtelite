#pragma once

#include "elite_state.h"      // Unified header for constants, structures, and globals
#include "elite_utils.h"      // For minimum_value
#include "elite_player_ship.h" // For PlayerShip structure
#include "platform_compat.h"  // For StringCompareIgnoreCase
#include <math.h>             // For floor (used in execute_buy_order)
#include <string.h>           // For string operations (snprintf, strcmp, etc.)

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
// Copies names for the first NUM_STANDARD_COMMODITIES.
// Clears remaining entries up to LAST_TRADE.
static inline void init_tradnames(void)
{
    uint16_t i;
    // Copy names from the Commodities array
    for (i = 0; i < NUM_STANDARD_COMMODITIES; i++)
    {        if (i < (sizeof(Commodities) / sizeof(Commodities[0])))
        {
            snprintf(g_state.tradnames[i], MAX_LEN, "%s", Commodities[i].name);
        }
        else
        {
            g_state.tradnames[i][0] = '\0';
        }
    }
    // Initialize the remaining part of tradnames up to LAST_TRADE.
    for (; i <= LAST_TRADE; i++)
    {
        if (i < (LAST_TRADE + 1))
        {
            g_state.tradnames[i][0] = '\0';
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
        uint16_t cargoQty = 0;
        if (g_state.PlayerShipPtr != NULL)
        {
            for (int j = 0; j < MAX_CARGO_SLOTS; j++)
            {
                if (g_state.PlayerShipPtr->cargo[j].quantity > 0 &&
                    StringCompareIgnoreCase(g_state.PlayerShipPtr->cargo[j].name, Commodities[i].name) == 0)
                {
                    cargoQty += g_state.PlayerShipPtr->cargo[j].quantity;
                }
            }
        }

        printf("\n");
        printf("%-12s", Commodities[i].name);
        printf("   %5.1f", ((float)(marketData.price[i]) / 10));
        printf("   %3u", marketData.quantity[i]);
        printf(" %-3s", UnitNames[Commodities[i].units]);
        printf("   %3u", cargoQty);
    }
    printf("\n");
}

// Executes a buy order for a given item and amount.
static inline uint16_t execute_buy_order(uint16_t itemIndex, uint16_t amount)
{
    uint16_t t;
    if (g_state.Cash < 0)
    {
        t = 0;
    }
    else
    {
        if (itemIndex >= COMMODITY_ARRAY_SIZE)
            return 0;

        t = minimum_value(g_state.LocalMarket.quantity[itemIndex], amount);

        if (itemIndex < NUM_STANDARD_COMMODITIES)
        {
            if ((Commodities[itemIndex].units) == TONNES_UNIT)
            {
                uint16_t holdSpace = 0;
                if (g_state.PlayerShipPtr != NULL)
                {
                    holdSpace = g_state.PlayerShipPtr->attributes.cargoCapacityTons - g_state.PlayerShipPtr->attributes.currentCargoTons;
                }
                t = minimum_value(holdSpace, t);
            }
        }

        if (g_state.LocalMarket.price[itemIndex] > 0)
        {
            t = minimum_value(t, (uint16_t)floor((double)g_state.Cash / g_state.LocalMarket.price[itemIndex]));
        }
        else if (g_state.Cash > 0 && g_state.LocalMarket.quantity[itemIndex] > 0 && g_state.LocalMarket.price[itemIndex] == 0)
        {
            // Free item, t is already min(available, requested)
        }
        else
        {
            t = 0;
        }
    }

    if (itemIndex >= COMMODITY_ARRAY_SIZE || t == 0)
        return 0;

    // Update PlayerShip cargo
    if (g_state.PlayerShipPtr != NULL)
    {
        int slot = -1;
        // Try to find existing slot
        for (int i = 0; i < MAX_CARGO_SLOTS; i++)
        {
            if (g_state.PlayerShipPtr->cargo[i].quantity > 0 &&
                StringCompareIgnoreCase(g_state.PlayerShipPtr->cargo[i].name, Commodities[itemIndex].name) == 0)
            {
                slot = i;
                break;
            }
        }

        // If not found, find empty slot
        if (slot == -1)
        {
            for (int i = 0; i < MAX_CARGO_SLOTS; i++)
            {
                if (g_state.PlayerShipPtr->cargo[i].quantity == 0)
                {
                    slot = i;
                    snprintf(g_state.PlayerShipPtr->cargo[slot].name, MAX_SHIP_NAME_LENGTH, "%s", Commodities[itemIndex].name);
                    g_state.PlayerShipPtr->cargo[slot].purchasePrice = g_state.LocalMarket.price[itemIndex] / 10;
                    break;
                }
            }
        }

        if (slot != -1)
        {
            g_state.PlayerShipPtr->cargo[slot].quantity += t;
            if (Commodities[itemIndex].units == TONNES_UNIT)
            {
                g_state.PlayerShipPtr->attributes.currentCargoTons += t;
            }
        }
        else
        {
            // No cargo slots available
            return 0;
        }
    }

    g_state.LocalMarket.quantity[itemIndex] -= t;
    g_state.Cash -= (int32_t)t * (g_state.LocalMarket.price[itemIndex]);

    return t;
}

// Executes a sell order for a given item and amount.
static inline uint16_t execute_sell_order(uint16_t itemIndex, uint16_t amount)
{
    if (itemIndex >= COMMODITY_ARRAY_SIZE || g_state.PlayerShipPtr == NULL)
        return 0;

    uint16_t cargoQty = 0;
    int slot = -1;
    for (int i = 0; i < MAX_CARGO_SLOTS; i++)
    {
        if (g_state.PlayerShipPtr->cargo[i].quantity > 0 &&
            StringCompareIgnoreCase(g_state.PlayerShipPtr->cargo[i].name, Commodities[itemIndex].name) == 0)
        {
            cargoQty = g_state.PlayerShipPtr->cargo[i].quantity;
            slot = i;
            break;
        }
    }

    uint16_t t = minimum_value(cargoQty, amount);
    if (t == 0)
        return 0;

    g_state.PlayerShipPtr->cargo[slot].quantity -= t;
    if (g_state.PlayerShipPtr->cargo[slot].quantity == 0)
    {
        snprintf(g_state.PlayerShipPtr->cargo[slot].name, MAX_SHIP_NAME_LENGTH, "Empty");
    }

    if (Commodities[itemIndex].units == TONNES_UNIT)
    {
        g_state.PlayerShipPtr->attributes.currentCargoTons -= t;
    }

    g_state.LocalMarket.quantity[itemIndex] += t;
    g_state.Cash += (int32_t)t * (g_state.LocalMarket.price[itemIndex]);

    return t;
}
