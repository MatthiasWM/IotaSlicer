//
//  IAFramebuffer.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_FRAMEBUFFER_H
#define IA_FRAMEBUFFER_H


#include <FL/gl.h>
#include <FL/glu.h>


/**
 * Manage an OpenGL framebuffer object as a texture.
 *
 * Framebuffers are used to represent a single Z-layer of the overall
 * scene voxel.
 *
 * Framebuffers are filled by rendering slices, outlines, shadows, or such
 * into them. Colors can represent object ID's but can also contain true
 * color information of textured objects.
 *
 * Framebuffers can be filtered to grow or shrink objects. They can be mreged
 * to add object information, support structure information, and whatever
 * else is needed to create a vector representation.
 *
 * Framebuffer can be analysed and converted into a vector format using potrace.
 *
 * A typical GCode pipeline would render a slice through a triangle mesh.
 * A second framebuffer can be rendered defining support material.
 * In a textured model, another framebuffer can hold the shell color information
 *
 * By repeatedly vectorizing and shrinking the image in the framebuffer, GCode
 * for the outer shell is generated. The remaining graphics in the image can
 * be used to overlay a vectorized fill pattern.
 */
class IAFramebuffer
{
public:
    IAFramebuffer();
    ~IAFramebuffer();

    void drawBegin();
    void drawEnd();

    unsigned char *makeIntoBitmap();
    void draw();
    int saveAsJpeg(const char *filename);

    int pWidth = 256, pHeight = 256;  // TODO: for now, this is fixed

protected:
    bool hasFBO();
    void activateFBO();
    void createFBO();
    void deleteFBO();

    bool pFramebufferCreated = false;
    GLuint color_tex;
    GLuint fb;
    GLuint depth_rb;

};


#endif /* IA_FRAMEBUFFER_H */


