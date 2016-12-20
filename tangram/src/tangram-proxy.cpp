#include "tangram-proxy.h"

#include "tangram.h"        // Tangram-ES
#include "data/clientGeoJsonSource.h"
#include "debug/textDisplay.h"
#include "platform.h"       // Tangram platform specifics
#include "log.h"
#include "gl.h"

#include "context.h"
#include "platform_posix.h" // Darwin Linux and RPi

#include <iostream>
#include <curl/curl.h>      // Curl

#define KEY_ZOOM_IN  45     // -
#define KEY_ZOOM_OUT 61     // =
#define KEY_ROTATE   82     // r
#define KEY_TILT     84     // t

#ifdef PLATFORM_RPI
#define KEY_ESC      113    // q
#define KEY_UP       119    // w
#define KEY_LEFT     97     // a
#define KEY_RIGHT    115    // s
#define KEY_DOWN     122    // z
#else
#define KEY_ESC      256
#define KEY_UP       265
#define KEY_LEFT     263
#define KEY_RIGHT    262
#define KEY_DOWN     264
#endif 

// Tangram
Tangram::Map* map = nullptr;

bool bFinish = false;
std::string sceneFile = "scene.yaml";

const double double_tap_time = 0.5; // seconds
const double scroll_span_multiplier = 0.05; // scaling for zoom and rotation
const double scroll_distance_multiplier = 5.0; // scaling for shove
float pixel_scale = 1.0;

double last_time_released = -double_tap_time; // First click should never trigger a double tap
int keyPressed = 0;

std::shared_ptr<Tangram::ClientGeoJsonSource> data_source;
Tangram::LngLat last_point;

void init(int width, int height, char * style) {
     // Initialize cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);

    sceneFile = std::string(style);

    // Start OpenGL ES context
    LOG("Creating OpenGL ES context");
    initGL(width, height);

    LOG("Creating a new TANGRAM instances");
    map = new Tangram::Map();
    map->loadSceneAsync(style);
    map->setupGL();
    pixel_scale = getDevicePixelRatio();
    map->setPixelScale(pixel_scale);
    map->resize(getWindowWidth(), getWindowHeight());
}

bool isRunning() {
    return map != nullptr;
}

void loadScene(char * style, bool _useScenePosition) {
    if (map) {
        sceneFile = std::string(style);
        map->loadScene(style, _useScenePosition);
    }
}

void loadSceneAsync(char * style, bool _useScenePosition) {
    if (map) {
        sceneFile = std::string(style);
        map->loadSceneAsync(style, _useScenePosition);
    }
}

void queueSceneUpdate(const char* _path, const char* _value) {
    if (map) {
        map->queueSceneUpdate(_path, _value);
    }
}

void applySceneUpdates() {
    if (map) {
        map->applySceneUpdates();
    }
}

bool update() {
    // Update Network Queue
    processNetworkQueue();

    if (map) {
        // Update Tangram
        updateGL();
        bFinish = map->update(getDelta());

        map->render();
    }

    renderGL();
    return bFinish;
}

void close() {
    finishUrlRequests();
    curl_global_cleanup();

    if (map) {
        delete map;
        map = nullptr;
    }

    closeGL();
}

float getPixelScale() {
    if (map) {
        return map->getPixelScale();
    } else {
        return 0.0;
    }
}

int getViewportHeight() {
    if (map) {
        return map->getViewportHeight();
    } else {
        return 0.0;
    }
}

int getViewportWidth() {
    if (map) {
        return map->getViewportWidth();
    } else {
        return 0.0;
    }
}

void setPosition(double _lng, double _lat) {
    if (map) {
        map->setPosition(_lng,_lat);
    }
}

void setPositionEased(double _lng, double _lat, float _duration, EaseType _e) {
    if (map) {
        map->setPositionEased(_lng, _lat, _duration, Tangram::EaseType(_e));
    }
}

void setPosition(LngLat _lngLat) {
    if (map) {
        map->setPosition(_lngLat.lng, _lngLat.lat);
    }
}

void setPositionEased(LngLat _lngLat, float _duration, EaseType _e) {
    map->setPositionEased(_lngLat.lng, _lngLat.lat, _duration, Tangram::EaseType(_e));
}

LngLat getPosition() {
    LngLat rta;
    if (map) {
        map->getPosition(rta.lng,rta.lat);
    }
    return rta;
}

LngLat screenPositionToLngLat(double _x, double _y) {
    LngLat rta;
    if (map) {
        map->screenPositionToLngLat(_x,_y, &rta.lng, &rta.lat);
    }
    return rta;
}

PointXY lngLatToScreenPosition(double _lng, double _lat) {
    PointXY rta;
    if (map) {
        map->screenPositionToLngLat(_lng,_lat, &rta.x, &rta.y);
    }
    return rta;
}
PointXY lngLatToScreenPosition(LngLat _lngLat) {
    if (map) {
        return lngLatToScreenPosition(_lngLat.lng, _lngLat.lat);
    } else {
        PointXY rta;
        return rta;
    }
}

void setZoom(float _z) {
    if (map) {
        map->setZoom(_z);
    }
}

void setZoomEased(float _z, float _duration, EaseType _e) {
    if (map) {
        map->setZoomEased(_z, _duration, Tangram::EaseType(_e));
    }
}

float getZoom() {
    if (map) {
        return map->getZoom();
    } else {
        return 0.0;
    }
}

void setRotation(float _radians) {
    if (map) {
        map->setRotation(_radians);
    }
}

void setRotationEased(float _radians, float _duration, EaseType _e) {
    if (map) {
        map->setRotationEased(_radians, _duration, Tangram::EaseType(_e));
    }
}

float getRotation() {
    if (map) {
        return map->getRotation();
    } else {
        return 0.0;
    }
}

void setTilt(float _radians) {
    if (map) {
        map->setTilt(_radians);
    }
}

void setTiltEased(float _radians, float _duration, EaseType _e) {
    if (map) {
        map->setTiltEased(_radians, _duration, Tangram::EaseType(_e));
    }
}

float getTilt() {
    if (map) {
        return map->getTilt();
    } else {
        return 0.;
    }
    
}

void setCameraType(int _type) {
    if (map) {
        map->setCameraType(_type);
    }
}

int getCameraType() {
    if (map) {
        return map->getCameraType();
    } else {
        return -1;
    }
}

void setPixelScale(float _pixelsPerPoint) {
    if (map) {
        map->setPixelScale(_pixelsPerPoint);
    }
}

void onKeyPress(int _key) {
    if (map) {
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
        }
    }
}

void onMouseMove(float _x, float _y) {

}

void onMouseClick(float _x, float _y, int _button) {
    double time = getTime();

    if (map && (time - last_time_released) < double_tap_time) {
        Tangram::LngLat p;
        map->screenPositionToLngLat(_x, _y, &p.longitude, &p.latitude);

        logMsg("pick feature\n");
        map->clearDataSource(*data_source, true, true);

        map->pickFeatureAt(_x, _y, [](auto item) {
            if (!item) { return; }
            LOG("%s", item->properties->toJson().c_str());
            std::string name = "noname";
            item->properties->getString("name", name);
            LOG("%s", name.c_str());
        });
    }

    last_time_released = time;
}

void onScroll(float _x, float _y, float _scrollx, float _scrolly, ScrollType _type) {
    if (map) {
        if (_type == SHOVE) {
            map->handleShoveGesture(scroll_distance_multiplier * _scrolly);
        } else if (_type == ROTATE) {
            map->handleRotateGesture(_x, _y, scroll_span_multiplier * _scrolly);
        } else {
            map->handlePinchGesture(_x, _y, 1.0 + scroll_span_multiplier * _scrolly, 0.f);
        }
    }
}

void onMouseDrag(float _x, float _y, int _button) {
    if (map) {
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
}

void onDrop(int count, const char** paths) {
    if (map) {
        sceneFile = std::string(paths[0]);
        map->loadSceneAsync(sceneFile.c_str());
    }
}

void onViewportResize(int _newWidth, int _newHeight) {
    if (map) {
        pixel_scale = getDevicePixelRatio();
        map->setPixelScale(pixel_scale);
        map->resize(getWindowWidth(), getWindowHeight());
    }
}
