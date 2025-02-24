#!/bin/bash
set -e
sudo rmmod redtidekm
sudo rm -f /dev/redtide
echo "Module unloaded and device node removed."
