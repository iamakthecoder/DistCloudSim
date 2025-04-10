#!/bin/bash
echo "[*] Stopping all cloud simulation services..."
pkill -f ./build/controller
pkill -f ./build/compute
echo "[*] Done."
