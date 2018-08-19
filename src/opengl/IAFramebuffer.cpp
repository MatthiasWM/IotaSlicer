//
//  IAFramebuffer.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAFramebuffer.h"

#include "../userinterface/IAGUIMain.h"
#include "../toolpath/IAToolpath.h"

#include <stdio.h>
#include <libjpeg/jpeglib.h>


extern "C" int potrace_main(const char *filename, unsigned char *pixels256x256);


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
 * Delete the framebuffer, if we ever create one.
 */
IAFramebuffer::~IAFramebuffer()
{
    if (hasFBO()) {
        deleteFBO();
    }
}


/**
 * Activate this buffer for drawing into it at global coordinates.
 */
void IAFramebuffer::drawBegin()
{
    activateFBO();

    // set matrices, lighting, etc. for this FBO
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

//    glColor3f(1.0, 0.5, 0.5);
//    glBegin(GL_POLYGON);
//    glVertex3f(-10.0, -10.0, 0.0);
//    glVertex3f(-10.0,  10.0, 0.0);
//    glVertex3f( 10.0,  10.0, 0.0);
//    glVertex3f( 10.0, -10.0, 0.0);
//    glEnd();
}


/**
 * Reactivate the regular framebuffer and signal the scene viewer to rectreate
 * all settings.
 */
void IAFramebuffer::drawEnd()
{
    // deactivate the FBO and set render target to FL_BACKBUFFER
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    // make sure that our scene viewer completely reinitializes
    gSceneView->valid(0);
}


/**
 * Convert the color buffer into a bitmap that potrace will understand.
 *
 * \return pointer to data as a shared pointer (use 'auto' as a type an
 *         worry no longer.
 */
std::shared_ptr<unsigned char> IAFramebuffer::makeIntoBitmap()
{
    size_t size = pWidth*pHeight*3;
    // use a shared_pointer, so the user does not have to worry about deleting
    // the array ever.
    std::shared_ptr<unsigned char> pixels(new unsigned char[size], std::default_delete<unsigned char[]>());
    // read the FBO content into RAM and make a bitmap for potrace
    // FIXME: at this point, potrace converts from RGB to bitmap.
    GLvoid *px = pixels.get();
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pFramebuffer);
    glReadPixels(0, 0, pWidth, pHeight, GL_RGB, GL_UNSIGNED_BYTE, px);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    return pixels;
}


/**
 * Crude code to trace around the image and write an outline to a file.
 */
int IAFramebuffer::saveAsOutline(const char *filename)
{
    auto pixels = makeIntoBitmap();
    Iota.pToolpath->clear();
    potrace_main(filename, pixels.get());
    return 0;
}


/**
 * Write the RGB components of the image buffer into a jpeg file.
 */
int IAFramebuffer::saveAsJpeg(const char *filename)
{
    GLubyte *imgdata = (GLubyte*)malloc(pWidth*pHeight*3);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pFramebuffer);
    glReadPixels(0, 0, pWidth, pHeight, GL_RGB, GL_UNSIGNED_BYTE, imgdata);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

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
 * Draw the RGBA buffer into the scene viewer.
 */
void IAFramebuffer::draw()
{
    if (!hasFBO()) return;

    // set as texture and render out
    double z = zSlider1->value();
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
}


/**
 * Delete the framebuffer object.
 */
void IAFramebuffer::deleteFBO()
{
    //Delete resources
    glDeleteTextures(1, &pColorbuffer);
    glDeleteRenderbuffersEXT(1, &pDepthbuffer);
    //Bind 0, which means render to back buffer, as a result, fb is unbound
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glDeleteFramebuffersEXT(1, &pFramebuffer);
    // TODO: get rid of this thing
    pFramebufferCreated = false;
}


