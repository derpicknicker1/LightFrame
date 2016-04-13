#include "PImage.h"

uint16_t PImage::read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t PImage::read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

RgbColor PImage::readCol(File f){
  uint8_t b = f.read();
  uint8_t g = f.read();
  uint8_t r = f.read();
  return RgbColor(r, g, b);
}

imgLoad_t PImage::loadImage(String fileName) {
  File     bmpFile;
  _flip  = false;        // BMP is stored bottom-to-top
  _valid=false;

  if(!SPIFFS.exists(fileName))
    return IMG_FILE_NOT_FOUND; // load error
  
  bmpFile = SPIFFS.open(fileName, "r") ;
  
  if (!bmpFile) 
    return IMG_ERR_FILE_OPEN; // load error
  
  _bmpFile = bmpFile;
  
  if(read16(bmpFile) != 0x4D42) 
    return IMG_NOT_BMP;
  
  _size = read32(bmpFile);
  (void)read32(bmpFile); // Read & ignore creator bytes
  _bmpImageoffset = read32(bmpFile); // Start of image data
  (void)read32(bmpFile);// Read & header size
  _bmpWidth  = read32(bmpFile);
  _bmpHeight = read32(bmpFile);
  
  if(read16(bmpFile) != 1) 
    return IMG_ERR_N_O_PLANES;

  _bmpDepth = read16(bmpFile); // bits per pixel

  if((_bmpDepth != 24) || (read32(bmpFile) != 0)) 
    return IMG_ERR_PX_FORMAT;

  if((_bmpWidth > cfg.width) || (_bmpHeight > cfg.height) || (_bmpWidth < cfg.width) || (_bmpHeight < cfg.height))
    return IMG_ERR_IMG_SIZE;

  _rowSize = (_bmpWidth * 3 + 3) & ~3;

  if(_bmpHeight < 0) {
    _bmpHeight = -_bmpHeight;
    _flip      = true;
  }
  
  _valid=true;
  return IMG_LOAD_OK;
  
}

void PImage::draw(NeoPixelBus* strip){
  if(_bmpFile.position() != _bmpImageoffset)
    _bmpFile.seek(_bmpImageoffset,SeekSet);

  for(int x=0;x<cfg.width;x++)
    for(int y=0;y<cfg.height;y++)
      if(_flip)
        strip->SetPixelColor(XY(x,y),readCol(_bmpFile));
      else
        strip->SetPixelColor(XY(cfg.width-x-1,y),readCol(_bmpFile));
 } 


