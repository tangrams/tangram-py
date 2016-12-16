%module tangram
%{
#define SWIG_FILE_WITH_INIT

void load(char * style, int width, int height);
bool update();
void render();
void close();

void setPosition(double _lon, double _lat);
void setZoom(float _z);
void setRotation(float _radians);
void setTilt(float _radians);
%}

void load(char * style, int width, int height);
bool update();
void render();
void close();

void setPosition(double _lon, double _lat);
void setZoom(float _z);
void setRotation(float _radians);
void setTilt(float _radians);
