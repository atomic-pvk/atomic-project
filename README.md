# ATOMIC

Hello World!

## Using the docker image

First build the docker image:
```bash
docker build -t freertos-qemu .
```

Then run the docker image:
```bash
docker run -it --rm freertos-qemu
```

You then follow the steps under the [
Building and executing the demo application - GCC Makefile](https://www.freertos.org/freertos-on-qemu-mps2-an385-model.html) section of the FreeRTOS website.

Alternatively follow the steps below:

```bash
cd FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC/build/gcc
```

```bash
make -j4
```

```bash
qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -kernel ./output/RTOSDemo.out -monitor none -nographic -serial stdio
```

This will run the simple blicky demo. You can run the full demo by modifying the mainCREATE_SIMPLE_BLINKY_DEMO_ONLY variable in the main.c file in the FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC folder.

