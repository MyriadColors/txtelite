#ifndef ELITE_COMMAND_HANDLER_H
#define ELITE_COMMAND_HANDLER_H

#include "elite_state.h" // Unified header for constants, structures, and globals
#include "elite_utils.h"
#include "elite_commands.h" // For do_* functions

// Command definitions
static char commands[NUM_COMMANDS][MAX_LEN] =
	{
		"buy", "sell", "fuel", "jump",
		"cash", "mkt", "help", "hold",
		"sneak", "local", "info", "galhyp",
		"quit", "rand", "save", "load",
		"system", "travel", "scan", "dock",
		"compare", "land"};

// Array of function pointers to command functions
static bool (*comfuncs[NUM_COMMANDS])(char *) =
	{
		do_buy, do_sell, do_fuel, do_jump,
		do_cash, do_market_display, do_help, do_hold,
		do_sneak, do_local_systems_display, do_planet_info_display, do_galactic_hyperspace,
		do_quit, do_tweak_random_native, do_save, do_load,
		do_system_info, do_travel, do_scan, do_dock,
		do_compare_markets, do_land};

// Function to parse and execute commands
static inline bool parse_and_execute_command(char *commandString)
{
	uint16_t i;
	char c[MAX_LEN];
	commandString = strip_leading_trailing_spaces(commandString);
	if (strlen(commandString) == 0)
		return false;
	split_string_at_first_space(commandString, c);
	i = match_string_in_array(c, commands, NUM_COMMANDS);
	if (i)
		return (*comfuncs[i - 1])(commandString);
	printf("\nBad command (%s)", c);
	return false;
}

#endif // ELITE_COMMAND_HANDLER_H
