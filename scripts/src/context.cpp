#include "context.h"

#include <time.h>
#include <sys/time.h>
#include "glm/gtc/matrix_transform.hpp"
#include "utils.h"

// Common global variables
//----------------------------------------------------
const std::string appTitle = "Tangram";
static glm::mat4 orthoMatrix;
typedef struct {
    float   x,y;
    float   velX,velY;
    int     button;
} Mouse;
struct timeval tv;
static Mouse mouse;
static glm::ivec4 viewport;
static double fTime = 0.0f;
static double fDelta = 0.0f;

#ifdef PLATFORM_RPI
#include <assert.h>
#include <fcntl.h>
#include <iostream>
#include <termios.h>
#include <string>
#include <fstream>

#define check() assert(glGetError() == 0)

// Raspberry globals
//----------------------------------------------------
EGLDisplay display;
EGLSurface surface;
EGLContext context;

unsigned long long timeStart;
unsigned long long timePrev;
static bool bBcm = false;
#else

// OSX/Linux globals
//----------------------------------------------------
static GLFWwindow* window;
static float devicePixelRatio = 1.0;
#endif

void initGL (int _width, int _height) {

    #ifdef PLATFORM_RPI
        // RASPBERRY_PI

        // Start clock
        gettimeofday(&tv, NULL);
        timeStart = (unsigned long long)(tv.tv_sec) * 1000 +
                    (unsigned long long)(tv.tv_usec) / 1000; 

        // Start OpenGL ES
        if (!bBcm) {
            bcm_host_init();
            bBcm = true;
        }

        // Clear application state
        EGLBoolean result;
        EGLint num_config;

        static EGL_DISPMANX_WINDOW_T nativeviewport;

        DISPMANX_ELEMENT_HANDLE_T dispman_element;
        DISPMANX_DISPLAY_HANDLE_T dispman_display;
        DISPMANX_UPDATE_HANDLE_T dispman_update;
        VC_RECT_T dst_rect;
        VC_RECT_T src_rect;

        static const EGLint attribute_list[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            // EGL_SAMPLE_BUFFERS, 1, EGL_SAMPLES, 4,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
        };

        static const EGLint context_attributes[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };

        EGLConfig config;

        // get an EGL display connection
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        assert(display!=EGL_NO_DISPLAY);
        check();

        // initialize the EGL display connection
        result = eglInitialize(display, NULL, NULL);
        assert(EGL_FALSE != result);
        check();

        // get an appropriate EGL frame buffer configuration
        result = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
        assert(EGL_FALSE != result);
        check();

        // get an appropriate EGL frame buffer configuration
        result = eglBindAPI(EGL_OPENGL_ES_API);
        assert(EGL_FALSE != result);
        check();

        // create an EGL rendering context
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
        assert(context!=EGL_NO_CONTEXT);
        check();

        //  Initially the viewport is for all the screen
        dst_rect.x = 0;
        dst_rect.y = 0;
        dst_rect.width = _width;
        dst_rect.height = _height;

        src_rect.x = 0;
        src_rect.y = 0;
        src_rect.width = _width << 16;
        src_rect.height = _height << 16;

        dispman_display = vc_dispmanx_display_open(0); // LCD
        dispman_update = vc_dispmanx_update_start(0);

        dispman_element = vc_dispmanx_element_add(  dispman_update, dispman_display,
                                                    0/*layer*/, &dst_rect, 0/*src*/,
                                                    &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, (DISPMANX_TRANSFORM_T)0/*transform*/);

        nativeviewport.element = dispman_element;
        nativeviewport.width = _width;
        nativeviewport.height = _height;
        vc_dispmanx_update_submit_sync( dispman_update );
        check();

        surface = eglCreateWindowSurface( display, config, &nativeviewport, NULL );
        assert(surface != EGL_NO_SURFACE);
        check();

        // connect the context to the surface
        result = eglMakeCurrent(display, surface, surface, context);
        assert(EGL_FALSE != result);
        check();

        setWindowSize(_width,_height);
    #else
        // OSX/LINUX use GLFW
        // ---------------------------------------------
        if(!glfwInit()) {
            std::cerr << "ABORT: GLFW init failed" << std::endl;
            exit(-1);
        }

        window = glfwCreateWindow(_width, _height, appTitle.c_str(), NULL, NULL);
        
        if(!window) {
            glfwTerminate();
            std::cerr << "ABORT: GLFW create window failed" << std::endl;
            exit(-1);
        }

        devicePixelRatio = getDevicePixelRatio();
        setWindowSize(_width*devicePixelRatio, _height*devicePixelRatio);

        glfwMakeContextCurrent(window);
        glfwSetWindowSizeCallback(window, [](GLFWwindow* _window, int _w, int _h) {
            devicePixelRatio = getDevicePixelRatio();
            setWindowSize(_w*devicePixelRatio,_h*devicePixelRatio);
        });

        glfwSetKeyCallback(window, [](GLFWwindow* _window, int _key, int _scancode, int _action, int _mods) {
            onKeyPress(_key);
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow* _window, double x, double y) {
            // Update stuff
            x *= devicePixelRatio;
            y *= devicePixelRatio;

            mouse.velX = x - mouse.x;
            mouse.velY = (viewport.w - y) - mouse.y;
            mouse.x = x;
            mouse.y = viewport.w - y;

            if (mouse.x < 0) mouse.x = 0;
            if (mouse.y < 0) mouse.y = 0;
            if (mouse.x > viewport.z) mouse.x = viewport.z;
            if (mouse.y > viewport.w) mouse.y = viewport.w;

            int action1 = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1);
            int action2 = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2);
            int button = 0;

            if (action1 == GLFW_PRESS) button = 1;
            else if (action2 == GLFW_PRESS) button = 2;

            // Lunch events
            if (mouse.button == 0 && button != mouse.button) {
                mouse.button = button;
                onMouseClick(mouse.x,mouse.y,mouse.button);
            } 
            else {
                mouse.button = button;
            }

            if (mouse.velX != 0.0 || mouse.velY != 0.0) {
                if (button != 0) onMouseDrag(mouse.x,mouse.y,mouse.button);
                else onMouseMove(mouse.x,mouse.y);
            }    
        });

        glfwSwapInterval(1);
    #endif
}

bool isGL(){
    #ifdef PLATFORM_RPI
        // RASPBERRY_PI
        return bBcm;
    #else
        // OSX/LINUX
        return !glfwWindowShouldClose(window);
    #endif
}

void updateGL(){
    // Update time
    // --------------------------------------------------------------------
    #ifdef PLATFORM_RPI
        // RASPBERRY_PI
        gettimeofday(&tv, NULL);
        unsigned long long timeNow =    (unsigned long long)(tv.tv_sec) * 1000 +
                                        (unsigned long long)(tv.tv_usec) / 1000;

        fTime = (timeNow - timeStart)*0.001;
        fDelta = (timeNow - timePrev)*0.001;
        timePrev = timeNow;
    #else
        // OSX/LINUX
        double now = glfwGetTime();
        fDelta = now - fTime;
        fTime = now;

        static int frame_count = 0.;
        if (fDelta > 0.25) {
            std::string title = appTitle + ":..: FPS:" + toString(frame_count / fDelta);
            glfwSetWindowTitle(window, title.c_str());
            frame_count = 0;
        }
        frame_count++;
    #endif

    // EVENTS
    // --------------------------------------------------------------------
    #ifdef PLATFORM_RPI
        // RASPBERRY_PI
        static int fd = -1;
        const int XSIGN = 1<<4, YSIGN = 1<<5;
        if (fd<0) {
            fd = open("/dev/input/mouse0",O_RDONLY|O_NONBLOCK);
        }
        if (fd>=0) {
            // Set values to 0
            mouse.velX=0;
            mouse.velY=0;
            
            // Extract values from driver
            struct {char buttons, dx, dy; } m;
            while (1) {
                int bytes = read(fd, &m, sizeof m);
                
                if (bytes < (int)sizeof m) {
                    return;
                } else if (m.buttons&8) {
                    break; // This bit should always be set
                }
                
                read(fd, &m, 1); // Try to sync up again
            }
            
            // Set button value
            int button = m.buttons&3;
            if (button) mouse.button = button;
            else mouse.button = 0;
            
            // Set deltas
            mouse.velX=m.dx;
            mouse.velY=m.dy;
            if (m.buttons&XSIGN) mouse.velX-=256;
            if (m.buttons&YSIGN) mouse.velY-=256;
            
            // Add movement
            mouse.x+=mouse.velX;
            mouse.y+=mouse.velY;
            
            // Clamp values
            if (mouse.x < 0) mouse.x=0;
            if (mouse.y < 0) mouse.y=0;
            if (mouse.x > viewport.z) mouse.x = viewport.z;
            if (mouse.y > viewport.w) mouse.y = viewport.w;

            // Lunch events
            if (mouse.button == 0 && button != mouse.button) {
                mouse.button = button;
                onMouseClick(mouse.x, mouse.y, mouse.button);
            } 
            else {
                mouse.button = button;
            }

            if (mouse.velX != 0.0 || mouse.velY != 0.0) {
                if (button != 0) onMouseDrag(mouse.x, mouse.y, mouse.button);
                else onMouseMove(mouse.x, mouse.y);
            }
        }
    #else
        // OSX/LINUX
        glfwPollEvents();
    #endif
}

void renderGL(){
    #ifdef PLATFORM_RPI
        // RASPBERRY_PI
        eglSwapBuffers(display, surface);
    #else
        // OSX/LINUX
        glfwSwapBuffers(window);
        glfwPollEvents();
    #endif
}

void closeGL(){
    #ifdef PLATFORM_RPI
        // RASPBERRY_PI
        eglSwapBuffers(display, surface);
        // Release OpenGL resources
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(display, surface);
        eglDestroyContext(display, context);
        eglTerminate(display);
    #else
        // OSX/LINUX
        glfwSetWindowShouldClose(window, GL_TRUE);
        glfwTerminate();
    #endif
}
//-------------------------------------------------------------

void setWindowSize(int _width, int _height) {
    viewport.z = _width;
    viewport.w = _height;
    glViewport(0.0, 0.0, (float)viewport.z, (float)viewport.w);
    orthoMatrix = glm::ortho((float)viewport.x, (float)viewport.z, (float)viewport.y, (float)viewport.w);

    onViewportResize(viewport.z, viewport.w);
}

glm::ivec2 getScreenSize() {
    glm::ivec2 screen;
    
    #ifdef PLATFORM_RPI
        // RASPBERRYPI
        
        if (!bBcm) {
            bcm_host_init();
            bBcm = true;
        }
        uint32_t screen_width;
        uint32_t screen_height;
        int32_t success = graphics_get_display_size(0 /* LCD */, &screen_width, &screen_height);
        assert(success >= 0);
        screen = glm::ivec2(screen_width, screen_height);
    #else
        // OSX/Linux
        glfwGetMonitorPhysicalSize(glfwGetPrimaryMonitor(), &screen.x, &screen.y);
    #endif

    return screen;
}

float getDevicePixelRatio() {
    #ifdef PLATFORM_RPI
        // RASPBERRYPI
        return 1.;
    #else
        // OSX/LINUX
        int window_width, window_height, framebuffer_width, framebuffer_height;
        glfwGetWindowSize(window, &window_width, &window_height);
        glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
        return framebuffer_width/window_width;
    #endif
}

int getWindowWidth() {
    return viewport.z;
}

int getWindowHeight() {
    return viewport.w;
}

glm::mat4 getOrthoMatrix() {
    return orthoMatrix;
}

double getTime() {
    return fTime;
}

double getDelta() {
    return fDelta;
}

glm::vec4 getDate() {
    gettimeofday(&tv, NULL);
    struct tm *tm;
    tm = localtime(&tv.tv_sec);
    return glm::vec4(tm->tm_year,
                     tm->tm_mon,
                     tm->tm_mday,
                     tm->tm_hour*3600.0f+tm->tm_min*60.0f+tm->tm_sec+tv.tv_usec*0.000001);
}

float getMouseX(){
    return mouse.x;
}

float getMouseY(){
    return mouse.y;
}

glm::vec2 getMousePosition() {
    return glm::vec2(mouse.x,mouse.y);
}

float getMouseVelX(){
    return mouse.velX;
}

float getMouseVelY(){
    return mouse.velY;
}

glm::vec2 getMouseVelocity() {
    return glm::vec2(mouse.velX,mouse.velY);
}

int getMouseButton(){
    return mouse.button;
}
