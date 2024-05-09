#!/bin/ash
VM_IP_ADDRESS="172.21.0.3"
VM_CIDR="16"
VM_NETWORK_INTERFACE="eth0"
BRIDGE="virbr0"
BRIDGE_NIC="virbr0-nic"

if ! ip link show $BRIDGE > /dev/null 2>&1; then

    ip link add $BRIDGE type bridge
    ip tuntap add dev $BRIDGE_NIC mode tap

    ip addr add $VM_IP_ADDRESS/$VM_CIDR dev $BRIDGE

    brctl addif $BRIDGE $VM_NETWORK_INTERFACE
    brctl addif $BRIDGE $BRIDGE_NIC

    ip link set $BRIDGE up
    ip link set $BRIDGE_NIC up
fi

exit 0