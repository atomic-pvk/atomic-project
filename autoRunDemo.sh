#!bin/ash

cd FreeRTOS_Demo/Demo/gcc
make clean
make -j
qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -kernel ./output/RTOSDemo.out -monitor none -nographic -serial stdio
