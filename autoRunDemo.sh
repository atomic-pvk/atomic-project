#!/bin/ash

set -e

RED="\e[31m"
GREEN="\e[32m"
BOLDGREEN="\e[1;${GREEN}"
ENDCOLOR="\e[0m"

run() {
    echo -e "${BOLDGREEN}running cmd ${ENDCOLOR}$1"
    eval "$1"
}

run "cd FreeRTOS_Demo/Demo/gcc"
run "make clean"
run "make -j"
run "qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -kernel ./output/RTOSDemo.out -monitor none -nographic -serial stdio"
