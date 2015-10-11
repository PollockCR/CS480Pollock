#include "planet.h"

Planet::Planet()
{
  rotationAxis.x = 0.0;
  rotationAxis.y = 1.0;
  rotationAxis.z = 0.0;
}

Planet::~Planet()
{
}

bool Planet::loadImage( const char* imageFilepath )
{
  // initialize magick
  Magick::InitializeMagick(NULL);

  // try to load image
  try
  {
    // save image to image pointer
    m_pImage = new Magick::Image( imageFilepath );
  }
  // output error if not loaded
  catch(Magick::Error& err)
  {
    std::cerr << "[F] IMAGE NOT LOADED CORRECTLY: " << err.what() << std::endl;
    return false;
  }

  // write data to blob
  m_pImage->write(&m_blob, "RGBA");	

  //return success
  return true;
}
