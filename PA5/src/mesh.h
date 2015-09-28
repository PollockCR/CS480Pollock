#ifndef MESH_H
#define  MESH_H

// Resource used: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html

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

struct Vertex
{
   glm::vec3 position;
   glm::vec2 uv;
   glm::vec3 normal;

   Vertex(){}

   Vertex( const glm::vec3& t_position, const glm::vec2& t_uv, const glm::vec3 t_normal )
   {
      position = t_position;
      uv = t_uv;
      normal = t_normal;
   }
};

// Mesh class represents interface between Assimp and OpenGL program
// An object of this class takes a file name to loadMesh and uses Assimp to load model
// Next, creates vertex buffers, index buffers, and texture objects, which contain model data

class Mesh
{
public:
   Mesh();
   ~Mesh();
   bool loadMesh( const std::string& filename );
   void render();
private:
   bool initFromScene( const aiScene* pscene, const std::string& filename );
   void initMesh( unsigned int index, const aiMesh* paiMesh );
   bool initMaterials( const aiScene* pscene, const std::string& filename );
   void clear();

   #define INVALID_MATERIAL 0xFFFFFFFF
   struct MeshEntry
   {
      MeshEntry();
      ~MeshEntry();
      void init( const std:: vector<Vertex> & vertices, const std::vector<unsigned int>& indices );

      GLuint vertexbuffer;
      GLuint indexbuffer;
      unsigned int numIndices;
      unsigned int materialIndex;
   };
   

   std::vector<MeshEntry> m_Entries;
   std::vector<Texture*> m_Textures;   
};

#endif

