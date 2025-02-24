#!/bin/bash
set -e
sudo insmod out/blacktidekm.ko
#major=$(cat /proc/devices | grep blacktide | awk '{print $1}')
#sudo mknod /dev/blacktide c $major 0
#sudo chmod 666 /dev/blacktide
echo "Module loaded and device node created."
