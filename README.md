# Text Mode Elite - Modern C Port by MyriadColors

This is a cleaned up version of the [Text Mode Elite](http://www.iancgbell.clara.net/elite/text/) source code published by Ian Bell on his website. I found that the source code would neither compile or work properly on modern systems and was not proper ANSI C. All credit belongs to Ian. [Original Message by fraglet]

## Fork Information

This repository is a fork of [fragglet's txtelite repository](https://github.com/fragglet/txtelite), building upon their work to further modernize and enhance the codebase. Thanks to fragglet for their contributions and for starting the effort to bring Ian Bell's original code to a more contemporary state.

## Branch Information - Full Game Branch

**IMPORTANT**: You are viewing the `full-game` branch of this repository, which contains an expanded implementation aimed at creating a complete text-based Elite clone with additional features. This branch diverges from the minimal, modernized core implementation in the `main` branch.

This branch aims to transform the original single-file implementation into a more modular, maintainable codebase that will serve as a foundation for gameplay expansions while preserving the core mechanics of the original Text Elite.

## Project Scope and Branching Strategy

The `main` branch of this repository remains a clean, baseline implementation of the modernized Text Elite code with no additional gameplay features.

This `full-game` branch is where the active development of a full game built upon the core mechanics takes place. New gameplay elements, UI improvements, and additional systems will be implemented here.

## Recent Refactoring Changes

The following changes have been made to improve code organization and maintainability, driven by a recent large-scale refactoring and modernization effort:

### Modular Header-Only Architecture

The original single-file implementation (`txtelite.c`) has been expanded and refactored into a modular architecture logically grouped by domain:

* **Core & State Management**:
  * `elite_state.h`: Unified header now encapsulating global variables into a `GameState` struct.
  * `elite_utils.h`: Utility functions for random numbers, string manipulation, and math helpers.
  * `platform_compat.h`: Cross-platform compatibility for case-insensitive string and directory handling.
  * `elite_save.h`: Player save/load system.
* **Ship Systems**:
  * `elite_player_ship.h`, `elite_ship_cargo.h`, `elite_ship_components.h`, `elite_ship_equipment.h`, `elite_ship_inventory.h`, `elite_ship_maintenance.h`, `elite_ship_registry.h`, `elite_ship_trading.h`, `elite_ship_upgrades.h`, `elite_equipment_constants.h`: A comprehensive refactor of player ship management, including a full upgrade system, travel mechanics, cargo, and inventory.
* **Galaxy & Star Systems**:
  * `elite_galaxy.h`: Galaxy procedural generation logic, now isolating state mutation.
  * `elite_star_system.h`: Advanced system generation for better realism.
  * `elite_planet_info.h`: Enhanced planetary descriptions featuring detailed habitability analysis.
* **Navigation & Market**:
  * `elite_navigation.h`, `elite_navigation_types.h`: System navigation and distance calculations.
  * `elite_market.h`: Market data and trading functions.
* **Commands**:
  * `elite_commands.h`: Command implementation functions.
  * `elite_command_handler.h`: Command parsing and execution, now utilizing a structured data-driven approach.
  * `elite_player_state.h`: Player state initialization and management.

### Implementation Details

1. **Header-Only Approach**: All functions are implemented as `static inline` to maintain the single compilation unit approach while improving code organization.
2. **Unified State Management**: Consolidated all constants, types, and global variables into a unified `GameState` structure in `elite_state.h` to reduce namespace pollution.
3. **Advanced Ship Systems**: Added a comprehensive ship upgrade system and refined travel mechanics.
4. **Enhanced System Info**: Improved the system info display featuring detailed planet habitability analysis.
5. **Platform Compatibility**: Added cross-platform case-insensitive string comparison and directory iteration (POSIX-compatible) via `platform_compat.h`.
6. **Type Safety & Readability**: Standardized naming conventions, replaced boolean literals with integer constants, and improved overall type safety.
7. **Developer Environment**: Added `clangd` configuration support for C/C++ analysis.

### Bug Fixes

1. Fixed segmentation fault in the market display by preventing out-of-bounds array access.
2. Corrected initialization order to ensure galaxy data and market information are properly set up.
3. Resolved parameter type mismatches in function calls.
4. Fixed memory corruption issues by properly sizing arrays using consistent constants.
5. Fixed buffer overflow in `goat_soup` random character generation.
6. Replaced `strnlen` with `compat_strnlen` for better portability.

## Changelog

This section details the significant changes made to the original codebase to modernize it, improve readability, and ensure it compiles on contemporary systems.

**1. Modernization & Build:**

* Migrated to modern C23 standards.
* Added `clangd` configuration for developer tooling.

**2. Naming Conventions & Readability:**

* Function Parameters: Updated to `lowerCamelCase` (e.g., `initialSeed` instead of `s`). Single-letter parameter names were replaced with more descriptive ones.
* Constants: Ensured all constants adhere to `ALL_CAPS` (e.g., `MAX_LEN`, `GAL_SIZE`).
* Structs and Types: Converted to `CapitalCase` (e.g., `plan_sys_t`, `SeedType`, `TradeGood`, `MarketType`, `DescChoice`).
* Struct Members: Updated to `lowerCamelCase` within structs (e.g., `planetSystem.techLev`, `Commodities[i].basePrice`).
* Function Names: Refactored to `snake_case` (e.g., `make_system`, `do_buy`, `print_system_info`).
* Defines: Renamed `nocomms` to `NUM_COMMANDS` for clarity.

**3. Bug Fixes & Refactoring:**

* Encapsulated global state into the `GameState` structure.
* Structured data-driven approach for commands instead of hardcoded help.
* Replaced boolean literals with integer constants throughout the codebase.
* Struct Definitions:
  * Updated `desc_choice` struct to `DescChoice` and its member `option` to `options`.
  * Updated members of `FastSeedType` (e.g., `A` to `a`), `SeedType` (e.g., `W0` to `w0`), `plan_sys_t` (e.g., `X` to `x`, `Economy` to `economy`), `TradeGood` (e.g., `BasePrice` to `basePrice`), and `MarketType` (e.g., `Quantity` to `quantity`) to use `lowerCamelCase`.

**3. Build System:**

* Makefile:
  * Created a `Makefile` to streamline the compilation process.
  * Includes targets for:
    * `all`: Default debug build (`gcc -std=c23 -Wall -Werror -Wextra`).
    * `release`: Release build (omitting `-Wall -Werror -Wextra`).
    * `run`: Executes the compiled program.
    * `clean`: Removes build artifacts.
  
  * Added `clang` as the default compiler, but can be changed by modifying the `CC` variable in the `Makefile`.

### Compilation Instructions

**Important Note on C23 Standard:** This project uses C23 features. While the `Makefile` is configured to use `clang -std=c23`, full C23 support can vary between compilers and their versions.

**Linux:**

1. Ensure you have `clang` and `make` installed.
2. Open a terminal in the project root directory.
3. Run `make` to build the executable (default is `main`).
4. To run the compiled program, execute `./main`.

**Windows (Native LLVM/Clang):**

1. Install LLVM/Clang for Windows from [official LLVM releases](https://releases.llvm.org/download.html).
2. Make sure to add LLVM to your system PATH during installation.
3. Install Make for Windows:
   * You can get it via [chocolatey](https://chocolatey.org/): `choco install make`
   * Or download a standalone version from [GnuWin32](http://gnuwin32.sourceforge.net/packages/make.htm).
4. Open Command Prompt or PowerShell in the project root directory.
5. Run `make` to build the executable (`main.exe`).
6. To run the compiled program, execute `make run` or `main.exe` from the Command Prompt/PowerShell in the project directory.

**Windows (using MSYS2 UCRT64):**

1. Install [MSYS2](https://www.msys2.org/).
2. Open the **MSYS2 UCRT64** terminal.
3. Update packages: `pacman -Syu` (you might need to close and reopen the terminal and run it again).
4. Install Clang and Make: `pacman -S mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-make`.
5. Navigate to the project root directory within the MSYS2 UCRT64 terminal.
6. Run `make` to build the Windows executable (`main.exe`).
7. To run the compiled program, execute `make run` or `main.exe` from the MSYS2 UCRT64 terminal.

Note: If modifying the code, you should ruin 'make clean' before running 'make run' again to ensure a clean build. Or you can use 'make clean run' to do both in one command.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
