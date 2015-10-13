#ifndef PLANET_H
#define PLANET_H

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

class Planet
{
public:
   Planet();
   ~Planet();
   //Planet& operator=(const Planet&);
   bool loadImage( const char* imageFilepath );
   std::string nameOfPlanet;
   GLfloat scale;
   float rotationAngle;
   float orbitAngle;
   float rotationSpeed;
   float orbitSpeed;
   glm::vec3 rotationAxis;
   glm::vec3 orbitPath;
   int orbitIndex;
   glm::mat4 model;
   glm::mat4 mvp;
   Magick::Blob m_blob;
   GLuint vbo_geometry;
   GLuint texture;	
   GLuint imageCols;
   GLuint imageRows;
   int geometrySize;
private:

};

#endif