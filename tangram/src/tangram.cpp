#include "tangram.h"        // Tangram-ES
#include "platform.h"       // Tangram platform specifics
#include "log.h"
#include "gl.h"

#include "context.h"
#include "platform_posix.h" // Darwin Linux and RPi

#include <iostream>
#include <curl/curl.h>      // Curl

#define KEY_ZOOM_IN  45     // -
#define KEY_ZOOM_OUT 61     // =
#define KEY_UP       265
#define KEY_LEFT     263
#define KEY_RIGHT    262
#define KEY_DOWN     264
#define KEY_ROTATE   82     // r
#define KEY_TILT     84     // t
#define KEY_ESC      256

// Tangram
Tangram::Map* map = nullptr;

// std::string style = "http://tangrams.github.io/tangram-sandbox/styles/default.yaml";
double lat = 0.0f;   // Default lat position
double lon = 0.0f;   // Default lng position
float zoom = 0.0f;   // Default zoom of the scene
float rot = 0.0f;    // Default rotation of the scene (deg)
float tilt = 0.0f;   // Default tilt angle (deg)
int width = 800;     // Default Width of the image (will be multipl by 2 for the antialiasing)
int height = 480;    // Default height of the image (will be multipl by 2 for the antialiasing)
int keyPressed = 0;

void load(char * style, int width, int height) {
     // Initialize cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Start OpenGL ES context
    LOG("Creating OpenGL ES context");
    initGL(width, height);

    LOG("Creating a new TANGRAM instances");
    map = new Tangram::Map();
    map->loadSceneAsync(style);
    map->setupGL();
    map->setPixelScale(getDevicePixelRatio());
    map->resize(getWindowWidth(), getWindowHeight());
}

bool update() {
    // Update Network Queue
    processNetworkQueue();

    // Update Tangram
    updateGL();
    return map->update(getDelta());
}

void render() {
    map->render();
    renderGL();
}

void close() {
    finishUrlRequests();
    curl_global_cleanup();    
    closeGL();

    if (map) {
        delete map;
        map = nullptr;
    }
}

void setPosition(double _lon, double _lat) {
    map->resize(_lon,_lat);
}

void setZoom(float _z) {
    map->setZoom(_z);
}

void setRotation(float _radians) {
    map->setRotation(_radians);
}

void setTilt(float _radians) {
    map->setTilt(_radians);
}



void onKeyPress(int _key) {
    keyPressed = _key;

    switch (_key) {
        case KEY_ZOOM_IN:
            map->handlePinchGesture(0.0,0.0,0.5,0.0);
            break;
        case KEY_ZOOM_OUT:
            map->handlePinchGesture(0.0,0.0,2.0,0.0);
            break;
        case KEY_UP:
            map->handlePanGesture(0.0,0.0,0.0,100.0);
            break;
        case KEY_DOWN:
            map->handlePanGesture(0.0,0.0,0.0,-100.0);
            break;
        case KEY_LEFT:
            map->handlePanGesture(0.0,0.0,100.0,0.0);
            break;
        case KEY_RIGHT:
            map->handlePanGesture(0.0,0.0,-100.0,0.0);
            break;
        case KEY_ESC:
            close();
            break;
        default:
            LOG(" -> %i\n",_key);
    }
}

void onMouseMove(float _x, float _y) {

}

void onMouseClick(float _x, float _y, int _button) {

}

void onMouseDrag(float _x, float _y, int _button) {
    if( _button == 1 ){
        map->handlePanGesture(_x - getMouseVelX(), _y + getMouseVelY(), _x, _y);
    } else if( _button == 2 ){
        if ( keyPressed == KEY_ROTATE) {
            float scale = -0.05;
            float rot = atan2(getMouseVelY(),getMouseVelX());
            if( _x < getWindowWidth()/2.0 ) {
                scale *= -1.0;
            }
            map->handleRotateGesture(getWindowWidth()/2.0, getWindowHeight()/2.0, rot*scale);
        } else if ( keyPressed == KEY_TILT) {
            map->handleShoveGesture(getMouseVelY()*0.1);
        } else {
            map->handlePinchGesture(getWindowWidth()/2.0, getWindowHeight()/2.0, 1.0 + getMouseVelY()*0.001, 0.f);
        }
    }
}

void onViewportResize(int _newWidth, int _newHeight) {
    if (map) {
        map->resize(getWindowWidth(), getWindowHeight());
    }
}

