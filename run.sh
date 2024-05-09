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

run "./setup_network.sh"
run "cd src"
run "make clean"
run "make -j"

QEMU_MAC_ADDRESS="52:54:00:12:34:AD"
CMD="qemu-system-arm -machine mps2-an385 -cpu cortex-m3 \
    -kernel ./build/freertos_tcp_mps2_demo.axf \
    -netdev tap,id=mynet0,ifname=virbr0-nic,script=no,downscript=no \
    -net nic,macaddr=$QEMU_MAC_ADDRESS,model=lan9118,netdev=mynet0 \
    -object filter-dump,id=tap_dump,netdev=mynet0,file=/tmp/qemu_tap_dump \
    -display gtk -m 16M -nographic -serial stdio \
    -rtc base=utc,clock=host \
    -monitor null -semihosting -semihosting-config enable=on,target=native \
    -d guest_errors,cpu_reset -D qemu.log"

run "$CMD"
