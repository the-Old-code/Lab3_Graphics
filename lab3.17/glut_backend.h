#ifndef GLUT_BACKEND_H
#define GLUT_BACKEND_H

#include "callbacks.h"

/*�������� ������������� GLUT'� � ������� ���� ��������� ������� �������*/
void GLUTBackendInit(int argc, char** argv);

bool GLUTBackendCreateWindow(unsigned int Width, unsigned int Height, unsigned int bpp, bool isFullScreen, const char* pTitle);

/*��� ������������ - ��������� ICallbacks, ������� ������� � ����������� ������� ��������� ������*/
void GLUTBackendRun(ICallbacks* pCallbacks);

#endif /* GLUT_BACKEND_H */