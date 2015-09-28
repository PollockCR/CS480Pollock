#include <assert.h>
#include "mesh.h"

// Resource used: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html

Mesh::MeshEntry::MeshEntry()
{
   vertexbuffer = INVALID_OGL_VALUE;
   indexbuffer = INVALID_OGL_VALUE;
   numIndices  = 0;
   materialIndex = INVALID_MATERIAL;
};

Mesh::MeshEntry::~MeshEntry()
{
   if (vertexbuffer != INVALID_OGL_VALUE)
   {
      glDeleteBuffers(1, &vertexbuffer);
   }

   if (indexbuffer != INVALID_OGL_VALUE)
   {
      glDeleteBuffers(1, &indexbuffer);
   }
}

void Mesh::MeshEntry::init(const std::vector<Vertex>& vertices,
                          const std::vector<unsigned int>& indices)
{
   numIndices = indices.size();

   glGenBuffers(1, &vertexbuffer);
   glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

   glGenBuffers(1, &indexbuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numIndices, &indices[0], GL_STATIC_DRAW);
}

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
    clear();
}


void Mesh::clear()
{
    /*for (unsigned int i = 0 ; i < m_Textures.size() ; i++) {
        SAFE_DELETE(m_Textures[i]);
    }*/
}

bool Mesh::loadMesh(const std::string& filename)
{
   // Release the previously loaded mesh (if it exists)
   clear();
   
   bool result = false;
   Assimp::Importer importer;
   
   //read from file
   const aiScene* pScene = Importer.ReadFile(filename.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
   
   // if scene is good
   if (pScene)
   {
      // initialize scene
      result = InitFromScene(pScene, filename);
   }
   // if scene is not good
   else
   {
      // print erorr
      printf("Error parsing '%s': '%s'\n", filename.c_str(), Importer.GetErrorString());
   }

   return result;
}

bool Mesh::initFromScene(const aiScene* pScene, const std::string& filename)
{  
   m_Entries.resize(pScene->mNumMeshes);
   m_Textures.resize(pScene->mNumMaterials);

   // Initialize the meshes in the scene one by one
   for(unsigned int i = 0 ; i < m_Entries.size() ; i++)
   {
     const aiMesh* paiMesh = pScene->mMeshes[i];
     initMesh(i, paiMesh);
   }

   return initMaterials(pScene, filename);
}

void Mesh::initMesh(unsigned int index, const aiMesh* paiMesh)
{
   m_Entries[index].materialIndex = paiMesh->mMaterialIndex;
   
   std::vector<Vertex> vertices;
   std::vector<unsigned int> indices;

   const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

   for(unsigned int i = 0 ; i < paiMesh->mNumVertices ; i++)
   {
      const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
      const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
      const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &zero3D;

      Vertex v(Vector3f(pPos->x, pPos->y, pPos->z),
               Vector2f(pTexCoord->x, pTexCoord->y),
               Vector3f(pNormal->x, pNormal->y, pNormal->z));

      Vertices.push_back(v);
   }

   for (unsigned int i = 0 ; i < paiMesh->mNumFaces ; i++)
   {
      const aiFace& face = paiMesh->mFaces[i];
      assert(face.mNumIndices == 3);
      indices.push_back(face.mIndices[0]);
      indices.push_back(face.mIndices[1]);
      indices.push_back(face.mIndices[2]);
   }

   m_Entries[index].init(vertices, indices);
}

bool Mesh::initMaterials(const aiScene* pScene, const std::string& filename)
{
   // Extract the directory part from the file name
   std::string::size_type slashIndex = filename.find_last_of("/");
   std::string dir;

   if (slashIndex == std::string::npos)
   {
      dir = ".";
   }
   else if (SlashIndex == 0)
   {
      dir = "/";
   }
   else
   {
      dir = filename.substr(0, slashIndex);
   }

   bool result = true;

   // Initialize the materials
   for (unsigned int i = 0 ; i < pScene->mNumMaterials ; i++)
   {
      const aiMaterial* pMaterial = pScene->mMaterials[i];

      m_Textures[i] = NULL;

      if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
      {
         aiString path;

         if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
         {
            std::string fullPath = dir + "/" + path.data;
            m_Textures[i] = new Texture(GL_TEXTURE_2D, fullPath.c_str());

            if (!m_Textures[i]->Load())
            {
               printf("Error loading texture '%s'\n", fullPath.c_str());
               delete m_Textures[i];
               m_Textures[i] = NULL;
               result = false;
            }
            else
            {
               //printf("Loaded texture '%s'\n", fullPath.c_str());
            }
         }
     }

     // Load a white texture in case the model does not include its own texture
     if (!m_Textures[i]) {
         m_Textures[i] = new Texture(GL_TEXTURE_2D, "../bin/white.png");

         result = m_Textures[i]->Load();
     }
   }

   return result;
}