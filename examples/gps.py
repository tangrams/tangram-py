#!/usr/bin/env python3

# NOTE: you need to set up you GPS hat first, follow this tutorial
#       https://learn.adafruit.com/adafruit-ultimate-gps-hat-for-raspberry-pi/
# then install gps3 module: sudo pip install gps3 
import sys, gps3
the_connection = gps3.GPSDSocket() 
the_fix = gps3.Fix()
try:
    for new_data in the_connection:
        if new_data:
            the_fix.refresh(new_data)
        if not isinstance(the_fix.TPV['lat'], str): # lat as determinate of when data is 'valid'
            speed = the_fix.TPV['speed']
            latitude = the_fix.TPV['lat']
            longitude = the_fix.TPV['lon']
            altitude  = the_fix.TPV['alt']

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
