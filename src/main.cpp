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
double lastTime;

//============================================================================== CONTROL THREADS
void consoleThread() {
    std::string line;
    while (bRun.load() && std::getline(std::cin, line)) {
        std::lock_guard<std::mutex> lock(queueMutex);
        queue.push_back(line);
    }
}

//============================================================================== MAIN FUNCTION
void updateTangram();
void processCommand (std::string &_command);

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
    updateTangram();

    lastTime = getTime();
    while (bRun.load()) {
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
                    updateTangram();
                    LOG("FINISH %s", command.c_str());
                }
                LOG("< OK");
            }
        } 
    }

    finishUrlRequests();
    curl_global_cleanup();    
    closeGL();

    if (map) {
        delete map;
        map = nullptr;
    }


    // Force cinWatcher to finish (because is waiting for input)
    pthread_t consoleHandler = console.native_handle();
    pthread_cancel(consoleHandler);
    console.join();

    LOG("END\n");

    // Go home
    return 0;
}

//============================================================================== MAIN FUNCTION
void updateTangram () {
    bool bFinish = false;
    while (!bFinish) {
        double currentTime = getTime();
        double delta = currentTime - lastTime;
        lastTime = currentTime;

        // Update Network Queue
        processNetworkQueue();
        bFinish = map->update(delta);
    }
}

void processCommand (std::string &_command) {
    if (map) {
        std::vector<std::string> elements = split(_command, ' ');
        if (elements[0] == "scene") {
            if (elements.size() == 1) {
                std::cout << style << std::endl;
            }
            else {
                style = elements[1];
                resetTimer(_command);
                map->loadSceneAsync(style.c_str());
            }
        }
        else if (elements[0] == "zoom") {
            if (elements.size() == 1) {
                std::cout << zoom << std::endl;
            }
            else if (zoom != toFloat(elements[1])) {
                zoom = toFloat(elements[1]);
                resetTimer(_command);
                LOG("Set zoom: %f", zoom);
                map->setZoom(zoom);
            }
        }
        else if (elements[0] == "tilt") {
            if (elements.size() == 1) {
                std::cout << tilt << std::endl;
            }
            else if (tilt != toFloat(elements[1])) {
                tilt = toFloat(elements[1]);
                resetTimer(_command);
                LOG("Set tilt: %f", tilt);
                map->setTilt(tilt);
            }
        }
        else if (elements[0] == "rotate") {
            if (elements.size()) {
                std::cout << rot << std::endl;
            }
            else if (rot != toFloat(elements[1])) {
                rot = toFloat(elements[1]);
                resetTimer(_command);
                LOG("Set rotation: %f", rot);
                map->setRotation(rot);
            }
        }
        else if (elements.size() > 2 && elements[0] == "position") {
            if (elements.size() == 1) {
                std::cout << lon << 'x' << lat << std::endl;
            }
            else if (lon != toDouble(elements[1]) || lat != toDouble(elements[2])) {
                lon = toDouble(elements[1]);
                lat = toDouble(elements[2]);
                resetTimer(_command);
                LOG("Set position: %f (lon), %f (lat)", lon, lat);
                map->setPosition(lon, lat); 
            }

            if (elements.size() == 4) {
                zoom = toFloat(elements[3]);
                resetTimer(_command);
                LOG("Set zoom: %f", zoom);
                map->setZoom(zoom);
            }  
        }
        else if (elements[0] == "size") {
            if (elements.size() == 1) {
                std::cout << width << "x" << height << std::endl;
            }
            else if (width != toInt(elements[1]) || height != toInt(elements[2])) {
                width = toInt(elements[1]);
                height = toInt(elements[2]);
                resetTimer(_command);
                // resize(width, height);
                map->resize(width, height);
            }
        }
    }
    else {
        LOG("No TANGRAM instance");
    }
}
