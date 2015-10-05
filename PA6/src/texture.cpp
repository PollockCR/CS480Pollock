#include <iostream>
#include "texture.h"

Texture::Texture(const std::string& textureFilename )
{
   m_filename = textureFilename;
}

bool Texture::load()
{
   try
   {
      m_image.read(m_filename);
      m_image.write(&m_blob, "RGBA");
   }
   catch( Magick::Error& error )
   {
      std::cout << "[F] TEXTURE " << m_filename << " NOT LOADED CORRECTLY: " << error.what() << std::endl;
      return false;
   }

   imageWidth = m_image.columns();
   imageHeight = m_image.rows();
   imageData = m_blob.data();

   return true;
}