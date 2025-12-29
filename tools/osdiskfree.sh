#!/bin/bash


THRESHOLD=10   
TARGET="$1"

if [ -z "$TARGET" ]; then
    echo "usage: $0 <path>"
    exit 1
fi

if [ ! -e "$TARGET" ]; then
    echo "path does not exist."
    exit 1
fi

DF_OUTPUT=$(df -hT "$TARGET" 2>/dev/null | tail -1)

if [ -z "$DF_OUTPUT" ]; then
    echo "unable to read disk info."
    exit 1
fi

FILESYSTEM=$(echo "$DF_OUTPUT" | awk '{print $1}')
FSTYPE=$(echo "$DF_OUTPUT" | awk '{print $2}')
SIZE=$(echo "$DF_OUTPUT" | awk '{print $3}')
USED=$(echo "$DF_OUTPUT" | awk '{print $4}')
AVAILABLE=$(echo "$DF_OUTPUT" | awk '{print $5}')
USE_PERCENT=$(echo "$DF_OUTPUT" | awk '{print $6}' | tr -d '%')
MOUNT=$(echo "$DF_OUTPUT" | awk '{print $7}')

FREE_PERCENT=$((100 - USE_PERCENT))

echo "Filesystem     : $FILESYSTEM"
echo "Filesystem Type: $FSTYPE"
echo "Mounted On     : $MOUNT"
echo "Total Size     : $SIZE"
echo "Used Space     : $USED"
echo "Free Space     : $AVAILABLE"
echo "Usage          : $USE_PERCENT%"
echo "Free Percent   : $FREE_PERCENT%"

if [ "$FREE_PERCENT" -lt "$THRESHOLD" ]; then
    echo "free disk space is below $THRESHOLD%!"
    exit 2
fi

echo "disk space is within safe limits."
exit 0
