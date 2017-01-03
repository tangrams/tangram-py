#!/usr/bin/env python3
import sys, os
sys.path.append('../')

from tangram import TangramMap
TangramMap.init(800,600, os.path.abspath("scene.yaml"))

TangramMap.setPosition(-73.97715657655, 40.781098831465)
TangramMap.setZoom(10);


pos_A = TangramMap.LngLat(-73.97715657655, 40.781098831465)
marker_A = TangramMap.markerAdd()
TangramMap.markerSetPoint(marker_A, pos_A)
TangramMap.markerSetStyling(marker_A, "sdk-point-overlay");
TangramMap.markerSetVisible(marker_A, True);

while TangramMap.isRunning():
    TangramMap.update()
