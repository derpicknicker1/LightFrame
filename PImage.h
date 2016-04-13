#ifndef _PIMAGE_H
#define _PIMAGE_H

#include "./config.h"

#define XY(x,y)          ( 255 - (( (y) & 0x01)?((y) * WIDTH + (x)):((y) * WIDTH + WIDTH - 1 -  (x))))

enum IMGLOAD_ENUM{
  IMG_LOAD_OK,
  IMG_FILE_NOT_FOUND,
  IMG_ERR_FILE_OPEN,
  IMG_NOT_BMP,
  IMG_ERR_N_O_PLANES,
  IMG_ERR_PX_FORMAT,
  IMG_ERR_IMG_SIZE
} ;

typedef enum IMGLOAD_ENUM imgLoad_t;



class PImage {
public:
  PImage() :  
    _bmpWidth(0), 
    _bmpHeight(0),
    _valid(false){ }

  PImage(String file){ loadImage(file);}

  ~PImage(){ _bmpFile.close();}
  
  void draw(NeoPixelBus* strip);
  
  imgLoad_t loadImage(String fileName);
  
  void      close()   { _bmpFile.close(); }
  bool      isValid() { return _valid; }
  int       width()   { return _bmpWidth; }
  int       height()  { return _bmpHeight; }
  uint8_t   depth()   { return _bmpDepth; }              // Bit depth (currently must be 24)
  uint32_t  offset()  { return _bmpImageoffset; }        // Start of image data in file
  uint32_t  rowSize() { return _rowSize; }               // Not always = bmpWidth; may have padding
  bool      flip()    { return _flip; } 
  
private:
  File _bmpFile;
  int  _bmpWidth, _bmpHeight;   // W+H in pixels
  uint8_t  _bmpDepth;              // Bit depth (currently must be 24)
  uint32_t _bmpImageoffset;        // Start of image data in file
  uint32_t _rowSize;               // Not always = bmpWidth; may have padding
  uint32_t _size;
  bool     _flip; 
  bool     _valid;
  
  PImage(File & bmpFile, int bmpWidth, int bmpHeight, uint8_t  bmpDepth, uint32_t bmpImageoffset, uint32_t rowSize, bool     flip) : 
  _bmpFile(bmpFile),
  _bmpWidth(bmpWidth),
  _bmpHeight(bmpHeight),
  _bmpDepth(bmpDepth),
  _bmpImageoffset(bmpImageoffset),
  _rowSize(rowSize),
  _flip(flip),
  _valid(true) // since Adafruit_GFX is friend, we could just let it write the variables and save some CPU cycles
  { }
  
  static uint16_t read16(File f);
  static uint32_t read32(File f);
  static RgbColor readCol(File f);
  
  // TODO close the file in ~PImage and PImage(const PImage&)

};

#endif // _PIMAGE_H


