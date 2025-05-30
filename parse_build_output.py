#!/usr/bin/env python3
"""
Compiler Output Filter - Removes verbose CRT macro expansions while preserving
the actual source code warnings and errors, grouping errors by file.
Now with more features and no regex for pattern matching!
"""

import sys
import subprocess
import collections
import argparse # For command-line arguments
from typing import List, Optional, Dict, Any

# --- Configuration and Colors ---
class Config:
    def __init__(self):
        self.input_file: Optional[str] = None
        self.output_file: Optional[str] = None
        self.debug_filter: bool = False
        self.show_all_notes: bool = False
        self.no_grouping: bool = False
        self.no_color: bool = False
        self.make_command: List[str] = ['make']

class AnsiColors:
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    WARNING = '\033[93m' # Yellow
    ERROR = '\033[91m'   # Red
    DEBUG = '\033[90m'  # Grey
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    ENDC = '\033[0m'
    DISABLED = False

    @classmethod
    def disable(cls):
        cls.DISABLED = True
        for attr in dir(cls):
            if isinstance(getattr(cls, attr), str) and getattr(cls, attr).startswith('\033'):
                setattr(cls, attr, "")

    @classmethod
    def format(cls, text: str, *styles: str) -> str:
        if cls.DISABLED:
            return text
        return "".join(styles) + text + cls.ENDC

# Global config object
APP_CONFIG = Config()

# --- Helper Functions (No Regex for pattern matching) ---
def is_source_code_line(line: str) -> bool:
    stripped = line.strip()
    if not stripped: return False
    # Check if there's a pipe and the part before it is a digit
    pipe_idx = stripped.find('|')
    if pipe_idx == -1 or pipe_idx == 0: return False
    return stripped[:pipe_idx].strip().isdigit()

def is_caret_line(line: str) -> bool:
    stripped = line.strip()
    if not stripped: return False
    pipe_idx = stripped.find('|')
    if pipe_idx == -1 or pipe_idx + 1 >= len(stripped): return False
    
    after_pipe = stripped[pipe_idx+1:].strip()
    return '^' in after_pipe and after_pipe.startswith('^')

def is_crt_noise(line: str) -> bool: # config is not directly used here now
    crt_indicators = [
        '_CRT_', '__declspec(deprecated', '_Check_return_',
        '__DEFINE_CPP_OVERLOAD', 'has been explicitly marked deprecated here',
        'Windows Kits', 'ucrt', '_INSECURE_DEPRECATE',
        '_DEPRECATE_TEXT', '__cdecl'
    ]
    # A note is CRT noise if it contains CRT indicators.
    # The decision to show it (if APP_CONFIG.show_all_notes) is handled in the main loop.
    return any(indicator in line for indicator in crt_indicators)

def is_note_line(line: str) -> bool:
    return line.strip().startswith('note:')

def _is_valid_filename_candidate(name_part: str) -> bool:
    if not name_part:
        return False
    name_part_lower = name_part.lower()
    # Must contain a dot (for extension) or a path separator
    # And not be a keyword or just digits
    if ('.' in name_part or '/' in name_part or '\\' in name_part) and \
       name_part_lower not in ["error", "warning", "note", "fatal error", "makefile"] and \
       not name_part.isdigit():
        # Avoid matching on things like "1.o" if it's not a source file,
        # but this is hard without knowing context.
        # For now, this basic check is okay.
        return True
    return False

def extract_filename_from_error_line_no_regex(line_text: str) -> Optional[str]:
    stripped_line = line_text.strip()
    markers = [ # text, is_msvc_style (bool)
        (": fatal error:", False), (": error:", False), (": warning:", False), (": note:", False),
        (" fatal error C", True), (" error C", True), (" warning C", True)
    ]
    marker_pos = -1
    for marker_text, is_msvc in markers:
        current_pos = stripped_line.find(marker_text)
        if current_pos != -1:
            if is_msvc:
                code_start_idx = current_pos + len(marker_text)
                if not (code_start_idx < len(stripped_line) and stripped_line[code_start_idx].isdigit()):
                    continue
            if marker_pos == -1 or current_pos < marker_pos:
                marker_pos = current_pos
    if marker_pos == -1: return None
    candidate_part = stripped_line[:marker_pos].strip()

    if candidate_part.endswith(')'): # MSVC style: "file(line,col)" or "file(line)"
        open_paren_idx = candidate_part.rfind('(')
        if open_paren_idx > 0: # Ensure there's a filename part before '('
            filename_cand = candidate_part[:open_paren_idx].strip()
            loc_cand = candidate_part[open_paren_idx+1:-1].strip()
            is_loc_valid = False
            if ',' in loc_cand:
                parts = loc_cand.split(',', 1)
                if len(parts) == 2 and parts[0].strip().isdigit() and parts[1].strip().isdigit():
                    is_loc_valid = True
            elif loc_cand.isdigit():
                is_loc_valid = True
            if is_loc_valid and _is_valid_filename_candidate(filename_cand): return filename_cand
    
    parts = candidate_part.split(':') # GCC/Clang style: "file:line:col" or "file:line"
    if len(parts) >= 3 and parts[-1].isdigit() and parts[-2].isdigit(): # file:line:col
        filename_cand = ":".join(parts[:-2]).strip()
        if _is_valid_filename_candidate(filename_cand): return filename_cand
    if len(parts) >= 2 and parts[-1].isdigit(): # file:line
        filename_cand = ":".join(parts[:-1]).strip()
        if _is_valid_filename_candidate(filename_cand): return filename_cand
    if _is_valid_filename_candidate(candidate_part) and not candidate_part.endswith(':'):
        return candidate_part # Fallback for "file.c: message"
    return None

def is_primary_error_or_warning_no_regex(line_text: str) -> bool:
    stripped = line_text.strip()
    if stripped.startswith("note:"): return False # Notes are not primary triggers

    direct_starters = ["fatal error:", "error:", "warning:"]
    for starter in direct_starters:
        if stripped.startswith(starter): return True

    # Embedded markers like "file:line:col: error: message"
    # or "file(line,col): error CXXXX: message"
    embedded_markers = [ # text, is_msvc_style (bool)
        (": fatal error:", False), (": error:", False), (": warning:", False),
        (" fatal error C", True), (" error C", True), (" warning C", True)
    ]
    for marker_text, is_msvc in embedded_markers:
        pos = stripped.find(marker_text)
        if pos > 0: # Marker is present and *not* at the beginning
            if is_msvc:
                code_start_idx = pos + len(marker_text)
                if not (code_start_idx < len(stripped) and stripped[code_start_idx].isdigit()):
                    continue
            note_idx = stripped.find("note:") # Avoid "note: ... error: ..."
            if note_idx != -1 and note_idx < pos: continue
            return True
    return False

# --- Main Filtering Logic ---
def filter_compiler_output(lines: List[str]) -> List[str]:
    if APP_CONFIG.no_grouping:
        return filter_compiler_output_no_grouping(lines)
    
    output_groups: Dict[str, List[str]] = collections.defaultdict(list)
    ordered_file_keys: List[str] = [] 
    PREAMBLE_KEY, POSTAMBLE_KEY, GENERAL_KEY = "__PRE__", "__POST__", "__GEN__"
    current_group_key = PREAMBLE_KEY
    pending_includes: List[str] = []
    i = 0

    while i < len(lines):
        line = lines[i].rstrip()
        original_line_for_debug = lines[i].rstrip() # Keep original for debug
        i += 1 # Consume line now

        if not line.strip(): continue

        def _debug_print(reason: str, l: str):
            if APP_CONFIG.debug_filter:
                print(AnsiColors.format(f"DEBUG Filter: {reason}: {l}", AnsiColors.DEBUG))
        
        # Preamble/Postamble
        is_preamble = any(kw in line for kw in ["Compiling ", "Running ", "Clean "]) or \
                      any(line.startswith(kw) for kw in ["clang ", "cc ", "gcc ", "cl ", "make:"])
        is_postamble = "Build complete" in line or "errors generated" in line or \
                       "warnings generated" in line or \
                       ("error(s)," in line and "warning(s)" in line)

        if is_preamble:
            if pending_includes: output_groups[GENERAL_KEY].extend(pending_includes); pending_includes = []
            current_group_key = PREAMBLE_KEY
            output_groups[PREAMBLE_KEY].append(line)
            continue
        if is_postamble:
            if pending_includes: output_groups[GENERAL_KEY].extend(pending_includes); pending_includes = []
            current_group_key = POSTAMBLE_KEY
            output_groups[POSTAMBLE_KEY].append(line)
            continue
        
        if line.strip().startswith("In file included from"):
            pending_includes.append(line)
            while i < len(lines) and lines[i].strip().startswith("from ") and lines[i].strip().endswith(':'):
                pending_includes.append(lines[i].rstrip())
                i += 1
            continue

        is_primary = is_primary_error_or_warning_no_regex(line)
        filename = extract_filename_from_error_line_no_regex(line) if is_primary else None

        if is_primary:
            active_group = GENERAL_KEY
            if filename:
                active_group = filename
                if filename not in ordered_file_keys: ordered_file_keys.append(filename)
            else: # Primary error but no filename extracted, goes to general
                active_group = GENERAL_KEY
            
            if pending_includes: output_groups[active_group].extend(pending_includes); pending_includes = []
            current_group_key = active_group
            output_groups[active_group].append(line)
            continue # Next line will be processed by main loop

        # Flush pending includes if not consumed by a primary error and we encounter other content
        if pending_includes and (is_source_code_line(line) or is_note_line(line)):
            output_groups[GENERAL_KEY].extend(pending_includes)
            pending_includes = []
            if current_group_key == PREAMBLE_KEY: current_group_key = GENERAL_KEY
        
        active_group_for_details = current_group_key
        if active_group_for_details in [PREAMBLE_KEY, POSTAMBLE_KEY]:
            active_group_for_details = GENERAL_KEY # Details go to general if not in file context

        is_line_crt_noise = is_crt_noise(line)
        is_line_a_note = is_note_line(line)

        if is_line_a_note:
            if APP_CONFIG.show_all_notes or not is_line_crt_noise:
                output_groups[active_group_for_details].append(line)
            else: _debug_print("Skipping CRT note", original_line_for_debug)
            continue

        if is_source_code_line(line):
            if not is_line_crt_noise:
                output_groups[active_group_for_details].append(line)
                if i < len(lines) and is_caret_line(lines[i]):
                    output_groups[active_group_for_details].append(lines[i].rstrip())
                    i += 1
                # Inner loop for subsequent notes/CRT after user code (simplified)
                # This part might need more refinement if complex note chains appear
            else:
                _debug_print("Skipping CRT source line", original_line_for_debug)
                if i < len(lines) and is_caret_line(lines[i]): # Skip its caret too
                    _debug_print("Skipping CRT source caret", lines[i].rstrip())
                    i += 1
            continue
        
        if is_line_crt_noise: # General CRT noise not caught above
            _debug_print("Skipping general CRT noise", original_line_for_debug)
            continue
            
        # Default: Keep unclassified lines (e.g. linker messages not fitting patterns)
        _debug_print("Keeping unclassified", original_line_for_debug)
        output_groups[active_group_for_details].append(line)
        
    if pending_includes: output_groups[GENERAL_KEY].extend(pending_includes)

    # Assemble final output
    final_lines: List[str] = []
    if output_groups[PREAMBLE_KEY]: final_lines.extend(output_groups[PREAMBLE_KEY])
    
    def _add_section(key_list, header_prefix, default_header_text):
        for key_idx, key in enumerate(key_list):
            if output_groups[key]:
                if final_lines and final_lines[-1].strip(): final_lines.append("")
                header = f"{header_prefix}{key}" if key not in [GENERAL_KEY] else default_header_text
                final_lines.append(AnsiColors.format(header, AnsiColors.HEADER, AnsiColors.BOLD))
                final_lines.extend(output_groups[key])
    
    if output_groups[GENERAL_KEY]:
         _add_section([GENERAL_KEY], "", "--- General Messages/Errors ---")

    _add_section(ordered_file_keys, "--- Errors in file: ", "") # Default header handled by general

    if output_groups[POSTAMBLE_KEY]:
        if final_lines and final_lines[-1].strip(): final_lines.append("")
        final_lines.extend(output_groups[POSTAMBLE_KEY])
    
    # Clean up excessive blank lines
    cleaned_final_lines = []
    if final_lines:
        cleaned_final_lines.append(final_lines[0])
        for idx in range(1, len(final_lines)):
            if final_lines[idx].strip() or final_lines[idx-1].strip():
                cleaned_final_lines.append(final_lines[idx])
    return cleaned_final_lines

def filter_compiler_output_no_grouping(lines: List[str]) -> List[str]:
    filtered_lines: List[str] = []
    i = 0
    while i < len(lines):
        line = lines[i].rstrip()
        original_line_for_debug = lines[i].rstrip()
        i += 1

        def _debug_print(reason: str, l: str):
            if APP_CONFIG.debug_filter:
                print(AnsiColors.format(f"DEBUG Filter (no-group): {reason}: {l}", AnsiColors.DEBUG))

        if not line.strip(): continue # Skip empty lines

        is_line_crt_noise = is_crt_noise(line)
        is_line_a_note = is_note_line(line)

        # Keep important markers
        if any(kw in line for kw in ["Compiling ", "Running ", "Clean ", "Build complete", "errors generated"]) or \
           any(line.startswith(kw) for kw in ["clang ", "cc ", "gcc ", "cl ", "make:"]) or \
           is_primary_error_or_warning_no_regex(line):
            filtered_lines.append(line)
            continue

        if is_line_a_note:
            if APP_CONFIG.show_all_notes or not is_line_crt_noise:
                filtered_lines.append(line)
            else: _debug_print("Skipping CRT note", original_line_for_debug)
            continue
        
        if is_source_code_line(line):
            if not is_line_crt_noise:
                filtered_lines.append(line)
                if i < len(lines) and is_caret_line(lines[i]):
                    filtered_lines.append(lines[i].rstrip())
                    i += 1
            else:
                _debug_print("Skipping CRT source line", original_line_for_debug)
                if i < len(lines) and is_caret_line(lines[i]):
                     _debug_print("Skipping CRT source caret", lines[i].rstrip())
                     i += 1
            continue

        if is_line_crt_noise:
            _debug_print("Skipping general CRT noise", original_line_for_debug)
            continue
        
        _debug_print("Keeping unclassified", original_line_for_debug)
        filtered_lines.append(line)
    return filtered_lines


# --- run_make_with_filter and main ---
def run_command_and_filter(command_args: List[str]) -> tuple[List[str], int]:
    output_lines: List[str] = []
    try:
        process = subprocess.Popen(
            command_args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, bufsize=1 
        )
        if process.stdout:
            for line in iter(process.stdout.readline, ''):
                output_lines.append(line) # Keep original newlines for now
            process.stdout.close()
        return_code = process.wait()
        return output_lines, return_code
    except FileNotFoundError:
        print(AnsiColors.format(f"Error: '{command_args[0]}' command not found", AnsiColors.ERROR, AnsiColors.BOLD), file=sys.stderr)
        return [], 1
    except Exception as e:
        print(AnsiColors.format(f"Error running {' '.join(command_args)}: {e}", AnsiColors.ERROR, AnsiColors.BOLD), file=sys.stderr)
        return [], 1

def main():
    parser = argparse.ArgumentParser(description="Filters compiler output, removing CRT noise and grouping errors.")
    parser.add_argument('make_args', nargs='*', help="Arguments to pass to 'make' (or the specified command).")
    parser.add_argument('-c', '--command', default='make', help="Command to run (default: make).")
    parser.add_argument('-i', '--input-file', help="Process output from a file instead of running a command.")
    parser.add_argument('-o', '--output-file', help="Save filtered output to a file.")
    parser.add_argument('--debug-filter', '--verbose-filter', action='store_true', help="Show verbose debugging info about filtered lines.")
    parser.add_argument('--show-all-notes', action='store_true', help="Show all 'note:' lines, even if CRT-related.")
    parser.add_argument('--no-grouping', action='store_true', help="Disable grouping of errors by file (flat output).")
    parser.add_argument('--no-color', action='store_true', help="Disable colored output.")
    parser.add_argument('--run-tests', action='store_true', help="Run internal basic tests for the filter logic.")


    args = parser.parse_args()

    APP_CONFIG.input_file = args.input_file
    APP_CONFIG.output_file = args.output_file
    APP_CONFIG.debug_filter = args.debug_filter
    APP_CONFIG.show_all_notes = args.show_all_notes
    APP_CONFIG.no_grouping = args.no_grouping
    APP_CONFIG.no_color = args.no_color
    
    if APP_CONFIG.no_color:
        AnsiColors.disable()

    if args.run_tests:
        _run_tests()
        sys.exit(0)

    all_output_lines: List[str] = []
    exit_code = 0

    if APP_CONFIG.input_file:
        try:
            with open(APP_CONFIG.input_file, 'r') as f:
                all_output_lines = f.readlines()
            print(AnsiColors.format(f"Processing from file: {APP_CONFIG.input_file}", AnsiColors.BLUE))
        except FileNotFoundError:
            print(AnsiColors.format(f"Error: Input file not found: {APP_CONFIG.input_file}", AnsiColors.ERROR, AnsiColors.BOLD), file=sys.stderr)
            sys.exit(1)
        except Exception as e:
            print(AnsiColors.format(f"Error reading input file: {e}", AnsiColors.ERROR, AnsiColors.BOLD), file=sys.stderr)
            sys.exit(1)
    else:
        APP_CONFIG.make_command = [args.command] + args.make_args
        print(AnsiColors.format(f"Running: {' '.join(APP_CONFIG.make_command)}", AnsiColors.BLUE))
        print(AnsiColors.format("=" * 50, AnsiColors.BLUE))
        all_output_lines, exit_code = run_command_and_filter(APP_CONFIG.make_command)

    filtered_output = filter_compiler_output(all_output_lines)

    if APP_CONFIG.output_file:
        try:
            with open(APP_CONFIG.output_file, 'w') as f:
                for line in filtered_output:
                    f.write(line + '\n') # filter_compiler_output rstrips
            print(AnsiColors.format(f"Filtered output saved to: {APP_CONFIG.output_file}", AnsiColors.GREEN))
        except Exception as e:
            print(AnsiColors.format(f"Error writing to output file: {e}", AnsiColors.ERROR, AnsiColors.BOLD), file=sys.stderr)
            # Still print to stdout if file write fails
            for line in filtered_output: print(line)
    else:
        for line in filtered_output:
            print(line)
            
    sys.exit(exit_code)

# --- Basic Test Cases (Conceptual) ---
def _run_tests():
    print(AnsiColors.format("\n--- Running Basic Tests ---", AnsiColors.HEADER, AnsiColors.BOLD))
    # Test cases: list of (description, input_lines, expected_relevant_lines_pattern)
    # Due to complexity of exact output with grouping/colors, these are conceptual.
    # For real tests, you'd compare against exact expected string list.
    
    APP_CONFIG.no_color = True # Disable color for test comparison simplicity
    AnsiColors.disable()
    APP_CONFIG.debug_filter = False # Keep test output clean

    test_cases = [
        ("Simple Error", 
         ["make: Entering directory '/src'",
          "gcc -c main.c -o main.o",
          "main.c:5:10: error: expected ';' before 'return'",
          "    5 |   int x = 5",
          "      |          ^",
          "      |          ;",
          "note: some note here",
          "make: Leaving directory '/src'"],
         ["main.c:5:10: error", "int x = 5", "^", "note: some note here"]), # Simplified expected
        
        ("CRT Noise Skip",
         ["main.c:10:5: warning: '_CRT_SECURE_NO_WARNINGS' should be defined",
          "  10 |   strcpy(dst, src);",
          "     |   ^~~~~~",
          "C:\\Program Files\\...\\ucrt\\corecrt_deprecated.h:50:1: note: 'strcpy' has been explicitly marked deprecated here",
          "   50 | _CRT_DEPRECATE_TEXT(\"Don't use strcpy\") _CRT_INSECURE_DEPRECATE(strcpy_s)"],
         ["main.c:10:5: warning", "strcpy(dst, src)", "^~~~~~"]), # CRT note should be skipped by default

        ("Show All Notes with CRT",
         ["main.c:10:5: warning: '_CRT_SECURE_NO_WARNINGS' should be defined",
          "  10 |   strcpy(dst, src);",
          "     |   ^~~~~~",
          "C:\\Program Files\\...\\ucrt\\corecrt_deprecated.h:50:1: note: 'strcpy' has been explicitly marked deprecated here"],
         ["main.c:10:5: warning", "strcpy(dst, src)", "^~~~~~", "note: 'strcpy' has been explicitly marked deprecated here"]),
    ]

    for i, (desc, input_lines, expected_patterns) in enumerate(test_cases):
        print(f"\nTest {i+1}: {desc}")
        
        # Special handling for "Show All Notes" test case
        original_show_all_notes = APP_CONFIG.show_all_notes
        if "Show All Notes" in desc:
            APP_CONFIG.show_all_notes = True
        
        output = filter_compiler_output(input_lines)
        
        APP_CONFIG.show_all_notes = original_show_all_notes # Reset for next test

        print("Input:")
        for l_in in input_lines: print(f"  {l_in.rstrip()}")
        print("Output:")
        for l_out in output: print(f"  {l_out}")
        
        # Basic check: does output contain expected patterns?
        # This is a very loose check. Proper testing needs exact matches.
        passed = True
        output_str = "\n".join(output)
        for pattern in expected_patterns:
            if pattern not in output_str:
                print(AnsiColors.format(f"  MISSING PATTERN: '{pattern}'", AnsiColors.ERROR))
                passed = False
        
        if passed:
            print(AnsiColors.format("  Test basic check: PASSED (patterns found)", AnsiColors.GREEN))
        else:
            print(AnsiColors.format("  Test basic check: FAILED (see missing patterns)", AnsiColors.ERROR))

    print(AnsiColors.format("\n--- Basic Tests Complete ---", AnsiColors.HEADER, AnsiColors.BOLD))


if __name__ == "__main__":
    main()