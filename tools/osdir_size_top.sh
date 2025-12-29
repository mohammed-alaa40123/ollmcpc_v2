#!/bin/bash


read -rp "Enter directory path: " DIR

if [[ ! -d "$DIR" ]]; then
    echo "'$DIR' is not a valid directory"
    exit 1
fi

read -rp "How many top heavy items to show? [default 10]: " TOPN
TOPN=${TOPN:-10}

echo
echo "Analyzing: $DIR -------------------------------------"

TOTAL_SIZE=$(du -sh "$DIR" 2>/dev/null | cut -f1)
echo "total size: $TOTAL_SIZE"
echo

echo "top $TOPN heaviest files/folders: -------------------------------------"

du -h --max-depth=1 "$DIR" 2>/dev/null \
    | sort -hr \
    | head -n "$((TOPN + 1))"
BIG_DIR=$(du -h --max-depth=1 "$DIR" 2>/dev/null | sort -hr | sed -n '2p')

if [[ -n "$BIG_DIR" ]]; then
    echo "largest subdirectory/file:"
    echo "  $BIG_DIR"
fi

exit 0
