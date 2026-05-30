#include <stdlib.h>
#include <string.h>
/* txtelite.c  1.5 */
/* Textual version of Elite trading (C implementation) */
/* Converted by Ian Bell from 6502 Elite s      if (CheckEquipmentActive(PlayerShipPtr, EQUIP_ECM))
        safe_strcat(equipmentStatus, sizeof(equipmentStatus), "ECM ");
      if (CheckEquipmentActive(PlayerShipPtr, EQUIP_FUEL_SCOOP))
        safe_strcat(equipmentStatus, sizeof(equipmentStatus), "FuelScoop ");
      if (CheckEquipmentActive(PlayerShipPtr, EQUIP_DOCKING_COMPUTER))
        safe_strcat(equipmentStatus, sizeof(equipmentStatus), "DockCmp ");
   Original 6502 Elite by Ian Bell & David Braben. */

/* ----------------------------------------------------------------------
  The nature of basic mechanisms used to generate the Elite socio-economic
universe are now widely known. A competant games programmer should be able to
produce equivalent functionality. A competant hacker should be able to lift
the exact system from the object code base of official conversions.

  This file may be regarded as defining the Classic Elite universe.

  It contains a C implementation of the precise 6502 algorithms used in the
 original BBC Micro version of Acornsoft Elite (apart from Galctic Hyperspace
 target systems) together with a parsed textual command testbed.

  Note that this is not the universe of David Braben's 'Frontier' series.


ICGB 13/10/99	; 21/07/15
ian@ianbell.me
www.ianbellelite.com
  ---------------------------------------------------------------------- */

/* Note that this program is "quick-hack" text parser-driven version
of Elite with no combat or missions.
*/

#include <inttypes.h> // For PRIu64 and other format macros
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "elite_command_handler.h"     // For command parsing
#include "elite_commands.h"            // For game commands
#include "elite_equipment_constants.h" // For equipment indices
#include "elite_player_state.h"        // For player state initialization
#include "elite_player_ship.h"    // For ship initialization and status functions
#include "elite_ship_upgrades.h" // For equipment access
#include "elite_star_system.h"   // For star system data
#include "elite_state.h" // Unified header for constants, structures, and globals
#include "elite_navigation.h" // For NavigationState definition
#include "elite_utils.h" // For string handling and other utilities
#include "platform_compat.h"

// =====================================
// Global State Definition
// =====================================
GameState g_state = {
    .ExitStatus = EXIT_SUCCESS,
    .CurrentSystemName = "Lave",
    .CurrentSystemEconomy = 0,
    .PlayerLocationType = 0,
    .InCombat = false,
    .currentGameTimeSeconds = 0,
    .PlayerShipPtr = NULL,
    .CurrentStarSystem = NULL,
    .PlayerNavState = {0}
};

const uint16_t BASE_0 = 0x5A4A;
const uint16_t BASE_1 = 0x0248;
const uint16_t BASE_2 = 0xB753;

char GovNames[GOV_MAX_COUNT][MAX_LEN] = {
    "Anarchy", "Feudal", "Multi-gov", "Dictatorship",
    "Communist", "Confederacy", "Democracy", "Corporate State"};

char EconNames[ECON_MAX_COUNT][MAX_LEN] = {
    "Rich Ind", "Average Ind", "Poor Ind", "Mainly Ind",
    "Mainly Agri", "Rich Agri", "Average Agri", "Poor Agri"};

/**
 * Gets the fuel cost per unit based on ship type
 *
 * @return Cost of fuel unit (for 0.1 LY of travel)
 */
int GetFuelCost(void) {
  if (g_state.PlayerShipPtr == NULL) {
    return 2; // Default value if ship not initialized
  }
  return (int)g_state.PlayerShipPtr->shipType->fuelConsumptionRate;
}

/**
 * Gets the maximum fuel capacity of the current ship in tenths of LY
 *
 * @return Maximum fuel capacity in tenths of LY
 */
int GetMaxFuel(void) {
  if (g_state.PlayerShipPtr == NULL) {
    return 70; // Default value if ship not initialized
  }
  return (int)(g_state.PlayerShipPtr->shipType->maxFuelLY *
               10.0); // Convert from LY to 0.1 LY units
}

/* ================= *
 * General functions *
 * ================= */

// Function to display the current game status
static void display_game_status(char *location_buffer) {
  // Enhanced status display with ship information
  if (g_state.PlayerShipPtr != NULL) { // Calculate hull percentage
    int hull_percentage = (g_state.PlayerShipPtr->attributes.hullStrength * 100) /
                          g_state.PlayerShipPtr->shipType->baseHullStrength;
    // Prepare equipment status string
    char equipment_status[MAX_LEN] = "";
    if (CheckEquipmentActive(g_state.PlayerShipPtr, EQUIP_ECM_SYSTEM)) {
      safe_strcat(equipment_status, sizeof(equipment_status), "ECM ");
    }
    if (CheckEquipmentActive(g_state.PlayerShipPtr, EQUIP_FUEL_SCOOP)) {
      safe_strcat(equipment_status, sizeof(equipment_status), "FuelScoop ");
    }
    if (CheckEquipmentActive(g_state.PlayerShipPtr, EQUIP_DOCKING_COMPUTER)) {
      safe_strcat(equipment_status, sizeof(equipment_status), "DockCmp ");
    }
    if (CheckEquipmentActive(g_state.PlayerShipPtr, EQUIP_MINING_LASER)) {
      safe_strcat(equipment_status, sizeof(equipment_status), "Mining ");
    }
    if (CheckEquipmentActive(g_state.PlayerShipPtr, EQUIP_BEAM_LASER)) {
      safe_strcat(equipment_status, sizeof(equipment_status), "Beam ");
    }
    if (CheckEquipmentActive(g_state.PlayerShipPtr, EQUIP_MILITARY_LASER)) {
      safe_strcat(equipment_status, sizeof(equipment_status), "Military ");
    }
    if (CheckEquipmentActive(g_state.PlayerShipPtr, EQUIP_SCANNER_UPGRADE)) {
      safe_strcat(equipment_status, sizeof(equipment_status), "Scanner ");
    }
    if (CheckEquipmentActive(g_state.PlayerShipPtr, EQUIP_ESCAPE_POD)) {
      safe_strcat(equipment_status, sizeof(equipment_status), "EscPod ");
    }
    if (strlen(equipment_status) == 0) {
      (void)snprintf(equipment_status, sizeof(equipment_status), "None");
    }

    (void)printf("\n\nLocation: %s | Cash: %.1f | Fuel: %.1fLY | Hull: %d%% | "
           "Equip: %s | Time: %" PRIu64 " seconds > ",
           location_buffer, ((double)g_state.Cash) / 10.0, ((double)g_state.Fuel) / 10.0,
           hull_percentage, equipment_status,
           g_state.currentGameTimeSeconds);
  } else {
    printf("\n\nLocation: %s | Cash: %.1f | Fuel: %.1fLY | Time: %" PRIu64
           " seconds > ",
           location_buffer, ((double)g_state.Cash) / 10.0, ((double)g_state.Fuel) / 10.0,
           g_state.currentGameTimeSeconds);
  }
}

int main(int argc, char *argv[]) {
  char getcommand[MAX_LEN];
  unsigned int seed = 12345; // Default seed

  // Process command-line arguments
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
      // Get seed value from next argument
      seed = (unsigned int)strtol(argv[i + 1], NULL, 10);
      i++; // Skip the next argument as we've already processed it
      printf("\nUsing custom seed: %u\n", seed);
    }
  }

  printf("\nWelcome to Text Elite 1.5.\n");

  my_srand(seed);
  initialize_player_state();
  game_time_initialize();

#define PARSER(S)                                                              \
  {                                                                            \
    char buf[sizeof(S) > 0x10 ? 0x10 : sizeof(S)];                             \
    snprintf(buf, sizeof(buf), "%s", S);                                       \
    if (!parse_and_execute_command(buf)) {                                     \
      fprintf(stderr, "Error: Failed to parse initial command '%s'\n", S);     \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  }

  PARSER("help");

#undef PARSER
  for (;;) {
    char location_buffer[MAX_LEN];
    get_current_location_name(&g_state.PlayerNavState, location_buffer, sizeof(location_buffer));

    // Periodically update all markets in the system as time passes
    update_all_system_markets();
    display_game_status(location_buffer); // Call the new function to display status

    if (!fgets(getcommand, sizeof(getcommand) - 1, stdin))
      break;
    getcommand[sizeof(getcommand) - 1] = '\0';
    parse_and_execute_command(getcommand);
  }

  printf("\n");

  exit(g_state.ExitStatus);
}

