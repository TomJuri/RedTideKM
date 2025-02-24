#!/bin/bash
set -e
sudo insmod out/redtidekm.ko
#major=$(cat /proc/devices | grep redtide | awk '{print $1}')
#sudo mknod /dev/redtide c $major 0
#sudo chmod 666 /dev/redtide
echo "Module loaded and device node created."
