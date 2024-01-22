FROM alpine:latest

# Update and install necessary packages
RUN apk update && apk add \
    build-base \
    gcc-arm-none-eabi \
    gdb-multiarch \
    qemu-system-arm \
    git \
    vim \
    openssh

# Create a user for SSH login
RUN adduser -D username && \
    echo "username:password" | chpasswd

# Generate host keys (required for SSHD to run)
RUN ssh-keygen -A

# Clone FreeRTOS source code
RUN git clone https://github.com/FreeRTOS/FreeRTOS.git --recurse-submodules /FreeRTOS

# Set the working directory
WORKDIR /FreeRTOS

# Expose port 22 for SSH
EXPOSE 22

# Start SSH server when container launches
CMD ["/usr/sbin/sshd", "-D"]
