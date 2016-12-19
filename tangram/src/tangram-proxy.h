#pragma once

void load(char * style, int width, int height);
bool update();
void render();
void close();

void setPixelScale(float _pixelsPerPoint);

void setPosition(double _lon, double _lat);
void setZoom(float _z);
void setRotation(float _radians);
void setTilt(float _radians);
