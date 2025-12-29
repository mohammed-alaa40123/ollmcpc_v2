#!/bin/bash

while true; do
    read -rp "myos> " cmd args

    case "$cmd" in
        exit|osshutdown)
            echo "Shutting down Mini-OS..."
            break
            ;;
        "")
            continue
            ;;
        *)
            if command -v "./$cmd.sh" >/dev/null 2>&1; then
                ./"$cmd.sh" $args
            else
                echo "Unknown command"
            fi
            ;;
    esac
done
