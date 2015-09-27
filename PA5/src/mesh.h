#ifndef MESH_H
#define  MESH_H

#include "mesh.h" // header file of object loader
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <vector>

// Assimp
#include <Importer.hpp> // C++ importer interface
#include <scene.h> // Output data structure
#include <postprocess.h> // Post processing flags
#include <color4.h> // Post processing flags

// GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // makes passing matrices to shaders easier

class Vertex
{
public:
   glm::vec3 position;
   glm::vec3 color;
   glm::vec2 uv;
   glm::vec3 normal;

   Vertex(){}

   Vertex( const glm::vec3& t_position, const glm::vec3& t_color, const glm::vec2& t_uv, const glm::vec3 t_normal )
   {
      position = t_position;
      color = t_color;
      uv = t_uv;
      normal = t_normal;
   }
private:
};

class Mesh
{
public:
   Mesh();
   ~Mesh();
private:

};

#endif

