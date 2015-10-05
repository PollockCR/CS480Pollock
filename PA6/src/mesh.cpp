#include <assert.h>
#include "mesh.h"

// Resource used: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
  clearMesh();
}


void Mesh::clearMesh()
{
  unsigned int index;
  for(index = 0 ; index < m_Textures.size() ; index++)
  {
    delete m_Textures[index];
  }
}


bool Mesh::loadMesh( char * objectFilename )
{
  Assimp::Importer importer;
  unsigned int index, vertexNum;
  const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

  //read from file
  const aiScene* pScene = importer.ReadFile( objectFilename, aiProcess_Triangulate );

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
      if( mesh->HasTextureCoords(0) )
      {
        aiVector3D tempUv = mesh->mTextureCoords[0][face.mIndices[vertexNum]];

        // set color for each vertex
        tempVertex.uv[0] = tempUv.x;
        tempVertex.uv[1] = tempUv.y;
      }
      else
      {
        aiVector3D tempUv = zero3D;

        // set color for each vertex
        tempVertex.uv[0] = tempUv.x;
        tempVertex.uv[1] = tempUv.y;        
      }

      // save x y and z for each vertex
      tempVertex.position[0] = tempPos.x;
      tempVertex.position[1] = tempPos.y;
      tempVertex.position[2] = tempPos.z;

      // push vertex to geometry
      geometry.push_back(tempVertex);      
    }
  }
  
  // return success
  return loadMaterials( objectFilename, pScene );
}

bool Mesh::loadMaterials( char* objectFilename, const aiScene* pScene )
{
  // initialize variables
  unsigned int index;
  bool isGood = true;
  std::string tempFilename = objectFilename;

  // find file with materials
  std::string::size_type slashIndex = tempFilename.find_last_of("/");
  std::string directory;

  if(slashIndex == std::string::npos)
  {
    directory = ".";
  }
  else if( slashIndex == 0 )
  {
    directory = "/";
  }
  else
  {
    directory = tempFilename.substr(0, slashIndex);
  }

  m_Textures.resize(pScene->mNumMaterials);

  // initialize materials
  for( index = 0; index < pScene->mNumMaterials; index++ )
  {
    const aiMaterial* pMaterial = pScene->mMaterials[index];

    m_Textures[index] = NULL;

    if( pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0 )
    {
      aiString path;
      if( pMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS )
      {
        std::string fullPath = directory + "/" + path.data;
        m_Textures[index] = new Texture(fullPath.c_str());

        if( !m_Textures[index]->load())
        {
          printf("[F] TEXTURE '%s' NOT LOADED CORRECTLY\n", fullPath.c_str());
          delete m_Textures[index];
          m_Textures[index] = NULL;
          isGood = false;
        }
      }
    }
  }

  return isGood;
}
