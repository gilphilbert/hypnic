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
EXT=""

if [[ -f /etc/os-release ]]; then
  if grep -Fq "piCorePlayer" /etc/os-release; then
    echo "PiCorePlayer system detected"
    MODE="pcp"
  fi
fi

# if it's not pCp, is it systemd-based
if [[ "$MODE" == "fail" ]]; then
  # try to detect OS
  for LOC in "/lib" "/usr/lib"; do
    if [[ -d "$LOC/systemd/system-shutdown" ]]; then
      DIR="$LOC/systemd"
      MODE="systemd"
      echo "systemd-based system detected at $DIR"
      break
    fi
  done
fi

if [[ "$MODE" == "fail" ]]; then
  echo "Could not detect your OS. Please open a ticket at https://github.com/gilphilbert/hypnic/issues"
  exit 1
fi

if [[ "$MODE" == "pcp" ]]; then
  DEST_FILE=/etc/sysconfig/tcedir/optional/hypnic.tcz
  wget -qO $DEST_FILE https://raw.githubusercontent.com/gilphilbert/hypnic/main/sbc/pcp/hypnic.tcz && su -c "tce-load -i $DEST_FILE" tc && /bin/hypnic.sh --install
  if ! grep -Fq "hypnic.tcz" /etc/sysconfig/tcedir/onboot.lst; then
    echo "hypnic.tcz" >> /etc/sysconfig/tcedir/onboot.lst
  fi
  EXT="sh"
  echo -e "\nNote: SAFE notification requires a reboot\n"
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
  EXT="py"
fi


echo ""
echo "Installation complete. The default pins are:"
echo "  HALT: GPIO17"
echo "  SAFE: GPIO27"
echo ""
echo "To change these pins, please run:"
echo "  sudo hypnic.$EXT"
echo ""
