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

#include "elite_state.h"
#include "elite_utils.h"
#include "elite_galaxy.h"
#include "elite_market.h"
#include "elite_navigation.h"
#include "elite_planet_info.h"
#include "elite_commands.h"
#include "elite_command_handler.h"
#include "elite_player_state.h"

/* ================= * 
 * General functions *
 * ================= */

int main()
{

	char getcommand[MAX_LEN];
	printf("\nWelcome to Text Elite 1.5.\n");

	my_srand(12345);
  initialize_player_state();

#define PARSER(S) { char buf[sizeof(S) > 0x10 ? 0x10 : sizeof(S)]; strncpy(buf,S, sizeof(buf)-1); buf[sizeof(buf)-1] = '\0'; parse_and_execute_command(buf); }   

	PARSER("help");

#undef PARSER

	for(;;)
	{
		printf("\n\nCash :%.1f Fuel: %.1fLY >", ((float)Cash) / 10.0f, ((float)Fuel) / 10.0f);
		if (!fgets(getcommand, sizeof(getcommand) - 1, stdin))
			break;
		getcommand[sizeof(getcommand) - 1] = '\0';
		parse_and_execute_command(getcommand);
	}

	printf("\n");

	exit(ExitStatus);
}

/**+end **/

