#!/bin/bash

COUNT=3
CMD="sleep 10"
WAIT_FOR_CHILDREN=true


while [[ $# -gt 0 ]]; do
    case "$1" in
        -n|--number)
            COUNT="$2"
            shift 2
            ;;
        -c|--command)
            shift
            if [ $# -eq 0 ]; then
                echo "command requires a value"
                exit 1
            fi
            CMD="$1"
            shift
            while [ $# -gt 0 ]; do
                case "$1" in
                    -n|--number|--nowait)
                        break
                        ;;
                    *)
                        CMD="$CMD $1"
                        shift
                        ;;
                esac
            done
            ;;
        --nowait)
            WAIT_FOR_CHILDREN=false
            shift
            ;;
        *)
            echo "unknown option: $1"
            exit 1
            ;;
    esac
done

if ! [[ "$COUNT" =~ ^[0-9]+$ ]] || [[ "$COUNT" -le 0 ]]; then
    echo "count must be a positive integer"
    exit 1
fi

echo "Parent PID: $$"
echo "Spawning $COUNT child processes"
echo "Command: $CMD"

declare -a CHILD_PIDS=()

for ((i=1; i<=COUNT; i++)); do
    bash -c "$CMD" &
    pid=$!
    CHILD_PIDS+=("$pid")
    echo "spawned child $i → PID $pid"
done


echo "All child PIDs: ${CHILD_PIDS[*]}"

if $WAIT_FOR_CHILDREN; then
    echo "waiting for child processes to exit..."
    for pid in "${CHILD_PIDS[@]}"; do
        wait "$pid"
        echo "child PID $pid exited with  $?"
    done
    echo "all children killed."
else
    echo "not waiting for children (may create zombies)."
fi
