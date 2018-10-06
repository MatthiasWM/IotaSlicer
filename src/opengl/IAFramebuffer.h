//
//  IAFramebuffer.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_FRAMEBUFFER_H
#define IA_FRAMEBUFFER_H

#include "Iota.h"
#include "toolpath/IAToolpath.h"
#include "potrace/potracelib.h"

#include <FL/gl.h>
#include <FL/glu.h>

#include <memory>


// Abundant error checking: why did glDebugMessageCallback not exist since OpenGL 1.0? Sigh.
#define IA_HANDLE_GL_ERRORS() \
do { \
GLenum err; \
while ((err=glGetError())) \
printf("*** OpenGL ERROR %d: %s\n%s:%d\n", err, glIAErrorString(err), __FILE__, __LINE__); \
} while(0)

extern const char *glIAErrorString(int err);

class IAToolpath;
class IAPrinter;


/**
 * MSWindows needs a lot of persuasion to provide some of the OpenGL calls.
 */
extern bool initializeOpenGL();



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
    typedef unsigned long bm_word;

    typedef enum {
        NONE = 0,
        RGBA,
        RGBAZ,
        BITMAP
    } Buffers;

    IAFramebuffer(IAPrinter*, Buffers type);
    IAFramebuffer(IAFramebuffer*);
    ~IAFramebuffer();
    void clear(int color=0);

    void bindForRendering();
    void unbindFromRendering();

    void draw(double z);
    uint8_t *getRawImageRGB();
    uint8_t *getRawImageRGBA();
    int traceOutline(IAToolpathList *toolpathList, double z);
    int saveAsJpeg(const char *filename, GLubyte *imgdata=nullptr);
    int saveAsPng(const char *filename, int components, GLubyte *imgdata=nullptr);

    /** Width in pixels.
     \return the width of the buffer. */
    int width() { return pWidth; }

    /** Height in pixels.
     \return the height of the buffer. */
    int height() { return pHeight; }

    /** Buffer type */
    Buffers buffers() { return pBuffers; }

    void logicAndNot(IAFramebuffer*);
    void logicAnd(IAFramebuffer*);

    void subtract(IAToolpathListSP, double r);
    void add(IAToolpathListSP, double r);
    IAToolpathListSP toolpathFromLassoAndContract(double z, double r);
    IAToolpathListSP toolpathFromLassoAndExpand(double z, double r);
    IAToolpathListSP toolpathFromLasso(double z);

    void overlayLidPattern(int i, double w);
    void overlayInfillPattern(int i, double w);

    void drawLid(IAEdgeList &rim);

    void beginComplexPolygon();
    void endComplexPolygon(int color);
    void addPoint(IAVector3d&);
    void addPoint(double x, double y);
    void addGap();

    void beginClipAboveZ(double z);
    void beginClipBelowZ(double z);
    void endClip();

protected:
    bool hasFBO();
    void activateFBO();
    void createFBO();
    void deleteFBO();

    void addPointRaw(float x, float y, bool gap=false);

    class Vertex {
    public:
        void set(float x, float y, bool gap = false) { pX = x; pY = y; pIsGap = gap; }
        float pX, pY;
        bool pIsGap;
    };

    int pnVertex = 0, pNVertex = 0, pVertexGapStart = 0;
    Vertex *pVertex = nullptr;


    /** Width of the framebuffer in pixles */
    int pWidth = kFramebufferSize;

    /** Height of the framebuffer in pixles */
    int pHeight = kFramebufferSize; // see Iota.cpp

    /** Set this flag if the OpenGL framebuffer object is created */
    bool pFramebufferCreated = false;

    /** The OpenGL framebuffer object, containing an RGBA and a depth buffer */
    GLuint pFramebuffer = 0;

    /** OpenGL reference to the RGBA color part of the framebuffer */
    GLuint pColorbuffer = 0;

    /** OpenGL reference to the depth buffer part of the framebuffer */
    GLuint pDepthbuffer = 0;

    /** An enum that lists the buffers used in this framebuffer */
    Buffers pBuffers = NONE;

    /** Use this to retrieve the build volume when rendering. */
    IAPrinter *pPrinter = nullptr;

public:
    potrace_bitmap_t *pBitmap = nullptr;
};


#endif /* IA_FRAMEBUFFER_H */


