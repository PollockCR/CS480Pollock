#include <iostream>
#include <string>
#include <stdio.h>
#include "shaderClass.h"
#include <fstream>


//load shaders function
std::string shaderLoaderClass::loadShader ( const char* fileName)
{
    std::ifstream f(fileName, std::ifstream::in);
    std::string shader;


    if (!f.is_open ())
        {

	std::cerr << "[F] FAILED TO READ FILE!" << fileName << std::endl;
	
        }

    std::string line;

    while (!f.eof ())
        {
	std::getline (f, line);
	shader.append (line+ "\n");
	}
    f.close ();

    return shader;  
}

