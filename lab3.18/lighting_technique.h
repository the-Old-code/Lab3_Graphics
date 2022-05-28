#ifndef LIGHTINGTECHNIQUE_H
#define LIGHTINGTECHNIQUE_H

#include "technique.h"
#include "math_3d.h"

/*Это новая структура направленного света. 
  Появились 2 новых члена: направление в виде 3 вектора,
  указываемое в мировом пространстве, и интенсивность как вещественное число.*/

struct DirectionLight
{
    Vector3f Color;
    float AmbientIntensity;
    Vector3f Direction;
    float DiffuseIntensity;
};

class LightingTechnique : public Technique
{
public:
    LightingTechnique();
    virtual bool Init();

    void SetWVP(const Matrix4f& WVP);
    void SetWorldMatrix(const Matrix4f& WVP);
    void SetTextureUnit(unsigned int TextureUnit);
    void SetDirectionalLight(const DirectionLight& Light);

private:
    GLuint m_WVPLocation;
    GLuint m_WorldMatrixLocation;
    GLuint m_samplerLocation;
    struct {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint Direction;
        GLuint DiffuseIntensity;
    } m_dirLightLocation;
};

#endif // LIGHTINGTECHNIQUE_H