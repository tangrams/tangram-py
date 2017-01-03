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
    LngLat() { lng = 0.0; lat = 0.0; };
    LngLat(double _lng, double _lat) { lng = _lng; lat = _lat; };
};

struct PointXY {
    double x;
    double y;
    PointXY() { x = 0.0; y = 0.0; };
    PointXY(double _x, double _y) { x = _x; y = _y; };
};

typedef long MarkerID;

void init(int width, int height, const char * style = "scene.yaml");

bool isRunning();

// Load the scene at the given absolute file path synchronously
void loadScene(const char * style, bool _useScenePosition = false);
// Load the scene at the given absolute file path asynchronously
void loadSceneAsync(const char * style, bool _useScenePosition = false);

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

// Add a marker object to the map and return an ID for it; an ID of 0 indicates an invalid marker;
// the marker will not be drawn until both styling and geometry are set using the functions below.
MarkerID markerAdd();

 // Remove a marker object from the map; returns true if the marker ID was found and successfully
// removed, otherwise returns false.
bool markerRemove(MarkerID _marker);

// Set the styling for a marker object; _styling is a string of YAML that specifies a 'draw rule'
// according to the scene file syntax; returns true if the marker ID was found and successfully
// updated, otherwise returns false.
bool markerSetStyling(MarkerID _marker, const char* _styling);

// Set a bitmap to use as the image for a point marker; _data is a buffer of RGBA pixel data with
// length of _width * _height; pixels are in row-major order beginning from the bottom-left of the
// image; returns true if the marker ID was found and successfully updated, otherwise returns false.
bool markerSetBitmap(MarkerID _marker, int _width, int _height, const unsigned int* _data);

// Set the geometry of a marker to a point at the given coordinates; markers can have their
// geometry set multiple times with possibly different geometry types; returns true if the
// marker ID was found and successfully updated, otherwise returns false.
bool markerSetPoint(MarkerID _marker, LngLat _lngLat);

// Set the geometry of a marker to a point at the given coordinates; if the marker was previously
// set to a point, this eases the position over the given duration in seconds with the given EaseType;
// returns true if the marker ID was found and successfully updated, otherwise returns false.
bool markerSetPointEased(MarkerID _marker, LngLat _lngLat, float _duration, EaseType _ease);

// Set the geometry of a marker to a polyline along the given coordinates; _coordinates is a
// pointer to a sequence of _count LngLats; markers can have their geometry set multiple times
// with possibly different geometry types; returns true if the marker ID was found and
// successfully updated, otherwise returns false.
bool markerSetPolyline(MarkerID _marker, LngLat* _coordinates, int _count);

// Set the geometry of a marker to a polygon with the given coordinates; _counts is a pointer
// to a sequence of _rings integers and _coordinates is a pointer to a sequence of LngLats with
// a total length equal to the sum of _counts; for each integer n in _counts, a polygon is created
// by taking the next n LngLats from _coordinates, with winding order and internal polygons
// behaving according to the GeoJSON specification; markers can have their geometry set multiple
// times with possibly different geometry types; returns true if the marker ID was found and
// successfully updated, otherwise returns false.
// bool markerSetPolygon(MarkerID _marker, LngLat* _coordinates, int* _counts, int _rings);

// Set the visibility of a marker object; returns true if the marker ID was found and successfully
// updated, otherwise returns false.
bool markerSetVisible(MarkerID _marker, bool _visible);

// Set the ordering of point marker object relative to other markers; higher values are drawn 'above';
// returns true if the marker ID was found and successfully updated, otherwise returns false.
bool markerSetDrawOrder(MarkerID _marker, int _drawOrder);

// Remove all marker objects from the map; Any marker IDs previously returned from 'markerAdd'
// are invalidated after this.
void markerRemoveAll();

