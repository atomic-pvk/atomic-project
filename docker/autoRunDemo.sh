#!bin/ash

folder="./FreeRTOS_Demo"

if [ ! -d "$folder" ]; then
  unzip FreeRTOS_Demo.zip
fi

cd FreeRTOS_Demo/Demo/CORTEX_MPS2_QEMU_IAR_GCC/build/gcc
make clean
make -j
qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -kernel ./output/RTOSDemo.out -monitor none -nographic -serial stdio
