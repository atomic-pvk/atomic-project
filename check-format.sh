#!/bin/bash

# Find all C++ files and run clang-format on them
find . -regex '.*\.\(cpp\|hpp\|c\|h\)' -exec clang-format -style=file -output-replacements-xml {} + | grep "<replacement " >/dev/null
if [ $? -ne 1 ]; then
    echo "Code formatting differs from clang-format's style"
    exit 1
fi
