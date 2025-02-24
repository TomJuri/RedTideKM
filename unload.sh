#!/bin/bash
set -e
sudo rmmod blacktidekm
sudo rm -f /dev/blacktide
echo "Module unloaded and device node removed."
