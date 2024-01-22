FROM alpine:latest

# Update and install necessary packages
RUN apk update && apk add \
    build-base \
    gcc-arm-none-eabi \
    newlib-arm-none-eabi \
    gdb-multiarch \
    qemu-system-arm \
    git

# Clone FreeRTOS source code (modify the branch/tag as needed)
RUN git clone https://github.com/FreeRTOS/FreeRTOS.git --recurse-submodules /FreeRTOS

# Set the working directory
WORKDIR /FreeRTOS

# Any additional setup or scripts can be added here

CMD ["/bin/sh"]
