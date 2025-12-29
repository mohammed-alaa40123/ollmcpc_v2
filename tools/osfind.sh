#!/bin/bash


MAX_DEPTH=5

BASE_DIR="$1"
NAME_PATTERN="$2"
TYPE_CHOICE="$3"

if [ -z "$BASE_DIR" ] || [ -z "$NAME_PATTERN" ]; then
    echo "usage: $0 <base_dir> <name_pattern> [files|dirs|all]"
    exit 1
fi

if [ ! -d "$BASE_DIR" ]; then
    echo "'$BASE_DIR' is not a valid directory."
    exit 1
fi

case "$TYPE_CHOICE" in
    files)
        TYPE_OPT="-type f"
        ;;
    dirs)
        TYPE_OPT="-type d"
        ;;
    all|"")
        TYPE_OPT=""
        ;;
    *)
        echo "Error"
        exit 1
        ;;
esac

echo "Searching..."
echo "Base directory : $BASE_DIR"
echo "Name pattern   : $NAME_PATTERN"
echo "Max depth      : $MAX_DEPTH"

RESULTS=$(find "$BASE_DIR" -maxdepth "$MAX_DEPTH" $TYPE_OPT -name "$NAME_PATTERN" 2>/dev/null)

if [ -z "$RESULTS" ]; then
    echo "No matching items found."
    exit 0
fi

echo "Matches found:"
echo "$RESULTS"

exit 0
