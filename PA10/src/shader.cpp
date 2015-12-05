#include "shader.h"
#include <GL/glew.h>
#include <GL/glut.h> 
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

// resources used to create shader loader:
// http://www.nexcius.net/2012/11/20/how-to-load-a-glsl-shader-in-opengl-using-c/

// reads in file to a string
std::string ShaderLoader::readFile(const char *filePath)
{
    std::string content; // holds string to return
    std::ifstream ifs(filePath, std::ifstream::in); // input file stream

    // check for invalid file
    if(!ifs.is_open())
    {
        std::cerr << "[F] FAILED TO READ FILE!" << filePath << std::endl;
        return "";
    }

    // save each line to string
    std::string line = "";
    while(!ifs.eof()) {
        std::getline(ifs, line);
        content.append(line + "\n");
    }

    // close file 
    ifs.close();

    // return file contents in string
    return content;
}

// load shaders
bool ShaderLoader::loadShader(const char *vertex_path, const char *fragment_path, GLuint &program)
{
    // create shaders
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    // read shaders
    std::string vertShaderStr = readFile(vertex_path);
    std::string fragShaderStr = readFile(fragment_path);
    const char *vertShaderSrc = vertShaderStr.c_str();
    const char *fragShaderSrc = fragShaderStr.c_str();

    // hold results
    GLint result = GL_FALSE;
    int logLength;

    // compile vertex shader
    glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
    glCompileShader(vertShader);

    // check vertex shader
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);

        // failed compile
        if(!result)
        {
            glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> vertShaderError((logLength > 1) ? logLength : 1);
            glGetShaderInfoLog(vertShader, logLength, NULL, &vertShaderError[0]);
            std::cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << &vertShaderError[0] << std::endl;
            return false;
        }

    // compile fragment shader
    glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragShader);

    // check fragment shader
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);

        // failed compile
        if(!result)
        {
            glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> fragShaderError((logLength > 1) ? logLength : 1);
            glGetShaderInfoLog(fragShader, logLength, NULL, &fragShaderError[0]);
            std::cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << &fragShaderError[0] << std::endl;
            return false;
        }

    // create program and link with shaders
    program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    // check program status
    glGetProgramiv(program, GL_LINK_STATUS, &result);

        // failed program
        if(!result)
        {
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> programError( (logLength > 1) ? logLength : 1 );
            glGetProgramInfoLog(program, logLength, NULL, &programError[0]);
            std::cerr << "[F] THE SHADER PROGRAM FAILED TO LINK" << &programError[0] << std::endl;
            return false;
        }

    // delete shaders after attachment
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    // return program with shaders
    return true;
}

