#ifndef LIGHTINGTECHNIQUE_H
#define LIGHTINGTECHNIQUE_H

#include "technique.h"
#include "math_3d.h"

/*��� ������ ����������� ������������� �����.
  ��������� ������ 2 ���� - ���� � ������� �������������.*/

struct DirectionLight
{
    Vector3f Color;
    float AmbientIntensity;
};

/*��� ������ ������ ������������� ������ Technique.
  LightingTechnique ��������� ���� �����, � ������������ ����� ��������� �����,
  ��������� ������� ���������� ���������� � ��������, ������� ���������� ������� �����.*/

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