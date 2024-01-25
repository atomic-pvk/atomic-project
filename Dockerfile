FROM alpine:latest

# Update and install necessary packages
RUN apk update && apk add \
    build-base \
    gcc-arm-none-eabi \
    newlib-arm-none-eabi \
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

# Set the working directory
WORKDIR /FreeRTOS

# Copy autoRunDemo.sh to the container
COPY docker/autoRunDemo.sh /FreeRTOS/autoRunDemo.sh

# Copy demo to the container
COPY docker/FreeRTOS_Demo.zip /FreeRTOS/FreeRTOS_Demo.zip

# Make autoRunDemo.sh executable
RUN chmod +x /FreeRTOS/autoRunDemo.sh

# Expose port 22 for SSH
EXPOSE 22

# Start SSH server when container launches
CMD ["/usr/sbin/sshd", "-D"]
