#!/bin/bash

echo "Finding devices on your network to ping from PineOS"
echo "=================================================="

# Get the default gateway (router)
GATEWAY=$(ip route | grep default | awk '{print $3}' | head -1)
if [ -n "$GATEWAY" ]; then
    echo "Your Router/Gateway: $GATEWAY"
    echo "  - This is usually your main router"
    echo "  - Try: ping $GATEWAY"
    echo ""
fi

# Get your current IP and network
CURRENT_IP=$(ip route get 8.8.8.8 | grep -oP 'src \K\S+')
NETWORK=$(ip route | grep "$CURRENT_IP" | grep -v default | awk '{print $1}' | head -1)

if [ -n "$NETWORK" ]; then
    echo "Your Network: $NETWORK"
    echo "Your Current IP: $CURRENT_IP"
    echo ""
    
    # Extract network base for scanning
    NETWORK_BASE=$(echo $NETWORK | cut -d'/' -f1 | cut -d'.' -f1-3)
    
    echo "Scanning for active devices on your network..."
    echo "This may take a moment..."
    echo ""
    
    # Quick ping scan of common IPs
    for i in {1..10} {100..110} {200..210}; do
        IP="$NETWORK_BASE.$i"
        if ping -c 1 -W 1 "$IP" &>/dev/null; then
            # Try to get hostname
            HOSTNAME=$(nslookup "$IP" 2>/dev/null | grep "name =" | awk '{print $4}' | sed 's/\.$//')
            if [ -n "$HOSTNAME" ]; then
                echo "Found device: $IP ($HOSTNAME)"
            else
                echo "Found device: $IP"
            fi
            echo "  - Try: ping $IP"
        fi
    done
fi

echo ""
echo "Common devices to try pinging from your OS:"
echo "  ping 8.8.8.8           # Google DNS"
echo "  ping 1.1.1.1           # Cloudflare DNS"
echo "  ping $GATEWAY          # Your router"
echo "  ping $CURRENT_IP       # Your host computer"
echo ""
echo "After booting your OS in VirtualBox:"
echo "1. Run: dhcp eth0        # Get IP address"
echo "2. Run: ifconfig eth0    # Check your OS got an IP"
echo "3. Run: ping [device-ip] # Ping any device above"
echo ""