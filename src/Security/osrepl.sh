#!/bin/bash

# Configuration
KERNEL="./dispatcher"

# Colors for CLI
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check if kernel exists/is compiled
if [ ! -f "$KERNEL" ]; then
    echo -e "${RED}Error: Kernel dispatcher not found.${NC}"
    echo "Please compile the kernel first: gcc kernel_dispatcher.c -o kernel_dispatcher"
    exit 1
fi

clear
echo -e "${BLUE}=======================================${NC}"
echo -e "${BLUE}      Mini OS - User Shell (v1.0)      ${NC}"
echo -e "${BLUE}=======================================${NC}"
echo "Type 'help' for available commands or 'exit' to quit."
echo "Protection Boundary is ACTIVE."
echo "---------------------------------------"

while true; do
    # Display prompt with current user
    echo -ne "${GREEN}MiniOS(${USER})>${NC} "
    read input_line

    # Split input into array
    IFS=' ' read -r -a args <<< "$input_line"
    cmd="${args[0]}"

    # Handle internal shell commands
    if [ "$cmd" == "exit" ]; then
        echo "Logging out..."
        break
    elif [ "$cmd" == "help" ]; then
        echo -e "\nAvailable System Commands:"
        echo -e "  ${GREEN}plist${NC}    : List Processes (User)"
        echo -e "  ${GREEN}meminfo${NC}  : Show Memory (User)"
        echo -e "  ${GREEN}mutex${NC}    : Test Locking (User)"
        echo -e "  ${RED}sched${NC}    : Scheduler Adj (Admin)"
        echo -e "  ${RED}kill${NC}     : Kill Process (Dangerous)"
        echo -e "  ${RED}wipe${NC}     : Clear Cache (Dangerous)"
        echo -e "  help     : Show this menu"
        echo -e "  exit     : Shutdown shell\n"
        continue
    elif [ -z "$cmd" ]; then
        continue
    fi

    # --- CROSSING THE PROTECTION BOUNDARY ---
    # The shell does NOT execute the logic. It passes the request 
    # to the kernel_dispatcher.
    
    $KERNEL $input_line
    
    # Capture exit status from Kernel
    status=$?
    if [ $status -ne 0 ]; then
        echo -e "${RED}[Shell] Command failed or denied by kernel.${NC}"
    fi
    echo ""
done