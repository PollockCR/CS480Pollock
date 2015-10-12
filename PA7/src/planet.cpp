#include "planet.h"

Planet::Planet()
{
  // set default valuess
  rotationAxis.x = 0.0;
  rotationAxis.y = 1.0;
  rotationAxis.z = 0.0;

  orbitPath.x = 0.0;
  orbitPath.y = 0.0;
  orbitPath.z = 0.0;  

  scale = 1.0;
  rotationAngle = 0.0;
  orbitAngle = 0.0;
  rotationSpeed = M_PI/2;
  orbitSpeed = M_PI/2;
  orbitIndex = 0;
}

Planet::~Planet()
{
}

bool Planet::loadImage( const char* imageFilepath )
{
  // initialize magick
  Magick::InitializeMagick(NULL);
  Magick::Image* temp_pImage;

  // try to load image
  try
  {
    // save image to image pointer
    temp_pImage = new Magick::Image( imageFilepath );
  }
  // output error if not loaded
  catch(Magick::Error& err)
  {
    std::cerr << "[F] IMAGE NOT LOADED CORRECTLY: " << err.what() << std::endl;
    return false;
  }

  // save image dimensions
  imageCols = temp_pImage->columns();
  imageRows = temp_pImage->rows();

  // write data to blob
  temp_pImage->write(&m_blob, "RGBA");

  //return success
  return true;
}
