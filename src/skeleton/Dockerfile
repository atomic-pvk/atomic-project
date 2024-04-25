FROM alpine:latest

# Update and install necessary packages
RUN apk update && apk add \
    build-base \
    gcc \
    git \
    vim \
    iproute2 \
    openssh

# Create a user for SSH login
RUN adduser -D username && \
    echo "username:password" | chpasswd

# Generate host keys (required for SSHD to run)
RUN ssh-keygen -A

# Set the working directory
WORKDIR /ntp-client

# Expose port 22 for SSH
EXPOSE 22

# Start SSH server when container launches
CMD ["/usr/sbin/sshd", "-D"]
