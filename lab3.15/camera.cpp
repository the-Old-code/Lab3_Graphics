#include <GL/freeglut.h>

#include "camera.h"

const static float STEP_SCALE = 0.1f;
const static int MARGIN = 10;

Camera::Camera(int WindowWidth, int WindowHeight)
{
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = Vector3f(0.0f, 0.0f, 0.0f);
    m_target = Vector3f(0.0f, 0.0f, 1.0f);
    m_target.Normalize();
    m_up = Vector3f(0.0f, 1.0f, 0.0f);

    Init();
}

//Конструктор камеры теперь принимает размеры окна.(для перемещения курсора в центр экрана)
Camera::Camera(int WindowWidth, int WindowHeight, const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up)
{
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = Pos;

    m_target = Target;
    m_target.Normalize();

    m_up = Up;
    m_up.Normalize();

    Init(); //установит внутренние параметры камеры
}


void Camera::Init()
{
    Vector3f HTarget(m_target.x, 0.0, m_target.z);//HTarget (направление по горизонтали)
    HTarget.Normalize(); //нормируем
    //вычисления горизонтального угла
    if (HTarget.z >= 0.0f)
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = 360.0f - ToDegree(asin(HTarget.z));
        }
        else
        {
            m_AngleH = 180.0f + ToDegree(asin(HTarget.z));
        }
    }
    else
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = ToDegree(asin(-HTarget.z));
        }
        else
        {
            m_AngleH = 90.0f + ToDegree(asin(-HTarget.z));
        }
    }
    //подсчитываем вертикальный угол
    m_AngleV = -ToDegree(asin(m_target.y));
    //4 новых параметра для проверки не касается ли курсор границ экрана
    m_OnUpperEdge = false;
    m_OnLowerEdge = false;
    m_OnLeftEdge = false;
    m_OnRightEdge = false;
    //Следующие 2 строки кода вычисляют центр экрана
    m_mousePos.x = m_windowWidth / 2;
    m_mousePos.y = m_windowHeight / 2;
    //перемещает курсор
    glutWarpPointer(m_mousePos.x, m_mousePos.y);
}


bool Camera::OnKeyboard(int Key)
{
    bool Ret = false;

    switch (Key) {

    case GLUT_KEY_UP:
    {
        m_pos += (m_target * STEP_SCALE);
        Ret = true;
    }
    break;

    case GLUT_KEY_DOWN:
    {
        m_pos -= (m_target * STEP_SCALE);
        Ret = true;
    }
    break;

    case GLUT_KEY_LEFT:
    {
        Vector3f Left = m_target.Cross(m_up);
        Left.Normalize();
        Left *= STEP_SCALE;
        m_pos += Left;
        Ret = true;
    }
    break;

    case GLUT_KEY_RIGHT:
    {
        Vector3f Right = m_up.Cross(m_target);
        Right.Normalize();
        Right *= STEP_SCALE;
        m_pos += Right;
        Ret = true;
    }
    break;
    }

    return Ret;
}

//Эта функция используется что бы сообщить камере, что положение мыши изменилось.
void Camera::OnMouse(int x, int y)
{
    //подсчет разницы между новыми координатами и предыдущими по осям и X и Y.
    const int DeltaX = x - m_mousePos.x;
    const int DeltaY = y - m_mousePos.y;

    m_mousePos.x = x;
    m_mousePos.y = y;
    //обновляем текущие горизонтальные и вертикальные углы на эту разность в значениях.
    m_AngleH += (float)DeltaX / 20.0f;
    m_AngleV += (float)DeltaY / 20.0f;
    //обновляем значения 'm_OnEdge' согласно положению курсора
    if (DeltaX == 0) {
        if (x <= MARGIN) { //если у левой границы вращение влево
            //    m_AngleH -= 1.0f;
            m_OnLeftEdge = true;
        }
        else if (x >= (m_windowWidth - MARGIN)) { //у правой границы вращение вправо
            //    m_AngleH += 1.0f;
            m_OnRightEdge = true;
        }
    }
    else {
        m_OnLeftEdge = false;
        m_OnRightEdge = false;
    }

    if (DeltaY == 0) {
        if (y <= MARGIN) {
            m_OnUpperEdge = true;
        }
        else if (y >= (m_windowHeight - MARGIN)) {
            m_OnLowerEdge = true;
        }
    }
    else {
        m_OnUpperEdge = false;
        m_OnLowerEdge = false;
    }

    Update();//для перерасчета векторов направления и вектора вверх,
             //основанных на новых горизонтальном и вертикальном углах
}

//для случаев, когда мышь не движется, но находится около одной из границ экрана.
void Camera::OnRender()
{
    bool ShouldUpdate = false;
    //проверяем не установлен ли хоть один из флагов, 
    //и если таковой найден, то изменится один из углов.
    if (m_OnLeftEdge) {
        m_AngleH -= 0.1f;
        ShouldUpdate = true;
    }
    else if (m_OnRightEdge) {
        m_AngleH += 0.1f;
        ShouldUpdate = true;
    }

    if (m_OnUpperEdge) {
        if (m_AngleV > -90.0f) {
            m_AngleV -= 0.1f;
            ShouldUpdate = true;
        }
    }
    else if (m_OnLowerEdge) {
        if (m_AngleV < 90.0f) {
            m_AngleV += 0.1f;
            ShouldUpdate = true;
        }
    }

    if (ShouldUpdate) {
        Update(); //для обновления векторов камеры.
    }
}
//функция обновляет значения векторов направления и вверх согласно горизонтальному и вертикальному углам.
void Camera::Update()
{
    const Vector3f Vaxis(0.0f, 1.0f, 0.0f);//вектор параллелен земле

    // Rotate the view vector by the horizontal angle around the vertical axis
    Vector3f View(1.0f, 0.0f, 0.0f);
    View.Rotate(m_AngleH, Vaxis);
    View.Normalize();

    // Rotate the view vector by the vertical angle around the horizontal axis
    Vector3f Haxis = Vaxis.Cross(View);
    Haxis.Normalize();
    View.Rotate(m_AngleV, Haxis);

    m_target = View;//меняется вектор направленмя
    m_target.Normalize();
    //меняется вектор вверх
    m_up = m_target.Cross(Haxis);
    m_up.Normalize();
}