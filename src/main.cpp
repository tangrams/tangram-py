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

std::unique_ptr<Tangram::Map> map;

// Internal communication between THREADS
std::atomic<bool> bRun(true);
std::vector<std::string> queue; // Commands Queue
std::mutex queueMutex;

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
    initGL(800, 600);

    LOG("Creating a new TANGRAM instances");
    map = std::unique_ptr<Tangram::Map>(new Tangram::Map());
    map->loadSceneAsync("scene.yaml");
    map->setupGL();
    map->setPixelScale(1.);
    map->resize(800, 600);
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
