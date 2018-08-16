//
//  IAFmtTextJpeg.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#include "IAFmtTexJpeg.h"

#include "Iota.h"

#include <FL/Fl_JPEG_Image.H>


/**
 Load a JPEG image from a file as the current texture.
 */
void loadTexture(const char *filename)
{
    Fl_JPEG_Image *img = new Fl_JPEG_Image(filename);
    if (img->d()) {
        Iota.texture = img;
    } else {
        delete img;
    }
}


/**
 Load a JPEG image from memory as the current texture.
 */
void loadTexture(const char *name, unsigned char *imageDate)
{
    Fl_JPEG_Image *img = new Fl_JPEG_Image(name, imageDate);
    if (img->d()) {
        Iota.texture = img;
    } else {
        delete img;
    }
}


