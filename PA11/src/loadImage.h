#ifndef LOADIMAGE_H
#define LOADIMAGE_H

// Resource used: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <string>

// Assimp
#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/color4.h> // Post processing flags

// MAGICK++
#include <Magick++.h>

// GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // makes passing matrices to shaders easier

class Image
{
public:
   Image();
   ~Image();
   bool loadImage( const char* imageFilepath );
   glm::mat4 model;
   glm::mat4 view;
   glm::mat4 mvp;
   Magick::Blob m_blob;
   GLuint vbo_geometry;
   GLuint texture;
   GLuint normalbuffer;	
   GLuint imageCols;
   GLuint imageRows;
   int geometrySize;
private:

};

#endif
