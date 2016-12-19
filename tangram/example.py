#!/usr/bin/env python3
import tangram

tangram.init('http://tangrams.github.io/tangram-sandbox/styles/default.yaml', 800,600)

tangram.setZoom(5);
tangram.setPositionEased(-73.97715657655,40.781098831465,10.,tangram.LINEAR)

while True:
    tangram.update()
    tangram.render()