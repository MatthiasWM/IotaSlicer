//
//  IAFmtTextJpeg.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#include "IAFmtTexJpeg.h"

#include "main.h"

#include <FL/Fl_JPEG_Image.H>


/**
 Load any image file as a texture.
 */
void loadTexture(const char *filename)
{
    Fl_JPEG_Image *img = new Fl_JPEG_Image(filename);
    if (img->d()) {
        texture = img;
    } else {
        delete img;
    }
}

void loadTexture(const char *name, unsigned char *imageDate)
{
    Fl_JPEG_Image *img = new Fl_JPEG_Image(name, imageDate);
    if (img->d()) {
        texture = img;
    } else {
        delete img;
    }
}


