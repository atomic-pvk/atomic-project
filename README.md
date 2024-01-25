# ATOMIC Project Documentation

Welcome to ATOMIC! This guide will assist you in setting up and using the FreeRTOS environment for your development needs.

## Quick Start Guide


### Install Docker: 
 Docker is required for containerization. Follow the installation guide available [here](https://docs.docker.com/get-docker/) to set it up on your system.

### (Optional) Download FreeRTOS Source Code

If you want direct access to the source code and libraries, download the FreeRTOS source code from the official site. Use this [link](https://www.freertos.org/a00104.html) for direct access.

_Alternatively_, you can clone the FreeRTOS repository using the following Git command:

```bash
git clone https://github.com/FreeRTOS/FreeRTOS.git --recurse-submodules ./FreeRTOS
```

---

### Docker Image Management

#### Building the Docker Image

Build the Docker image using the following command:

```bash
docker build -t freertos .
```

#### Running the Docker Image

To run the Docker image, use:

```bash
docker run -d -p 2222:22 freertos
```

#### Stopping the Docker Image

To stop the running Docker image, execute:

```bash
docker stop freertos
```

#### Removing the Docker Image

For removing the Docker image, enter:

```bash
docker rmi freertos
```

---

### Developing with Docker

1. **Accessing the Docker Image via SSH**:
   Connect to the Docker image with SSH using:

   ```bash
   ssh username@localhost -p 2222
   ```

2. **Using VSCode for Development**:
   Enhance your development experience with the `Remote Explorer` and `Remote - SSH` extensions in VSCode. This setup allows you to edit files directly in the Docker environment. Connect to the Docker image via VSCode by selecting Remote-SSH: Connect to Host, then add:

   ```bash
   ssh username@localhost -p 2222
   ```

   Choose the 'freertos' container to open a VSCode window linked to the Docker image files.

---

### Building and Running the Project

#### Automatic

The container includes a file named `autoRunDemo.sh`. Running this ash script automatically runs the demo in its standard configuration. To run it in debug mode, refer to the FreeRTOS documentation or follow the manual instructions below.

Command to run:

```bash
ash ./autoRunDemo.sh
```

#### Manual

1. **Preparing the Build**:
   Navigate to the build directory:

   ```bash
   cd FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC/build/gcc
   ```

2. **Build Commands**:
   Clean the build environment:

   ```bash
   make clean
   ```

   Compile the project (use `-j` for parallel compilation):

   ```bash
   make -j
   ```

3. **Running the Demo**:
   Execute the demo with:

   ```bash
   qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -kernel ./output/RTOSDemo.out -monitor none -nographic -serial stdio
   ```

   For debug mode, use:

   ```bash
   qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -kernel ./output/RTOSDemo.out -monitor none -nographic -serial stdio -s -S
   ```

#### Modifying the Demo

   To run the full demo, modify the `mainCREATE_SIMPLE_BLINKY_DEMO_ONLY` variable in `main.c` located at `FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC` to a value other than `1`.

---

### Additional Resources

- [FreeRTOS on QEMU MPS2 AN385 Model](https://www.freertos.org/freertos-on-qemu-mps2-an385-model.html)
- [FreeRTOS Demo Applications](https://www.freertos.org/a00102.html#comprehensive_demo)
- [FreeRTOS API Reference](https://www.freertos.org/a00106.html)
- [FreeRTOS Kernel Quick Start Guide](https://www.freertos.org/FreeRTOS-quick-start-guide.html)
- [Understanding the FreeRTOS Directory Structure](https://www.freertos.org/a00017.html)
