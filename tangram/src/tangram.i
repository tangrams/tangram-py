%module tangram
%{
#define SWIG_FILE_WITH_INIT
extern void start(int width, int height);
extern void update();
extern void render();
extern void close();
%}

extern void start(int width, int height);
extern void update();
extern void render();
extern void close();
