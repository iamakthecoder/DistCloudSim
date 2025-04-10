#!/bin/bash
echo "[+] Starting controller..."
./build/controller &
sleep 1

echo "[+] Starting compute node..."
./build/compute compute1 &
sleep 1
./build/compute compute2 &
sleep 1

echo "[+] All services started."
