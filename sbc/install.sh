#!/bin/bash

#-----------------------------------------
# Install script for Hypnic Power Manager
#-----------------------------------------
CYAN="\033[0;36m"
NC="\033[0m"
echo ""
echo ""
echo -e "${CYAN}██${NC}╗░░${CYAN}██${NC}╗${CYAN}██${NC}╗░░░${CYAN}██${NC}╗${CYAN}██████${NC}╗░${CYAN}███${NC}╗░░${CYAN}██${NC}╗${CYAN}██${NC}╗░${CYAN}█████${NC}╗░"
echo -e "${CYAN}██║░░██║╚██╗░██╔╝██╔══██╗████╗░██║██║██╔══██╗"
echo -e "${CYAN}███████║░╚████╔╝░██████╔╝██╔██╗██║██║██║░░╚═╝"
echo -e "${CYAN}██╔══██║░░╚██╔╝░░██╔═══╝░██║╚████║██║██║░░██╗"
echo -e "${CYAN}██║░░██║░░░██║░░░██║░░░░░██║░╚███║██║╚█████╔╝"
echo -e "${CYAN}╚═╝░░╚═╝░░░╚═╝░░░╚═╝░░░░░╚═╝░░╚══╝╚═╝░╚════╝░"
echo -e "${NC}"
echo ""

exit

if [ "$EUID" -ne 0 ]
  then
    echo "This installer requires elevated privileges to exectute."
    echo "Please run with sudo to continue."
  exit
fi

DIR=""

for LOC in "/lib" "/usr/lib"; do
  if [[ -f "$LOC/systemd/systemd-shutdown" ]]; then
    DIR="/usr/lib/systemd/"
  fi
done

if [[ $DIR == "" ]]; then
  echo "Could not find your systemd installation. Hypnic is currently only supported on systems running systemd"
  exit
fi

echo "Downloading and installing application..."
wget -qO /etc/hypnic.conf https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/hypnic.conf
wget -qO /usr/bin/hypnic.py https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/hypnic.py
wget -qO "$DIR/systemd/system/hypnic.service" https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/hypnic.service
wget -qO "$DIR/systemd/system-shutdown/hypnic-shutdown.py" https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/hypnic-shutdown.py
chmod 755 /usr/bin/hypnic.py
chmod 755 "$DIR/systemd/system-shutdown/hypnic-shutdown.py"

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
