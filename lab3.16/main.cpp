﻿#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "pipeline.h"
#include "camera.h"
#include "texture.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 1024

struct Vertex
{
    Vector3f m_pos;
    Vector2f m_tex;

    Vertex() {}

    Vertex(Vector3f pos, Vector2f tex)
    {
        m_pos = pos;
        m_tex = tex;
    }
};


GLuint VBO; //переменная для хранения указателя на буфер вершин
GLuint IBO; //указатель на буферный объект для буфера индексов
GLuint gWVPLocation; //указатель для доступа к всемирной матрице
GLuint gSampler;
Texture* pTexture = NULL; //указатель на текстуру
Camera* pGameCamera = NULL; //указатель на камеру

//обновленный вершинный шейдер
static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
layout (location = 1) in vec2 TexCoord;                                             \n\
                                                                                    \n\
uniform mat4 gWVP;                                                                  \n\
                                                                                    \n\
out vec2 TexCoord0;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position = gWVP * vec4(Position, 1.0);                                       \n\
    TexCoord0 = TexCoord;                                                           \n\
}";
//новый фрагментный шейдер
static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
in vec2 TexCoord0;                                                                  \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
uniform sampler2D gSampler;                                                         \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = texture2D(gSampler, TexCoord0.xy);                                  \n\
}";

/*Где-то в функции рендера мы должны вызвать камеру.
  Это дает ей шанс для действий, если мышь не двигалась, но находится около границы экрана.*/
static void RenderSceneCB()
{
    pGameCamera->OnRender();

    glClear(GL_COLOR_BUFFER_BIT);

    /*используем статическую переменную типа float, которую мы будем по-немного увеличивать каждый вызов функции рендера*/
    static float Scale = 0.0f;

    Scale += 0.1f;

    /*Мы создаем объект конвейера, настраиваем его и отправляем результат в шейдер.*/
    Pipeline p;
    p.Rotate(0.0f, Scale, 0.0f);
    p.WorldPos(0.0f, 0.0f, 3.0f);
    p.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 100.0f);

    glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //обратно привязываем буфер, приготавливая его для отрисовки
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); //говорим конвейеру как воспринимать данные внутри буфера
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    pTexture->Bind(GL_TEXTURE0);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0); //отключаем атрибут вершины
    glDisableVertexAttribArray(1);

    glutSwapBuffers(); //меняем фоновый буфер и буфер кадра местами
}

/*Здесь мы регистрируем новую функцию обратного вызова для получения специальных событий клавиатуры*/
static void SpecialKeyboardCB(int Key, int x, int y)
{
    pGameCamera->OnKeyboard(Key);
}

/*При нажатии 'q' мы выходим*/
static void KeyboardCB(unsigned char Key, int x, int y)
{
    switch (Key) {
    case 'q':
        glutLeaveMainLoop();
    }
}


static void PassiveMouseCB(int x, int y)
{
    pGameCamera->OnMouse(x, y);
}


static void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(RenderSceneCB); //указываем функцию рендера в качестве ленивой

     /*Мы регистрируем 2 новых функции обратного вызова. Одна для мыши и другая для нажатия специальных клавиш*/
    glutSpecialFunc(SpecialKeyboardCB);
    glutPassiveMotionFunc(PassiveMouseCB);

    glutKeyboardFunc(KeyboardCB);
}


static void CreateVertexBuffer()
{
    Vertex Vertices[4] = { Vertex(Vector3f(-1.0f, -1.0f, 0.5773f), Vector2f(0.0f, 0.0f)),
                           Vertex(Vector3f(0.0f, -1.0f, -1.15475), Vector2f(0.5f, 0.0f)),
                           Vertex(Vector3f(1.0f, -1.0f, 0.5773f),  Vector2f(1.0f, 0.0f)),
                           Vertex(Vector3f(0.0f, 1.0f, 0.0f),      Vector2f(0.5f, 1.0f)) };

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}


static void CreateIndexBuffer()
{
    /*Буфер индексов заполняется с помощью массива индексов. Индексы указывают на расположение вершин в вершинном буфере.*/
    unsigned int Indices[] = { 0, 3, 1,
                               1, 3, 2,
                               2, 3, 0,
                               1, 2, 0 };

    /*Мы создаем, а затем заполняем буфер индексов используя массив индексов.*/
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType); //начинаем процесс разработки шейдеров через создание программного объекта

    /*проверяем ошибки*/
    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    /*создаём шейдер*/
    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);
    glCompileShader(ShaderObj);
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj); //присоединяем скомпилированный объект шейдера к объекту программы
}


static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER);
    AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);

    gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
    assert(gWVPLocation != 0xFFFFFFFF);
    gSampler = glGetUniformLocation(ShaderProgram, "gSampler");
    assert(gSampler != 0xFFFFFFFF);
}


int main(int argc, char** argv)
{
    glutInit(&argc, argv); //инициализируем GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // настраиваем некоторые опции GLUT
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Tutorial 16");

    /*Эта функция glut'а разрешает вашему приложению запускаться в полноэкранном режиме, называемом как 'игровой режим'.*/
    glutGameModeString("1280x1024@32");
    glutEnterGameMode();

    InitializeGlutCallbacks(); //присоединяем функцию RenderSceneCB к GLUT

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT);

    /*Инициализируем GLEW и проверяем на ошибки*/
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //устанавливаем цвет, который будет использован во время очистки буфера кадра
    /*Они включают отброс задней поверхности для дополнительной оптимизации, и используется что бы
      отбраковывать треугольники до затратных процессов растеризации*/
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    CreateVertexBuffer();
    CreateIndexBuffer();

    CompileShaders();

    glUniform1i(gSampler, 0); //Здесь мы устанавливаем индексы модулей текстуры

    Magick::InitializeMagick(nullptr); // добавили эту строку

    pTexture = new Texture(GL_TEXTURE_2D, "D:/labs/Graphics/lab3.16/car.png"); //Здесь мы создаем объект Текстуры и загружаем его D:/labs/Graphics/lab3.16/car.png

    if (!pTexture->Load()) {
        return 1;
    }

    glutMainLoop(); //передаём контроль GLUT'у

    return 0;
}