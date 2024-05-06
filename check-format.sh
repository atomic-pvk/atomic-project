#!/bin/bash

start_dir="."
echo "Starting format check in directory: $start_dir"

file_count=$(find $start_dir -type f \( -iname "*.cpp" -or -iname "*.hpp" -or -iname "*.c" -or -iname "*.h" \) -not -path "./external/*" | wc -l)
echo "Total files to check: $file_count"

find $start_dir -type f \( -iname "*.cpp" -or -iname "*.hpp" -or -iname "*.c" -or -iname "*.h" \) -not -path "./external/*" -exec echo "Processing file: {}" \; -exec clang-format -style=file -output-replacements-xml {} + | grep "<replacement " >/dev/null
if [ $? -ne 1 ]; then
    echo "Code formatting differs from clang-format's style"
    exit 1
else
    echo "All files formatted correctly."
fi
