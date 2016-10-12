#!/usr/bin/env python
import os, subprocess

class Tangram:
    TANGRAM_BINNARY = 'tangram-py'
    process = {}

    def __init__(self):
        print os.path.dirname(__file__)
        self.process = subprocess.Popen([os.path.dirname(__file__) + '/' + self.TANGRAM_BINNARY], shell=False, stdin=subprocess.PIPE) #, stdout=subprocess.PIPE) #, stderr=subprocess.PIPE)

    def loadScene(self, _scene):
        self.process.stdin.write('scene '+_scene+'\n')

    def queueSceneUpdate(self, _path, _value):
        print 'TODO'

    def setPosition(self, _lon, _lat, _duration = ''):
        self.process.stdin.write('position ' + str(_lon) + ' ' + str(_lat) + ' ' + str(_duration) + '\n')

    def setZoom(self, _z, _duration = ''):
        self.process.stdin.write('zoom ' + str(_z) + ' ' + str(_duration) + '\n')

    def setRotation(self, _radians, _duration = ''):
        self.process.stdin.write('rotation ' + str(_radians) + ' ' + str(_duration) + '\n')

    def setTilt(self, _radians, _duration = ''):
        self.process.stdin.write('tilt ' + str(_radians) + ' ' + str(_duration) + '\n')

    def resize(self, _newWidth, _newHeight):
        self.process.stdin.write('size ' + str(_newWidth) + ' ' + str(_newHeight) + '\n')