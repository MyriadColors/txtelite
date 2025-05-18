/* txtelite.c  1.5 */
/* Textual version of Elite trading (C implementation) */
/* Converted by Ian Bell from 6502 Elite sources.
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

#include <time.h>
#include <math.h>
#include <ctype.h>

#include "elite_state.h"		// Unified header for constants, structures, and globals
#include "elite_utils.h"		// For string handling and other utilities
#include "elite_galaxy.h"		// For galaxy generation
#include "elite_market.h"		// For market calculations
#include "elite_navigation.h"	// For distance and jump calculations
#include "elite_planet_info.h"	// For planet info printing
#include "elite_commands.h"		// For game commands
#include "elite_command_handler.h"	// For command parsing
#include "elite_player_state.h"	// For player state initialization
#include "elite_star_system.h"	// For star system data
#include "elite_ship_upgrades.h" // For equipment access
#include "elite_equipment_constants.h" // For equipment indices

/* ================= *
 * General functions *
 * ================= */

// Definition of the global game time variable (in seconds)
// This will be linked with the extern declaration in elite_state.h
uint64_t currentGameTimeSeconds = 0;

int main(void)
{

	char getcommand[MAX_LEN];
	printf("\nWelcome to Text Elite 1.5.\n");

	my_srand(12345);
	initialize_player_state();
	game_time_initialize();

#define PARSER(S)                                      \
	{                                                  \
		char buf[sizeof(S) > 0x10 ? 0x10 : sizeof(S)]; \
		strncpy(buf, S, sizeof(buf) - 1);              \
		buf[sizeof(buf) - 1] = '\0';                   \
		parse_and_execute_command(buf);                \
	}

	PARSER("help");

#undef PARSER
	
	// Track last time energy was regenerated
	uint64_t lastEnergyRegenTime = currentGameTimeSeconds;
	
	for (;;)
	{
		char locBuffer[MAX_LEN];
		get_current_location_name(&PlayerNavState, locBuffer, sizeof(locBuffer));

		// Periodically update all markets in the system as time passes
		update_all_system_markets();
				// Regenerate ship energy over time
		if (PlayerShipPtr != NULL && currentGameTimeSeconds > lastEnergyRegenTime) {
			// Calculate time elapsed since last regeneration
			uint64_t timeElapsed = currentGameTimeSeconds - lastEnergyRegenTime;
			
			// Regenerate energy at a rate of 1 unit per 5 seconds, up to max
			if (timeElapsed >= 5) {
				float energyToAdd = timeElapsed / 5.0f;
				
				// Add energy, but don't exceed maximum
				PlayerShipPtr->attributes.energyBanks = 
					(PlayerShipPtr->attributes.energyBanks + energyToAdd > PlayerShipPtr->attributes.maxEnergyBanks) ? 
					PlayerShipPtr->attributes.maxEnergyBanks : PlayerShipPtr->attributes.energyBanks + energyToAdd;
				
				// Update last regeneration time
				lastEnergyRegenTime = currentGameTimeSeconds;
			}
		}

		// Enhanced status display with ship information
		if (PlayerShipPtr != NULL) {			// Calculate hull percentage
			int hullPercentage = (PlayerShipPtr->attributes.hullStrength * 100) / PlayerShipPtr->shipType->baseHullStrength;
			// Calculate energy percentage
			int energyPercentage = (int)((PlayerShipPtr->attributes.energyBanks * 100) / PlayerShipPtr->attributes.maxEnergyBanks);// Prepare equipment status string
			char equipmentStatus[MAX_LEN] = "";
			if (CheckEquipmentActive(PlayerShipPtr, EQUIP_ECM_SYSTEM)) strcat(equipmentStatus, "ECM ");
			if (CheckEquipmentActive(PlayerShipPtr, EQUIP_FUEL_SCOOP)) strcat(equipmentStatus, "FuelScoop ");
			if (CheckEquipmentActive(PlayerShipPtr, EQUIP_ENERGY_BOMB)) strcat(equipmentStatus, "E-Bomb ");
			if (CheckEquipmentActive(PlayerShipPtr, EQUIP_DOCKING_COMPUTER)) strcat(equipmentStatus, "DockCmp ");
			if (CheckEquipmentActive(PlayerShipPtr, EQUIP_MINING_LASER)) strcat(equipmentStatus, "Mining ");
			if (CheckEquipmentActive(PlayerShipPtr, EQUIP_BEAM_LASER)) strcat(equipmentStatus, "Beam ");
			if (CheckEquipmentActive(PlayerShipPtr, EQUIP_MILITARY_LASER)) strcat(equipmentStatus, "Military ");
			if (CheckEquipmentActive(PlayerShipPtr, EQUIP_SCANNER_UPGRADE)) strcat(equipmentStatus, "Scanner ");
			if (CheckEquipmentActive(PlayerShipPtr, EQUIP_ESCAPE_POD)) strcat(equipmentStatus, "EscPod ");
			if (CheckEquipmentActive(PlayerShipPtr, EQUIP_GALACTIC_HYPERSPACE)) strcat(equipmentStatus, "GalHyp ");
			if (strlen(equipmentStatus) == 0) strcpy(equipmentStatus, "None");


			printf("\n\nLocation: %s | Cash: %.1f | Fuel: %.1fLY | Hull: %d%% | Energy: %d%% | Equip: %s| Time: %llu seconds > ",
				   locBuffer, ((float)Cash) / 10.0f, ((float)Fuel) / 10.0f, 
				   hullPercentage, energyPercentage, equipmentStatus, currentGameTimeSeconds);
		} else {
			printf("\n\nLocation: %s | Cash: %.1f | Fuel: %.1fLY | Time: %llu seconds > ",
				   locBuffer, ((float)Cash) / 10.0f, ((float)Fuel) / 10.0f, currentGameTimeSeconds);
		}
		if (!fgets(getcommand, sizeof(getcommand) - 1, stdin))
			break;
		getcommand[sizeof(getcommand) - 1] = '\0';
		parse_and_execute_command(getcommand);
	}

	printf("\n");

	exit(ExitStatus);
}
