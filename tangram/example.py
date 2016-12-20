#!/usr/bin/env python3
import tangram

tangram.init(800,600, 'https://tangrams.github.io/walkabout-style/walkabout-style.yaml')

tangram.setPositionEased(-73.97715657655, 40.781098831465, 5., tangram.LINEAR)
tangram.setZoom(5);

while tangram.isRunning():
    # While is the view is NOT complete
    if (not tangram.update()):
        # Print the position
        print("Lng/Lat/Zoom", tangram.getPosition().lng, tangram.getPosition().lat, tangram.getZoom())
