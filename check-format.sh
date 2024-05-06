#!/bin/bash

# Start searching from the root of the project directory
start_dir="."

# Find all C++ and C header and source files, check if they need formatting
find $start_dir -type f \( -iname "*.cpp" -or -iname "*.hpp" -or -iname "*.c" -or -iname "*.h" \) -exec sh -c '
    for file do
        clang-format -style=file -output-replacements-xml "$file" | grep "<replacement " >/dev/null
        if [ $? -eq 0 ]; then
            echo "File requires formatting: $file"
            exit 1
        fi
    done
' sh {} +
