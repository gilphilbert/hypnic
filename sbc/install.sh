#!/bin/bash

#-----------------------------------------
# Install script for Hypnic Power Manager
#-----------------------------------------

if [ "$EUID" -ne 0 ]
  then
    echo "This installer requires elevated privileges to exectute."
    echo "Please run with sudo to continue."
  exit
fi

echo "Downloading and installing files..."

wget -O /etc/hypnic.conf https://github.com/gilphilbert/ hypnic.conf
#cp hypnic.service /usr/lib/systemd/system/
#cp hypnic-service.py /usr/bin/
#cp hypnic-shutdown.py /usr/lib/systemd/system-shutdown/
#systemctl daemon-reload
#systemctl enable hypnic.service
#systemctl start hypnic.service
