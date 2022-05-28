#ifndef GLUT_BACKEND_H
#define GLUT_BACKEND_H

#include "callbacks.h"

/*упрощает инициализацию GLUT'а и создает окно использу€ простую функцию*/
void GLUTBackendInit(int argc, char** argv);

bool GLUTBackendCreateWindow(unsigned int Width, unsigned int Height, unsigned int bpp, bool isFullScreen, const char* pTitle);

/*Ёто нововведение - интерфейс ICallbacks, который поможет в регистрации функций обратного вызова*/
void GLUTBackendRun(ICallbacks* pCallbacks);

#endif /* GLUT_BACKEND_H */