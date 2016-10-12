#include <thread>
#include <pthread.h>
#include <mutex>
#include <atomic>
#include <iostream>
#include <memory>

#include <curl/curl.h>      // Curl
#include "glm/trigonometric.hpp" // GLM for the radians/degree calc

#include "tangram.h"        // Tangram-ES
#include "platform.h"       // Tangram platform specifics
#include "log.h"
#include "gl.h"

#include "context.h"        // This set the headless context
#include "platform_posix.h" // Darwin Linux and RPi

#include "utils.h"

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

    LOG("Creating a new TANGRAM instances");
    map = new Tangram::Map();
    map->loadSceneAsync(style.c_str());
    map->setupGL();
    map->setPixelScale(1.);
    map->resize(width, height);

    double lastTime = getTime();
    while (bRun.load()) {
        double currentTime = getTime();
        double delta = currentTime - lastTime;
        lastTime = currentTime;

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
                    LOG("FINISH %s", command.c_str());
                }
                LOG("< OK");
            }
        }

        // Update Network Queue
        processNetworkQueue();

        // Update Tangram
        map->update(delta);
        updateGL();

        map->render();
        renderGL();
    }

    onExit();

    // Force cinWatcher to finish (because is waiting for input)
    pthread_t consoleHandler = console.native_handle();
    pthread_cancel(consoleHandler);
    console.join();

    LOG("END\n");

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
    else {
        LOG("No TANGRAM instance");
    }
}

void onKeyPress(int _key) {
    if ( _key == 'q' || _key == 'Q'){
        bRun = false;
        bRun.store(false);
    }
}

void onMouseMove(float _x, float _y) {

}

void onMouseClick(float _x, float _y, int _button) {

}

void onMouseDrag(float _x, float _y, int _button) {

}

void onViewportResize(int _newWidth, int _newHeight) {
    if (map) {
        map->resize(_newWidth, _newHeight);
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