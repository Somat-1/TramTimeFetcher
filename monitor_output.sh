#!/bin/bash
# Monitor serial output and save to file
~/.platformio/penv/bin/platformio device monitor --baud 115200 | tee serial_output.log
