#include "platform.h"
#include "log.h"
#include "context.h"

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <string>
#include <list>

#include "urlWorker.h"
#include "platform_posix.h"
#include "gl/hardware.h"

#include <libgen.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/syscall.h>

#define NUM_WORKERS 10

static bool s_isContinuousRendering = false;

static UrlWorker s_Workers[NUM_WORKERS];
static std::list<std::unique_ptr<UrlTask>> s_urlTaskQueue;

static double startTime = 0.0;

void logMsg(const char* fmt, ...) {
    std::string str = std::to_string(getTime()-startTime) + " - " + fmt;
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, str.c_str(), args);
    va_end(args);
}

void resetTimer(std::string _msg) {
    LOG(" ");
    startTime = getTime();
    LOG("START %s",_msg.c_str());
}

void processNetworkQueue() {
    // attach workers to NetWorkerData
    auto taskItr = s_urlTaskQueue.begin();
    for (auto& worker:s_Workers) {
        if (taskItr == s_urlTaskQueue.end()) {
            break;
        }
        if (worker.isAvailable()) {
            worker.perform(std::move(*taskItr));
            taskItr = s_urlTaskQueue.erase(taskItr);
        }
    }
}

void requestRender() {
    #ifndef PLATFORM_RPI
    glfwPostEmptyEvent();
    #endif
}

void setContinuousRendering(bool _isContinuous) {
    s_isContinuousRendering = _isContinuous;
}

bool isContinuousRendering() {
    return s_isContinuousRendering;
}

std::string stringFromFile(const char* _path) {
    size_t length = 0;
    unsigned char* bytes = bytesFromFile(_path, length);
    if (!bytes) { return {}; }

    std::string out(reinterpret_cast<char*>(bytes), length);
    free(bytes);

    return out;
}

unsigned char* bytesFromFile(const char* _path, size_t& _size) {

    std::ifstream resource(_path, std::ifstream::ate | std::ifstream::binary);

    if(!resource.is_open()) {
        logMsg("Failed to read file at path: %s\n", _path);
        _size = 0;
        return nullptr;
    }

    _size = resource.tellg();

    resource.seekg(std::ifstream::beg);

    char* cdata = (char*) malloc(sizeof(char) * (_size));

    resource.read(cdata, _size);
    resource.close();

    return reinterpret_cast<unsigned char *>(cdata);
}


// No system fonts implementation (yet!)
std::string systemFontPath(const std::string& _name, const std::string& _weight,
                           const std::string& _face) {
    return "";
}

// No system fonts fallback implementation (yet!)
std::string systemFontFallbackPath(int _importance, int _weightHint) {
    return "";
}

bool startUrlRequest(const std::string& _url, UrlCallback _callback) {
    std::unique_ptr<UrlTask> task(new UrlTask(_url, _callback));
    for (auto& worker:s_Workers) {
        if(worker.isAvailable()) {
            worker.perform(std::move(task));
            return true;
        }
    }
    s_urlTaskQueue.push_back(std::move(task));
    return true;

}

void cancelUrlRequest(const std::string& _url) {

    // Only clear this request if a worker has not started operating on it!
    // otherwise it gets too convoluted with curl!
    auto itr = s_urlTaskQueue.begin();
    while(itr != s_urlTaskQueue.end()) {
        if((*itr)->url == _url) {
            itr = s_urlTaskQueue.erase(itr);
        } else {
            itr++;
        }
    }
}

void finishUrlRequests() {
    for (auto& worker:s_Workers) {
        worker.join();
    }
}

void setCurrentThreadPriority(int priority){
    int tid = syscall(SYS_gettid);
    //int  p1 = getpriority(PRIO_PROCESS, tid);

    setpriority(PRIO_PROCESS, tid, priority);

    //int  p2 = getpriority(PRIO_PROCESS, tid);
    //logMsg("set niceness: %d -> %d\n", p1, p2);
}

void initGLExtensions() {
    Tangram::Hardware::supportsMapBuffer = true;
    // #ifdef PLATFORM_LINUX
    // glBindVertexArrayOESEXT = (PFNGLBINDVERTEXARRAYPROC)glfwGetProcAddress("glBindVertexArray");
    // glDeleteVertexArraysOESEXT = (PFNGLDELETEVERTEXARRAYSPROC)glfwGetProcAddress("glDeleteVertexArrays");
    // glGenVertexArraysOESEXT = (PFNGLGENVERTEXARRAYSPROC)glfwGetProcAddress("glGenVertexArrays");
    // #endif
}

// #ifdef PLATFORM_RPI
// // Dummy VertexArray functions
// GL_APICALL void GL_APIENTRY glBindVertexArray(GLuint array) {}
// GL_APICALL void GL_APIENTRY glDeleteVertexArrays(GLsizei n, const GLuint *arrays) {}
// GL_APICALL void GL_APIENTRY glGenVertexArrays(GLsizei n, GLuint *arrays) {}
// #endif

