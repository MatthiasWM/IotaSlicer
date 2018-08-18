//
//  IAFramebuffer.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAFramebuffer.h"


#include <FL/gl.h>
#include <FL/glu.h>


IAFramebuffer::IAFramebuffer()
{
}


IAFramebuffer::~IAFramebuffer()
{
}


void IAFramebuffer::drawBegin()
{
    activateFBO();
}


void IAFramebuffer::drawEnd()
{
}


unsigned char *IAFramebuffer::makeIntoBitmap()
{
    return nullptr;
}


bool IAFramebuffer::hasFBO()
{
    return false;
}


void IAFramebuffer::activateFBO()
{
}


void IAFramebuffer::createFBO()
{
}


void IAFramebuffer::deleteFBO()
{
}




#if 0

void IASlice::save(double z, const char *filename)
{
#ifdef __APPLE__ // VisualStudio 2017 misses a lot of the OpenGL names that I used here. Must investigate.
//    const int w = 800, h = 600;

    // https://www.khronos.org/opengl/wiki/Framebuffer_Object_Extension_Examples#Quick_example.2C_render_to_texture_.282D.29

    GLuint color_tex, fb, depth_rb;
    //RGBA8 2D texture, 24 bit depth texture, 256x256
    glGenTextures(1, &color_tex);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //NULL means reserve texture memory, but texels are undefined
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    //-------------------------
    glGenFramebuffersEXT(1, &fb);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    //Attach 2D texture to this FBO
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, color_tex, 0);
    //-------------------------
    glGenRenderbuffersEXT(1, &depth_rb);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_rb);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, 256, 256);
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
    //-------------------------
    //and now you can render to GL_TEXTURE_2D
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //-------------------------
    glViewport(0, 0, 256, 256);
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
    //RenderATriangle, {0.0, 0.0}, {256.0, 0.0}, {256.0, 256.0}
    //Read http://www.opengl.org/wiki/VBO_-_just_examples
//    glColor3f(1.0, 0.5, 0.5);
//    glBegin(GL_POLYGON);
//    glVertex3f(-10.0, -10.0, 0.0);
//    glVertex3f(-10.0,  10.0, 0.0);
//    glVertex3f( 10.0,  10.0, 0.0);
//    glVertex3f( 10.0, -10.0, 0.0);
//    glEnd();
    this->drawFlange();
    // render...

    //    glGetTexImage(<#GLenum target#>, <#GLint level#>, <#GLenum format#>, <#GLenum type#>, <#GLvoid *pixels#>);
    //    glReadPixels(<#GLint x#>, <#GLint y#>, <#GLsizei width#>, <#GLsizei height#>, <#GLenum format#>, <#GLenum type#>, <#GLvoid *pixels#>)
    //-------------------------
    GLubyte pixels[256*256*4];
    glReadPixels(0, 0, 256, 256, GL_BGR, GL_UNSIGNED_BYTE, pixels);
    //pixels 0, 1, 2 should be white
    //pixel 4 should be black

//    writejpeg((char*)filename, 256, 256, (unsigned char *)pixels);

    potrace_main((unsigned char *)pixels);


    //----------------
    //Bind 0, which means render to back buffer
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    //Delete resources
    glDeleteTextures(1, &color_tex);
    glDeleteRenderbuffersEXT(1, &depth_rb);
    //Bind 0, which means render to back buffer, as a result, fb is unbound
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glDeleteFramebuffersEXT(1, &fb);

    gSceneView->valid(0);


//    FILE *f = fopen(filename, "wb");
//    fclose(f);
#endif
}

#endif


