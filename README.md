# Text Mode Elite - Modern C Port by MyriadColors

This is a cleaned up version of the [Text Mode Elite](http://www.iancgbell.clara.net/elite/text/) source code published by Ian Bell on his website. I found that the source code would neither compile or work properly on modern systems and was not proper ANSI C. All credit belongs to Ian. [Original Message by fraglet]

## Fork Information

This repository is a fork of [fragglet's txtelite repository](https://github.com/fragglet/txtelite), building upon their work to further modernize and enhance the codebase. Thanks to fragglet for their contributions and for starting the effort to bring Ian Bell's original code to a more contemporary state.

## Project Scope and Branching Strategy

Please note that the `main` branch of this repository is intended to remain a clean, baseline implementation of the modernized Text Elite code. No new game features will be added to this branch.

Future development of a full game built upon this codebase will occur in a separate branch. This approach ensures that `main` always represents a stable and faithful port of the core mechanics, while new gameplay elements can be developed independently.

## Changelog

This section details the significant changes made to the original codebase to modernize it, improve readability, and ensure compilability on contemporary systems.

**1. Naming Conventions & Readability:**

* Function Parameters: Updated to `lowerCamelCase` (e.g., `initialSeed` instead of `s`). Single-letter parameter names were replaced with more descriptive ones.
* Constants: Ensured all constants adhere to `ALL_CAPS` (e.g., `MAX_LEN`, `GAL_SIZE`).
* Structs and Types: Converted to `CapitalCase` (e.g., `PlanSys`, `SeedType`, `TradeGood`, `MarketType`, `DescChoice`).
* Struct Members: Updated to `lowerCamelCase` within structs (e.g., `planetSystem.techLev`, `Commodities[i].basePrice`).
* Function Names: Refactored to `snake_case` (e.g., `make_system`, `do_buy`, `print_system_info`).
* Defines: Renamed `nocomms` to `NUM_COMMANDS` for clarity.

**2. Bug Fixes & Refactoring:**

* Struct Definitions:
  * Updated `desc_choice` struct to `DescChoice` and its member `option` to `options`.
  * Updated members of `FastSeedType` (e.g., `A` to `a`), `SeedType` (e.g., `W0` to `w0`), `PlanSys` (e.g., `X` to `x`, `Economy` to `economy`), `TradeGood` (e.g., `BasePrice` to `basePrice`), and `MarketType` (e.g., `Quantity` to `quantity`) to use `lowerCamelCase`.

**3. Build System:**

* Makefile:
  * Created a `Makefile` to streamline the compilation process.
  * Includes targets for:
    * `all`: Default debug build (`gcc -std=c23 -Wall -Werror -Wextra`).
    * `release`: Release build (omitting `-Wall -Werror -Wextra`).
    * `run`: Executes the compiled program.
    * `clean`: Removes build artifacts.

### Compilation Instructions

**Important Note on C23 Standard:** This project uses C23 features. While the `Makefile` is configured to use `gcc -std=c23`, full C23 support can vary between compilers and their versions. **Using a recent version of GCC is the recommended and safest option to ensure compatibility.**

**Linux:**

1. Ensure you have `gcc` and `make` installed.
2. Open a terminal in the project root directory.
3. Run `make` to build the executable (default is `main`).
4. To run the compiled program, execute `./main`.

**Windows (using MSYS2 UCRT64):**

1. Install [MSYS2](https://www.msys2.org/).
2. Open the **MSYS2 UCRT64** terminal.
3. Update packages: `pacman -Syu` (you might need to close and reopen the terminal and run it again).
4. Install GCC and Make: `pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make`.
5. Navigate to the project root directory within the MSYS2 UCRT64 terminal.
6. Run `make windows` to build the Windows executable (`main.exe`).
7. To run the compiled program, execute `./main.exe` from the MSYS2 UCRT64 terminal or `main.exe` from Command Prompt/PowerShell in the project directory.

### Future Enhancements / To-Do

* Add a Makefile option or script for native Windows compilation (e.g., using MSVC or MinGW without requiring MSYS2).
