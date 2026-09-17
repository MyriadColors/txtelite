#pragma once

#include "elite_equipment_constants.h"
#include "elite_navigation_types.h"
#include "elite_ship_components.h"
#include "elite_utils.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#define KGRN "\x1b[32m"
#define KRED "\x1b[31m"
#define KNRM "\x1b[0m"

// Include headers for necessary typedefs and forward declarations
#include "elite_star_system.h" // For Star, Planet, station_t, etc.

// Include the rest of the headers
#include "elite_market.h"           // For execute_buy_order, execute_sell_order, display_market_info
#include "elite_navigation.h"       // For distance, find_matching_system_name, execute_jump_to_planet
#include "elite_planet_info.h"      // For print_system_info (and goat_soup)
#include "elite_player_ship.h"      // For ship status display functions
#include "elite_player_state.h"     // For calculate_fuel_purchase, display_ship_status_brief
#include "elite_save.h"             // For save_game, load_game
#include "elite_ship_cargo.h"       // For cargo management functions
#include "elite_ship_inventory.h"   // For inventory management functions
#include "elite_ship_maintenance.h" // For ship maintenance functions including ConsumeFuel
#include "elite_ship_trading.h"     // For ship trading commands
#include "elite_ship_upgrades.h"    // For ship upgrades
#include "elite_state.h"            // Unified header for constants, structures, and globals
#include "platform_compat.h"        // For cross-platform file operations and StringCompareIgnoreCase
#include <ctype.h>                  // For toupper, tolower
#include <errno.h>                  // For errno and ERANGE
#include <math.h>                   // For floor, fabs
#include <stdatomic.h>              // For atomic random state
#include <stdlib.h>                 // For atoi, atof
#include <string.h>                 // For string operations
#include <time.h>                   // For time functions

// Command help structure
typedef struct {
    const char *command;
    const char *aliases; // Space-separated list of aliases or nullptr
    const char *short_description;
    const char *long_description;
    const char *category;
    bool (*is_available)(void); // Function to check if the command is available
} command_help_t;

// Availability checks for commands
static inline bool is_always_available(void) { return true; }
static inline bool is_docked(void) { return g_state.PlayerLocationType == 10; }
static inline bool is_not_docked(void) { return g_state.PlayerLocationType != 10; }
static inline bool is_at_planet(void) { return g_state.PlayerNavState.currentLocationType == CELESTIAL_PLANET; }
static inline bool is_at_station(void) { return g_state.PlayerNavState.currentLocationType == CELESTIAL_STATION; }

// Command help data
static const command_help_t COMMAND_HELP[] = {
    // TRADING COMMANDS
    {"buy", "b", "Purchase goods from the market",
     "BUY <good> <amount> - Purchase goods from the market\n"
     "  <good>   - Type of trade good (e.g., Food, Computers)\n"
     "  <amount> - Quantity to buy (default: 1)\n"
     "  Example: buy Food 5\n"
     "  Note: You must be docked at a station with a market to buy goods.",
     "TRADING COMMANDS", is_docked},

    {"sell", "s", "Sell goods to the market",
     "SELL <good> <amount> - Sell goods to the market\n"
     "  <good>   - Type of trade good (e.g., Food, Computers)\n"
     "  <amount> - Quantity to sell (default: 1)\n"
     "  Example: sell Computers 3\n"
     "  Note: You must be docked at a station with a market to sell goods.",
     "TRADING COMMANDS", is_docked},

    {"mkt", "m", "Display market information",
     "MKT - Display market information\n"
     "  Shows current market prices, cash, fuel level, and cargo status.\n"
     "  No parameters required.\n"
     "  Note: Market prices vary between systems based on economy type.",
     "TRADING COMMANDS", is_docked},

    {"compare", nullptr, "Compare markets across different stations in the system",
     "COMPARE - Compare markets across different stations in the system\n"
     "  Shows price differences and profit opportunities between stations.\n"
     "  Lists all stations in the system with their distance from you.\n"
     "  Highlights best commodities to buy or sell at each station.\n"
     "  Shows estimated travel times to other stations.\n"
     "  Note: You must be docked at a station to use this command.",
     "TRADING COMMANDS", is_docked},

    // INTERSTELLAR NAVIGATION
    {"jump", "j", "Jump to another star system",
     "JUMP <planetname> - Jump to another star system\n"
     "  <planetname> - Name of the destination system\n"
     "  Example: jump Lave\n"
     "  Note: Requires fuel equal to the distance in light years.\n"
     "        Use 'local' to see systems within jump range.",
     "INTERSTELLAR NAVIGATION", is_not_docked},

    {"local", "l", "List star systems within jump range",
     "LOCAL - List star systems within jump range\n"
     "  Systems marked with * are within current fuel range.\n"
     "  Systems marked with - are within maximum fuel capacity but require refueling.\n"
     "  Distances are shown in light years (LY).",
     "INTERSTELLAR NAVIGATION", is_not_docked},

    {"galhyp", "g", "Perform a galactic hyperspace jump",
     "GALHYP - Perform a galactic hyperspace jump\n"
     "  Jumps to the next galaxy (1-8).\n"
     "  No fuel is required for this special jump.",
     "INTERSTELLAR NAVIGATION", is_not_docked},

    {"info", "i", "Display information about a system",
     "INFO <planetname> - Display information about a system\n"
     "  <planetname> - Name of the system to get information about\n"
     "  Example: info Lave\n"
     "  Shows economy, government, tech level, and other system details.",
     "INTERSTELLAR NAVIGATION", is_always_available},

    {"fuel", "f", "Purchase fuel for your ship",
     "FUEL <amount> - Purchase fuel for your ship\n"
     "  <amount> - Amount of fuel to buy in light years\n"
     "  Example: fuel 2.5\n"
     "  Note: You must be docked at a station to buy fuel.\n"
     "        g_state.Fuel costs %.1f credits per 0.1 LY unit for your current ship",
     "INTERSTELLAR NAVIGATION", is_docked},

    // STAR SYSTEM NAVIGATION
    {"system", "sys", "Displays detailed information about the current star system",
     "SYSTEM - Displays detailed information about the current star system\n"
     "  Shows all celestial bodies, stations, their locations, and travel times.\n"
     "  Note: This command (formerly also available as 'scan') scans the system\n"
     "        for points of interest and costs 1 minute of game time.",
     "STAR SYSTEM NAVIGATION", is_not_docked},

    {"travel", "t", "Travel within the current star system",
     "TRAVEL [destination] - Travel within the current star system\n"
     "  Without parameters: Lists all available destinations.\n"
     "  [destination]: The location to travel to, using the numbering system:\n"
     "    0       - Travel to the central star\n"
     "    1-8     - Travel to a planet (number depends on system)\n"
     "    1.1-8.5 - Travel to a station (format: planet.station)\n"
     "    N       - Travel to the Nav Beacon\n"
     "  Example: travel 2    - Travel to the second planet\n"
     "  Example: travel 1.3  - Travel to the third station orbiting the first planet\n"
     "  Example: travel N    - Travel to the Nav Beacon\n"
     "  Note: Travel consumes game time based on distance.\n"
     "        g_state.Fuel is also consumed at a rate of 0.025 liters per AU.",
     "STAR SYSTEM NAVIGATION", is_not_docked},

    {"dock", "d", "Dock with the current station",
     "DOCK - Dock with the current station\n"
     "  Must be at a station location before docking.\n"
     "  Use 'travel' to navigate to a station first.\n"
     "  Docking provides access to market and other station services.\n"
     "  No parameters required.",
     "STAR SYSTEM NAVIGATION", is_at_station},

    {"land", nullptr, "Land on a planet surface",
     "LAND - Land on a planet surface\n"
     "  Allows you to land on a planet when your ship is at a planet location.\n"
     "  You must be at a planet before landing.\n"
     "  Use 'travel' to navigate to a planet first.\n"
     "  Landing provides access to the planet's market and services.\n"
     "  No parameters required.",
     "STAR SYSTEM NAVIGATION", is_at_planet},

    // SHIP MANAGEMENT
    {"ship", nullptr, "Display basic ship status information",
     "SHIP - Display basic ship status information\n"
     "  Shows hull integrity, fuel, and cargo capacity",
     "SHIP MANAGEMENT", is_always_available},

    {"shipinfo", nullptr, "Display detailed ship information",
     "SHIPINFO - Display detailed ship information\n"
     "  Shows comprehensive information about your ship's systems,\n"
     "  equipment, and cargo hold contents",
     "SHIP MANAGEMENT", is_always_available},

    {"fuelinfo", nullptr, "Display detailed fuel information for your ship",
     "FUELINFO - Display detailed fuel information for your ship\n"
     "  Shows current fuel level, maximum capacity, consumption rate,\n"
     "  estimated range, and refill cost based on your ship's specifications\n"
     "  This command has no parameters",
     "SHIP MANAGEMENT", is_always_available},

    {"repair", nullptr, "Repair your ship's hull damage",
     "REPAIR - Repair your ship's hull damage\n"
     "  This command will repair your ship to 100% hull integrity\n"
     "  Cost is 10 credits per hull point repaired\n"
     "  Note: You must be docked at a station to repair your ship",
     "SHIP MANAGEMENT", is_docked},

    {"shipyard", nullptr, "View ships available for purchase",
     "SHIPYARD - View ships available for purchase\n"
     "  Shows a list of ships available at the current station.\n"
     "  Displays hull strength, cargo capacity, and price.\n"
     "  Includes your current ship's trade-in value.\n"
     "  You must be docked at a station to use this command.\n"
     "  No parameters required.",
     "SHIP MANAGEMENT", is_docked},

    {"compareship", nullptr, "Compare your ship with another ship type",
     "COMPARESHIP <shipname> - Compare your ship with another ship type\n"
     "  Displays a side-by-side comparison of ship specifications.\n"
     "  Shows differences in hull, shields, cargo, etc.\n"
     "  Usage: compareship <shipname> (e.g., 'compareship Viper')\n"
     "  Works anywhere, docking not required.",
     "SHIP MANAGEMENT", is_always_available},

    {"buyship", nullptr, "Purchase a new ship",
     "BUYSHIP <ID or shipname> [notrade] - Purchase a new ship\n"
     "  Buys a new ship from the current station's shipyard.\n"
     "  <ID> - The ship ID number shown in the shipyard list\n"
     "  <shipname> - The name of the ship (for backward compatibility)\n"
     "  By default, trades in your current ship for a credit.\n"
     "  Use 'notrade' flag to buy without trading in (e.g., 'buyship 1 notrade').\n"
     "  Equipment and cargo are transferred when possible.\n"
     "  You must be docked at a station to use this command.\n"
     "  Examples: 'buyship 1' or 'buyship \"Cobra Mk III\"'",
     "SHIP MANAGEMENT", is_docked},

    {"upgrade", nullptr, "View and purchase ship upgrades",
     "UPGRADE [ID] [quantity] - View and purchase ship upgrades (hull, shields, etc.)\n"
     "  Without parameters: Lists all available upgrades\n"
     "  [ID]: The upgrade ID to purchase\n"
     "  [quantity]: Number of upgrades to purchase (default: 1)",
     "SHIP MANAGEMENT", is_docked},

    // EQUIPMENT AND INVENTORY
    {"equip", nullptr, "Purchase and install ship equipment",
     "EQUIP [equipment] - Purchase and install ship equipment\n"
     "  Without parameters: Lists all available equipment\n"
     "  [equipment]: The specific equipment item to purchase\n"
     "  Available equipment types:\n"
     "    ecm      - Electronic Counter Measures (600 CR)\n"
     "    fuelscoop - g_state.Fuel Scoop for collecting fuel from stars (525 CR)\n"
     "    dockcomp - Docking Computer for automated docking (1500 CR)\n"
     "    escape   - Escape Pod for emergency escape (1000 CR)\n"
     "    cargo    - Cargo Bay Extension for +4 tons capacity (400 CR)\n"
     "    pulse    - Pulse Laser for basic combat (400 CR)\n"
     "    beam     - Beam Laser for improved combat (1000 CR)\n"
     "    military - Military Laser for maximum firepower (2500 CR)\n"
     "    mining   - Mining Laser for resource extraction (800 CR)\n"
     "    scanner  - Scanner Upgrade for improved detection (700 CR)\n"
     "    missile  - Homing Missile for one-shot attacks (300 CR)\n"
     "  Example: equip beam\n"
     "  Note: You must be docked at a station to purchase equipment\n"
     "        Equipment availability depends on the system's tech level",
     "EQUIPMENT AND INVENTORY", is_docked},

    {"inv", nullptr, "Display equipment inventory",
     "INV - Display equipment inventory\n"
     "  Shows all equipment items stored in your ship's inventory.\n"
     "  Each item is shown with its inventory slot index for use with the 'use' command.",
     "EQUIPMENT AND INVENTORY", is_always_available},

    {"store", nullptr, "Remove equipment and store in inventory",
     "STORE <slot_number> - Remove equipment and store in inventory\n"
     "  <slot_number> - The equipment slot to remove equipment from\n"
     "  Example: store 0\n"
     "  Note: Use 'shipinfo' to see your equipment slots and what's installed in them.",
     "EQUIPMENT AND INVENTORY", is_always_available},

    {"use", nullptr, "Equip item from inventory",
     "USE <inventory_index> <slot_number> - Equip item from inventory\n"
     "  <inventory_index> - The inventory slot containing the equipment to use\n"
     "  <slot_number> - The equipment slot to install the equipment into\n"
     "  Example: use 2 1\n"
     "  Note: Equipment can only be installed in compatible slots.\n"
     "        Use 'inv' to see your inventory and 'shipinfo' to see slots.",
     "EQUIPMENT AND INVENTORY", is_always_available},

    // CARGO AND MONEY
    {"hold", "h", "Set cargo hold capacity",
     "HOLD <amount> - Set cargo hold capacity\n"
     "  <amount> - Total cargo hold space in tonnes\n"
     "  Example: hold 20\n"
     "  Note: Cannot reduce hold space below current cargo volume.",
     "CARGO AND MONEY", is_always_available},

    {"jettison", "j", "Discard cargo into space",
     "JETTISON <good> <amount> or JETTISON ALL - Discard cargo into space\n"
     "  <good>   - Type of trade good to jettison (e.g., Food, Computers)\n"
     "  <amount> - Quantity to jettison (default: 1)\n"
     "  ALL      - Special flag to jettison all cargo at once\n"
     "  Examples: jettison Food 5\n"
     "            jettison all\n"
     "  Note: Jettisoned cargo is lost permanently with no payment received.\n"
     "        Useful in emergencies or when carrying illegal goods and avoiding authorities.",
     "CARGO AND MONEY", is_not_docked},

    // GAME MANAGEMENT
    {"save", nullptr, "Save the current game state",
     "SAVE [description] - Save the current game state\n"
     "  [description] - Optional description of the save (e.g., 'At Lave')\n"
     "  Example: save Trading at Lave\n"
     "  Note: Save files are timestamped and stored in the 'saves' directory.",
     "GAME MANAGEMENT", is_always_available},

    {"load", nullptr, "List and load saved games",
     "LOAD - List and load saved games\n"
     "  Shows a list of available save files, sorted by most recent first.\n"
     "  Enter the number of the save file to load when prompted.\n"
     "  Note: Loading a save will discard your current game state.",
     "GAME MANAGEMENT", is_always_available},

    {"reset", nullptr, "Restart the game with an optional random seed",
     "RESET [seed] - Restart the game with an optional random seed\n"
     "  Without parameters: Reinitializes the game with default seed 54321\n"
     "  [seed]: A positive integer to use as the random seed\n"
     "  Example: reset, reset 12345\n"
     "  Note: Resetting will discard your current game state and begin a new game.",
     "GAME MANAGEMENT", is_always_available},

    {"quit", "q", "Exit the game",
     "QUIT - Exit the game\n"
     "  Exits the game without saving. Use 'save' first to preserve your progress.",
     "GAME MANAGEMENT", is_always_available},

    // DEBUG COMMANDS
    {"cash", "c", "Adjust cash balance",
     "CASH <+/-amount> - Adjust cash balance\n"
     "  <+/-amount> - Amount to add or subtract from cash balance\n"
     "  Example: cash +100.0  - Add 100 credits\n"
     "  Example: cash -50.5   - Subtract 50.5 credits\n"
     "  Note: This is a debug command for testing purposes.",
     "DEBUG COMMANDS", is_always_available},

    {"rand", nullptr, "Toggle random number generator",
     "RAND - Toggle random number generator\n"
     "  Switches between native and portable RNG implementations.\n"
     "  This is a debug command for testing purposes.",
     "DEBUG COMMANDS", is_always_available},

    {"sneak", nullptr, "Jump to another system without using fuel",
     "SNEAK <planetname> - Jump to another system without using fuel\n"
     "  <planetname> - Name of the destination system\n"
     "  Example: sneak Lave\n"
     "  Note: This is a debug command for testing purposes.",
     "DEBUG COMMANDS", is_not_docked},

    // Terminator
    {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}};

// Helper function to find command help
static const command_help_t *find_command_help(const char *command) {
    for (int i = 0; COMMAND_HELP[i].command != nullptr; i++) {
        if (StringCompareIgnoreCase(command, COMMAND_HELP[i].command) == 0) {
            return &COMMAND_HELP[i];
        }
        // Check aliases
        if (COMMAND_HELP[i].aliases != nullptr) {
            char aliases_copy[MAX_LEN];
            int aliases_length = safe_snprintf(aliases_copy, sizeof(aliases_copy), "%s", COMMAND_HELP[i].aliases);
            if (aliases_length < 0 || (size_t)aliases_length >= sizeof(aliases_copy)) {
                continue;
            }
            char *saveptr = nullptr; // Declare saveptr for safe_strtok
            char *alias = safe_strtok(aliases_copy, " ", &saveptr);
            while (alias != nullptr) {
                if (StringCompareIgnoreCase(command, alias) == 0) {
                    return &COMMAND_HELP[i];
                }
                alias = safe_strtok(nullptr, " ", &saveptr);
            }
        }
    }
    return nullptr;
}

// Helper function to print formatted help text
static void print_help_text(const char *text) {
    // Special handling for fuel command to show dynamic fuel cost
    char buffer[1024];
    if (strstr(text, "FUEL <amount>") != nullptr) {
        const char *placeholder = strstr(text, "%.1f");
        if (placeholder == nullptr) {
            if (fputs(text, stdout) == EOF) {
                return;
            }
        } else {
            int fuel_cost = get_fuel_cost();
            int fractional_cost = fuel_cost % 10;
            if (fractional_cost < 0) {
                fractional_cost = -fractional_cost;
            }
            int written = safe_snprintf(buffer, sizeof(buffer), "%d.%d", fuel_cost / 10, fractional_cost);
            if (written < 0 || (size_t)written >= sizeof(buffer)) {
                return;
            }

            size_t prefix_length = (size_t)(placeholder - text);
            if (fwrite(text, 1, prefix_length, stdout) != prefix_length || fputs(buffer, stdout) == EOF ||
                fputs(placeholder + 4, stdout) == EOF) {
                return;
            }
        }
    } else {
        if (fputs(text, stdout) == EOF) {
            return;
        }
    }
}

[[maybe_unused]] static inline bool do_tweak_random_native(const char *command_arguments) {
    (void)command_arguments; // Mark 's' as unused
    g_state.NativeRand ^= 1;
    return true;
}

[[maybe_unused]] static inline bool do_local_systems_display(const char *command_arguments) {
    (void)command_arguments; // No distance limit is parsed for this command.
    uint16_t d;

    printf("g_state.Galaxy number %i", g_state.GalaxyNum);
    for (planet_num_t syscount = 0; syscount < GAL_SIZE; ++syscount) {
        d = distance(g_state.Galaxy[syscount], g_state.Galaxy[g_state.CurrentPlanet]);

        if (d <= get_max_fuel()) {
            if (d <= g_state.Fuel) {
                printf("\n * ");
            } else {
                printf("\n - ");
            }

            print_system_info(g_state.Galaxy[syscount], true);
            printf(" (%.1f LY)", (double)d / 10.0);
        }
    }
    printf("\n");
    return true;
}

static inline bool do_jump(const char *command_arguments) {
    uint16_t d;
    planet_num_t dest = find_matching_system_name(command_arguments);

    if (dest == g_state.CurrentPlanet) {
        printf("\nBad jump");
        return false;
    }
    d = distance(g_state.Galaxy[dest], g_state.Galaxy[g_state.CurrentPlanet]);

    // Get the fuel cost per distance unit based on ship type
    int fuel_cost_per_unit = get_fuel_cost();

    // Calculate the fuel needed for this jump based on ship's consumption rate
    uint16_t fuel_needed = (uint16_t)((double)d * ((double)fuel_cost_per_unit / 2.0)); // Scale based on ship efficiency
    if (fuel_needed > g_state.Fuel) {
        printf("\nJump too far - requires %d fuel units, you have %d", fuel_needed, g_state.Fuel);
        return false;
    }

    // Use the new ConsumeFuel function to update both global and ship fuel
    if (g_state.PlayerShipPtr != nullptr) {
        if (!consume_fuel((double)fuel_needed, false)) {
            printf("\nJump failed: Insufficient fuel");
            return false;
        }

        // Small chance of minor hull damage during jump.
        static atomic_uint_fast32_t s_random_state = 0x9E3779B9U;
        uint_fast32_t state = atomic_load_explicit(&s_random_state, memory_order_relaxed);
        uint_fast32_t next;
        do {
            next = (state * 1664525U) + 1013904223U;
        } while (!atomic_compare_exchange_weak_explicit(&s_random_state, &state, next, memory_order_relaxed,
                                                        memory_order_relaxed));

        if ((next % 100U) < 5U) {                    // 5% chance
            int damage_taken = (int)(next % 5U) + 1; // 1-5 points of damage
            g_state.PlayerShipPtr->attributes.hullStrength =
                (g_state.PlayerShipPtr->attributes.hullStrength > damage_taken)
                    ? g_state.PlayerShipPtr->attributes.hullStrength - damage_taken
                    : 1;

            printf("\nHyperspace stress caused minor hull damage (-%d)", damage_taken);
        }
    } // Update global g_state.Fuel based on ship's fuel if g_state.PlayerShipPtr is available
    if (g_state.PlayerShipPtr != nullptr) {
        // Sync the ship's fuel with the global value (exact match)
        g_state.PlayerShipPtr->attributes.fuelLiters = (double)g_state.Fuel * (double)10.0F;
    }

    execute_jump_to_planet(dest);
    print_system_info(g_state.Galaxy[g_state.CurrentPlanet], false);
    return true;
}

[[maybe_unused]] static inline bool do_sneak(const char *command_arguments) {
    uint16_t fuelkeep = g_state.Fuel;
    double ship_fuel_keep = (g_state.PlayerShipPtr != nullptr) ? g_state.PlayerShipPtr->attributes.fuelLiters : 0.0;
    bool b;
    g_state.Fuel = 666; // Arbitrary large fuel value for sneak
    if (g_state.PlayerShipPtr != nullptr) {
        g_state.PlayerShipPtr->attributes.fuelLiters = 6660.0;
    }
    b = do_jump(command_arguments);
    g_state.Fuel = fuelkeep;
    if (g_state.PlayerShipPtr != nullptr) {
        g_state.PlayerShipPtr->attributes.fuelLiters = ship_fuel_keep;
    }
    return b;
}

[[maybe_unused]] static inline bool do_galactic_hyperspace(const char *command_arguments) {
    (void)(command_arguments); /* Discard s */

    // This feature has been removed from the game
    printf("\nThe Galactic Hyperspace technology has been deemed unsafe and is "
           "no longer available.");
    printf("\nPlease use standard Hyperspace jumps (jump command) for "
           "interstellar travel.");

    return false;
}

[[maybe_unused]] static inline bool do_planet_info_display(const char *command_arguments) {
    planet_num_t dest = find_matching_system_name(command_arguments);
    if (dest < GAL_SIZE) { // Check if a valid planet was found
        print_system_info(g_state.Galaxy[dest], false);
    } else {
        printf("\nPlanet not found: %s", command_arguments);
        return false;
    }
    return true;
}

[[maybe_unused]] static inline bool do_hold(const char *command_arguments) {
    char *endptr = nullptr;
    long parsed_capacity;

    if (command_arguments == nullptr) {
        printf("\nInvalid hold capacity.");
        return false;
    }

    errno = 0;
    parsed_capacity = strtol(command_arguments, &endptr, 10);
    if (errno == ERANGE || endptr == command_arguments || *endptr != '\0' || parsed_capacity < 0 ||
        parsed_capacity > UINT16_MAX) {
        printf("\nInvalid hold capacity: %s", command_arguments);
        return false;
    }

    uint16_t a = (uint16_t)parsed_capacity;
    uint16_t t = 0;

    if (g_state.PlayerShipPtr != nullptr) {
        for (int i = 0; i < MAX_CARGO_SLOTS; i++) {
            if (g_state.PlayerShipPtr->cargo[i].quantity > 0) {
                for (int j = 0; j < NUM_STANDARD_COMMODITIES; j++) {
                    if (StringCompareIgnoreCase(g_state.PlayerShipPtr->cargo[i].name, g_commodities[j].name) == 0) {
                        if (g_commodities[j].units == TONNES_UNIT) {
                            t += g_state.PlayerShipPtr->cargo[i].quantity;
                        }
                        break;
                    }
                }
            }
        }
    }

    if (a < t) // Can't set hold space to less than current cargo
    {
        printf("\nHold too full to reduce size to %u. Current cargo: %u tonnes.", a, t);
        return false;
    }

    if (g_state.PlayerShipPtr != nullptr) {
        g_state.PlayerShipPtr->attributes.cargoCapacityTons = a;
        g_state.PlayerShipPtr->attributes.currentCargoTons = t;
    }
    printf("\nHold space set to %u. Available: %u tonnes.", a, a - t);
    return true;
}

[[maybe_unused]] static inline bool do_sell(const char *command_arguments) {
    uint16_t i;
    uint16_t t;
    char s2[MAX_LEN];
    char arg_copy[MAX_LEN];
    int copy_result = safe_snprintf(arg_copy, sizeof(arg_copy), "%s", command_arguments != nullptr ? command_arguments : "");
    if (copy_result < 0 || (size_t)copy_result >= sizeof(arg_copy)) {
        printf("\nSell command arguments are too long or could not be copied.");
        return false;
    }
    split_string_at_first_space(arg_copy, s2);
    auto a = (uint16_t)strtol(arg_copy, nullptr, 10);

    if (a == 0) {
        a = 1;
    }

    i = match_string_in_array(s2, g_state.tradnames, LAST_TRADE + 1);

    if (i == 0) {
        printf("\nUnknown trade good: '%s'", s2);
        return false;
    }

    i -= 1; // Adjust index for 0-based array

    t = execute_sell_order(i, a);

    if (t == 0) {
        printf("Cannot sell any %s", g_state.tradnames[i]);
    } else {
        printf("\nSelling %i%s of %s", t, g_unit_names[g_commodities[i].units], g_state.tradnames[i]);

        // Synchronize the cargo systems after selling
        if (g_state.PlayerShipPtr != nullptr) {
        }
    }
    return true;
}

[[maybe_unused]] static inline bool do_buy(const char *command_arguments) {
    uint16_t i;
    uint16_t t;
    char s2[MAX_LEN];
    char arg_copy[MAX_LEN];
    int copy_result = safe_snprintf(arg_copy, sizeof(arg_copy), "%s", command_arguments != nullptr ? command_arguments : "");
    if (copy_result < 0) {
        arg_copy[0] = '\0';
    }
    split_string_at_first_space(arg_copy, s2);
    char *endptr = nullptr;
    errno = 0;
    long parsed_amount = strtol(arg_copy, &endptr, 10);
    uint16_t a = (errno == ERANGE || endptr == arg_copy || parsed_amount < 0 || parsed_amount > UINT16_MAX)
                     ? 1
                     : (uint16_t)parsed_amount;

    if (a == 0) {
        a = 1;
    }

    i = match_string_in_array(s2, g_state.tradnames, LAST_TRADE + 1);

    if (i == 0) {
        printf("\nUnknown trade good: '%s'", s2);
        return false;
    }
    i -= 1; // Adjust index

    t = execute_buy_order(i, a);
    if (t == 0) {
        printf("Cannot buy any %s", g_state.tradnames[i]);
    } else {
        printf("\nBuying %i%s of %s", t, g_unit_names[g_commodities[i].units], g_state.tradnames[i]);
        // Synchronize the cargo systems after buying
        if (g_state.PlayerShipPtr != nullptr) {
        }
    }
    return true;
}

[[maybe_unused]] static inline bool do_fuel(const char *command_arguments) {
    if (command_arguments == nullptr || command_arguments[0] == '\0') {
        printf("\nUsage: fuel <amount>");
        return false;
    }
    char *parse_end = nullptr;
    const double REQUESTED_FUEL = strtod(command_arguments, &parse_end);
    if (parse_end == command_arguments || *parse_end != '\0' || REQUESTED_FUEL < 0.0) {
        printf("\nNumber not understood for fuel command.");
        return false;
    }
    const double REQUESTED_UNITS_DOUBLE = floor(10.0 * REQUESTED_FUEL);
    const uint16_t REQUESTED_UNITS =
        REQUESTED_UNITS_DOUBLE > (double)UINT16_MAX ? UINT16_MAX : (uint16_t)REQUESTED_UNITS_DOUBLE;
    const uint16_t F = calculate_fuel_purchase(REQUESTED_UNITS);
    if (F == 0) {
        printf("\nCan't buy any fuel");
    } else {
        // Deduct the cost from cash
        g_state.Cash -= F * get_fuel_cost();

        // Add the fuel to the current fuel level, making sure not to exceed max
        // fuel for the ship
        const int CURRENT_MAX_FUEL = get_max_fuel();
        g_state.Fuel =
            (g_state.Fuel + F > CURRENT_MAX_FUEL) ? (uint16_t)(CURRENT_MAX_FUEL) : (uint16_t)(g_state.Fuel + F);

        // Also update the ship's fuel levels
        if (g_state.PlayerShipPtr != nullptr) {
            // Convert game units to liters (1 fuel unit = 0.1 LY = 10 liters)
            const double FUEL_LITERS = (double)F * 10.0;
            const double MAX_FUEL_LITERS = (double)g_state.PlayerShipPtr->ship_type_t->maxFuelLY * 100.0;

            g_state.PlayerShipPtr->attributes.fuelLiters =
                (g_state.PlayerShipPtr->attributes.fuelLiters + FUEL_LITERS > MAX_FUEL_LITERS)
                    ? MAX_FUEL_LITERS
                    : g_state.PlayerShipPtr->attributes.fuelLiters + FUEL_LITERS;
        }

        printf("\nBuying %.1fLY fuel", (double)F / 10.0);
    }
    return true;
}

[[maybe_unused]] static inline bool do_cash(const char *command_arguments) {
    if (command_arguments == nullptr || command_arguments[0] == '\0') {
        printf("\nUsage: cash <amount>");
        return false;
    }

    char *end = nullptr;
    errno = 0;
    const double AMOUNT = strtod(command_arguments, &end);
    const double SCALED_AMOUNT = 10.0 * AMOUNT; // Amount is in tenths of credits
    while (end != nullptr && isspace((unsigned char)*end) != 0) {
        ++end;
    }
    if (end == command_arguments || (end != nullptr && *end != '\0') || errno == ERANGE || !isfinite(SCALED_AMOUNT)) {
        printf("Number not understood for cash command.");
        return false;
    }

    const long SCALED_AMOUNT_CONSTANT = (long)SCALED_AMOUNT;
    g_state.Cash += (long)SCALED_AMOUNT_CONSTANT;

    if (SCALED_AMOUNT_CONSTANT != 0) {
        printf("\nCash adjusted by %.1f. Current cash: %.1f CR.", (double)SCALED_AMOUNT_CONSTANT / 10.0,
               (double)g_state.Cash / 10.0);
        return true;
    }

    printf("Number not understood for cash command.");
    return false;
}

[[maybe_unused]] static inline bool do_market_display(const char *command_arguments) {
    (void)command_arguments;

    // Display basic market information
    display_market_info(g_state.LocalMarket);

    // Display current location economic info if we're at a station
    if (g_state.PlayerNavState.currentLocationType == CELESTIAL_STATION &&
        g_state.PlayerNavState.currentLocation.station != nullptr && g_state.CurrentStarSystem != nullptr &&
        g_state.CurrentStarSystem->plan_sys_t != nullptr) {

        station_t *station = g_state.PlayerNavState.currentLocation.station;

        // Find parent planet for context
        planet_t *parent_planet = nullptr;
        for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets && !parent_planet; i++) {
            planet_t *planet = &g_state.CurrentStarSystem->planets[i];
            if (!planet) {
                continue;
            }

            for (uint8_t j = 0; j < planet->numStations; j++) {
                if (planet->stations[j] == station) {
                    parent_planet = planet;
                    break;
                }
            }
        }

        // Display economy information
        printf("\n\n=== STATION ECONOMY ===");
        printf("\nSystem Economy: %s", g_econ_names[g_state.CurrentStarSystem->plan_sys_t->economy]);

        // Display specialization
        const char *spec_names[] = {"Balanced", "Industrial", "Agricultural", "Mining"};
        if (station->specialization < 4) {
            printf("\nStation Specialization: %s", spec_names[station->specialization]);
        }

        // Display market update time
        uint64_t time_since_update = game_time_get_seconds() - station->lastMarketUpdate;
        printf("\nLast Market Update: %llu seconds ago", (unsigned long long)time_since_update);

        // Add hint about best trades based on specialization
        printf("\n\nTrade Opportunities:");
        switch (station->specialization) {
        case 1: // Industrial
            printf("\n- Sells machinery and computers at good prices");
            printf("\n- Looking to buy food and textiles");
            break;
        case 2: // Agricultural
            printf("\n- Sells food and textiles at good prices");
            printf("\n- Looking to buy machinery and alloys");
            break;
        case 3: // Mining
            printf("\n- Sells radioactives and alloys at good prices");
            printf("\n- Looking to buy luxuries and computers");
            break;
        default: // Balanced
            printf("\n- No special trade opportunities");
            break;
        }
    }

    printf("\n\nFuel :%.1fLY", (double)(g_state.Fuel) / 10.0);
    if (g_state.PlayerShipPtr != nullptr) {
        printf("      Holdspace :%dt", g_state.PlayerShipPtr->attributes.cargoCapacityTons -
                                           g_state.PlayerShipPtr->attributes.currentCargoTons);
    }
    printf("\nCurrent Cash: %.1f CR\n", (double)(g_state.Cash) / 10.0);
    return true;
}

[[maybe_unused]] static inline bool do_quit(const char *command_arguments) {
    (void)(command_arguments);
    printf("\nExiting Text Elite. Goodbye!\n");
    // Use the immediate process termination primitive here; unlike exit(), it
    // does not run process-wide cleanup handlers that are unsafe concurrently.
    if (fflush(stdout) != 0) {
        g_state.ExitStatus = EXIT_FAILURE;
    }
    _Exit(g_state.ExitStatus);
    // This line will not be reached if g_state.ExitStatus leads to a successful exit.
    // It's here to satisfy the function signature if exit() somehow didn't
    // terminate.
    return true;
}

/**
 * Resets the game state to a new game, either with a custom seed or the default
 * seed. Usage: reset [seed] If no seed is provided, uses the default seed
 * "54321"
 *
 * @param command_arguments Optional seed to use for the new game
 * @return 1 if the reset was successful
 */
[[maybe_unused]] static inline bool do_reset(const char *command_arguments) {
    unsigned int seed = 54321; // Default seed

    // If a seed is provided, use it
    if (command_arguments != nullptr && command_arguments[0] != '\0') {
        char *end = nullptr;
        errno = 0;
        const unsigned long PARSED_SEED = strtoul(command_arguments, &end, 10);
        while (end != nullptr && isspace((unsigned char)*end)) {
            ++end;
        }

        if (end != command_arguments && *end == '\0' && errno != ERANGE && PARSED_SEED > 0 && PARSED_SEED <= UINT_MAX) {
            const unsigned int PROVIDED_SEED = (unsigned int)PARSED_SEED;
            seed = PROVIDED_SEED;
            printf("\nResetting game with custom seed: %u", seed);
        } else {
            printf("\nInvalid seed provided. Using default seed: %u", seed);
        }
    } else {
        printf("\nResetting game with default seed: %u", seed);
    }

    // Initialize a new game with the seed
    my_srand(seed);
    initialize_player_state();
    game_time_initialize();

    printf("\nGame reset complete. You are now at planet %s in g_state.Galaxy %d.",
           g_state.Galaxy[g_state.CurrentPlanet].name, g_state.GalaxyNum);

    return true;
}

[[maybe_unused]] static inline bool do_help(const char *command_arguments) {
    char arg_copy[MAX_LEN];
    const int COPY_RESULT =
        safe_snprintf(arg_copy, sizeof(arg_copy), "%s", command_arguments != nullptr ? command_arguments : "");
    if (COPY_RESULT < 0) {
        arg_copy[0] = '\0';
    } else if ((size_t)(COPY_RESULT) >= sizeof(arg_copy)) {
        arg_copy[sizeof(arg_copy) - 1] = '\0';
    }
    char *cmd = strip_leading_trailing_spaces(arg_copy);

    if (cmd == nullptr || strlen(cmd) == 0) {
        // Display general help
        printf("\n=== TXTELITE COMMAND REFERENCE ===\n");

        const char *current_category = nullptr;
        for (int i = 0; COMMAND_HELP[i].command != nullptr; i++) {
            if (current_category == nullptr || strcmp(current_category, COMMAND_HELP[i].category) != 0) {
                current_category = COMMAND_HELP[i].category;
                printf("\n--- %s ---\n", current_category);
            }

            bool available = COMMAND_HELP[i].is_available();
            printf("  %s%-12s%s - %s %s\n", (int)available ? KGRN : KRED, COMMAND_HELP[i].command, KNRM,
                   COMMAND_HELP[i].short_description, (int)available ? "" : "(unavailable)");
        }
        printf("\nFor detailed help on any command, type 'help <command>'.\n");
    } else {
        // Display specific help for a command
        const command_help_t *help = find_command_help(cmd);
        if (help) {
            print_help_text(help->long_description);
            if (!help->is_available()) {
                printf("\n%s(Currently unavailable)%s", KRED, KNRM);
            }
            printf("\n");
        } else {
            printf("\nUnknown command: %s", cmd);
            printf("\nUse 'help' without parameters to see all available commands.\n");
        }
    }
    return true;
}

[[maybe_unused]] static inline bool do_save(const char *command_arguments) {
    // Generate filename with current date and time
    char filename[64];
    time_t now = time(nullptr);
    struct tm timeinfo;
    if (safe_localtime(&now, &timeinfo) == 0) {
        if (strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.sav", &timeinfo) == 0) {
            return false;
        }
    } else {
        if (safe_snprintf(filename, sizeof(filename), "save_%ld.sav", (long)now) < 0) {
            return false;
        }
    }

    // Use the provided description if available
    const char *description = nullptr;

    // If command arguments were provided, use them as description
    if (command_arguments && command_arguments[0] != '\0') {
        description = command_arguments;
    }

    // Save the game
    bool success = save_game(filename, description);

    return success;
}

[[maybe_unused]] static inline bool do_load(const char *command_arguments) {
    (void)command_arguments; // Unused parameter

    printf("\nAvailable save files:\n");

    // Structure to store save file information
    typedef struct {
        char filename[MAX_PATH];
        time_t timestamp;
    } save_file_info_t;

    // Use cross-platform directory iterator
    directory_iterator_t iter;
    char search_pattern[MAX_PATH];
    platform_make_pattern(search_pattern, sizeof(search_pattern), "saves", "*.sav");

    if (!platform_find_first_file(&iter, search_pattern)) {
        printf("No save files found in the 'saves' directory.\n");
        return false;
    }

    // Count the number of save files
    int file_count = 0;
    save_file_info_t save_files[100]; // Array to store up to 100 save files

    do {
        const char *filename = platform_get_filename(&iter);
        if (filename) {
            // Store filename
            int filename_length = safe_snprintf(save_files[file_count].filename, MAX_PATH, "%s", filename);
            if (filename_length < 0 || filename_length >= MAX_PATH) {
                // Ignore filenames that cannot be represented safely.
                continue;
            }

            // Get file timestamp using cross-platform function
            save_files[file_count].timestamp = platform_get_file_time(&iter);

            file_count++;
            if (file_count >= 100) {
                break; // Limit to 100 files
            }
        }
    } while (platform_find_next_file(&iter));

    platform_find_close(&iter);

    if (file_count == 0) {
        printf("No save files found.\n");
        return false;
    }

    // Sort save files by timestamp (most recent first)
    for (int i = 0; i < file_count - 1; i++) {
        for (int j = 0; j < file_count - i - 1; j++) {
            if (save_files[j].timestamp < save_files[j + 1].timestamp) {
                // Swap
                save_file_info_t temp = save_files[j];
                save_files[j] = save_files[j + 1];
                save_files[j + 1] = temp;
            }
        }
    }

    // Display the sorted save files
    for (int i = 0; i < file_count; i++) { // Read save header to get the description
        save_header_t header;
        char full_path[MAX_PATH];
        platform_make_path(full_path, sizeof(full_path), "saves", save_files[i].filename);
        FILE *file = safe_fopen(full_path, "rb");
        bool header_valid = false;

        if (file) {
            if (fread(&header, sizeof(header), 1, file) == 1) {
                header_valid = (strncmp(header.signature, SAVE_SIGNATURE, strlen(SAVE_SIGNATURE)) == 0);
            }
            if (fclose(file) != 0) {
                header_valid = false;
            }
        } // Format date and time
        struct tm timeinfo;
        int localtime_result = safe_localtime(&save_files[i].timestamp, &timeinfo);
        char time_str[32];
        if (localtime_result == 0) {
            size_t formatted_length = strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
            if (formatted_length == 0) {
                int fallback_length = safe_snprintf(time_str, sizeof(time_str), "%s", "Invalid Date");
                if (fallback_length < 0 || (size_t)fallback_length >= sizeof(time_str)) {
                    time_str[0] = '\0';
                }
            }
        } else {
            int fallback_length = safe_snprintf(time_str, sizeof(time_str), "%s", "Invalid Date");
            if (fallback_length < 0 || (size_t)fallback_length >= sizeof(time_str)) {
                time_str[0] = '\0';
            }
        }

        printf("%2d. %s - %s", i + 1, save_files[i].filename, time_str);
        if (header_valid) {
            printf(" - %s", header.description);
        }
        printf("\n");
    }

    // Prompt user to select a save file
    if (file_count > 0) {
        printf("\nEnter the number of the save file to load (or 0 to cancel): ");
        char input[10];
        if (fgets(input, sizeof(input), stdin) != nullptr) {
            char *endptr = nullptr;
            errno = 0;
            long selection = strtol(input, &endptr, 10);
            if (errno != ERANGE && endptr != input && selection > 0 && selection <= file_count) {
                char full_path[MAX_PATH];
                platform_make_path(full_path, sizeof(full_path), "saves", save_files[selection - 1].filename);
                return load_game(full_path);
            }
        }
    }

    return false;
}

// =============================
// Star System Commands
// =============================

// Displays detailed information about the current star system, including scan
// data
[[maybe_unused]] static inline bool do_system_info(const char *command_arguments) {
    (void)(command_arguments); // Unused parameter
    // Validate star system data
    if (!g_state.CurrentStarSystem) {
        printf("\nError: Star system data not available. System might not be "
               "properly initialized.");
        return false;
    }

    // Validate pointer to plan_sys_t data
    if (!g_state.CurrentStarSystem->plan_sys_t) {
        printf("\nError: planet_tsystem data not available.");
        return false;
    } // Get current location information
    char loc_buffer[MAX_LEN];
    get_current_location_name(&g_state.PlayerNavState, loc_buffer, sizeof(loc_buffer));

    // System header with basic information
    printf("\n==== SYSTEM SCAN: %s ====", g_state.CurrentStarSystem->plan_sys_t->name);
    printf("\nCurrent location: %s (%.2f AU from star)", loc_buffer, g_state.PlayerNavState.distanceFromStar);

    // Economic and political information
    printf("\nEconomy: %s", g_econ_names[g_state.CurrentStarSystem->plan_sys_t->economy]);
    printf("\nGovernment: %s", g_gov_names[g_state.CurrentStarSystem->plan_sys_t->govType]);
    printf("\nTech Level: %d", g_state.CurrentStarSystem->plan_sys_t->techLev + 1);
    printf("\nPopulation: %u Billion",
           (g_state.CurrentStarSystem->plan_sys_t->population) >> 3); // Star information with spectral classification
    const char *spectral_classes[] = {"O", "B", "A", "F", "G", "K", "M"};
    printf("\n\nStar: %s", g_state.CurrentStarSystem->centralStar.name);
    if (g_state.CurrentStarSystem->centralStar.spectralClass < 7) {
        printf("\n  Class: %s (%.1f solar masses, %.1f luminosity, %.0F K)",
               spectral_classes[g_state.CurrentStarSystem->centralStar.spectralClass],
               g_state.CurrentStarSystem->centralStar.mass, g_state.CurrentStarSystem->centralStar.luminosity,
               g_state.CurrentStarSystem->centralStar.temperature);
        printf("\n  Age: %.1f billion years", g_state.CurrentStarSystem->centralStar.age);
        printf("\n  Habitable Zone: %.2f - %.2f AU", g_state.CurrentStarSystem->centralStar.habitableZoneInner,
               g_state.CurrentStarSystem->centralStar.habitableZoneOuter);
    }

    // Planets information
    printf("\n\nPlanets: %d", g_state.CurrentStarSystem->numPlanets);
    if (g_state.CurrentStarSystem->numPlanets > 0) {
        // planet_ttype information for display
        const char *planet_types[] = {"Rocky/Airless", "Terrestrial", "Gas Giant", "Ice Giant"};

        for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets; i++) {
            planet_t *planet = &g_state.CurrentStarSystem->planets[i];
            if (!planet) {
                printf("\\n  %d. [Error: Invalid planet data]", i + 1);
                continue;
            } // planet_tbasic info
            double dist_to_planet = fabs(g_state.PlayerNavState.distanceFromStar - planet->orbitalDistance);
            uint32_t time_to_planet =
                calculate_travel_time(g_state.PlayerNavState.distanceFromStar, planet->orbitalDistance);
            double fuel_to_planet = calculate_travel_fuel_requirement(dist_to_planet);
            printf("\n  %d. %s (%.2f AU from star, %.2f AU away, %u min travel, %.3f fuel L required)", i + 1,
                   planet->name, planet->orbitalDistance, dist_to_planet, time_to_planet / 60,
                   fuel_to_planet); // planet_ttype and physical characteristics
            if (planet->type < 4) {
                printf("\n     Type: %s", planet_types[planet->type]);
            } else {
                printf("\n     Type: Unknown");
            }
            printf("\n     Radius: %.0F km", planet->radius);
            printf("\n     Surface Temperature: %.0F K (%.0F C)", planet->surfaceTemperature,
                   planet->surfaceTemperature - 273.15);

            // Enhanced habitability analysis
            double habitability_score = calculate_habitability_score(planet, &g_state.CurrentStarSystem->centralStar);
            const char *habitability_rating = get_habitability_rating(habitability_score);
            const char *temp_category = get_temperature_category(planet->surfaceTemperature);
            bool has_atmosphere = check_planetary_atmosphere_potential(planet, &g_state.CurrentStarSystem->centralStar);
            bool tidally_locked = check_tidal_locking(planet, &g_state.CurrentStarSystem->centralStar);
            double radiation_level = calculate_radiation_exposure(planet, &g_state.CurrentStarSystem->centralStar);

            printf("\n     Habitability: %.1f/100 (%s)", habitability_score, habitability_rating);
            printf("\n     Temperature: %s", temp_category);
            printf("\n     Atmosphere: %s", (int)has_atmosphere ? "Potential" : "Unlikely");
            printf("\n     Rotation: %s", (int)tidally_locked ? "Tidally Locked" : "Normal");
            printf("\n     Radiation: %.1fx Earth levels", radiation_level);

            if (planet->isInHabitableZone) {
                printf("\n     Status: In Habitable Zone *");
            } else if (planet->surfaceTemperature > 273.15 && planet->surfaceTemperature < 373.15) {
                printf("\n     Status: Potentially habitable temperature");
            } else if (planet->surfaceTemperature < 200.0) {
                printf("\n     Status: Frozen world");
            } else if (planet->surfaceTemperature > 500.0) {
                printf("\n     Status: Scorched world");
            } // station_t information for this planet
            if (planet->numStations > 0) {
                printf("\n     Stations: %d", planet->numStations);

                bool has_valid_stations = false;
                for (uint8_t j = 0; j < planet->numStations; j++) {
                    station_t *station = planet->stations[j];
                    if (!station) {
                        continue; // Skip nullptr stations
                    }

                    has_valid_stations = 1; // station_t type information
                    const char *station_types[] = {"Orbital", "Coriolis", "Ocellus"};
                    double station_dist_absolute = planet->orbitalDistance + station->orbitalDistance;
                    double dist_to_station = fabs(g_state.PlayerNavState.distanceFromStar - station_dist_absolute);
                    uint32_t time_to_station =
                        calculate_travel_time(g_state.PlayerNavState.distanceFromStar, station_dist_absolute);
                    double fuel_to_station = calculate_travel_fuel_requirement(dist_to_station);

                    printf("\n     %d.%d. %s (%.3f AU from planet, %.2f AU away, %u min "
                           "travel, %.3f fuel L required)",
                           i + 1, j + 1, station->name, station->orbitalDistance, dist_to_station, time_to_station / 60,
                           fuel_to_station);

                    // Display station type if valid
                    if (station->type < 3) {
                        printf("\n          Type: %s", station_types[station->type]);
                    }

                    // List available services
                    printf("\n          Services: ");
                    if (station->hasMarket) {
                        printf("Market ");
                    }
                    if (station->hasShipyard) {
                        printf("Shipyard ");
                    }
                    if (station->hasMissions) {
                        printf("Missions ");
                    }
                    if (station->hasDockingComputer) {
                        printf("DockingComputer ");
                    }
                    if (!station->hasMarket && !station->hasShipyard && !station->hasMissions &&
                        !station->hasDockingComputer) {
                        printf("None");
                    }
                }

                if (!has_valid_stations) {
                    printf("\n     [No valid stations data]");
                }
            } else {
                printf("\n     Stations: None");
            }
        }
    } else {
        printf("\n  (None)");
    } // Nav Beacon information
    double dist_to_nav_beacon =
        fabs(g_state.PlayerNavState.distanceFromStar - g_state.CurrentStarSystem->navBeaconDistance);
    uint32_t time_to_nav_beacon =
        calculate_travel_time(g_state.PlayerNavState.distanceFromStar, g_state.CurrentStarSystem->navBeaconDistance);
    double fuel_to_nav_beacon = calculate_travel_fuel_requirement(dist_to_nav_beacon);
    printf("\n\nNav Beacon: %.2f AU from star (%.2f AU away, %u min travel, %.3f fuel L required)",
           g_state.CurrentStarSystem->navBeaconDistance, dist_to_nav_beacon, time_to_nav_beacon / 60,
           fuel_to_nav_beacon);
    printf("\n  Travel code: N");

    // Current player location
    printf("\n\nCurrent location: %s (%.2f AU from star)", loc_buffer, g_state.PlayerNavState.distanceFromStar);
    // Star distance and travel time (from current location)
    double dist_to_star = g_state.PlayerNavState.distanceFromStar;
    uint32_t time_to_star = calculate_travel_time(g_state.PlayerNavState.distanceFromStar, 0.0);
    double fuel_to_star = calculate_travel_fuel_requirement(dist_to_star);
    printf("\nDistance to Star (%s): %.2f AU, %u min travel, %.3f fuel L required",
           g_state.CurrentStarSystem->centralStar.name, dist_to_star, time_to_star / 60, fuel_to_star);
    printf("\n  Travel code: 0");

    // Add travel hint
    printf("\n\n(Use 'travel <code>' to navigate to any location, e.g., 'travel "
           "2.1' or 'travel N')");

    // System time
    char time_buffer[MAX_LEN * 2];
    game_time_get_formatted(time_buffer, sizeof(time_buffer));
    printf("\n\nSystem Time: %s", time_buffer);

    // Small time cost for performing a system scan (1 minute)
    game_time_advance(60);
    printf("\n\nSystem scan complete. Elapsed time: 1 minute.");

    return true;
}

// Lists available destinations and allows travel within the system
[[maybe_unused]] static inline bool do_travel(const char *command_arguments) {
    // Check if star system data is properly initialized
    if (!g_state.CurrentStarSystem) {
        printf("\nError: Star system data not available. System might not be "
               "properly initialized.");
        return false;
    }

    // Get current location information for display
    char location_buffer[MAX_LEN];
    get_current_location_name(&g_state.PlayerNavState, location_buffer, sizeof(location_buffer));

    // If no arguments provided, just list destinations
    if (command_arguments == nullptr || strlen(command_arguments) == 0 ||
        strspn(command_arguments, " \t\n\r") == strlen(command_arguments)) {

        printf("\nCurrent location: %s (%.2f AU from star)", location_buffer, g_state.PlayerNavState.distanceFromStar);
        printf("\n\nAvailable destinations:");

        // Star
        printf("\n  0. %s (0.00 AU)", g_state.CurrentStarSystem->centralStar.name);

        // Planets and stations
        for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets; i++) {
            planet_t *planet = &g_state.CurrentStarSystem->planets[i];
            if (!planet) {
                printf("\n  %d. [Error: Invalid planet data]", i + 1);
                continue;
            }
            printf("\n  %d. %s (%.2f AU)", i + 1, planet->name, planet->orbitalDistance);

            for (uint8_t j = 0; j < planet->numStations; j++) {
                station_t *station = planet->stations[j];
                if (!station) {
                    continue; // Skip invalid stations
                }
                printf("\n     %d.%d. %s (%.2f AU)", i + 1, j + 1, station->name,
                       planet->orbitalDistance + station->orbitalDistance);
            }
        }

        // Nav Beacon
        printf("\n  N. Nav Beacon (%.2f AU)", g_state.CurrentStarSystem->navBeaconDistance);

        printf("\n\nUse 'travel <destination number>' to travel (e.g., 'travel 1' "
               "or 'travel 1.2' or 'travel N')");
        return true;
    }
    // Parse destination string, trimming whitespace
    char dest_str[MAX_LEN];
    int dest_len = safe_snprintf(dest_str, sizeof(dest_str), "%s", command_arguments);
    if (dest_len < 0 || (size_t)dest_len >= sizeof(dest_str)) {
        printf("\nDestination is too long.");
        return false;
    }

    // Trim leading and trailing whitespace
    char *start = dest_str;
    char *end = dest_str + strlen(dest_str) - 1;

    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    while (end > start && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }

    if (start != dest_str) {
        memmove(dest_str, start, strlen(start) + 1);
    }

    // If destination string is empty after trimming
    if (strlen(dest_str) == 0) {
        printf("\nNo destination specified. Use 'travel' to see available "
               "destinations.");
        return false;
    }

    // Check for Nav Beacon special case
    if (dest_str[0] == 'N' || dest_str[0] == 'n') { // Check if already at Nav Beacon
        if (g_state.PlayerNavState.currentLocationType == CELESTIAL_NAV_BEACON) {
            printf("\nAlready at Nav Beacon.");
            return true;
        }
        // Calculate fuel requirement
        double distance_delta =
            fabs(g_state.PlayerNavState.distanceFromStar - g_state.CurrentStarSystem->navBeaconDistance);
        double fuel_required = calculate_travel_fuel_requirement(distance_delta);

        printf("\nTravelling to Nav Beacon... (g_state.Fuel required: %.3f liters)", fuel_required);

        // Pass a dummy non-nullptr pointer for consistency with the function signature
        void *dummy = (void *)&g_state.CurrentStarSystem; // Using any valid address as a dummy
        bool result =
            travel_to_celestial(g_state.CurrentStarSystem, &g_state.PlayerNavState, CELESTIAL_NAV_BEACON, dummy);
        if (result) {
            printf("\nArrived at Nav Beacon (%.2f AU from star)", g_state.PlayerNavState.distanceFromStar);
            return true;
        }
        printf("\nFailed to travel to Nav Beacon.");
        return false;
    }

    // Parse destination index(es) for planets and stations
    int primary_index = -1;
    int secondary_index = -1;

    // Check for format "1.2" (planet.station)
    char *dot_pos = strchr(dest_str, '.');
    if (dot_pos) {
        *dot_pos = '\0'; // Split string at the dot

        // Validate that we have valid digits
        for (char *p = dest_str; *p; p++) {
            if (!isdigit((unsigned char)*p)) {
                printf("\nInvalid planet number: %s. Must be a number.", dest_str);
                return false;
            }
        }

        for (char *p = dot_pos + 1; *p; p++) {
            if (!isdigit((unsigned char)*p)) {
                printf("\nInvalid station number: %s. Must be a number.", dot_pos + 1);
                return false;
            }
        }

        char *end_ptr = nullptr;
        errno = 0;
        long parsed_primary = strtol(dest_str, &end_ptr, 10);
        if (errno == ERANGE || end_ptr == dest_str || *end_ptr != '\0' || parsed_primary > INT_MAX) {
            printf("\nInvalid planet number: %s. Must be a valid number.", dest_str);
            return false;
        }

        errno = 0;
        long parsed_secondary = strtol(dot_pos + 1, &end_ptr, 10);
        if (errno == ERANGE || end_ptr == dot_pos + 1 || *end_ptr != '\0' || parsed_secondary > INT_MAX) {
            printf("\nInvalid station number: %s. Must be a valid number.", dot_pos + 1);
            return false;
        }

        primary_index = (int)parsed_primary;
        secondary_index = (int)parsed_secondary;

        // Validate index ranges
        if (primary_index <= 0) {
            printf("\nInvalid planet number: %d. Must be a positive number.", primary_index);
            return false;
        }

        if (secondary_index <= 0) {
            printf("\nInvalid station number: %d. Must be a positive number.", secondary_index);
            return false;
        }
    } else {
        // For just a planet or star, validate that we have valid digits or '0'
        if (strcmp(dest_str, "0") == 0) {
            primary_index = 0;
        } else {
            for (char *p = dest_str; *p; p++) {
                if (!isdigit((unsigned char)*p)) {
                    printf("\nInvalid destination number: %s. Must be a number or 'N' "
                           "for Nav Beacon.",
                           dest_str);
                    return false;
                }
            }
            char *end_ptr = nullptr;
            errno = 0;
            long parsed_primary = strtol(dest_str, &end_ptr, 10);
            if (errno == ERANGE || end_ptr == dest_str || *end_ptr != '\0' || parsed_primary > INT_MAX) {
                printf("\nInvalid destination number: %s. Must be a valid non-negative number.", dest_str);
                return false;
            }
            primary_index = (int)parsed_primary;

            if (primary_index < 0) {
                printf("\nInvalid destination number: %d. Must be a non-negative number.", primary_index);
                return false;
            }
        }
    }

    // Special case for star (index 0)
    if (primary_index == 0) { // Check if already at star
        if (g_state.PlayerNavState.currentLocationType == CELESTIAL_STAR) {
            printf("\nAlready at %s.", g_state.CurrentStarSystem->centralStar.name);
            return true;
        }
        // Calculate fuel requirement
        double distance_delta = g_state.PlayerNavState.distanceFromStar; // Distance to star is just current distance
        double fuel_required = calculate_travel_fuel_requirement(distance_delta);

        printf("\nTravelling to %s... (g_state.Fuel required: %.3f liters)",
               g_state.CurrentStarSystem->centralStar.name, fuel_required);
        bool result = travel_to_celestial(g_state.CurrentStarSystem, &g_state.PlayerNavState, CELESTIAL_STAR,
                                          &g_state.CurrentStarSystem->centralStar);
        if (result) {
            printf("\nArrived at %s (0.00 AU from star)", g_state.CurrentStarSystem->centralStar.name);
            return true;
        }
        printf("\nFailed to travel to %s.", g_state.CurrentStarSystem->centralStar.name);
        return false;
    }

    // Adjust for 1-based indexing for planets
    primary_index--;

    // Check if planet index is valid
    if (primary_index < 0 || primary_index >= g_state.CurrentStarSystem->numPlanets) {
        printf("\nInvalid destination. planet_tnumber %d does not exist in this "
               "system.",
               primary_index + 1);
        printf("\nThis system has %d planets. Use 'travel' to see available "
               "destinations.",
               g_state.CurrentStarSystem->numPlanets);
        return false;
    }

    planet_t *planet = &g_state.CurrentStarSystem->planets[primary_index];
    if (!planet) {
        printf("\nError: Invalid planet data for planet %d.", primary_index + 1);
        return false;
    }

    // If no secondary index, travel to planet
    if (secondary_index == -1) { // Check if already at this planet
        if (g_state.PlayerNavState.currentLocationType == CELESTIAL_PLANET &&
            g_state.PlayerNavState.currentLocation.planet == planet) {
            printf("\nAlready at %s.", planet->name);
            return true;
        }
        // Calculate fuel requirement
        double distance_delta = fabs(g_state.PlayerNavState.distanceFromStar - planet->orbitalDistance);
        double fuel_required = calculate_travel_fuel_requirement(distance_delta);

        printf("\nTravelling to %s... (g_state.Fuel required: %.3f liters)", planet->name, fuel_required);
        bool result = travel_to_celestial(g_state.CurrentStarSystem, &g_state.PlayerNavState, CELESTIAL_PLANET, planet);
        if (result) {
            printf("\nArrived at %s (%.2f AU from star)", planet->name, g_state.PlayerNavState.distanceFromStar);
            return true;
        }
        printf("\nFailed to travel to %s.", planet->name);
        return false;
    }

    // Adjust for 1-based indexing for stations
    secondary_index--;

    // Check if station index is valid
    if (secondary_index < 0 || secondary_index >= planet->numStations) {
        printf("\nInvalid station. planet_t%s has %d stations (numbered 1 to %d).", planet->name, planet->numStations,
               planet->numStations);
        return false;
    }

    station_t *station = planet->stations[secondary_index];
    if (!station) {
        printf("\nError: station_t data not available for station %d of planet %s.", secondary_index + 1, planet->name);
        return false;
    }
    // Check if already at this station
    if (g_state.PlayerNavState.currentLocationType == CELESTIAL_STATION &&
        g_state.PlayerNavState.currentLocation.station == station) {
        printf("\nAlready at %s.", station->name);
        return true;
    }
    // Calculate fuel requirement
    double station_distance = planet->orbitalDistance + station->orbitalDistance;
    double distance_delta = fabs(g_state.PlayerNavState.distanceFromStar - station_distance);
    double fuel_required = calculate_travel_fuel_requirement(distance_delta);

    printf("\nTravelling to %s... (g_state.Fuel required: %.3f liters)", station->name, fuel_required);
    bool result = travel_to_celestial(g_state.CurrentStarSystem, &g_state.PlayerNavState, CELESTIAL_STATION, station);
    if (result) {
        printf("\nArrived at %s (%.2f AU from star)", station->name, g_state.PlayerNavState.distanceFromStar);
        return true;
    }
    printf("\nFailed to travel to %s.", station->name);
    return false;
}

// Docks with a station if at a station location
[[maybe_unused]] static inline bool do_dock(const char *command_arguments) {
    (void)(command_arguments); // Unused parameter

    // Validate star system data
    if (!g_state.CurrentStarSystem) {
        printf("\nError: Star system data not available. System might not be "
               "properly initialized.");
        return false;
    }

    // Check the player's current location type
    if (g_state.PlayerNavState.currentLocationType != CELESTIAL_STATION) {
        // Provide a helpful message based on current location
        char location_buffer[MAX_LEN];
        get_current_location_name(&g_state.PlayerNavState, location_buffer, sizeof(location_buffer));

        printf("\nCannot dock: Not at a station. You are currently at %s.", location_buffer);
        printf("\nUse 'travel' to navigate to a station first.");

        // List nearby stations as a convenience
        bool stations_found = false;
        printf("\n\nNearby stations:");

        for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets; i++) {
            planet_t *planet = &g_state.CurrentStarSystem->planets[i];
            if (!planet) {
                continue;
            }

            for (uint8_t j = 0; j < planet->numStations; j++) {
                station_t *station = planet->stations[j];
                if (!station) {
                    continue;
                }

                double station_dist = planet->orbitalDistance + station->orbitalDistance;
                double dist_to_station = fabs(g_state.PlayerNavState.distanceFromStar - station_dist);

                // Show stations within 1 AU as "nearby"
                if (dist_to_station <= 1.0) {
                    printf("\n  %s (%.2f AU away) - Use 'travel %d.%d' to reach", station->name, dist_to_station, i + 1,
                           j + 1);
                    stations_found = 1;
                }
            }
        }

        if (!stations_found) {
            printf("\n  No stations within 1 AU. Use 'scan' to find all stations in "
                   "the system.");
        }

        return false;
    }

    // Validate station data
    station_t *station = g_state.PlayerNavState.currentLocation.station;
    if (!station) {
        printf("\nError: station_t data not available. Cannot complete docking "
               "procedure.");
        return false;
    }

    // Find the parent planet for better location context
    planet_t *parent_planet = nullptr;
    for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets && !parent_planet; i++) {
        planet_t *planet = &g_state.CurrentStarSystem->planets[i];
        if (!planet) {
            continue;
        }

        for (uint8_t j = 0; j < planet->numStations; j++) {
            if (planet->stations[j] == station) {
                parent_planet = planet;
                break;
            }
        }
    } // Docking procedure and feedback
    printf("\nDocking at %s...", station->name);

    // Small time delay for docking
    game_time_advance(60); // 1 minute

    // Update the global docking status variable
    g_state.PlayerLocationType = 10; // 10 = docked at station

    printf("\nDocked successfully. Welcome to %s!", station->name);

    // If we have parent planet info, display it
    if (parent_planet) {
        printf("\nLocation: Orbiting %s", parent_planet->name);

        // Update and use this station's market if it has one
        if (station->hasMarket && g_state.CurrentStarSystem->plan_sys_t) {
            // Update the station's market to the current game time
            update_station_market(station, game_time_get_seconds(), parent_planet,
                                  g_state.CurrentStarSystem->plan_sys_t);

            // Set the global market to this station's market
            use_station_market(station, parent_planet, g_state.CurrentStarSystem->plan_sys_t);

            // Show economic specialization
            const char *spec_names[] = {"Balanced", "Industrial", "Agricultural", "Mining"};
            if (station->specialization < 4) {
                printf("\nEconomic specialization: %s", spec_names[station->specialization]);
            }
        }
    }

    // Display available services
    printf("\n\nAvailable services:");
    if (station->hasMarket) {
        printf("\n- Market (use 'mkt', 'buy', 'sell' commands)");
    }
    if (station->hasShipyard) {
        printf("\n- Shipyard (equipment upgrades available)");
    }
    if (station->hasMissions) {
        printf("\n- Mission Board (missions available)");
    }
    if (station->hasDockingComputer) {
        printf("\n- Docking Computer Installation");
    }

    // If no services are available
    if (!station->hasMarket && !station->hasShipyard && !station->hasMissions && !station->hasDockingComputer) {
        printf("\n- No services available at this station");
    }

    // Additional contextual information
    printf("\n\nLocal system time: ");
    char time_buffer[MAX_LEN * 2];
    game_time_get_formatted(time_buffer, sizeof(time_buffer));
    printf("%s", time_buffer);

    return true;
}

/**
 * Lands on a planet surface, similar to docking at a station.
 * This establishes a planetside base for trading and other activities.
 */
[[maybe_unused]] static inline bool do_land(const char *command_arguments) {
    (void)(command_arguments); // Unused parameter

    // Validate star system data

    if (!g_state.CurrentStarSystem) {
        printf("\nError: Star system data not available. System might not be "
               "properly initialized.");
        return false;
    }

    // Check the player's current location type
    if (g_state.PlayerNavState.currentLocationType != CELESTIAL_PLANET ||
        !g_state.PlayerNavState.currentLocation.planet) {
        // Provide a helpful message based on current location
        char location_buffer[MAX_LEN];
        get_current_location_name(&g_state.PlayerNavState, location_buffer, sizeof(location_buffer));

        printf("\nCannot land: Not at a planet. You are currently at %s.", location_buffer);
        printf("\nUse 'travel' to navigate to a planet first.");

        // List nearby planets as a convenience
        bool planets_found = false;
        printf("\n\nNearby planets:");

        for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets; i++) {
            planet_t *planet = &g_state.CurrentStarSystem->planets[i];
            if (!planet) {
                continue;
            }

            double dist_to_planet = fabs(g_state.PlayerNavState.distanceFromStar - planet->orbitalDistance);

            // Show planets within 1 AU as "nearby"
            if (dist_to_planet <= 1.0) {
                printf("\n  %s (%.2f AU away) - Use 'travel %d' to reach", planet->name, dist_to_planet, i + 1);
                planets_found = 1;
            }
        }

        if (!planets_found) {
            printf("\n  No planets within 1 AU. Use 'scan' to find all planets in "
                   "the system.");
        }

        return false;
    }
    // At this point, we're at a planet and can land
    planet_t *planet = g_state.PlayerNavState.currentLocation.planet;

    // Check if the planet type allows landing
    if (planet->type == 2) { // Gas Giant
        printf("\nCannot land on %s: Gas giants have no solid surface to land on.", planet->name);
        printf("\nGas giants are composed primarily of gases and liquids with no accessible solid surface.");
        printf("\nYou can only land on rocky or terrestrial planets.");
        printf("\nUse 'travel' to navigate to a different planet, or dock at an orbital station instead.");

        // Show nearby stations around this gas giant as alternatives
        if (planet->numStations > 0) {
            printf("\n\nOrbital stations around %s:", planet->name);
            for (uint8_t j = 0; j < planet->numStations; j++) {
                station_t *station = planet->stations[j];
                if (station) {
                    double station_dist = planet->orbitalDistance + station->orbitalDistance;
                    double dist_to_station = fabs(g_state.PlayerNavState.distanceFromStar - station_dist);
                    printf("\n  %s (%.3f AU away) - Use 'travel %d.%d' to dock", station->name, dist_to_station,
                           // Find planet index for this planet
                           (int)(planet - g_state.CurrentStarSystem->planets) + 1, j + 1);
                }
            }
        }
        return false;
    }

    if (planet->type == 3) { // Ice Giant
        printf("\nCannot land on %s: Ice giants have no accessible solid surface.", planet->name);
        printf("\nIce giants are composed primarily of water, methane, and ammonia ices in a thick atmosphere.");
        printf("\nAny solid surface is buried under thousands of kilometers of dense atmosphere and ice.");
        printf("\nYou can only land on rocky or terrestrial planets.");
        printf("\nUse 'travel' to navigate to a different planet, or dock at an orbital station instead.");

        // Show nearby stations around this ice giant as alternatives
        if (planet->numStations > 0) {
            printf("\n\nOrbital stations around %s:", planet->name);
            for (uint8_t j = 0; j < planet->numStations; j++) {
                station_t *station = planet->stations[j];
                if (station) {
                    double station_dist = planet->orbitalDistance + station->orbitalDistance;
                    double dist_to_station = fabs(g_state.PlayerNavState.distanceFromStar - station_dist);
                    printf("\n  %s (%.3f AU away) - Use 'travel %d.%d' to dock", station->name, dist_to_station,
                           // Find planet index for this planet
                           (int)(planet - g_state.CurrentStarSystem->planets) + 1, j + 1);
                }
            }
        }
        return false;
    }

    // Landing procedure and feedback
    printf("\nLanding on %s...", planet->name);

    // Small time delay for landing
    game_time_advance(120); // 2 minutes to land

    printf("\nLanded successfully. Welcome to %s!", planet->name);

    // Initialize or update the planet's market
    if (g_state.CurrentStarSystem->plan_sys_t) {

        // Generate or update the planet's market using the correct functions
        if (!planet->planetaryMarket.isInitialized) {
            // Set market fluctuation for this planet
            planet->marketFluctuation = (g_state.CurrentStarSystem->plan_sys_t->goatSoupSeed.c + planet->type) % 16;
            planet->lastMarketUpdate = game_time_get_seconds();

            // Create a temporary station to use the market generation function
            station_t temp_station;
            memset(&temp_station, 0, sizeof(station_t));
            temp_station.marketFluctuation = planet->marketFluctuation;

            // Set specialization based on planet type
            if (planet->type <= 1) {             // Rocky or Terrestrial
                temp_station.specialization = 2; // Agricultural focus for terrestrial planets
            } else {
                temp_station.specialization = 3; // Mining focus for gas giants and ice planets
            }

            // Generate market and store in planet's market
            temp_station.market = generate_station_market(&temp_station, planet, g_state.CurrentStarSystem->plan_sys_t);
            planet->planetaryMarket.market = temp_station.market;
            planet->planetaryMarket.isInitialized = 1;
        } else {
            // Update existing market based on elapsed time
            uint64_t current_time = game_time_get_seconds();

            // Only update if sufficient time has passed (at least 1 hour of game
            // time)
            const uint64_t UPDATE_INTERVAL = 3600; // 1 hour in seconds

            if (current_time - planet->lastMarketUpdate >= UPDATE_INTERVAL) {
                // Create temporary station for market update
                station_t temp_station;
                memset(&temp_station, 0, sizeof(station_t));
                temp_station.marketFluctuation = planet->marketFluctuation;
                temp_station.market = planet->planetaryMarket.market;
                temp_station.lastMarketUpdate = planet->lastMarketUpdate;

                // Set specialization based on planet type
                if (planet->type <= 1) {             // Rocky or Terrestrial
                    temp_station.specialization = 2; // Agricultural focus
                } else {
                    temp_station.specialization = 3; // Mining focus
                }

                // Update market using station market update function
                update_station_market(&temp_station, current_time, planet, g_state.CurrentStarSystem->plan_sys_t);

                // Store updated market back in planet
                planet->planetaryMarket.market = temp_station.market;
                planet->lastMarketUpdate = current_time;
            }
        }

        // Set the local market to the planet's market
        g_state.LocalMarket = planet->planetaryMarket.market;

        // Show information about the planet
        printf("\n\n=== PLANET INFORMATION ===");

        // Display planet type
        const char *planet_types[] = {"Rocky/Airless", "Terrestrial", "Gas Giant", "Ice Planet"};
        if (planet->type < 4) {
            printf("\nPlanet Type: %s", planet_types[planet->type]);
        }

        // Show economy information
        printf("\nSystem Economy: %s", g_econ_names[g_state.CurrentStarSystem->plan_sys_t->economy]);

        // Display resource specialization based on planet type
        const char *resource_types[] = {"Minerals", "Agriculture", "Gases", "Rare Elements"};
        printf("\nMain Resources: %s", resource_types[planet->type % 4]);

        // Display market update time
        uint64_t time_since_update = game_time_get_seconds() - planet->lastMarketUpdate;
        printf("\nLast Market Update: %llu seconds ago", (unsigned long long)time_since_update);

        printf("\n\nTrading post established. Use 'mkt' to view available goods.");
    }

    return true;
}

[[maybe_unused]] static inline void update_all_system_markets() {
    // Check if star system data is properly initialized

    if (!g_state.CurrentStarSystem) {
        return;
    }

    // Get current game time
    uint64_t current_time = game_time_get_seconds();

    // Update markets for all stations in the system
    for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets; i++) {
        planet_t *planet = &g_state.CurrentStarSystem->planets[i];
        if (!planet) {
            continue;
        }

        for (uint8_t j = 0; j < planet->numStations; j++) {
            station_t *station = planet->stations[j];
            if (!station) {
                continue;
            }

            // Update this station's market
            update_station_market(station, current_time, planet, g_state.CurrentStarSystem->plan_sys_t);
        }
    }
}

// Compare markets across different stations in the system or with the planet
// market
[[maybe_unused]] static inline bool do_compare_markets(const char *command_arguments) {
    (void)command_arguments;                                                  // Mark as unused
    if (!g_state.CurrentStarSystem || !g_state.CurrentStarSystem->plan_sys_t) // Added plan_sys_t check for safety
    {
        printf("\\nError: Star system data not available for market comparison.");
        return false;
    }

    market_type_t base_market_to_compare; // Changed MarketInfo to MarketType
    char base_location_name[MAX_LEN];
    bool is_planet_base = 0; // Initialize isPlanetBase

    // Determine the base market for comparison
    if (g_state.PlayerNavState.currentLocationType == CELESTIAL_PLANET &&
        g_state.PlayerNavState.currentLocation.planet) {
        planet_t *current_planet = g_state.PlayerNavState.currentLocation.planet;
        if (!current_planet) {
            printf("\nError: Current planet data is invalid for comparison.");
            return false;
        }

        if (safe_snprintf(base_location_name, MAX_LEN, "%s", current_planet->name) < 0) {
            printf("\nError: Failed to determine the current planet name.");
            return false;
        }
        is_planet_base = true;

        // Ensure the planetary market is initialized and up-to-date.
        // UpdatePlanetaryMarket handles both initialization and updates.
        // Use local market data since we're at the planet
        base_market_to_compare = g_state.LocalMarket;
    } else if (g_state.PlayerNavState.currentLocationType == CELESTIAL_STATION &&
               g_state.PlayerNavState.currentLocation.station) {
        station_t *current_station = g_state.PlayerNavState.currentLocation.station;
        if (!current_station) {
            printf("\nError: Current station data is invalid for comparison.");
            return false;
        }
        if (safe_snprintf(base_location_name, MAX_LEN, "%s", current_station->name) < 0) {
            printf("\nError: Failed to determine the current station name.");
            return false;
        }
        is_planet_base = false;

        planet_t *orbiting_planet = nullptr;
        // Find the planet this station orbits for market context
        for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets; ++i) {
            planet_t *current_planet = &g_state.CurrentStarSystem->planets[i];
            if (!current_planet) {
                continue;
            }
            for (uint8_t j = 0; j < current_planet->numStations; ++j) {
                if (current_planet->stations[j] == current_station) {
                    orbiting_planet = current_planet;
                    break;
                }
            }
            if (orbiting_planet) {
                break;
            }
        }

        if (orbiting_planet) {
            // Ensure the station market is initialized and up-to-date.
            update_station_market(current_station, game_time_get_seconds(), orbiting_planet,
                                  g_state.CurrentStarSystem->plan_sys_t);
            // Use the local market (which should be set to this station's market)
            base_market_to_compare = g_state.LocalMarket;
        } else {
            printf("\nError: Could not determine orbiting planet for station %s. "
                   "Using potentially stale local market data.",
                   current_station->name);
            // Fallback to g_state.LocalMarket if orbiting planet not found.
            base_market_to_compare = g_state.LocalMarket;
        }
    } else {
        printf("\nYou must be docked at a station or landed on a planet to compare "
               "markets.");
        return false;
    }

    printf("\n=== MARKET COMPARISON ===");
    printf("\nBase location: %s", base_location_name);

    bool found_stations_to_compare = false;
    for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets; i++) {
        planet_t *planet = &g_state.CurrentStarSystem->planets[i];
        if (!planet) {
            continue;
        }

        for (uint8_t j = 0; j < planet->numStations; j++) {
            station_t *station = planet->stations[j];
            if (!station || !station->hasMarket) {
                continue; // Skip stations without markets
            }

            // Skip comparing base station to itself if the base is a station
            if (!is_planet_base && g_state.PlayerNavState.currentLocation.station == station) {
                continue;
            }

            found_stations_to_compare = 1;
            // Update the "other" station's market to current time to ensure fair
            // comparison
            update_station_market(station, game_time_get_seconds(), planet, g_state.CurrentStarSystem->plan_sys_t);

            // Create a temporary market for comparison
            market_type_t other_market;

            // Use the current station's market
            // This assumes update_station_market updates the station's market data
            // directly
            use_station_market(station, planet, g_state.CurrentStarSystem->plan_sys_t);
            other_market = station->market;

            printf("\n\nStation: %s (Orbiting %s)", station->name, planet->name);
            printf("\n-----------------------------------");
            printf("\n%-12s %-8s %-8s %-8s %-8s", "Commodity", "Base", "Other", "Diff", "QtyDiff");

            for (uint16_t k = 0; k <= LAST_TRADE; k++) {
                // Skip invalid commodities
                if (g_commodities[k].basePrice == 0)
                    continue;

                double base_price = base_market_to_compare.price[k]; // Changed Price to price
                int base_qty = base_market_to_compare.quantity[k];   // Changed Quantity to quantity

                double other_price = other_market.price[k]; // Changed Price to price
                int other_qty = other_market.quantity[k];   // Changed Quantity to quantity

                printf("\n%-12s %-8.1f %-8.1f %-8.1f %-8d", g_state.tradnames[k], base_price / 10.0, other_price / 10.0,
                       (other_price - base_price) / 10.0, other_qty - base_qty);
            }
        }
    }

    if (!found_stations_to_compare) {
        if (is_planet_base) {
            printf("\n\nNo other stations in the system with markets to compare "
                   "against %s.",
                   base_location_name);
        } else {
            printf("\n\nNo other stations in the system with markets to compare "
                   "against your current station %s.",
                   base_location_name);
            printf("\nOr you are at the only station with a market.");
        }
    }

    if (is_planet_base) {
        printf("\n\nNote: Comparing all stations in the system to the planet "
               "market at %s.",
               base_location_name);
    } else {
        printf("\n\nNote: Comparing all other stations in the system to your "
               "current station %s.",
               base_location_name);
    }

    // Restore the original local market
    if (!is_planet_base && g_state.PlayerNavState.currentLocation.station) {
        // Find planet for current station
        planet_t *current_planet = nullptr;
        station_t *current_station = g_state.PlayerNavState.currentLocation.station;

        for (uint8_t i = 0; i < g_state.CurrentStarSystem->numPlanets && !current_planet; i++) {
            planet_t *p = &g_state.CurrentStarSystem->planets[i];
            if (!p) {
                continue;
            }

            for (uint8_t j = 0; j < p->numStations; j++) {
                if (p->stations[j] == current_station) {
                    current_planet = p;
                    break;
                }
            }
        }

        if (current_planet) {
            use_station_market(current_station, current_planet, g_state.CurrentStarSystem->plan_sys_t);
        }
    }

    return true;
}

[[maybe_unused]] static inline bool do_ship_status(const char *command_arguments) {
    (void)(command_arguments); // Mark commandArguments as unused

    if (g_state.PlayerShipPtr == nullptr) {
        printf("\nError: Ship data is not available.");
        return false;
    }

    // Note: We don't synchronize fuel here as the ship's fuel value should be
    // more precise and is the source of truth after travel operations

    // Display basic ship information
    printf("\n=== Ship Status: %s (%s) ===", g_state.PlayerShipPtr->shipName, g_state.PlayerShipPtr->shipClassName);

    // Display hull
    int hull_percentage =
        (g_state.PlayerShipPtr->attributes.hullStrength * 100) / g_state.PlayerShipPtr->ship_type_t->baseHullStrength;
    printf("\nHull Integrity: %d%%", hull_percentage);
    // Display fuel
    double current_fuel_ly = g_state.PlayerShipPtr->attributes.fuelLiters / 100.0;
    double max_fuel_ly = g_state.PlayerShipPtr->ship_type_t->maxFuelLY;
    double fuel_percent = (current_fuel_ly / max_fuel_ly) * 100.0;

    printf("\nFuel: %.1f/%.1f LY (%.0F%%) - Consumption: %.1f CR per 0.1 LY", current_fuel_ly, max_fuel_ly,
           fuel_percent, g_state.PlayerShipPtr->ship_type_t->fuelConsumptionRate / 10.0);

    // Display cargo
    printf("\nCargo Capacity: %d/%d tons", g_state.PlayerShipPtr->attributes.currentCargoTons,
           g_state.PlayerShipPtr->attributes.cargoCapacityTons);
    // Display equipment
    printf("\n\n=== Equipment ===");
    bool has_equipment = false;
    // Check all equipment slots for active equipment
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
        if (g_state.PlayerShipPtr->equipment[i].isActive && strlen(g_state.PlayerShipPtr->equipment[i].name) > 0 &&
            strcmp(g_state.PlayerShipPtr->equipment[i].name, "Empty") != 0) {

            has_equipment = true;
            printf("\n  - %s", g_state.PlayerShipPtr->equipment[i].name);
        }
    }

    if (!has_equipment) {
        printf("\n  No active equipment.");
    }

    printf("\n");
    return true;
}

[[maybe_unused]] static inline bool do_repair(const char *command_arguments) {
    (void)(command_arguments); // Mark commandArguments as unused

    if (g_state.PlayerShipPtr == nullptr) {
        printf("\nError: Ship data is not available.");
        return false;
    }

    // Check if repair is needed
    if (g_state.PlayerShipPtr->attributes.hullStrength >= g_state.PlayerShipPtr->ship_type_t->baseHullStrength) {
        printf("\nYour ship doesn't need any repairs.");
        return true;
    }

    // Calculate repair cost - 10 credits per unit of hull damage
    int damage_amount =
        g_state.PlayerShipPtr->ship_type_t->baseHullStrength - g_state.PlayerShipPtr->attributes.hullStrength;
    int repair_cost = damage_amount * 10;

    // Check if player can afford repairs
    if (g_state.Cash < repair_cost * 10) // Convert to internal units
    {
        printf("\nYou can't afford the repairs. Cost: %.1f credits", (double)repair_cost);
        return false;
    }

    // Perform the repair
    g_state.Cash -= repair_cost * 10; // Convert to internal units
    g_state.PlayerShipPtr->attributes.hullStrength = g_state.PlayerShipPtr->ship_type_t->baseHullStrength;

    printf("\nShip repaired for %.1f credits. Hull integrity restored to 100%%.", (double)repair_cost);
    return true;
}

[[maybe_unused]] static inline bool do_ship_details(const char *command_arguments) {
    (void)(command_arguments); // Mark commandArguments as unused

    if (g_state.PlayerShipPtr == nullptr) {
        printf("\nError: Ship data is not available.");
        return false;
    }

    // Synchronize ship fuel with global state before displaying details
    g_state.PlayerShipPtr->attributes.fuelLiters = g_state.Fuel * 10.0; // Convert game units to liters

    // Call the detailed ship status display function from elite_ship_types.h
    display_ship_status(g_state.PlayerShipPtr);
    return true;
}

/**
 * Command to purchase and install equipment on the player's ship.
 * Usage: equip <equipment_name>
 * Available equipment depends on the current system's tech level.
 */
[[maybe_unused]] static inline bool do_purchase_equipment(const char *command_arguments) {
    if (g_state.PlayerShipPtr == nullptr) {
        printf("\nError: Ship data not available.");
        return false;
    }

    // Check if we are docked at a station
    if (g_state.PlayerNavState.currentLocationType != CELESTIAL_STATION) {
        printf("\nYou must be docked at a station to purchase equipment.");
        return false;
    }

    // Check if an equipment name was provided
    if (command_arguments == nullptr || command_arguments[0] == '\0') {
        printf("\nUsage: equip <equipment_name>");
        printf("\n\nAvailable Equipment:");
        printf("\n- ecm          - Electronic Counter Measures (600 CR)");
        printf("\n- fuelscoop    - g_state.Fuel Scoop (525 CR)");
        printf("\n- dockcomp     - Docking Computer (1500 CR)");
        printf("\n- escape       - Escape Pod (1000 CR)");
        printf("\n- cargo        - Cargo Bay Extension (400 CR)");
        printf("\n- pulse        - Pulse Laser (400 CR)");
        printf("\n- beam         - Beam Laser (1000 CR)");
        printf("\n- military     - Military Laser (2500 CR)");
        printf("\n- mining       - Mining Laser (800 CR)");
        printf("\n- scanner      - Scanner Upgrade (700 CR)");
        printf("\n- missile      - Homing Missile (300 CR)");
        printf("\n  Example: equip beam");
        printf("\n  Note: You must be docked at a station to purchase equipment");
        printf("\n        Equipment availability depends on the system's tech level");
        return true;
    }
    // Normalize input to lowercase for case-insensitive matching
    char equip_name[MAX_LEN];
    safe_snprintf(equip_name, MAX_LEN, "%s", command_arguments);

    // Convert to lowercase
    for (char *p = equip_name; *p; ++p) {
        *p = (char)tolower((unsigned char)*p);
    }

    // Get current system tech level (0-based index)
    int tech_level = g_state.Galaxy[g_state.CurrentPlanet].techLev;
    // Prepare equipment parameters for purchase
    equipment_type_specifics_t equip_type;
    equipment_slot_type_t slot_type = EQUIPMENT_SLOT_TYPE_NONE;
    const char *formal_name = nullptr;
    int cost = 0;
    int required_tech_level = 0;
    double damage_output = 0.0;

    // Match equipment name to available options
    if (strcmp(equip_name, "ecm") == 0) {
        equip_type.defensiveType = DEFENSIVE_SYSTEM_TYPE_ECM;
        slot_type = EQUIPMENT_SLOT_TYPE_DEFENSIVE_1;
        formal_name = "ECM System";
        cost = COST_ECM;
        required_tech_level = 2;
    } else if (strcmp(equip_name, "fuelscoop") == 0) {
        equip_type.utilityType = UTILITY_SYSTEM_TYPE_FUEL_SCOOPS;
        slot_type = UTILITY_SYSTEM_1;
        formal_name = "g_state.Fuel Scoop";
        cost = COST_FUEL_SCOOPS;
        required_tech_level = 3;
    } else if (strcmp(equip_name, "dockcomp") == 0) {
        equip_type.utilityType = UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER;
        slot_type = UTILITY_SYSTEM_2;
        formal_name = "Docking Computer";
        cost = COST_DOCKING_COMPUTER;
        required_tech_level = 5;
    } else if (strcmp(equip_name, "escape") == 0) {
        equip_type.utilityType = UTILITY_SYSTEM_TYPE_ESCAPE_POD;
        slot_type = UTILITY_SYSTEM_3;
        formal_name = "Escape Pod";
        cost = COST_ESCAPE_POD;
        required_tech_level = 5;
    } else if (strcmp(equip_name, "cargo") == 0) {
        equip_type.utilityType = UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION;
        slot_type = UTILITY_SYSTEM_4;
        formal_name = "Cargo Bay Extension";
        cost = COST_CARGO_BAY_EXTENSION;
        required_tech_level = 1;
        // Apply the cargo upgrade directly
        if (g_state.Cash < cost) {
            printf("\nInsufficient credits to purchase Cargo Bay Extension. "
                   "Required: %d, Available: %.1f",
                   cost, (double)g_state.Cash / 10.0);
            return false;
        }

        if (tech_level < required_tech_level) {
            printf("\nCargo Bay Extensions not available at this tech level. "
                   "Required: %d, Current: %d",
                   required_tech_level + 1, tech_level + 1);
            return false;
        }
        // Directly apply the upgrade
        g_state.Cash -= cost;
        g_state.PlayerShipPtr->attributes.cargoCapacityTons += CARGO_BAY_EXTENSION_CAPACITY;

        // Update equipment mapping
        map_equipment_indices(g_state.PlayerShipPtr);

        printf("\nCargo Bay Extension installed. New capacity: %d tonnes.",
               g_state.PlayerShipPtr->attributes.cargoCapacityTons);
        return true;
    } else if (strcmp(equip_name, "pulse") == 0) {
        equip_type.weaponType = WEAPON_TYPE_PULSE_LASER;
        slot_type = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
        formal_name = "Pulse Laser";
        cost = COST_PULSE_LASER;
        required_tech_level = 1;
        damage_output = 5.0;
    } else if (strcmp(equip_name, "beam") == 0) {
        equip_type.weaponType = WEAPON_TYPE_BEAM_LASER;
        slot_type = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
        formal_name = "Beam Laser";
        cost = COST_BEAM_LASER;
        required_tech_level = 3;
        damage_output = 7.5;
    } else if (strcmp(equip_name, "military") == 0) {
        equip_type.weaponType = WEAPON_TYPE_MILITARY_LASER;
        slot_type = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
        formal_name = "Military Laser";
        cost = COST_MILITARY_LASER;
        required_tech_level = 6;
        damage_output = 10.0;
    } else if (strcmp(equip_name, "mining") == 0) {
        equip_type.weaponType = WEAPON_TYPE_MINING_LASER;
        slot_type = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
        formal_name = "Mining Laser";
        cost = COST_MINING_LASER;
        required_tech_level = 2;
        damage_output = 3.0;
    } else if (strcmp(equip_name, "scanner") == 0) {
        equip_type.utilityType = UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE;
        slot_type = UTILITY_SYSTEM_3;
        formal_name = "Advanced Scanner";
        cost = COST_SCANNER_UPGRADE;
        required_tech_level = 4;
    } else if (strcmp(equip_name, "missile") == 0) {
        // For missiles, we just add to the count
        if (g_state.PlayerShipPtr->attributes.missilesLoadedHoming >=
            g_state.PlayerShipPtr->attributes.missilePylons * MISSILE_PYLON_CAPACITY) {
            printf("\nCannot purchase more missiles. All pylons are full.");
            return false;
        }

        if (g_state.Cash < COST_MISSILE_HOMING * 10) {
            printf("\nInsufficient credits to purchase missile. Required: %d, "
                   "Available: %.1f",
                   COST_MISSILE_HOMING, (double)g_state.Cash / 10.0);
            return false;
        }

        g_state.Cash -= COST_MISSILE_HOMING * 10;
        g_state.PlayerShipPtr->attributes.missilesLoadedHoming++;
        printf("\nMissile purchased. Current missile count: %d/%d",
               g_state.PlayerShipPtr->attributes.missilesLoadedHoming,
               g_state.PlayerShipPtr->attributes.missilePylons * MISSILE_PYLON_CAPACITY);
        return true;
    } else {
        printf("\nUnknown equipment: %s", equip_name);
        printf("\nUse 'equip' without parameters to see available equipment.");
        return false;
    }
    // Attempt to purchase the selected equipment
    bool result = PurchaseEquipment(g_state.PlayerShipPtr, formal_name, slot_type, equip_type, cost,
                                    required_tech_level, damage_output);

    return result;
}

/**
 * Displays the equipment inventory of the player's ship.
 *
 * @param commandArguments Arguments provided to the command (unused)
 * @return 1 if the command was processed successfully
 */
[[maybe_unused]] static inline bool do_inventory_display(const char *command_arguments) {
    (void)command_arguments; // Mark as unused

    if (g_state.PlayerShipPtr == nullptr) {
        printf("\nError: Ship data not available.");
        return false;
    }

    list_equipment_inventory(g_state.PlayerShipPtr);
    return true;
}

/**
 * Stores equipment from a specified slot into the inventory.
 *
 * @param commandArguments Arguments provided to the command (slot number)
 * @return 1 if the equipment was stored successfully
 */
[[maybe_unused]] static inline bool do_store_equipment(const char *command_arguments) {
    if (g_state.PlayerShipPtr == nullptr) {
        printf("\nError: Ship data not available.");
        return false;
    }

    // Check if we're in combat
    if (g_state.InCombat) {
        printf("\nCannot modify ship configuration during combat.");
        return false;
    }

    // Check if a slot number was provided
    if (command_arguments == nullptr || command_arguments[0] == '\0') {
        printf("\nUsage: store <slot_number>");
        printf("\n\nAvailable Equipment Slots:");
        print_equipment_slots(g_state.PlayerShipPtr);
        return false;
    }

    // Parse the slot number
    char *endptr = nullptr;
    errno = 0;
    long parsed_slot_number = strtol(command_arguments, &endptr, 10);

    // Check if the slot number is valid
    if (endptr == command_arguments || *endptr != '\0' || errno == ERANGE || parsed_slot_number < 0 ||
        parsed_slot_number >= MAX_EQUIPMENT_SLOTS) {
        printf("\nInvalid slot number. Valid range: 0-%d", MAX_EQUIPMENT_SLOTS - 1);
        return false;
    }

    int slot_number = (int)parsed_slot_number;

    // Try to store the equipment
    return remove_equipment_to_inventory(g_state.PlayerShipPtr, (equipment_slot_type_t)slot_number);
}

/**
 * Equips an item from inventory into a specified slot.
 *
 * @param commandArguments Arguments provided to the command (inventory_index
 * slot_number)
 * @return 1 if the equipment was equipped successfully
 */
[[maybe_unused]] static inline bool do_equip_from_inventory(const char *command_arguments) {
    if (g_state.PlayerShipPtr == nullptr) {
        printf("\nError: Ship data not available.");
        return false;
    }

    // Check if we're in combat
    if (g_state.InCombat) {
        printf("\nCannot modify ship configuration during combat.");
        return false;
    }
    // Check if arguments were provided
    if (command_arguments == nullptr || command_arguments[0] == '\0') {
        printf("\nUsage: use <inventory_index> <slot_number>\n");
        printf("Example: use 0 1  (equips item from inventory slot 0 to equipment "
               "slot 1)\n");
        printf("\nUse 'inv' command to view your inventory and 'shipinfo' to see "
               "available slots.\n");
        return false;
    } // Parse the arguments - we need two numbers: inventory index and slot
    // number
    char arg_copy[MAX_LEN];
    safe_snprintf(arg_copy, sizeof(arg_copy), "%s", command_arguments);
    char arg1[MAX_LEN];
    char arg2[MAX_LEN];
    int invIndex = -1;
    int slotNumber = -1;
    // Extract the arguments
    char *saveptr;
    char *token = safe_strtok(arg_copy, " \t", &saveptr);
    if (token != nullptr) {
        safe_snprintf(arg1, MAX_LEN, "%s", token);
        invIndex = atoi(arg1);

        token = safe_strtok(nullptr, " \t", &saveptr);
        if (token != nullptr) {
            safe_snprintf(arg2, MAX_LEN, "%s", token);
            slotNumber = atoi(arg2);
        } else {
            printf("\nUsage: use <inventory_index> <slot_number>\n");
            printf("Example: use 0 1  (equips item from inventory slot 0 to "
                   "equipment slot 1)\n");
            printf("\nUse 'inv' command to view your inventory and 'shipinfo' to see "
                   "available slots.\n");
            return false;
        }
    } // Check if both arguments were provided and are valid
    if (invIndex < 0 || invIndex >= MAX_EQUIPMENT_INVENTORY) {
        printf("\nInvalid inventory index. Valid range: 0-%d\n", MAX_EQUIPMENT_INVENTORY - 1);
        return false;
    }

    if (slotNumber < 0 || slotNumber >= MAX_EQUIPMENT_SLOTS) {
        printf("\nInvalid slot number. Valid range: 0-%d\n", MAX_EQUIPMENT_SLOTS - 1);
        return false;
    }

    // Try to equip the item from inventory
    return equip_from_inventory(g_state.PlayerShipPtr, invIndex, slotNumber);
}

// Ship trading commands
[[maybe_unused]] static inline bool do_shipyard(const char *args) {
    (void)args; // Mark args as unused

    // Check if player is docked at a station

    // Need to be both at a station AND docked
    if (g_state.PlayerNavState.currentLocationType != CELESTIAL_STATION || g_state.PlayerLocationType != 10) {
        printf("Error: You must be docked at a station to access the shipyard.\n");
        return false;
    }

    // Get current system info

    // Display the shipyard
    display_shipyard(g_state.CurrentSystemName, g_state.CurrentSystemEconomy, g_state.PlayerShipPtr,
                     g_state.currentGameTimeSeconds);

    return true;
}

[[maybe_unused]] static inline bool do_compareship(const char *args) {
    // Check if arguments are provided
    if (args == nullptr || args[0] == '\0') {
        printf("Error: Please specify a ship to compare with.\n");
        printf("Usage: compareship <shipname>\n");
        return false;
    }

    // Get player ship

    // Compare ships
    compare_ships(g_state.PlayerShipPtr, (const char *)args);

    return true;
}

[[maybe_unused]] static inline bool do_buyship(const char *args) {
    // Check if player is docked at a station

    // Need to be both at a station AND docked
    if (g_state.PlayerNavState.currentLocationType != CELESTIAL_STATION || g_state.PlayerLocationType != 10) {
        printf("Error: You must be docked at a station to purchase a ship.\n");
        return false;
    }

    // Check if arguments are provided
    if (args == nullptr || args[0] == '\0') {
        printf("Error: Please specify a ship to buy.\n");
        printf("Usage: buyship <ID or shipname> [notrade]\n");
        printf("Example: buyship 1  or  buyship \"Cobra Mk III\"\n");
        return false;
    }

    // Get current system info and player ship

    // Parse arguments
    char shipNameOrID[64] = {0};
    bool tradeIn = 1;
    // Copy the first part of the arguments (up to the first space)
    const char *space = strchr(args, ' ');
    if (space != nullptr) {
        size_t nameLen = space - args;
        nameLen = (nameLen < 63) ? nameLen : 63;
        safe_snprintf(shipNameOrID, nameLen + 1, "%.*s", (int)nameLen, args);

        // Check for 'notrade' flag in the remaining part
        if (strstr(space + 1, "notrade") != nullptr) {
            tradeIn = 0;
        }
    } else {
        // No space, just copy the entire argument
        safe_snprintf(shipNameOrID, sizeof(shipNameOrID), "%s", args);
    }

    // Check if the argument is a number (ID) or a string (ship name)
    char actualShipName[MAX_SHIP_NAME_LENGTH] = {0};
    bool isID = 1;

    // Check if shipNameOrID is a number
    for (size_t i = 0; i < strlen(shipNameOrID); i++) {
        if (!isdigit(shipNameOrID[i])) {
            isID = 0;
            break;
        }
    }

    if (isID) {
        // Convert the ID to an integer
        int ship_id = atoi(shipNameOrID);

        // Get the ship name by ID
        if (!get_ship_name_by_id(g_state.CurrentSystemName, g_state.CurrentSystemEconomy, ship_id, actualShipName,
                                 MAX_SHIP_NAME_LENGTH)) {
            printf("Error: Invalid ship ID: %d\n", ship_id);
            return false;
        }
    } else {
        // The argument is a ship name, just copy it
        int name_length = safe_snprintf(actualShipName, MAX_SHIP_NAME_LENGTH, "%s", shipNameOrID);
        if (name_length < 0 || name_length >= MAX_SHIP_NAME_LENGTH) {
            printf("Error: Ship name is too long or could not be copied.\n");
            return false;
        }
    }
    // Buy the new ship
    return buy_new_ship(g_state.CurrentSystemName, g_state.CurrentSystemEconomy, g_state.PlayerShipPtr,
                        (const char *)actualShipName, g_state.currentGameTimeSeconds, tradeIn);
}

/**
 * Command handler for the 'upgrade' command
 * Shows available ship upgrades or purchases a specific upgrade
 */
[[maybe_unused]] static inline bool do_upgrade(const char *command_arguments) {
    return UpgradeCommand(command_arguments);
}

/**
 * Display detailed fuel information for the current ship
 */
[[maybe_unused]] static inline bool show_fuel_status(const char *command_arguments) {
    // Unused parameter
    (void)command_arguments;

    // Forward declaration for function from elite_player_state.h

    // Call the function that displays detailed fuel information
    display_ship_fuel_status();
    return true;
}

/**
 * Command to jettison cargo from the ship (discard cargo into space).
 * Usage: jettison <cargo_name> <quantity>
 *        jettison all
 * This will remove the specified cargo from the ship without receiving payment.
 * Using "jettison all" will discard all cargo currently in the ship.
 * Useful in emergencies or when carrying illegal goods and avoiding
 * authorities.
 *
 * @param commandArguments Arguments provided to the command
 * @return 1 if the cargo was successfully jettisoned
 */
[[maybe_unused]] static inline bool do_jettison(const char *command_arguments) {
    if (g_state.PlayerShipPtr == nullptr) {
        printf("\nError: Ship data not available.");
        return false;
    }

    if (command_arguments == nullptr || strlen(command_arguments) == 0) {
        printf("\nUsage: jettison <cargo_name> <quantity>");
        printf("\nUsage: jettison all");
        printf("\nExample: jettison Food 5");
        return false;
    }

    // Check if the "all" flag was used
    if (StringCompareIgnoreCase(command_arguments, "all") == 0) {
        // Special case: jettison all cargo
        return jettison_all_cargo(g_state.PlayerShipPtr);
    } // Parse the arguments
    char cargoName[MAX_LEN];
    char quantityStr[MAX_LEN];
    char arg_copy[MAX_LEN];
    safe_snprintf(arg_copy, sizeof(arg_copy), "%s", command_arguments);

    // Split the command arguments to get the cargo name
    split_string_at_first_space(arg_copy, cargoName);

    // Get the quantity part
    char *trimmed_qty = strip_leading_trailing_spaces(arg_copy);

    // If quantity is not provided, default to 1
    int quantity = 1;
    if (trimmed_qty != nullptr && strlen(trimmed_qty) > 0) {
        int quantity_length = safe_snprintf(quantityStr, sizeof(quantityStr), "%s", trimmed_qty);
        if (quantity_length < 0 || quantity_length >= (int)sizeof(quantityStr)) {
            printf("\nInvalid quantity.");
            return false;
        }
        quantity = atoi(quantityStr);
    }
    // Verify quantity is valid
    if (quantity <= 0) {
        printf("\nInvalid quantity. Please specify a positive number.");
        return false;
    }
    // No need to modify cargo name capitalization since we use case-insensitive
    // comparison The StringCompareIgnoreCase function will handle different
    // capitalizations

    // First, check if the cargo exists in the player's ship
    bool cargoFound = 0;
    if (!get_cargo_quantity(g_state.PlayerShipPtr, cargoName)) {
        printf("\nError: %s not found in cargo hold.", cargoName);
        return false;
    }

    // Find the cargo index in the global g_state.tradnames array
    for (uint16_t i = 0; i <= LAST_TRADE; i++) {
        if (StringCompareIgnoreCase(g_state.tradnames[i], cargoName) == 0) {
            cargoFound = 1;
            break;
        }
    }

    if (!cargoFound) {
        printf("\nError: Unable to find cargo in global inventory. Please report "
               "this bug.");
        return false;
    }

    // Call the JettisonCargo function to remove from player ship
    if (jettison_cargo(g_state.PlayerShipPtr, cargoName, quantity)) {
        printf("\nSuccessfully jettisoned %d units of %s.", quantity, cargoName);
        return true;
    }
    printf("\nFailed to jettison %s. Check cargo name and quantity.", cargoName);
    return false;
}
