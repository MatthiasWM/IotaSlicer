//
//  IAFramebuffer.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAFramebuffer.h"

#include "userinterface/IAGUIMain.h"
#include "toolpath/IAToolpath.h"
#include "potrace/IAPotrace.h"

#include <stdio.h>
#include <libjpeg/jpeglib.h>
#include <libpng/png.h>

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


/**
 * Initialize OpenGL for every supported platform.
 *
 * \return false, if OpenGL on this machine does not support the required
 *      features.
 */
bool initializeOpenGL()
{
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
	if (!a##EXT) { Iota.setError("Initializing OpenGL", Error::OpenGLFeatureNotSupported_STR, #a); return false; }

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
IAFramebuffer::IAFramebuffer(Buffers buffers)
:   pBuffers(buffers)
{
    // variables are initialized inline
}


/**
 * Create a framebuffer by copying another framebuffer including contents.
 */
IAFramebuffer::IAFramebuffer(IAFramebuffer *src)
:   pBuffers(src->pBuffers)
{
    if (src->hasFBO()) {
        bindForRendering();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src->pFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pFramebuffer);
        glBlitFramebuffer(0, 0, pWidth, pHeight,
                          0, 0, pWidth, pHeight,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
//        GLenum err = glGetError();
//        printf("GL Error %d\n", err);
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
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src->pFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pFramebuffer);

        // FIXME: we can push and pop these
        glDisable(GL_DEPTH_TEST);
        // create a point if the destination point is 1 and the src is 0
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
        glEnable(GL_BLEND);
//        GLenum err = glGetError(); printf("1 GL Error %d\n", err);

        glRasterPos2d(0.0, 0.0);
        glCopyPixels(0, 0, pWidth, pHeight, GL_COLOR);

        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

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
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src->pFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pFramebuffer);

        // FIXME: we can push and pop these
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_MIN); // FIXME: not all drivers support this
        glEnable(GL_BLEND);
//        GLenum err = glGetError(); printf("2 GL Error %d\n", err);

        glRasterPos2d(0.0, 0.0);
        glCopyPixels(0, 0, pWidth, pHeight, GL_COLOR);

        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

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
void IAFramebuffer::clear()
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

    // set matrices, lighting, etc. for this FBO
    glViewport(0, 0, pWidth, pHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    IAPrinter *p = Iota.pCurrentPrinter;
    IAVector3d vol = p->pBuildVolume;
    /** \todo why is the range below [vol.z(), 0] negative? I tested the
              slice, and it does draw at the correct (positive) Z. */
    glOrtho(0, vol.x(), 0, vol.y(), -vol.z()-1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // draw a aquare, just to see if this works at all
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
}


/**
 * Reactivate the regular framebuffer and signal the scene viewer to rectreate
 * all settings.
 */
void IAFramebuffer::unbindFromRendering()
{
    // deactivate the FBO and set render target to FL_BACKBUFFER
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    // make sure that our scene viewer completely reinitializes
    gSceneView->valid(0);
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
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pFramebuffer);
    glReadPixels(0, 0, pWidth, pHeight, GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
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
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pFramebuffer);
    glReadPixels(0, 0, pWidth, pHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
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
int IAFramebuffer::traceOutline(IAToolpath *toolpath, double z)
{
    toolpath->clear(z);
    potrace(this, toolpath, z);
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
 *
 * \todo use the current printer coordinates, not fixed numbers!
 */
void IAFramebuffer::draw(double z)
{
    if (!hasFBO()) return;

    // set as texture and render out
    glBindTexture(GL_TEXTURE_2D, pColorbuffer);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_POLYGON);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.0, 0.0, z);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(0.0, 214.0, z);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(214.0, 214.0, z);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(214.0, 0.0, z);
    glEnd();
    glDisable(GL_TEXTURE_2D);
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
    /** \todo what if there was an error and FBO is still not created */
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pFramebuffer);
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

    //RGBA8 2D texture, 24 bit depth texture
    glGenTextures(1, &pColorbuffer);
    glBindTexture(GL_TEXTURE_2D, pColorbuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //NULL means reserve texture memory, but texels are undefined
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pWidth, pHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);

    glGenFramebuffersEXT(1, &pFramebuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pFramebuffer);
    //Attach 2D texture to this FBO
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, pColorbuffer, 0);

    if (pBuffers==RGBAZ) {
        glGenRenderbuffersEXT(1, &pDepthbuffer);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, pDepthbuffer);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, pWidth, pHeight);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, pDepthbuffer);
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
            printf("not so good\n");
            return;
    }
    pFramebufferCreated = true;
    clear();
}


/**
 * Delete the framebuffer object.
 */
void IAFramebuffer::deleteFBO()
{
    //Bind 0, which means render to back buffer, as a result, fb is unbound
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    //Delete resources
    glDeleteTextures(1, &pColorbuffer);
    glDeleteRenderbuffersEXT(1, &pDepthbuffer);
    glDeleteFramebuffersEXT(1, &pFramebuffer);
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
IAToolpathSP IAFramebuffer::toolpathFromLasso(double z)
{
    // use a shared pointer, so we don't have to worry about deallocating
    auto tp0 = std::make_shared<IAToolpath>(z);

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
IAToolpathSP IAFramebuffer::toolpathFromLassoAndContract(double z, double r)
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
void IAFramebuffer::subtract(IAToolpathSP tp, double r)
{
    if (tp) {
        // draw the outline to contract the image
        bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp->drawFlat(r*2.0);
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
    glDisable(GL_DEPTH_TEST);
    glColor3f(0.0, 0.0, 0.0);
    // draw spaces so the infill gets spread out nicely
    /** \todo We should not have to access global variables here. */
    double wdt = Iota.pCurrentPrinter->pBuildVolumeMax.x();
    double hgt = Iota.pCurrentPrinter->pBuildVolumeMax.y();
    // generate a lid
    glPushMatrix();
    if (i&1) {
        glRotated(90, 0, 0, 1);
    }
    for (double j=-wdt; j<wdt; j+=infillWdt*2) {
        glBegin(GL_POLYGON);
        glVertex2f(j+infillWdt, -hgt);
        glVertex2f(j, -hgt);
        glVertex2f(j, hgt);
        glVertex2f(j+infillWdt, hgt);
        glEnd();
    }
    glPopMatrix();
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
    glDisable(GL_DEPTH_TEST);
    glColor3f(0.0, 0.0, 0.0);
    // draw spaces so the infill gets spread out nicely
    /** \todo We should not have to access global variables here. */
    double wdt = Iota.pCurrentPrinter->pBuildVolumeMax.x();
    double hgt = Iota.pCurrentPrinter->pBuildVolumeMax.y();
    glPushMatrix();
    if (i&1)
        glRotated(45, 0, 0, 1);
    else
        glRotated(-45, 0, 0, 1);
    for (double j=-2*wdt; j<2*wdt; j+=infillWdt*2) {
        glBegin(GL_POLYGON);
        /** \todo Draw this large enough so that it renders the entire scene, even if rotated. */
        glVertex2f(j+infillWdt, -2*hgt);
        glVertex2f(j, -2*hgt);
        glVertex2f(j, 2*hgt);
        glVertex2f(j+infillWdt, 2*hgt);
        glEnd();
    }
    glPopMatrix();
    unbindFromRendering();
}





