#ifndef MESH_H
#define  MESH_H

// Resource used: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/freeglut.h>
#include <iostream>
#include <vector>

// Assimp
#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/color4.h> // Post processing flags

// GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // makes passing matrices to shaders easier

struct Vertex
{
   GLfloat position[3];
   GLfloat color[3];
};

// Mesh class represents interface between Assimp and OpenGL program
// An object of this class takes a file name to loadMesh and uses Assimp to load model
// Next, creates vertex buffers, index buffers, and texture objects, which contain model data

class Mesh
{
public:
   Mesh();
   ~Mesh();
   bool loadMesh( char* objectFilename );
   void render();
   std::vector<Vertex> geometry;
private:   
};

#endif

