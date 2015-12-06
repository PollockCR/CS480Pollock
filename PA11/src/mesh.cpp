#include <assert.h>
#include "mesh.h"

// Resource used: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

bool Mesh::loadMesh( const char * objectFilename, int &numOfMeshes )
{
  Assimp::Importer importer;
  unsigned int index, vertexNum;

  //read from file
  const aiScene* pScene = importer.ReadFile( objectFilename, aiProcess_Triangulate );
  
  // get number of meshes
  numOfMeshes = pScene -> mNumMeshes;

  // if scene is not good
  if( !pScene )
  {
    // print erorr
    printf("Error parsing '%s'\n", objectFilename);
    return false;
  }

  // create aiMesh
  aiMesh* mesh = pScene -> mMeshes[0];

  // loop through each face
  for( index = 0; index < mesh-> mNumFaces; index++ )
  {
    // create aiFace
    const aiFace& face = mesh -> mFaces[index];

    // loop through each face in each vertex
    for( vertexNum = 0; vertexNum < 3; vertexNum++ )
    {
      aiVector3D tempPos = mesh -> mVertices[face.mIndices[vertexNum]];
      Vertex tempVertex;

      // save x y and z for each vertex
      tempVertex.position[0] = tempPos.x;
      tempVertex.position[1] = tempPos.y;
      tempVertex.position[2] = tempPos.z;

      // if has texture, save uv values
      if( mesh->HasTextureCoords(0) )
      {
        aiVector3D tempTex = mesh -> mTextureCoords[0][face.mIndices[vertexNum]];
        tempVertex.uv[0] = tempTex.x;
        tempVertex.uv[1] = tempTex.y;
      }

      // if no texture, save 0 for uv values
      else 
      {
        aiVector3D tempTex(0.0f, 0.0f, 0.0f);
        tempVertex.uv[0] = tempTex.x;
        tempVertex.uv[1] = tempTex.y;
      }

      // if has texture, save uv values
      if( mesh->HasNormals() )
      {
        aiVector3D tempNormals = mesh -> mNormals[face.mIndices[vertexNum]];
        tempVertex.normals[0] = tempNormals.x;
        tempVertex.normals[1] = tempNormals.y;
        tempVertex.normals[2] = tempNormals.z;
      }

      // push vertex to geometry
      geometry.push_back(tempVertex);      
    }
  }

  // return success
  return true;
}

