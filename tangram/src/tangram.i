%module example
%{
extern void TangramInit(int width, int height);
extern void TangramUpdate();
extern void TangramRender();
extern void TangramClose();
%}

extern void TangramInit(int width, int height);
extern void TangramUpdate();
extern void TangramRender();
extern void TangramClose();
