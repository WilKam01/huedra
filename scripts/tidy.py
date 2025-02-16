import os
import subprocess
import argparse
import re

# Text colors
BLACK = "\033[30m"
RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
BLUE = "\033[34m"
MAGENTA = "\033[35m"
CYAN = "\033[36m"
WHITE = "\033[37m"

# Bright colors
BRIGHT_BLACK = "\033[90m"
BRIGHT_RED = "\033[91m"
BRIGHT_GREEN = "\033[92m"
BRIGHT_YELLOW = "\033[93m"
BRIGHT_BLUE = "\033[94m"
BRIGHT_MAGENTA = "\033[95m"
BRIGHT_CYAN = "\033[96m"
BRIGHT_WHITE = "\033[97m"

# Background colors
BG_BLACK = "\033[40m"
BG_RED = "\033[41m"
BG_GREEN = "\033[42m"
BG_YELLOW = "\033[43m"
BG_BLUE = "\033[44m"
BG_MAGENTA = "\033[45m"
BG_CYAN = "\033[46m"
BG_WHITE = "\033[47m"

# Styles
BOLD = "\033[1m"
DIM = "\033[2m"
UNDERLINE = "\033[4m"
BLINKING = "\033[5m"
REVERSE = "\033[7m"
HIDDEN = "\033[8m"

# Reset color to default
RESET = "\033[0m"

parser = argparse.ArgumentParser()
parser.add_argument("dirs", help="Root directories with files to check", type=str, nargs="*")
parser.add_argument("-f", "--file", help="File to check", type=str)

num_files = 0
num_errors = 0
num_warnings = 0

def run_clang_tidy_dir(dir):
    for root, dirs, files in os.walk(dir):
        for file in files:
            run_clang_tidy_file(root, file)

def run_clang_tidy_file(root, file):
    global num_files
    global num_errors
    global num_warnings

    if(file.endswith(".hpp") or file.endswith(".cpp")):
        path = os.path.join(root, file)
        command = ["clang-tidy", "-p", "build", "--use-color", f"{path}"]
        try:
            print(f"{UNDERLINE}{BOLD}{BRIGHT_WHITE}{path}:{RESET}")
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

            if result.returncode != 0:
                print(f"Found {BRIGHT_RED}errors{RESET} running clang-tidy:")
                num_files += 1
                num_errors += len(re.findall("error:", result.stdout))
                num_warnings += len(re.findall("warning:", result.stdout))

            if len(re.findall("warning:", result.stdout)) != 0:
                print(f"Found {BRIGHT_YELLOW}warnings{RESET} running clang-tidy:")
                num_files += 1
                num_errors += len(re.findall("error:", result.stdout))
                num_warnings += len(re.findall("warning:", result.stdout))
                
            if result.returncode != 0 or len(re.findall("warning:", result.stdout)) != 0:
                print(result.stdout)

        except Exception as e:
            print(f"Failed to run clang-tidy on path: {path}")
            print(e)

if __name__ == "__main__":
    args = parser.parse_args()
    os.system("color") # For colored output to work

    print("Running clang-tidy...")

    if args.file is not None:
        run_clang_tidy_file("", args.file)

    for dir in args.dirs:
        run_clang_tidy_dir(dir)
    
    if num_files != 0:
        print(f"Found {num_errors} {BRIGHT_RED}error(s){RESET} and {num_warnings} {BRIGHT_YELLOW}warning(s){RESET} in {num_files} file(s)")
    else:
        print(f"{BRIGHT_GREEN}0 errors/warnings found!{RESET}")
