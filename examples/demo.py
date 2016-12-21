#!/usr/bin/env python3
import sys
sys.path.append('../')

from tangram import TangramMap
TangramMap.init(800,600, 'https://tangrams.github.io/walkabout-style/walkabout-style.yaml')

TangramMap.setPositionEased(-73.97715657655, 40.781098831465, 10., TangramMap.LINEAR)
TangramMap.setZoom(5);
TangramMap.setZoomEased(10., 10., TangramMap.LINEAR);

while TangramMap.isRunning():
    # While is the view is NOT complete
    if (not TangramMap.update()):
        # Print the position
        print("Lng/Lat/Zoom", TangramMap.getPosition().lng, TangramMap.getPosition().lat, TangramMap.getZoom())
