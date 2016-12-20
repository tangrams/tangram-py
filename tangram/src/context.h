#pragma once

#include "platform_gl.h"
#undef countof

#include "glm/glm.hpp"

enum ScrollType {
    NONE = 0,
    ROTATE,
    SHOVE,
    PINCH
};

//  GL Context
//----------------------------------------------
void initGL(int _width, int _height);
bool isGL();
void updateGL();
void renderGL();
void closeGL();

//  SET
//----------------------------------------------
void setWindowSize(int _width, int _height);

//  GET
//----------------------------------------------
glm::ivec2 getScreenSize();
float getDevicePixelRatio();

int getWindowWidth();
int getWindowHeight();
glm::ivec4 getViewport();
glm::mat4 getOrthoMatrix();

double getTime();
double getDelta();
glm::vec4 getDate();

float getMouseX();
float getMouseY();
glm::vec2 getMousePosition();
float getMouseVelX();
float getMouseVelY();
glm::vec2 getMouseVelocity();
int getMouseButton();

// EVENTS
//----------------------------------------------
void onKeyPress(int _key);
void onMouseMove(float _x, float _y);
void onMouseClick(float _x, float _y, int _button);
void onMouseDrag(float _x, float _y, int _button);
void onScroll(float _x, float _y, float _scrollx, float _scrolly, ScrollType _type);
void onDrop(int count, const char** paths);

void onViewportResize(int _width, int _height);
