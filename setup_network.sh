#!/bin/ash

VM_IP_ADDRESS="172.21.0.3"
VM_CIDR="16"
VM_NETWORK_INTERFACE="eth0"

ip link add virbr0 type bridge
ip tuntap add dev virbr0-nic mode tap

ip addr add $VM_IP_ADDRESS/$VM_CIDR dev virbr0

brctl addif virbr0 $VM_NETWORK_INTERFACE
brctl addif virbr0 virbr0-nic

ip link set virbr0 up
ip link set virbr0-nic up
