#ifndef OBJECTLOADER_H
#define OBJECTLOADER_H
#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

#include <glm/glm.hpp>

bool loadOBJ( const char * path, std::vector<glm::vec3> & out_vertices, std::vector<glm::vec2> & out_uvs, std::vector<glm::vec3> & out_normals, std::vector<int> & materials, std::vector<std::string> & materialInfo, std::vector<glm::vec3> & out_diffuses, std::vector<glm::vec3> & out_speculars );
int loadMaterialInfo( char *materialFilepath, std::vector<std::string> &materialInfo, std::vector<unsigned int> & diffuseValues, std::vector<unsigned int> & specularValues );
#endif