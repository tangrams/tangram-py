#include <thread>
#include <pthread.h>
#include <mutex>
#include <atomic>
#include <iostream>
#include <memory>

#include <curl/curl.h>      // Curl

#include "tangram.h"        // Tangram-ES
#include "platform.h"       // Tangram platform specifics
#include "log.h"
#include "gl.h"

#include "context.h"        // This set the headless context
#include "platform_posix.h" // Darwin Linux and RPi

#include "utils.h"

#define KEY_ZOOM_IN  45     // -
#define KEY_ZOOM_OUT 61     // =
#define KEY_UP       119    // w
#define KEY_LEFT     97     // a
#define KEY_RIGHT    115    // s
#define KEY_DOWN     122    // z
#define KEY_ROTATE   82     // r
#define KEY_TILT     84     // t

// Tangram
Tangram::Map* map = nullptr;

// Internal communication between THREADS
std::atomic<bool> bRun(true);
std::vector<std::string> queue; // Commands Queue
std::mutex queueMutex;

std::string style = "scene.yaml";
double lat = 0.0f;   // Default lat position
double lon = 0.0f;   // Default lng position
float zoom = 0.0f;   // Default zoom of the scene
float rot = 0.0f;    // Default rotation of the scene (deg)
float tilt = 0.0f;   // Default tilt angle (deg)
int width = 800;     // Default Width of the image (will be multipl by 2 for the antialiasing)
int height = 480;    // Default height of the image (will be multipl by 2 for the antialiasing)
int keyPressed = 0;

//============================================================================== CONTROL THREADS
void consoleThread() {
    std::string line;
    while (bRun.load() && std::getline(std::cin, line)) {
        std::lock_guard<std::mutex> lock(queueMutex);
        queue.push_back(line);
    }
}

//============================================================================== MAIN FUNCTION
void processCommand (std::string &_command);
void onExit();

int main(int argc, char* argv[]) {

    // CONTROL LOOP
    std::thread console(&consoleThread);

    // Initialize cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Start OpenGL ES context
    LOG("Creating OpenGL ES context");
    initGL(width, height);
    float deviceRatio = getDevicePixelRatio();

    LOG("Creating a new TANGRAM instances");
    map = new Tangram::Map();
    map->loadSceneAsync(style.c_str());
    map->setupGL();
    map->setPixelScale(deviceRatio);
    map->resize(getWindowWidth(), getWindowHeight());

    while (bRun.load()) {
        // Update Network Queue
        processNetworkQueue();

        // Update Tangram
        updateGL();
        map->update(getDelta());
        
        map->render();
        renderGL();

        // Queue Commands
        std::string lastStr;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (queue.size() > 0) {
                lastStr = queue.back();
                queue.pop_back();
            }
        }

        if (lastStr.size() > 0) {
            if (lastStr == "quit") {
                bRun.store(false);
            }
            else {
                std::vector<std::string> commands = split(lastStr, ';');
                for (std::string command : commands) {
                    processCommand(command);
                }
            }
        }

    }

    onExit();

    // Force cinWatcher to finish (because is waiting for input)
    pthread_t consoleHandler = console.native_handle();
    pthread_cancel(consoleHandler);
    console.join();

    // Go home
    return 0;
}

//============================================================================== MAIN FUNCTION
void processCommand (std::string &_command) {
    if (map) {
        std::vector<std::string> elements = split(_command, ' ');
        if (elements[0] == "scene") {
            if (elements.size() == 1) {
                std::cout << style << std::endl;
            }
            else {
                style = elements[1];
                map->loadSceneAsync(style.c_str());
            }
        }
        else if (elements[0] == "zoom") {
            if (elements.size() == 1) {
                std::cout << zoom << std::endl;
            }
            else if (elements.size() == 2) {
                zoom = toFloat(elements[1]);
                LOG("Set zoom: %f", zoom);
                map->setZoom(zoom);
            }
            else if (elements.size() == 3) {
                zoom = toFloat(elements[1]);
                float duration = toFloat(elements[3]);
                LOG("Set zoom: %f in %f (sec)", zoom, duration);
                map->setZoomEased(zoom, duration);
            }
        }
        else if (elements[0] == "tilt") {
            if (elements.size() == 1) {
                std::cout << tilt << std::endl;
            }
            else if (elements.size() == 2) {
                tilt = toFloat(elements[1]);
                LOG("Set tilt: %f", tilt);
                map->setTilt(tilt);
            }
            else if (elements.size() == 3) {
                tilt = toFloat(elements[1]);
                float duration = toFloat(elements[3]);
                LOG("Set tilt: %f in %f (sec)", tilt, duration);
                map->setTiltEased(tilt, duration);
            }
        }
        else if (elements[0] == "rotate") {
            if (elements.size()) {
                std::cout << rot << std::endl;
            }
            else if (elements.size() == 2) {
                rot = toFloat(elements[1]);
                LOG("Set rotation: %f", rot);
                map->setRotation(rot);
            }
            else if (elements.size() == 3) {
                rot = toFloat(elements[1]);
                float duration = toFloat(elements[3]);
                LOG("Set rotation: %f in %f (sec)", rot, duration);
                map->setRotationEased(rot, duration);
            }
        }
        else if (elements.size() > 2 && elements[0] == "position") {
            if (elements.size() == 1) {
                std::cout << lon << 'x' << lat << std::endl;
            }
            else if (elements.size() == 3) {
                lon = toDouble(elements[1]);
                lat = toDouble(elements[2]);
                LOG("Set position: %f (lon), %f (lat)", lon, lat);
                map->setPosition(lon, lat); 
            }
            else if (elements.size() == 4) {
                lon = toDouble(elements[1]);
                lat = toDouble(elements[2]);
                float duration = toFloat(elements[3]);
                LOG("Set position: %f (lon), %f (lat) in %f (sec)", lon, lat, duration);
                map->setPositionEased(lon, lat, duration);
            }  
        }
        else if (elements[0] == "size") {
            if (elements.size() == 1) {
                std::cout << width << "x" << height << std::endl;
            }
            else if (width != toInt(elements[1]) || height != toInt(elements[2])) {
                width = toInt(elements[1]);
                height = toInt(elements[2]);
                setWindowSize(width, height);
                map->resize(width, height);
            }
        }
    }
}

void onKeyPress(int _key) {
    keyPressed = _key;

    if ( _key == 'q' || _key == 'Q'){
        bRun = false;
        bRun.store(false);
    } else {
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
            // default:
            //     LOG(" -> %i\n",_key);
        }
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

void onExit() {
    finishUrlRequests();
    curl_global_cleanup();    
    closeGL();

    if (map) {
        delete map;
        map = nullptr;
    }
}
