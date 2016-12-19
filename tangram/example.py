#!/usr/bin/env python
import tangram

tangram.load('http://tangrams.github.io/tangram-sandbox/styles/default.yaml', 800,600)

tangram.setZoom(3);
while True:
    tangram.update()
    tangram.render()