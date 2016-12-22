#!/usr/bin/env python3

# NOTE: you need to set up you GPS hat first, follow this tutorial
# https://www.modmypi.com/blog/raspberry-pi-gps-hat-and-python
import sys, serial
import pynmea2

def parseGPS(str):
    if str.find('GGA') > 0:
        msg = pynmea2.parse(str)
        print ("Timestamp: %s -- Lat: %s %s -- Lon: %s %s -- Altitude: %s %s" % (msg.timestamp,msg.lat,msg.lat_dir,msg.lon,msg.lon_dir,msg.altitude,msg.altitude_units))

serialPort = serial.Serial("/dev/ttyAMA0", 9600, timeout=0.5)

while True:
    str = serialPort.readline()
    parseGPS(str)

# sys.path.append('../')
# from tangram import TangramMap

# TangramMap.init(800,600, 'https://tangrams.github.io/walkabout-style/walkabout-style.yaml')

# TangramMap.setPositionEased(-73.97715657655, 40.781098831465, 10., TangramMap.LINEAR)
# TangramMap.setZoom(5);
# TangramMap.setZoomEased(10., 10., TangramMap.LINEAR);

# while TangramMap.isRunning():
#     # While is the view is NOT complete
#     if (not TangramMap.update()):
#         # Print the position
#         print("Lng/Lat/Zoom", TangramMap.getPosition().lng, TangramMap.getPosition().lat, TangramMap.getZoom())
