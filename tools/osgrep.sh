#!/bin/bash

pattern="$1"
target="$2"
flag="$3"

if [ -z "$pattern" ] || [ -z "$target" ]; then
    echo "usage: $0 <pattern> <path> [--ci]"
    exit 1
fi

if [ ! -e "$target" ]; then
    echo "'$target' does not exist."
    exit 2
fi

GREP_OPTS="-n -H --color=auto"

if [ "$flag" = "--ci" ] || [ "$flag" = "-i" ] || [ "$flag" = "ci" ]; then
    GREP_OPTS="$GREP_OPTS -i"
fi

echo -e "\nSearching..."
if [ -f "$target" ]; then
    grep $GREP_OPTS "$pattern" "$target"
    status=$?

elif [ -d "$target" ]; then
    grep $GREP_OPTS -R "$pattern" "$target"
    status=$?

else
    echo "'$target' is not a regular file or directory."
    exit 3
fi


if [ $status -eq 1 ]; then
    echo "No matches found."

fi

exit 0
