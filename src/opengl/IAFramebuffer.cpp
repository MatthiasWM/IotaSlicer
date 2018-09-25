//
//  IAFramebuffer.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAFramebuffer.h"

#include "userinterface/IAGUIMain.h"
#include "toolpath/IAToolpath.h"
#include "potrace/IAPotrace.h"
#include "potrace/bitmap.h"
#include "printer/IAPrinter.h"

#include <stdio.h>
#include <math.h>
#include <libjpeg/jpeglib.h>
#include <libpng/png.h>


const char *glIAErrorString(int err)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return (const char *)gluErrorString(err);
#pragma clang diagnostic pop
}

#if 0
// VMWareDetector.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include <conio.h>
void CheckVM(void);
int main()
{
    CheckVM();
    _getch();
    return 0;
}

void CheckVM(void)
{
    unsigned int    a, b;

    __try {
        __asm {

            // save register values on the stack
            push eax
            push ebx
            push ecx
            push edx

            // perform fingerprint
            mov eax, 'VMXh' // VMware magic value (0x564D5868)
            mov ecx, 0Ah // special version cmd (0x0a)
            mov dx, 'VX' // special VMware I/O port (0x5658)

            in eax, dx // special I/O cmd

            mov a, ebx // data
            mov b, ecx // data (eax gets also modified
            // but will not be evaluated)

            // restore register values from the stack
            pop edx
            pop ecx
            pop ebx
            pop eax
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    printf("\n[+] Debug : [ a=%x ; b=%d ]\n\n", a, b);
    if (a == 'VMXh') { // is the value equal to the VMware magic value?
        printf("Result  : VMware detected\nVersion : ");
        if (b == 1)
            printf("Express\n\n");
        else if (b == 2)
            printf("ESX\n\n");
        else if (b == 3)
            printf("GSX\n\n");
        else if (b == 4)
            printf("Workstation\n\n");
        else
            printf("unknown version\n\n");
    }
    else
        printf("Result  : Not Detected\n\n");
}

#include <intrin.h>

bool isGuestOSVM()
{
    unsigned int cpuInfo[4];
    __cpuid((int*)cpuInfo,1);
    return ((cpuInfo[2] >> 31) & 1) == 1;
}

#endif


#ifdef __LINUX__
#include <GL/glext.h>
#endif

#ifdef _WIN32
#include <GL/glext.h>
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
PFNGLBLENDEQUATIONEXTPROC glBlendEquationEXT;
PFNGLBLITFRAMEBUFFEREXTPROC glBlitFramebufferEXT;
#endif
/*
 glColorMask(bool r, bool g, bool b, bool a);
 glLogicOp(x);

 glDepthMask(bool z);
 glDepthFunc();
 glDepthTest();

 glBlendFunc();
 glBlendColor();
 glBlendEquation();

 glStencilMask(uint bits);
 glStencilOp(a, b, c);
 glStencilFunc(a, b, c);

 Voxels are represented as multiple layers of RGBA image maps. R, G, and B
 represent the color of the object in 8 bits per component.

 The alpha channel uses bits to represent the property of the voxel.
 - bit 7: inside of the model
 - bit 6: infill or shell
 - bit 5: lid/bottom or shell
 - bit 4: external support structure
 - bit 3: contact to build platform
 ...

 The RGBA buffer usually comes with a depth buffer. We can use D24S8, which
 generates a 24 bit depth buffer and an 8 bit stencil buffer. We can use
 the stencil buffer to mark a mesh ID.

 https://www.khronos.org/opengl/wiki/Framebuffer_Object#Feedback_Loops
 https://www.khronos.org/opengl/wiki/Framebuffer_Object_Extension_Examples#Color_texture.2C_Depth_texture
 http://www.songho.ca/opengl/gl_fbo.html
 https://www.opengl.org/archives/resources/features/KilgardTechniques/oglpitfall/
 */

int isExtensionSupported(const char *extension)
{
    const GLubyte *extensions = NULL;
    const GLubyte *start;
    GLubyte *where, *terminator;

    /* Extension names should not have spaces. */
    where = (GLubyte *) strchr(extension, ' ');
    if (where || *extension == '\0')
        return 0;
    extensions = glGetString(GL_EXTENSIONS);
    /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
    start = extensions;
    for (;;) {
        where = (GLubyte *) strstr((const char *) start, extension);
        if (!where)
            break;
        terminator = where + strlen(extension);
        if (where == start || *(where - 1) == ' ')
            if (*terminator == ' ' || *terminator == '\0')
                return 1;
        start = terminator;
    }
    return 0;
}


/**
 * Initialize OpenGL for every supported platform.
 *
 * \return false, if OpenGL on this machine does not support the required
 *      features.
 */
bool initializeOpenGL()
{
    // "GL_EXT_blend_minmax"
    // "GL_EXT_blend_subtract"

    static bool beenHere = false;
    if (beenHere) return true;
    beenHere = true;
#ifdef _WIN32
    //char *gl_version = (char*)glGetString(GL_VERSION);
    //if (gl_version) printf("OpenGL Version: \"%s\"\n", gl_version);
    //char *gl_extensions = (char*)glGetString(GL_EXTENSIONS);
    //if (gl_extensions) printf("OpenGL Extensions: \"%s\"\n", gl_extensions);
#define FINDGL(a, b) \
a##EXT=(b)wglGetProcAddress(#a); \
if (!a##EXT) a##EXT=(b)wglGetProcAddress(#a"EXT"); \
if (!a##EXT) a##EXT=(b)wglGetProcAddress(#a"ARB"); \
if (!a##EXT) { Iota.Error.set("Initializing OpenGL", IAError::OpenGLFeatureNotSupported_STR, #a); return false; }

    FINDGL(glGenFramebuffers, PFNGLGENFRAMEBUFFERSEXTPROC);
    FINDGL(glDeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSEXTPROC);
    FINDGL(glBindFramebuffer, PFNGLBINDFRAMEBUFFEREXTPROC);
    FINDGL(glFramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DEXTPROC);
    FINDGL(glGenRenderbuffers, PFNGLGENRENDERBUFFERSEXTPROC);
    FINDGL(glBindRenderbuffer, PFNGLBINDRENDERBUFFEREXTPROC);
    FINDGL(glRenderbufferStorage, PFNGLRENDERBUFFERSTORAGEEXTPROC);
    FINDGL(glFramebufferRenderbuffer, PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC);
    FINDGL(glCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC);
    FINDGL(glDeleteRenderbuffers, PFNGLDELETERENDERBUFFERSEXTPROC);
    FINDGL(glBlendEquation, PFNGLBLENDEQUATIONEXTPROC);
    FINDGL(glBlitFramebuffer, PFNGLBLITFRAMEBUFFEREXTPROC);
#endif
    return true;
}


/**
 * Create a framebuffer object.
 *
 * For now, we do not allow any parameters. We create a 256x256 big buffer
 * with a color buffer with RGBA8 and a depth buffer of 24 bits.
 *
 * Creating the buffers is deferred until they are actually needed.
 */
IAFramebuffer::IAFramebuffer(IAPrinter *printer, Buffers buffers)
:   pBuffers( buffers ),
    pPrinter( printer )
{
    // variables are initialized inline
}


/**
 * Create a framebuffer by copying another framebuffer including contents.
 */
IAFramebuffer::IAFramebuffer(IAFramebuffer *src)
:   pBuffers( src->pBuffers ),
    pPrinter( src->pPrinter )
{
    if (src->hasFBO()) {
        bindForRendering();
        if (pBuffers==BITMAP) {
            // FIXME: assuming that all framebuffers have the same resolution
            pBitmap = bm_dup(src->pBitmap);
        } else {
            glBindFramebufferEXT(GL_READ_FRAMEBUFFER, src->pFramebuffer);
            IA_HANDLE_GL_ERRORS();
            glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, pFramebuffer);
            IA_HANDLE_GL_ERRORS();
            glBlitFramebufferEXT(0, 0, pWidth, pHeight,
                                 0, 0, pWidth, pHeight,
                                 GL_COLOR_BUFFER_BIT, GL_NEAREST);
            IA_HANDLE_GL_ERRORS();
        }
        unbindFromRendering();
    }
}


/**
 * Compose the src buffer onto this buffer in RGBA, assuming only 0 and 1
 * values for components.
 *
 * \param src a source frame buffer
 */
void IAFramebuffer::logicAndNot(IAFramebuffer *src)
{
    if (src && src->hasFBO()) {
        bindForRendering();
        if (pBuffers==BITMAP) {
            int dy = pBitmap->dy;
            int y;
            int i;
            potrace_word *pSrc, *pDst;
            if (dy < 0) {
                dy = -dy;
            }
            for (y=0; y < pBitmap->h; y++) {
                pSrc = bm_scanline(src->pBitmap, y);
                pDst = bm_scanline(pBitmap, y);
                for (i=0; i < dy; i++) {
                    pDst[i] = pDst[i] & ~pSrc[i];
                }
            }
        } else {
            glBindFramebufferEXT(GL_READ_FRAMEBUFFER, src->pFramebuffer);
            IA_HANDLE_GL_ERRORS();
            glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, pFramebuffer);
            IA_HANDLE_GL_ERRORS();

            // FIXME: we can push and pop these
            glDisable(GL_DEPTH_TEST);
            // create a point if the destination point is 1 and the src is 0
#if 0
            // this would be the obvious solution, but is not supported by all
            // OpenGL drivers
            glBlendFunc(GL_ONE, GL_ONE); // dst = 1*dst - 1*src
            glBlendEquationEXT(GL_FUNC_REVERSE_SUBTRACT);
#else
            // this is the not-so-obvious solution which should be supported
            // on all OpenGL drivers. We need to remember that R,G, and B should
            // only be 0.0 or 1.0, so multiplying the source and inverse
            // destination color will effectively be a logic AND NOT.
            glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR); // dst = 0*src + (1-src)*dst
            IA_HANDLE_GL_ERRORS();
            glBlendEquationEXT(GL_FUNC_ADD);
            IA_HANDLE_GL_ERRORS();
#endif
            glEnable(GL_BLEND);
            IA_HANDLE_GL_ERRORS();

            glRasterPos2d(0.0, 0.0);
            IA_HANDLE_GL_ERRORS();
            glCopyPixels(0, 0, pWidth, pHeight, GL_COLOR);
            IA_HANDLE_GL_ERRORS();

            glDisable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            IA_HANDLE_GL_ERRORS();
            glBlendEquationEXT(GL_FUNC_ADD);
            IA_HANDLE_GL_ERRORS();
        }
        unbindFromRendering();
    } else {
        // if src has no FBO, it is all 0, so AND NOT will not change this buffer
    }
}


/**
 * Compose the src buffer onto this buffer in RGBA, assuming only 0 and 1
 * values for components.
 *
 * \param src a source frame buffer
 */
void IAFramebuffer::logicAnd(IAFramebuffer *src)
{
    if (src && src->hasFBO()) {
        bindForRendering();
        bindForRendering();
        if (pBuffers==BITMAP) {
            int dy = pBitmap->dy;
            int y;
            int i;
            potrace_word *pSrc, *pDst;
            if (dy < 0) {
                dy = -dy;
            }
            for (y=0; y < pBitmap->h; y++) {
                pSrc = bm_scanline(src->pBitmap, y);
                pDst = bm_scanline(pBitmap, y);
                for (i=0; i < dy; i++) {
                    pDst[i] = pDst[i] & pSrc[i];
                }
            }
        } else {
            glBindFramebufferEXT(GL_READ_FRAMEBUFFER, src->pFramebuffer);
            IA_HANDLE_GL_ERRORS();
            glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, pFramebuffer);
            IA_HANDLE_GL_ERRORS();

            // FIXME: we can push and pop these
            glDisable(GL_DEPTH_TEST);
#if 0
            // this would be the obvious solution, but is not supported by many
            // OpenGL drivers
            glBlendFunc(GL_ONE, GL_ONE); // dst = min(src, dst)
            glBlendEquationEXT(GL_MIN);
#else
            // this is the not-so-obvious solution which should be supported
            // on all OpenGL drivers. We need to remember that R,G, and B should
            // only be 0.0 or 1.0, so multiplying the source and destination color
            // and then clipping it to [0...1] will effectively be a logic AND.
            glBlendFunc(GL_DST_COLOR, GL_ZERO); // dst = src * dst + null * dst
            IA_HANDLE_GL_ERRORS();
            glBlendEquationEXT(GL_FUNC_ADD);
            IA_HANDLE_GL_ERRORS();
#endif
            glEnable(GL_BLEND);
            IA_HANDLE_GL_ERRORS();

            glRasterPos2d(0.0, 0.0);
            IA_HANDLE_GL_ERRORS();
            glCopyPixels(0, 0, pWidth, pHeight, GL_COLOR);
            IA_HANDLE_GL_ERRORS();

            glDisable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            IA_HANDLE_GL_ERRORS();
            glBlendEquationEXT(GL_FUNC_ADD);
            IA_HANDLE_GL_ERRORS();
        }
        unbindFromRendering();
    } else {
        // if src has no FBO, it is all 0, so AND will make the result all 0
        clear();
    }
}


/**
 * Delete the framebuffer, if we ever created one.
 */
IAFramebuffer::~IAFramebuffer()
{
    if (pBitmap) {
        bm_free(pBitmap);
        pBitmap = nullptr;
    }
    if (hasFBO()) {
        deleteFBO();
        pFramebufferCreated = false;
    }
}


/**
 * Clear the framebuffer object for next use.
 *
 * This call does not delete any resources.
 */
void IAFramebuffer::clear(int color)
{
    if (hasFBO()) {
        bindForRendering();
        if (pBuffers==RGBA) {
            glClearColor(0.0, 0.0, 0.0, 0.0);
            glClear(GL_COLOR_BUFFER_BIT);
        } else if (pBuffers==RGBAZ) {
            glClearColor(0.0, 0.0, 0.0, 0.0);
            glClearDepth(1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        } else if (pBuffers==BITMAP) {
            bm_clear(pBitmap, color);
        }
        unbindFromRendering();
    }
}


/**
 * Activate this buffer for drawing into it at global coordinates.
 */
void IAFramebuffer::bindForRendering()
{
    activateFBO();

    if (pBuffers==BITMAP) {
        // nothing to do
    } else {
        // set matrices, lighting, etc. for this FBO
        glViewport(0, 0, pWidth, pHeight);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        IAVector3d vol = pPrinter->pBuildVolume;
        /** \todo why is the range below [vol.z(), 0] negative? I tested the
         slice, and it does draw at the correct (positive) Z. Maybe related: we set
         the z depth to 1.0 when clearing the buffers. Is the depth test flipped?
         Lastly, make sure that 0 is always within the z range. */
        glOrtho(pPrinter->pBuildVolumeMin.x(), pPrinter->pBuildVolumeMax.x(),
                pPrinter->pBuildVolumeMin.y(), pPrinter->pBuildVolumeMax.y(),
                -vol.z()-1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // draw a aquare, just to see if this works at all
        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glDisable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
    }
}


/**
 * Reactivate the regular framebuffer and signal the scene viewer to rectreate
 * all settings.
 */
void IAFramebuffer::unbindFromRendering()
{
    if (pBuffers==BITMAP) {
        // nothing to do
    } else {
        // deactivate the FBO and set render target to FL_BACKBUFFER
        IA_HANDLE_GL_ERRORS();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        IA_HANDLE_GL_ERRORS();
        // make sure that our scene viewer completely reinitializes
        gSceneView->valid(0);
    }
}


/**
 * Convert the color buffer into RGB data that potrace will understand.
 *
 * \return pointer to data, must be free'd by caller!
 */
uint8_t *IAFramebuffer::getRawImageRGB()
{
    size_t size = pWidth*pHeight*3;
    uint8_t *data = (uint8_t*)malloc(size);
    if (pBuffers==BITMAP) {
        uint8_t *dst = data;
        for (int y=0; y<pHeight; y++) {
            for (int x=0; x<pWidth; x++) {
                uint8_t lum = BM_UGET(pBitmap, x, y) ? 255 : 0;
                *dst++ = lum;
                *dst++ = lum;
                *dst++ = lum;
            }
        }
    } else {
        IA_HANDLE_GL_ERRORS();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pFramebuffer);
        IA_HANDLE_GL_ERRORS();
        glReadPixels(0, 0, pWidth, pHeight, GL_RGB, GL_UNSIGNED_BYTE, data);
        IA_HANDLE_GL_ERRORS();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        IA_HANDLE_GL_ERRORS();
    }
    return data;
}


/**
 * Convert the color buffer into an RGBA bytes array in user memory.
 *
 * \return pointer to data, must be free'd by caller!
 */
uint8_t *IAFramebuffer::getRawImageRGBA()
{
    size_t size = pWidth*pHeight*4;
    uint8_t *data = (uint8_t*)malloc(size);
    if (pBuffers==BITMAP) {
        uint8_t *dst = data;
        for (int y=0; y<pHeight; y++) {
            for (int x=0; x<pWidth; x++) {
                uint8_t lum = BM_UGET(pBitmap, x, y) ? 255 : 0;
                *dst++ = lum;
                *dst++ = lum;
                *dst++ = lum;
            }
        }
    } else {
        IA_HANDLE_GL_ERRORS();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pFramebuffer);
        IA_HANDLE_GL_ERRORS();
        glReadPixels(0, 0, pWidth, pHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
        IA_HANDLE_GL_ERRORS();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        IA_HANDLE_GL_ERRORS();
    }
    return data;
}


/**
 * Trace around the image and write an outline to a toolpath.
 *
 * \param toolpath add outline segments to this toolpath
 * \param z give all segments in the toolpath a z position
 *
 * \return returns 0 on success
 */
int IAFramebuffer::traceOutline(IAToolpathList *toolpathList, double z)
{
    toolpathList->clear(z);
    potrace(this, toolpathList, z);
    return 0;
}


/**
 * Write the RGB components of the image buffer into a jpeg file.
 *
 * \param filename the file name of the file that will be created
 * \param imgdata a pointer to an RGB buffer, or nullptr if this call will
 *        get and handle the image data.
 *
 * \return 0 on success
 *
 * \todo no error checking yet
 */
int IAFramebuffer::saveAsJpeg(const char *filename, GLubyte *imgdata)
{
    bool freeImgData = false;
    if (imgdata==nullptr) {
        imgdata = getRawImageRGB();
        freeImgData = true;
    }

    FILE *ofp;
    struct jpeg_compress_struct cinfo;   /* JPEG compression struct */
    struct jpeg_error_mgr jerr;          /* JPEG error handler */
    JSAMPROW row_pointer[1];             /* output row buffer */
    int row_stride;                      /* physical row width in output buf */

    if ((ofp = fl_fopen(filename, "wb"))) {
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
    }
    if (freeImgData)
        free(imgdata);

    return 0; /* No fatal errors */
}


/**
 * Write framebuffer as PNG image file.
 *
 * \param filename the file name of the file that will be created
 * \param components 3 for RGB, 4 for RGBA
 * \param imgdata a pointer to an RGB(A) buffer, or nullptr if this call will
 *        get and handle the image data.
 *
 * \return 0 on success
 *
 * \todo no error checking yet
 * \todo can we accelerate PNG writing by changing filters and compression?
 *       Size is not really an issue here.
 * \todo if we want to send data directly to a printhead, we may want to
 *       generate dithered files for color blending.
 */
int IAFramebuffer::saveAsPng(const char *filename, int components, GLubyte *imgdata)
{
    bool freeImgData = false;
    if (imgdata==nullptr) {
        if (components==3)
            imgdata = getRawImageRGB();
        else if (components==4)
            imgdata = getRawImageRGBA();
        freeImgData = true;
    }

    FILE *fp = fopen(filename, "wb");
    if(!fp) abort();

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    int fmt = 0;
    switch (components) {
        case 1: fmt = PNG_COLOR_TYPE_GRAY; break;
        case 2: fmt = PNG_COLOR_TYPE_GA; break;
        case 3: fmt = PNG_COLOR_TYPE_RGB; break;
        case 4: fmt = PNG_COLOR_TYPE_RGBA; break;
    }

    // Output is 8bit depth
    png_set_IHDR(png,
                 info,
                 pWidth, pHeight,
                 8,
                 fmt,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT
                 );
    png_write_info(png, info);

    for(int y = 0; y < pHeight; y++) {
        png_write_row( png, imgdata + y*pWidth*components );
    }
    fclose(fp);

    if (freeImgData)
        free(imgdata);

    return 0;
}


/**
 * Draw the RGBA buffer into the scene viewer at world coordinates.
 *
 * \param z draw the buffer at this z coordinate.
 */
void IAFramebuffer::draw(double z)
{
    if (!hasFBO()) return;

    if (pBuffers==BITMAP) {
        // FIXME: write this
    } else {
        // set as texture and render out
        IA_HANDLE_GL_ERRORS();
        glBindTexture(GL_TEXTURE_2D, pColorbuffer);
        IA_HANDLE_GL_ERRORS();
        glEnable(GL_TEXTURE_2D);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_POLYGON);
        glTexCoord2f(0.0, 0.0);
        glVertex3d(pPrinter->pBuildVolumeMin.x(), pPrinter->pBuildVolumeMin.y(), z);
        glTexCoord2f(0.0, 1.0);
        glVertex3d(pPrinter->pBuildVolumeMin.x(), pPrinter->pBuildVolumeMax.y(), z);
        glTexCoord2f(1.0, 1.0);
        glVertex3d(pPrinter->pBuildVolumeMax.x(), pPrinter->pBuildVolumeMax.y(), z);
        glTexCoord2f(1.0, 0.0);
        glVertex3d(pPrinter->pBuildVolumeMax.x(), pPrinter->pBuildVolumeMin.y(), z);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        IA_HANDLE_GL_ERRORS();
    }
}


/**
 * Return true if we have previously allocated the FBO.
 *
 * \return true if the framebuffer has been created.
 */
bool IAFramebuffer::hasFBO()
{
    return pFramebufferCreated;
}


/**
 * Activate the FBO for drawing; build an FBO if we didn't yet.
 */
void IAFramebuffer::activateFBO()
{
    if (!pFramebufferCreated) {
        createFBO();
    }
    if (pBuffers==BITMAP) {
        // nothing to do
    } else {
        /** \todo what if there was an error and FBO is still not created */
        IA_HANDLE_GL_ERRORS();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pFramebuffer);
        IA_HANDLE_GL_ERRORS();
    }
}


/**
 * Create a framebuffer object.
 *
 * \see https://www.khronos.org/opengl/wiki/Framebuffer_Object_Extension_Examples#Color_texture.2C_Depth_texture
 *
 * \todo return a potential error code and handle it upstream.
 */
void IAFramebuffer::createFBO()
{
    // Create this thing

    if (pBuffers==BITMAP) {
        pBitmap = bm_new(pWidth, pHeight);
    } else {
        //RGBA8 2D texture, 24 bit depth texture
        IA_HANDLE_GL_ERRORS();
        glGenTextures(1, &pColorbuffer);
        IA_HANDLE_GL_ERRORS();
        glBindTexture(GL_TEXTURE_2D, pColorbuffer);
        IA_HANDLE_GL_ERRORS();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //NULL means reserve texture memory, but texels are undefined
        IA_HANDLE_GL_ERRORS();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pWidth, pHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
        IA_HANDLE_GL_ERRORS();

        glGenFramebuffersEXT(1, &pFramebuffer);
        IA_HANDLE_GL_ERRORS();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pFramebuffer);
        IA_HANDLE_GL_ERRORS();
        //Attach 2D texture to this FBO
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, pColorbuffer, 0);
        IA_HANDLE_GL_ERRORS();

        if (pBuffers==RGBAZ) {
            IA_HANDLE_GL_ERRORS();
            glGenRenderbuffersEXT(1, &pDepthbuffer);
            IA_HANDLE_GL_ERRORS();
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, pDepthbuffer);
            IA_HANDLE_GL_ERRORS();
            glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, pWidth, pHeight);
            IA_HANDLE_GL_ERRORS();
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, pDepthbuffer);
            IA_HANDLE_GL_ERRORS();
        } else {
            pDepthbuffer = 0;
        }

        //Does the GPU support current FBO configuration?
        GLenum status;
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        switch(status)
        {
            case GL_FRAMEBUFFER_COMPLETE_EXT:
                //            printf("good\n");
                break;
            default:
                IA_HANDLE_GL_ERRORS();
                printf("not so good\n");
                return;
        }
    }
    pFramebufferCreated = true;
    clear();
//    //    FIXME: this is a test pattern for the bitmap mode
//    if (pBitmap) {
//        for (int y = 800; y<1600; y++)
//            bm_hline(pBitmap, 800, 1500, y);
//    }
}


/**
 * Delete the framebuffer object.
 */
void IAFramebuffer::deleteFBO()
{
    if (pBuffers==BITMAP) {
        bm_free(pBitmap);
    } else {
        //Bind 0, which means render to back buffer, as a result, fb is unbound
        IA_HANDLE_GL_ERRORS();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        IA_HANDLE_GL_ERRORS();
        //Delete resources
        if (pColorbuffer)
            glDeleteTextures(1, &pColorbuffer);
        IA_HANDLE_GL_ERRORS();
        if (pDepthbuffer)
            glDeleteRenderbuffersEXT(1, &pDepthbuffer);
        IA_HANDLE_GL_ERRORS();
        if (pFramebuffer)
            glDeleteFramebuffersEXT(1, &pFramebuffer);
        IA_HANDLE_GL_ERRORS();
    }
    pFramebufferCreated = false;
}


/**
 * Trace the framebuffer and create a toolpath.
 *
 * \param z create a toolptah at this layer
 *
 * \return nullptr, if tracing generates an empty toolpath
 * \return a new smart_pointer to a toolpath
 */
IAToolpathListSP IAFramebuffer::toolpathFromLasso(double z)
{
    // use a shared pointer, so we don't have to worry about deallocating
    auto tp0 = std::make_shared<IAToolpathList>(z);

    // create an outline for this slice image
    traceOutline(tp0.get(), z);

    if (tp0->isEmpty())
        return nullptr;
    else
        return tp0;
}


/**
 * Trace the framebuffer, create a toolpath, and reduce the framebuffer by
 * the toolpath pattern.
 *
 * \param z create a toolpath at this layer
 * \param r the pattern will be reduced by the amount in r
 *
 * \return nullptr, if tracing generates an empty toolpath
 * \return a new smart_pointer to a toolpath
 */
IAToolpathListSP IAFramebuffer::toolpathFromLassoAndContract(double z, double r)
{
    // use a shared pointer, so we don;t have to worry about deallocating
    auto tp0 = toolpathFromLasso(z);
    subtract(tp0, r);
    return tp0;
}


/**
 * Subtract a toolpath from this pattern.
 *
 * \param tp subtract this toolpath form the pattern
 * \param r the pattern will be reduced by the amount in r
 */
void IAFramebuffer::subtract(IAToolpathListSP tp, double r)
{
    if (tp) {
        // draw the outline to contract the image
        bindForRendering();
        if (pBuffers==BITMAP) {
            tp->drawFlatToBitmap(this, r*2.0);
        } else {
            glDisable(GL_DEPTH_TEST);
            glColor3f(0.0, 0.0, 0.0);
            tp->drawFlat(r*2.0);
        }
        unbindFromRendering();
    }
}


/**
 * Overlay the image with stripes across or lengthwise.
 *
 * \param i determines if the stripes are across or lengthwise.
 * \param infillWdt distance between lines. If this is the same es the extrusion
 *      width, the pattern will fill 100%.
 */
void IAFramebuffer::overlayLidPattern(int i, double infillWdt)
{
    bindForRendering();
    // draw spaces so the infill gets spread out nicely
    /** \todo What if the printer has negative coordintes as well? */
    double wdt = pPrinter->pBuildVolumeMax.x();
    double hgt = pPrinter->pBuildVolumeMax.y();
    if (pBuffers==BITMAP) {
        if (i&1) {
            int dx = infillWdt/pPrinter->pBuildVolume.x()*pWidth;
            if (dx<1) dx = 1;
            for (int x=0; x<pWidth; x+=2*dx) {
                for (int y=0; y<pHeight; y++) {
                    bm_hline(pBitmap, x, x+dx, y, 0);
                }
            }
        } else {
            int dy = infillWdt/pPrinter->pBuildVolume.y()*pHeight;
            if (dy<1) dy = 1;
            for (int y1=0; y1<pHeight; y1+=2*dy) {
                for (int y2=0; y2<dy; y2++) {
                    bm_hline(pBitmap, 0, pWidth, y1+y2, 0);
                }
            }
        }
    } else {
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        // generate a lid
        glPushMatrix();
        if (i&1) {
            glRotated(90, 0, 0, 1);
        }
        for (double j=-wdt; j<wdt; j+=infillWdt*2) {
            glBegin(GL_POLYGON);
            glVertex2d(j+infillWdt, -hgt);
            glVertex2d(j, -hgt);
            glVertex2d(j, hgt);
            glVertex2d(j+infillWdt, hgt);
            glEnd();
        }
        glPopMatrix();
    }
    unbindFromRendering();
}


/**
 * Overlay the image with diagonal alternating stripes.
 *
 * \param i determines if the stripes are ascending or descending
 * \param infillWdt distance between lines. If this is the same es the extrusion
 *      width, the pattern will fill 100%.
 */
void IAFramebuffer::overlayInfillPattern(int i, double infillWdt)
{
    bindForRendering();
    if (pBuffers==BITMAP) {
        infillWdt *= sqrt(2.0); // compensate that we draw at a 45 deg angle
        int dx = infillWdt/pPrinter->pBuildVolume.x()*pWidth;
        if (dx<1) dx = 1;
        if (i&1) {
            for (int x=-pWidth; x<pWidth; x+=2*dx) {
                for (int y=0; y<pHeight; y++) {
                    bm_hline(pBitmap, x+y, x+y+dx, y, 0);
                }
            }
        } else {
            for (int x=0; x<2*pWidth; x+=2*dx) {
                for (int y=0; y<pHeight; y++) {
                    bm_hline(pBitmap, x-y, x-y+dx, y, 0);
                }
            }
        }
    } else {
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        // draw spaces so the infill gets spread out nicely
        /** \todo What if the printer has negative coordinates as well? */
        double wdt = pPrinter->pBuildVolumeMax.x();
        double hgt = pPrinter->pBuildVolumeMax.y();
        glPushMatrix();
        if (i&1)
            glRotated(45, 0, 0, 1);
        else
            glRotated(-45, 0, 0, 1);
        for (double j=-2*wdt; j<2*wdt; j+=infillWdt*2) {
            glBegin(GL_POLYGON);
            /** \todo Draw this large enough so that it renders the entire scene, even if rotated. */
            glVertex2d(j+infillWdt, -2*hgt);
            glVertex2d(j, -2*hgt);
            glVertex2d(j, 2*hgt);
            glVertex2d(j+infillWdt, 2*hgt);
            glEnd();
        }
        glPopMatrix();
    }
    unbindFromRendering();
}


void IAFramebuffer::drawLid(IAEdgeList &rim)
{
    beginComplexPolygon();
    for (auto &e: rim) {
        if (e) {
            addPoint(e->pVertex[0]->pGlobalPosition);
        } else {
            addGap();
        }
    }
    endComplexPolygon(1);
}


void IAFramebuffer::beginComplexPolygon()
{
    pnVertex = 0;
    pVertexGapStart = 0;
}


void IAFramebuffer::endComplexPolygon(int color)
{
    if (pnVertex < 2) return;

    addGap(); // adds the first coordinate of this loop and marks it as a gap
    int begin = 0, end = pnVertex;

    Vertex *v = pVertex+0;
    int xMin = v->pX, xMax = xMin, yMin = v->pY, yMax = yMin;
    for (int i = begin+1; i < end; i++) {
        v = pVertex+i;
        if (v->pX < xMin) xMin = v->pX;
        if (v->pX > xMax) xMax = v->pX;
        if (v->pY < yMin) yMin = v->pY;
        if (v->pY > yMax) yMax = v->pY;
    }
    xMax++; yMax++;

    int nodes, pixelY, i, j, swap;
	int *nodeX = (int*)::malloc((end - begin) * sizeof(int));

    //  Loop through the rows of the image.
    for (pixelY = yMin; pixelY < yMax; pixelY++) {
        //  Build a list of nodes.
        nodes = 0;
        for (i = begin+1; i < end; i++) {
            j = i-1;
            if (pVertex[j].pIsGap)
                continue;
            if (   (pVertex[i].pY < pixelY && pVertex[j].pY >= pixelY)
                || (pVertex[j].pY < pixelY && pVertex[i].pY >= pixelY) )
            {
                float dy = pVertex[j].pY - pVertex[i].pY;
                if (fabsf(dy)>.0001) {
                    nodeX[nodes++] = (int)(pVertex[i].pX +
                                           (pixelY - pVertex[i].pY) / dy
                                           * (pVertex[j].pX - pVertex[i].pX));
                } else {
                    nodeX[nodes++] = pVertex[i].pX;
                }
            }
        }
        //Fl_Android_Application::log_e("%d nodes (must be even!)", nodes);

        //  Sort the nodes, via a simple “Bubble” sort.
        i = 0;
        while (i < nodes - 1) {
            if (nodeX[i] > nodeX[i + 1]) {
                swap = nodeX[i];
                nodeX[i] = nodeX[i + 1];
                nodeX[i + 1] = swap;
                if (i) i--;
            } else {
                i++;
            }
        }

        //  Fill the pixels between node pairs.
        for (i = 0; i < nodes; i += 2) {
            if (nodeX[i] >= xMax) break;
            if (nodeX[i + 1] > xMin) {
                if (nodeX[i] < xMin) nodeX[i] = xMin;
                if (nodeX[i + 1] > xMax) nodeX[i + 1] = xMax;
                bm_hline(pBitmap, nodeX[i], nodeX[i+1], pixelY, color);
            }
        }
    }
	::free((void*)nodeX);
}


void IAFramebuffer::addPointRaw(float x, float y, bool gap)
{
    if (pnVertex == pNVertex) {
        pNVertex += 16;
        pVertex = (Vertex*)::realloc(pVertex, pNVertex*sizeof(Vertex));
    }
    pVertex[pnVertex].set(x, y);
    pVertex[pnVertex].pIsGap = gap;
    pnVertex++;

}


void IAFramebuffer::addPoint(double x, double y)
{
    addPointRaw(x/pPrinter->pBuildVolume.x()*pWidth,
                y/pPrinter->pBuildVolume.y()*pHeight, false);
}


void IAFramebuffer::addPoint(IAVector3d &v)
{
    addPointRaw(v.x()/pPrinter->pBuildVolume.x()*pWidth,
             v.y()/pPrinter->pBuildVolume.y()*pHeight, false);
}


void IAFramebuffer::addGap()
{
    // drop gaps at the start or gap after gap
    if (pnVertex==0 || pnVertex==pVertexGapStart)
        return;

    // create a loop
    Vertex &v = pVertex[pVertexGapStart];
    addPointRaw(v.pX, v.pY, true);
    pVertexGapStart = pnVertex;
}



