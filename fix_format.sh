#!/bin/bash

# Start from the root of the project directory
start_dir="."

# Find all C++ and C header and source files and format them
find $start_dir -type f \( -iname "*.cpp" -or -iname "*.hpp" -or -iname "*.c" -or -iname "*.h" \) -exec clang-format -i -style=file {} \;

echo "All applicable files have been formatted."
