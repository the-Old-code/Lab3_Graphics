#ifndef LIGHTINGTECHNIQUE_H
#define LIGHTINGTECHNIQUE_H

#include "technique.h"
#include "math_3d.h"

/*Ёто начало определени€ направленного света.
  —труктура хранит 2 пол€ - цвет и фоновую интенсивность.*/

struct DirectionLight
{
    Vector3f Color;
    float AmbientIntensity;
};

/*Ёто первый пример использовани€ класса Technique.
  LightingTechnique наследует этот класс, и представл€ет собой инвентарь света,
  использу€ основой функционал компил€ции и линковки, которые предлагает базовый класс.*/

class LightingTechnique : public Technique
{
public:
    LightingTechnique();
    virtual bool Init();

    void SetWVP(const Matrix4f* WVP);
    void SetTextureUnit(unsigned int TextureUnit);
    void SetDirectionalLight(const DirectionLight& Light);

private:
    GLuint m_WVPLocation;
    GLuint m_samplerLocation;
    GLuint m_dirLightColorLocation;
    GLuint m_dirLightAmbientIntensityLocation;
};

#endif // LIGHTINGTECHNIQUE_H