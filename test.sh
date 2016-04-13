#!/usr/bin/env bash

TESTCASE=$(find ./test/ -name "*.cmm")

for file in $TESTCASE; do
    echo test $file
    NAME=$(echo "$file"|cut -d '/' -f 3|head -c -5)
    ASM=${NAME}.S
    EXE=${NAME}.out

    ./ncc $file $ASM
    if [ $? -ne 0 ]; then
        echo "Compilation failed."
        continue
    fi

    #spim -file $ASM 2> /dev/null # Only check the validation of the asm file
    gcc $ASM -o $EXE
    if [ $? -ne 0 ]; then
        echo "Compilation incorrect."
    fi
    mv $ASM ./test/
    mv $EXE ./test/
done

rm *.ir

exit 0
