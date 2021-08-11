#!/bin/sh
echo "Hypnic Power Manager (piCore Player)"

if [ $(/usr/bin/id -u) -ne 0 ]; then
  echo "This script needs to be run as root (sudo)" >&2
  exit 1
fi

# location of the configuration file
CONF_FILE="/etc/hypnic.conf"
# variables for pins
HALT_PIN=0
SAFE_PIN=0

PINS_CHANGED=0

readpins() {
  # check to see if the config file exists
  if [[ -f $CONF_FILE ]]
  then
    # grab the pin configuration
    HALT_PIN=`grep HALT_PIN $CONF_FILE|awk -F"=" '{ print $2 }'`
    SAFE_PIN=`grep SAFE_PIN $CONF_FILE|awk -F"=" '{ print $2 }'`
  else
    # no configuration, error and exit
    echo "Error: configuration missing: /etc/hypnic.conf (not found)"
    exit 1
  fi
  # check to see if we got pins from the config file
  if [[ $HALT_PIN == 0 || $SAFE_PIN == 0 ]]
  then
    # one (or both) pin(s) is/are missing
    echo "Error: Invalid configuration"
    exit 1
  fi
}

listen() {
  pcp-gpio mode $HALT_PIN input
  pcp-gpio mode $HALT_PIN up
  sleep 1
  # start listening
  echo "Listening for HALT signal"

  # loop forever
  while [[ 1 == 1 ]]
  do
    # get the current pin state
    ST=`pcp-gpio read $HALT_PIN`
    # if it's gone low (halt signal received)
    if [[ $ST == "0" ]]
    then
        # log what's happened
        echo "Recieved HALT Signal"
        # shutdown the system, running backups, etc.
        pcp bs
        exit 0
    fi
    # sleep for each loop (saves CPU)
    sleep 0.3
  done
}

quit() {
  if [[ $PINS_CHANGED -eq 1 ]]; then
    echo "Make sure you use 'filetool.sh -b' to save changes"
  fi
  echo "Bye!"
  exit 0
}

usage() {
# log the pin configuration
  echo "Configure or run the Hypnic service"
  echo ""
  echo "Usage:"
  echo "  hypnic.sh [-d|-h]"
  echo ""
  echo "Options"
  echo "-d|--daemon  start daemon"
  echo "-h|--help    help (this screen)"
  echo ""
  exit 1
}

setpin() {
  echo $1
  echo $2
  sed -i "s/^\($1_PIN=\).*/\1$2/" $CONF_FILE
  readpins
}

saveconfig() {
  if [ -z $(grep "hypnic.conf" /opt/.filetool.lst) ]; then
    ANSWER="V"
    while [[ "$ANSWER" != "" && "$ANSWER" != "n" && "$ANSWER" != "y" ]]; do
      echo -n "Hypnic configuration is not included in your backup. Add it now? (Y/n) "
      read ANSWER
    done
    if [[ "$ANSWER" == "" || "$ANSWER" == "y" ]]; then
      echo "Adding configuration to backup"
      echo "etc/hypnic.conf" >> /opt/.filetool.lst
      echo "Performing backup"
      filetool.sh -b
    else
      echo -e "\nRemember to add \"etc/hypnic.conf\" into /opt/.filetool.lst and run a\nbackup otherwise your changes will be lost.\n"
      PINS_CHANGED=1
      echo -n "Press [Enter] to continue..."
      read
    fi
  else
    echo "Performing backup"
    filetool.sh -b
  fi
}

changesafe() {
  echo "Updating SAFE pin configuration"
  mount /dev/mmcblk0p1 /mnt/mmcblk0p1
  if [ -z $(grep "gpio-poweroff" /mnt/mmcblk0p1/config.txt) ]; then
    #add the line to the file
    sed -i "/cmdline cmdline.txt/a dtoverlay=gpio-poweroff,gpiopin=$1" /mnt/mmcblk0p1/config.txt
  else
    # update the file
    sed -i "s/^\(dtoverlay=gpio-poweroff,gpiopin=\).*/\1$1/" /mnt/mmcblk0p1/config.txt
  fi
  sync
  umount /mnt/mmcblk0p1
}

function is_integer() {
    [ "$1" -eq "$1" ] > /dev/null 2>&1
    return $?
}

configuration() {
  while [ 1 -eq 1 ]; do
    TYPE=""
    GPIO=0
    while [ "$TYPE" == "" ]; do
	  clear
	  echo "Hypnic Power Manager (piCore Player)"
	  echo ""
      echo "Current configuration:"
      echo " 1) HALT pin: $HALT_PIN"
      echo " 2) SAFE pin: $SAFE_PIN"
      echo ""
      echo -n "Enter pin to change or press [Enter] to exit: "
      read pintype
      case $pintype in
        "1")
          TYPE="HALT"
          ;;
        "2")
          TYPE="SAFE"
          ;;
        "")
          quit
          ;;
        *)
          echo "Invalid selection"
          ;;
      esac
    done
    # set to value outside bounds
    newpin="30"
    #check to see if new pin fits within bounds
    while [ $newpin -lt 0 ] || [ $newpin -gt 27 ]; do
      echo -n "Enter new GPIO pin or press [Enter] to exit (0-27): "
      read newpin
      #request to leave
      if [ "$newpin" == "" ]; then
        quit
      fi
      # if we get a non-integer, set back out of bounds
      if ! is_integer $newpin; then
        newpin="30"
      fi
    done
    GPIO=$newpin
    setpin $TYPE $GPIO
    if [[ "$TYPE" == "SAFE" ]]; then
      changesafe $GPIO
    fi
    saveconfig
  done
}

readpins

if [[ $# == 0 ]]; then
  configuration
fi

while test $# -gt 0; do
  case "$1" in
    -h|--help)
      usage
      shift
      ;;
    -d|--daemon)
      echo "Daemon"
      listen
      shift
      ;;
    -i|--install)
      changesafe $SAFE_PIN
      shift
      ;;
    *)
      echo "Unknown option: $1"
      usage
      shift
      ;;
  esac
done

quit

