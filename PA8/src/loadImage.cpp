# include "loadImage.h"

Image::Image()
{
}

Image::~Image()
{
}


bool Image::loadImage( const char* imageFilepath )
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
