%module tangram
%{
extern void start(int width, int height);
extern void update();
extern void render();
extern void close();
%}

extern void start(int width, int height);
extern void update();
extern void render();
extern void close();
