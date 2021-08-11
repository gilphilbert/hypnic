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

if [ $(/usr/bin/id -u) -ne 0 ]; then
  echo "This installer requires elevated privileges to exectute."
  echo "Please run with sudo to continue."
  exit
fi

DIR=""
MODE="fail"

if [[ -f /etc/os-release ]]; then
  if grep -Fq "piCorePlayer" /etc/os-release; then
    echo "PiCorePlayer system detected"
    MODE="pcp"
  fi
else
  # try to detect OS
  for LOC in "/lib" "/usr/lib"; do
    if [[ -f "$LOC/systemd/systemd-shutdown" ]]; then
      DIR="$LOC/systemd"
      MODE="systemd"
      echo "systemd-based system detected at $DIR"
    fi
  done
fi

if [[ "$MODE" == "fail" ]]; then
  echo "Could not detect your OS. Please open a ticket at https://github.com/gilphilbert/hypnic/issues"
  exit 1
fi

if [[ "$MODE" == "pcp" ]]; then
  DEST_FILE=/etc/sysconfig/tcedir/optional/hypnic.tcz
  wget -qO $DEST_FILE https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/pcp/hypnic.tcz
  su -c 'tce-load -i hypnic.tcz' tc
fi

if [[ "$MODE" == "systemd" ]]; then
  echo "Installing required libraries..."
  DEBIAN_FRONTEND=noninteractive apt-get install -qq python3-dev python3-rpi.gpio < /dev/null > /dev/null

  echo "Downloading and installing application..."
  wget -qO /etc/hypnic.conf https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/hypnic.conf
  wget -qO /usr/bin/hypnic.py https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/hypnic.py
  wget -qO "$DIR/system/hypnic.service" https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/hypnic.service
  wget -qO "$DIR/system-shutdown/hypnic-shutdown.py" https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/hypnic-shutdown.py
  chmod 755 /usr/bin/hypnic.py
  chmod 755 "$DIR/system-shutdown/hypnic-shutdown.py"

  echo "Configuring service..."
  systemctl daemon-reload
  systemctl enable hypnic.service
  systemctl start hypnic.service
fi


echo ""
echo "Installation complete. The default pins are:"
echo "  HALT: GPIO17"
echo "  SAFE: GPIO27"
echo ""
echo "To change these pins, please run:"
echo "  sudo hypnic.py"
echo ""
