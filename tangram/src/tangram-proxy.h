#pragma once

#ifndef PYTHON_ENUM 
#define PYTHON_ENUM(x) enum x
#endif

PYTHON_ENUM(EaseType) {
  LINEAR=0,
  CUBIC=1,
  QUINT=2,
  SINE=3
};

struct LngLat {
    double lng;
    double lat;
};

struct PointXY {
    double x;
    double y;
};

void init(int width, int height, char * style = "scene.yaml");

bool isRunning();

// Load the scene at the given absolute file path synchronously
void loadScene(char * style, bool _useScenePosition = false);
// Load the scene at the given absolute file path asynchronously
void loadSceneAsync(char * style, bool _useScenePosition = false);

// Request an update to the scene configuration; the path is a series of yaml keys
// separated by a '.' and the value is a string of yaml to replace the current value
// at the given path in the scene
void queueSceneUpdate(const char* _path, const char* _value);
// Apply all previously requested scene updates
void applySceneUpdates();

// Update the map state with the time interval since the last update, returns
// true when the current view is completely loaded (all tiles are available and
// no animation in progress)
bool update();
void close();

// Set the ratio of hardware pixels to logical pixels (defaults to 1.0);
// this operation can be slow, so only perform this when necessary.
void setPixelScale(float _pixelsPerPoint);
// Gets the pixel scale
float getPixelScale();
// Gets the viewport height in physical pixels (framebuffer size)
int getViewportHeight();
// Gets the viewport width in physical pixels (framebuffer size)
int getViewportWidth();

// Set the position of the map view in degrees longitude and latitude; if duration
// (in seconds) is provided, position eases to the set value over the duration;
// calling either version of the setter overrides all previous calls
void setPosition(double _lon, double _lat);
void setPositionEased(double _lon, double _lat, float _duration, EaseType _e = QUINT);
void setPosition(LngLat _lngLat);
void setPositionEased(LngLat _lngLat, float _duration, EaseType _e = QUINT);
// Set the values of the arguments to the position of the map view in degrees
// longitude and latitude
LngLat getPosition();

// Given coordinates in screen space (x right, y down), set the output longitude and
// latitude to the geographic location corresponding to that point
LngLat screenPositionToLngLat(double _x, double _y);

// Given longitude and latitude coordinates, set the output coordinates to the
// corresponding point in screen space (x right, y down)
PointXY lngLatToScreenPosition(double _lng, double _lat);
PointXY lngLatToScreenPosition(LngLat _lngLat);

// Set the fractional zoom level of the view; if duration (in seconds) is provided,
// zoom eases to the set value over the duration; calling either version of the setter
// overrides all previous calls
void setZoom(float _z);
void setZoomEased(float _z, float _duration, EaseType _e = QUINT);
// Get the fractional zoom level of the view
float getZoom();

// Set the counter-clockwise rotation of the view in radians; 0 corresponds to
// North pointing up; if duration (in seconds) is provided, rotation eases to the
// the set value over the duration; calling either version of the setter overrides
// all previous calls
void setRotation(float _radians);
void setRotationEased(float _radians, float _duration, EaseType _e = QUINT);
// Get the counter-clockwise rotation of the view in radians; 0 corresponds to
// North pointing up
float getRotation();

// Set the tilt angle of the view in radians; 0 corresponds to straight down;
// if duration (in seconds) is provided, tilt eases to the set value over the
// duration; calling either version of the setter overrides all previous calls
void setTilt(float _radians);
void setTiltEased(float _radians, float _duration, EaseType _e = QUINT);
// Get the tilt angle of the view in radians; 0 corresponds to straight down
float getTilt();

// Set the camera type (0 = perspective, 1 = isometric, 2 = flat)
void setCameraType(int _type);
// Get the camera type (0 = perspective, 1 = isometric, 2 = flat)
int getCameraType();
