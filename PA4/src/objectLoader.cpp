#include <vector>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>

#include <glm/glm.hpp>

#include "objectLoader.h"

// resource used to create this file: http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/

bool loadOBJ( const char * path, std::vector<glm::vec3> & out_vertices, std::vector<glm::vec2> & out_uvs, std::vector<glm::vec3> & out_normals )
{
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices; 
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;
	//int materialCount = 0;
	//int currentMaterial = 0;
	int index;

	FILE * file = fopen(path, "r");
	if( file == NULL )
	{
		printf("Impossible to open the file! Are you in the right path? Please try again.\n");
		getchar();
		return false;
	}

	while( 1 )
	{
		char lineHeader[128];

		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
		{
			break; // EOF = End Of File. Quit the loop.
		}

		// else : parse lineHeader
		if ( strcmp( lineHeader, "v" ) == 0 )
		{
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			temp_vertices.push_back(vertex);
		}
		else if ( strcmp( lineHeader, "vt" ) == 0 )
		{
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y );
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if ( strcmp( lineHeader, "vn" ) == 0 )
		{
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			temp_normals.push_back(normal);
		}
		else if ( strcmp( lineHeader, "f" ) == 0 )
		{
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9)
			{
				printf("File can't be read by our simple parser. Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices    .push_back(uvIndex[0]);
			uvIndices    .push_back(uvIndex[1]);
			uvIndices    .push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
			// add material if given
			/*if( materialCount > 0 )
			{
				materials.push_back(currentMaterial);
			}*/
		}
		/* extra credit - not finished
		// if word is material library file
		else if ( strcmp( lineHeader, "mtllib" ) == 0 )
		{
			char materialFilepath[128];
			fscanf(file, "%s\n", materialFilepath);
			materialCount = loadMaterialInfo( materialFilepath, materialInfo, diffuseValues, specularValues );
		}
		// if word is material indicator
		else if( strcmp( lineHeader, "usemtl" ) == 0 )
		{
			char materialName[128];
			fscanf(file, "%s\n", materialName);
			std::string materialStr = materialName;
			for( index = 0; index < materialCount; index++ )
			{
				if(materialStr.compare(materialInfo[index]) == 0)
				{
					currentMaterial = index;
				}
			}			
		}*/
		else
		{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for( index = 0; index < (int) vertexIndices.size(); index++ )
	{
		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[index];
		unsigned int uvIndex = uvIndices[index];
		unsigned int normalIndex = normalIndices[index];
		
		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
		glm::vec2 uv = temp_uvs[ uvIndex-1 ];
		glm::vec3 normal = temp_normals[ normalIndex-1 ];
		
		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs     .push_back(uv);
		out_normals .push_back(normal);
	}
 	/* extra credit - not finished
	// For each material
	for( index = 0; index < materialCount; index++ )
	{
		// Get the indices of its attributes
		unsigned int diffuseValue = diffuseValues[index];
		unsigned int specularValue = specularValues[index];
		
		// Get the attributes thanks to the index
		glm::vec3 diffuse = temp_diffuses[ diffuseValue-1 ];
		glm::vec3 specular = temp_speculars[ specularValue-1 ];
		
		// Put the attributes in buffers
		out_diffuses.push_back(diffuse);
		out_speculars.push_back(specular);
	}*/

	// return success
	return true;
}

/* extra credit - not finished
// function to load material information
// found from: http://www.raywenderlich.com/48297/export-blender-models-opengl-es-part-2
int loadMaterialInfo( char *materialFilepath, std::vector<std::string> &materialInfo, std::vector<unsigned int> & diffuseValues, std::vector<unsigned int> & specularValues )
{
	// counter for materials
	int materialCount = 0;

	// open file for input
	std::ifstream inMaterial;
	inMaterial.open(materialFilepath);
	if( !inMaterial.good() )
	{
		std::cout << "Cannot open material file. Please try again." << std::endl;
		return -1;
	}

	// read data from material file
	while( !inMaterial.eof() )
	{
		//read in identifier
		std::string line;
		std::getline(inMaterial, line, ' ');
		std::string type = line.substr(0,2);
		unsigned int temp_diffuses[3], temp_speculars[3];

		// increment for ne type
		if(type.compare("ne") == 0)
		{
			std::getline(inMaterial, line);
			materialInfo.push_back(line);
			materialCount++;
		}
		// type diffuses
		else if ( type.compare("Kd") == 0 )
		{
			// copy line for parsing
			inMaterial >> temp_diffuses[0] >> temp_diffuses[1] >> temp_diffuses[2];
			diffuseValues.push_back(temp_diffuses[0]);
			diffuseValues.push_back(temp_diffuses[1]);
			diffuseValues.push_back(temp_diffuses[2]); 
		}
		// type speculars
		else if ( type.compare("Ks") == 0 )
		{
			// copy line for parsing
			inMaterial >> temp_speculars[0] >> temp_speculars[1] >> temp_speculars[2]; 
			specularValues.push_back(temp_speculars[0]);
			specularValues.push_back(temp_speculars[1]);
			specularValues.push_back(temp_speculars[2]);
		}		
	}

	// close material file
	inMaterial.close();

	// return number of materials
	return materialCount;

}
*/