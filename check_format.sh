#!/bin/bash

# Start searching from the root of the project directory
start_dir="."

# Initialize an empty string to hold files that require formatting
files_to_format=""

# Find all C++ and C header and source files, check if they need formatting
find $start_dir -type f \( -iname "*.cpp" -or -iname "*.hpp" -or -iname "*.c" -or -iname "*.h" \) -exec sh -c '
    for file do
        # Check each file for needed formatting replacements
        if clang-format -style=file -output-replacements-xml "$file" | grep -q "<replacement "; then
            # Append the file path to the list
            files_to_format="$files_to_format$file\n"
        fi
    done
    # If files require formatting, print them and exit with error
    if [ -n "$files_to_format" ]; then
        echo -e "These files require formatting:\n$files_to_format"
        exit 1
    else
        echo "All files are properly formatted."
    fi
' sh {} + 
