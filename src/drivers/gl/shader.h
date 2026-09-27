#pragma once
#include <GL/glew.h>
#include <string>

class Shader
{
public:
	Shader(const char* vertexSrc, const char* fragmentSrc);
    ~Shader();

    void use() const;
    GLuint id() const { return m_id; }

    // Convenience uniform setters (add more as engine grows)
    void setMat4(const char* name, const float* mat4x4) const;
    void setVec4(const char* name, float x, float y, float z, float w) const;

private:
    GLuint compile(GLenum type, const char* src);
    GLuint m_id = 0;
};