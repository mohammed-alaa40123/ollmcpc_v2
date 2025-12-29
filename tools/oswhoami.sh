#!/bin/bash

echo "User: $(whoami)"
echo "UID/GID: $(id -u)/$(id -g)"
echo "Groups: $(groups)"
echo "Hostname: $(hostname)"
echo "Directory: $(pwd)"
echo "TTY: $(tty)"
