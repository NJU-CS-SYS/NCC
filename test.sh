#!/usr/bin/env bash
# Define colors.
BLACK='\033[0;30m'
RED='\033[0;31m'
GREEN='\033[0;32m'
BROWN='\033[0;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
LIGHT_GRAY='\033[0;37m'
DARK_GRAY='\033[1;30m'
LIGHT_RED='\033[1;31m'
LIGHT_GREEN='\033[1;32m'
YELLOW='\033[1;33m'
LIGHT_BLUE='\033[1;34m'
LIGHT_PURPLE='\033[1;35m'
LIGHT_CYAN='\033[1;36m'
WHITE='\033[1;37m'
NO_COLOR='\033[0m'

printf "${YELLOW}Entering ./test${NO_COLOR}\n"
cd test

if [ -z $1 ]; then
    state="compile"
else
    state=$1
fi

TESTCASE=$(find . -name "*.cmm")

for file in $TESTCASE; do
    NAME=$(echo "$file"|cut -d '/' -f 2|head -c -5)
    printf "======== Testing $NAME ========\n"
    ASM=${NAME}.S
    EXE=${NAME}.out
    IR=${NAME}.ir
    DEB=${NAME}.debug

    ../ncc $file $ASM 2> $DEB
    if [ $? -ne 0 ]; then
        printf "${LIGHT_RED}NCC failed.${NO_COLOR}\n"
        continue
    else
        printf "${LIGHT_GREEN}NCC succeed.${NO_COLOR}\n"
    fi

    #spim -file $ASM 2> /dev/null # Only check the validation of the asm file
    gcc $ASM -o $EXE
    if [ $? -ne 0 ]; then
        printf "${LIGHT_RED}GCC failed.${NO_COLOR}\n"
        continue
    else
        printf "${LIGHT_GREEN}GCC succeed.${NO_COLOR}\n"
    fi
    
    if [ $state = "execute" ]; then
        ./$EXE
        result=$?
        if [ $result -ne 0 ]; then
            printf "${LIGHT_RED}Execution failed with exit code $result.${NO_COLOR}\n"
        else
            printf "${LIGHT_GREEN}Execution succeed.${NO_COLOR}\n"
        fi
    fi
    
    if [ -f test.ir ]; then
        mv test.ir $IR
    fi
done

exit 0
