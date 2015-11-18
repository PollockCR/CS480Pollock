#ifndef SHADER_H
#define SHADER_H
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>

class ShaderLoader
{
public:
   bool loadShader(const char *vertex_path, const char *fragment_path, GLuint &program);
   std::string readFile(const char *filePath);
private:

};

#endif

