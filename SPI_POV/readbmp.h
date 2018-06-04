
// Author: Christian Vallentin <mail@vallentinsource.com>
// Website: http://vallentinsource.com
// Repository: https://github.com/MrVallentin/LoadBMP
//
// Date Created: January 03, 2014
// Last Modified: August 13, 2016
//
// Version: 1.1.0

// Copyright (c) 2014-2016 Christian Vallentin <mail@vallentinsource.com>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.

// Include loadbmp.h as following
// to create the implementation file.
//
// #define LOADBMP_IMPLEMENTATION
// #include "loadbmp.h"

#ifndef LOADBMP_H
#define LOADBMP_H

#ifdef __cplusplus
extern "C" {
#endif


// Errors
#define LOADBMP_NO_ERROR 0

#define LOADBMP_OUT_OF_MEMORY 1

#define LOADBMP_FILE_NOT_FOUND 2
#define LOADBMP_FILE_OPERATION 3

#define LOADBMP_INVALID_FILE_FORMAT 4

#define LOADBMP_INVALID_SIGNATURE 5
#define LOADBMP_INVALID_BITS_PER_PIXEL 6


// Components
#define LOADBMP_RGB  3
#define LOADBMP_RGBA 4


#ifdef LOADBMP_IMPLEMENTATION
#  define LOADBMP_API
#else
# define LOADBMP_API extern
#endif


// LoadBMP uses raw buffers and support RGB and RGBA formats.
// The order is RGBRGBRGB... or RGBARGBARGBA..., from top left
// to bottom right, without any padding.


LOADBMP_API unsigned int loadbmp_decode_file(
  const char *filename, unsigned char **imageData, unsigned int *width, unsigned int *height, unsigned int components);

#ifdef LOADBMP_IMPLEMENTATION

#include <stdlib.h> /* malloc(), free() */
#include <string.h> /* memset(), memcpy() */
//#include <stdio.h> /* fopen(), fwrite(), fread(), fclose() */
#include "SD.h" /* Arduino specific file functions */


LOADBMP_API unsigned int loadbmp_decode_file(
  const char *filename, unsigned char **imageData, unsigned int *width, unsigned int *height, unsigned int components)
{
//  FILE *f = fopen(filename, "rb");
  File f = SD.open(filename);

  if (!f)
    return LOADBMP_FILE_NOT_FOUND;

  unsigned char bmp_file_header[14];
  unsigned char bmp_info_header[40];
  unsigned char bmp_pad[3];

  unsigned int w, h;
  unsigned char *data = NULL;

  unsigned int x, y, i, padding;

  memset(bmp_file_header, 0, sizeof(bmp_file_header));
  memset(bmp_info_header, 0, sizeof(bmp_info_header));

//  if (fread(bmp_file_header, sizeof(bmp_file_header), 1, f) == 0)
  if (f.read(bmp_file_header, sizeof(bmp_file_header)) == 0)
  {
    f.close();  // fclose(f);
    return LOADBMP_INVALID_FILE_FORMAT;
  }

  if (f.read(bmp_info_header, sizeof(bmp_info_header)) == 0)
  {
    f.close();  // fclose(f);
    return LOADBMP_INVALID_FILE_FORMAT;
  }

  if ((bmp_file_header[0] != 'B') || (bmp_file_header[1] != 'M'))
  {
    f.close();  // fclose(f);
    return LOADBMP_INVALID_SIGNATURE;
  }

  if ((bmp_info_header[14] != 24) && (bmp_info_header[14] != 32))
  {
    f.close();  // fclose(f);
    return LOADBMP_INVALID_BITS_PER_PIXEL;
  }

  w = (bmp_info_header[4] + (bmp_info_header[5] << 8) + (bmp_info_header[6] << 16) + (bmp_info_header[7] << 24));
  h = (bmp_info_header[8] + (bmp_info_header[9] << 8) + (bmp_info_header[10] << 16) + (bmp_info_header[11] << 24));
  
  if ((w > 0) && (h > 0))
  {
    data = (unsigned char*)malloc(w * h * components);

    if (!data)
    {
      f.close();  // fclose(f);
      return LOADBMP_OUT_OF_MEMORY;
    }

    for (y = (h - 1); y != -1; y--)
    {
      for (x = 0; x < w; x++)
      {
        //i = (x + y * w) * components;
        i = (x * h + y) * components;

        if (f.read(data + i, 3) == 0)
        {
          free(data);

          f.close();  // fclose(f);
          return LOADBMP_INVALID_FILE_FORMAT;
        }

        data[i] ^= data[i + 2] ^= data[i] ^= data[i + 2]; // BGR -> RGB

        if (components == LOADBMP_RGBA)
          data[i + 3] = 255;
      }

      padding = ((4 - (w * 3) % 4) % 4);

      if (f.read(bmp_pad, padding) != padding)
      {
        free(data);
        
        f.close();  // fclose(f);
        return LOADBMP_INVALID_FILE_FORMAT;
      }
    }
  }
  (*width) = w;
  (*height) = h;
  (*imageData) = data;

  f.close();

  return LOADBMP_NO_ERROR;
}


#endif

#ifdef __cplusplus
}
#endif

#endif
