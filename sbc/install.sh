#!/bin/bash

#-----------------------------------------
# Install script for Hypnic Power Manager
#-----------------------------------------

echo ""
echo ""
echo "██╗░░██╗██╗░░░██╗██████╗░███╗░░██╗██╗░█████╗░"
echo "██║░░██║╚██╗░██╔╝██╔══██╗████╗░██║██║██╔══██╗"
echo "███████║░╚████╔╝░██████╔╝██╔██╗██║██║██║░░╚═╝"
echo "██╔══██║░░╚██╔╝░░██╔═══╝░██║╚████║██║██║░░██╗"
echo "██║░░██║░░░██║░░░██║░░░░░██║░╚███║██║╚█████╔╝"
echo "╚═╝░░╚═╝░░░╚═╝░░░╚═╝░░░░░╚═╝░░╚══╝╚═╝░╚════╝░"
echo ""
echo ""

if [ "$EUID" -ne 0 ]
  then
    echo "This installer requires elevated privileges to exectute."
    echo "Please run with sudo to continue."
  exit
fi

echo "Downloading and installing application..."
wget -qO /etc/hypnic.conf https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/hypnic.conf
wget -qO /usr/bin/hypnic.py https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/hypnic.py
wget -qO /usr/lib/systemd/system/hypnic.service https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/hypnic.service
wget -qO /usr/lib/systemd/system-shutdown/hypnic-shutdown.py https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/hypnic-shutdown.py

echo "Configuring service..."
systemctl daemon-reload
systemctl enable hypnic.service
systemctl start hypnic.service
echo ""
echo "Installation complete. The default pins are:"
echo "  HALT: GPIO17"
echo "  SAFE: GPIO27"
echo ""
echo "To change these pins, please run:"
echo "  sudo hypnic.py"
echo ""
