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

//#ifdef __LINUX__
#include <GL/glext.h>
//#endif

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
IAFramebuffer::IAFramebuffer()
{
    // variables are initialized inline
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
 */
void IAFramebuffer::clear()
{
    if (hasFBO()) {
        bindForRendering();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
    IAPrinter &p = Iota.gPrinter;
    IAVector3d vol = p.pBuildVolume;
    // FIXME: why is the range below [vol.z(), 0] negative? I tested the slice, and it does draw at the correct (positive) Z.
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
 * Convert the color buffer into a bitmap that potrace will understand.
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
 * Trace around the image and write an outline to a toolpath.
 */
int IAFramebuffer::traceOutline(IAToolpath *toolpath, double z)
{
    toolpath->clear(z);
    potrace(this, toolpath, z);
    return 0;
}


/**
 * Write the RGB components of the image buffer into a jpeg file.
 */
int IAFramebuffer::saveAsJpeg(const char *filename)
{
    GLubyte *imgdata = getRawImageRGB();

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
	free(imgdata);

    return 0; /* No fatal errors */
}


/**
 * Draw the RGBA buffer into the scene viewer at world coordinates.
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
    // FIXME: what if there was an error and FBO is still not created
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pFramebuffer);
}


/**
 * Create a framebuffer object.
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
    //-------------------------
    glGenFramebuffersEXT(1, &pFramebuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pFramebuffer);
    //Attach 2D texture to this FBO
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, pColorbuffer, 0);
    //-------------------------
    glGenRenderbuffersEXT(1, &pDepthbuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, pDepthbuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, pWidth, pHeight);
    //-------------------------
    //Attach depth buffer to FBO
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, pDepthbuffer);
    //-------------------------
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

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pFramebuffer);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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


