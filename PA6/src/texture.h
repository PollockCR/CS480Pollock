#ifndef TEXTURE_H
#define TEXTURE_H

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

// Magick++
#include <Magick++.h>

// GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // makes passing matrices to shaders easier

class Texture
{
public:
   Texture( const std::string& textureFilename );
   bool load();
   GLsizei imageWidth;
   GLsizei imageHeight;
   const GLvoid* imageData;

private:
   std::string m_filename;
   Magick::Image m_image;
   Magick::Blob m_blob;
};

#endif