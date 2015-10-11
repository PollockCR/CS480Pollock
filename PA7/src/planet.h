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
	bool loadImage( const char* imageFilepath );
	GLfloat scale = 1.0;
	float rotationAngle = 0.0;
	float orbitAngle = 0.0;
  glm::vec3 rotationAxis;
	bool rotateCW = true;
	glm::mat4 model;
	glm::mat4 mvp;
	GLuint vbo_geometry;
	GLuint texture;	
  Magick::Blob m_blob;  	
  Magick::Image* m_pImage;
  int geometrySize;
private:

};

#endif