//
//  IAFramebuffer.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAFramebuffer.h"

#include <stdio.h>
#include <libjpeg/jpeglib.h>

extern "C" int potrace_main(unsigned char *pixels256x256);


IAFramebuffer::IAFramebuffer()
{
    // variables are initialized inline
}


IAFramebuffer::~IAFramebuffer()
{
    if (hasFBO()) {
        deleteFBO();
    }
}


void IAFramebuffer::drawBegin()
{
    activateFBO();
    // TODO: set matrices, lighting, etc. for this FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //-------------------------
    glViewport(0, 0, pWidth, pHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-100, 100, -100, 100, -200.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //-------------------------
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    //-------------------------
    //**************************
    //Read http://www.opengl.org/wiki/VBO_-_just_examples
    glColor3f(1.0, 0.5, 0.5);
    glBegin(GL_POLYGON);
    glVertex3f(-10.0, -10.0, 0.0);
    glVertex3f(-10.0,  10.0, 0.0);
    glVertex3f( 10.0,  10.0, 0.0);
    glVertex3f( 10.0, -10.0, 0.0);
    glEnd();
//    Iota.gSlice.drawFlange();
    // render...
}


void IAFramebuffer::drawEnd()
{
    // TODO: deactivate thie FBO and set render target to FL_BACKBUFFER
}


unsigned char *IAFramebuffer::makeIntoBitmap()
{
    // read the FBO content into RAM and make a bitmap for potrace
    GLubyte pixels[pWidth*pHeight*4];
    glReadPixels(0, 0, pWidth, pHeight, GL_BGR, GL_UNSIGNED_BYTE, pixels);
//    potrace_main((unsigned char *)pixels);

    return nullptr;
}


int IAFramebuffer::saveAsJpeg(const char *filename)
{
    GLubyte imgdata[pWidth*pHeight*3];
    glReadPixels(0, 0, pWidth, pHeight, GL_BGR, GL_UNSIGNED_BYTE, imgdata);

    FILE *ofp;
    struct jpeg_compress_struct cinfo;   /* JPEG compression struct */
    struct jpeg_error_mgr jerr;          /* JPEG error handler */
    JSAMPROW row_pointer[1];             /* output row buffer */
    int row_stride;                      /* physical row width in output buf */

    if ((ofp = fopen(filename, "wb")) == NULL) {
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, ofp);

    cinfo.image_width = pWidth;
    cinfo.image_height = pHeight;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 95, FALSE);

    jpeg_start_compress(&cinfo, TRUE);

    /* Calculate the size of a row in the image */
    row_stride = cinfo.image_width * cinfo.input_components;

    /* compress the JPEG, one scanline at a time into the buffer */
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &(imgdata[(pHeight - cinfo.next_scanline - 1)*row_stride]);
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fclose(ofp);

    return 0; /* No fatal errors */
}


void IAFramebuffer::draw()
{
    // TODO: set as texture and render out
}

bool IAFramebuffer::hasFBO()
{
    return pFramebufferCreated;
}


void IAFramebuffer::activateFBO()
{
    if (!pFramebufferCreated) {
        createFBO();
    }
    // FIXME: what if there was an error and FBO is still not created
    // TODO: activate
}


void IAFramebuffer::createFBO()
{
    // Create this thing

    //RGBA8 2D texture, 24 bit depth texture
    glGenTextures(1, &color_tex);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //NULL means reserve texture memory, but texels are undefined
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pWidth, pHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
    //-------------------------
    glGenFramebuffersEXT(1, &fb);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    //Attach 2D texture to this FBO
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, color_tex, 0);
    //-------------------------
    glGenRenderbuffersEXT(1, &depth_rb);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_rb);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, pWidth, pHeight);
    //-------------------------
    //Attach depth buffer to FBO
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_rb);
    //-------------------------
    //Does the GPU support current FBO configuration?
    GLenum status;
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status)
    {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            printf("good\n");
            break;
        default:
            printf("not so good\n");
            return;
    }
    pFramebufferCreated = true;
}


void IAFramebuffer::deleteFBO()
{
    //----------------
    //Bind 0, which means render to back buffer
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    //Delete resources
    glDeleteTextures(1, &color_tex);
    glDeleteRenderbuffersEXT(1, &depth_rb);
    //Bind 0, which means render to back buffer, as a result, fb is unbound
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glDeleteFramebuffersEXT(1, &fb);
    // TODO: get rid of this thing
    pFramebufferCreated = true;
}


