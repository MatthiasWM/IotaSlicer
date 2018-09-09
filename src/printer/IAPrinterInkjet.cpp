//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinterInkjet.h"

#include "Iota.h"
#include "userinterface/IAGUIMain.h"
#include "toolpath/IAToolpath.h"
#include "opengl/IAFramebuffer.h"


#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Choice.H>
#include <FL/filename.H>


/**
 * Virtual, implement this to open a file chooser with the require file
 * pattern and extension.
 *
 * \todo jpeg is certainly the wrong file format because we need an alpha channel
 *       and very little compression. Currently we create black for infills
 *       and key green (0x00FF00) for areas that must not be printed.
 * \todo the texture mapping currently projects only along the xy-normal, and
 *       not along the point normal as it should be.
 */
void IAPrinterInkjet::userSliceAs()
{
    if (queryOutputFilename("Save slices as images", "*.png", ".png")) {
        sliceAndWrite();
    }
}


/**
 * Virtual, implement the slicer for the given machine here.
 */
void IAPrinterInkjet::sliceAndWrite(const char *filename)
{
    if (!filename)
        filename = outputPath();
    char fn[FL_PATH_MAX];
    strcpy(fn, filename);
    char *a = (char*)fl_filename_ext(fn);
    const char *b = fl_filename_ext(filename);
    if ( a && b ) {
        sprintf(a, "_%%04d%s", b);
    } else {
        strcat(fn, "_%04d");
    }

    double hgt = Iota.pMesh->pMax.z() - Iota.pMesh->pMin.z();
    // initial height determines stickiness to bed

    double zMin = layerHeight() * 0.7;
    double zLayerHeight = layerHeight();
#if 1
    double zMax = hgt;
#else
    double zMax = 25;
#endif

    showProgressDialog();
    char buf[1024];
    strcpy(buf, "Genrating slices");
    wProgressText->copy_label(buf);
    wProgressValue->value(0);
    int i = 0, n = (int)((zMax-zMin)/zLayerHeight);

    for (double z=zMin; z<zMax; z+=zLayerHeight) {
        printf("Slicing at z=%g\n", z);

        sprintf(buf, "Slicing layer %d of %d at %.3fmm (%d%%)", i, n, z, i*100/n);
        wProgressText->copy_label(buf);
        wProgressValue->value(i*100/n);
        bool abort = updateProgressDialog();
        if (abort) break;

        gSlice.changeZ(z);
        gSlice.clear();
        gSlice.generateRim(Iota.pMesh);
        gSlice.tesselateLidFromRim();
        gSlice.drawFlat(false, 1, 1, 1);

        uint8_t *alpha = gSlice.pFramebuffer->getRawImageRGB();
        uint8_t *rgb = gSlice.pColorbuffer->getRawImageRGBA();

        /** \todo we can of course do all that in the OpenGL code already
            \todo infill should be white or user selectable
            \todo inkjet should generate support structurs for image based SLA
         */
        {
            int i = 0, n = gSlice.pColorbuffer->width()
                         * gSlice.pColorbuffer->height();
            uint8_t *s = alpha, *d = rgb;
            for (i=0; i<n; ++i) {
                d[3] = *s;
                s+=3; d+=4;
            }
        }

        char imgFilename[2048];
        sprintf(imgFilename, fn, i);
        gSlice.pColorbuffer->saveAsPng(imgFilename, 4, rgb);
// for testing, we also can write jpegs or other files.
//        fl_filename_setext(imgFilename, 2048, ".jpg");
//        gSlice.pColorbuffer->saveAsJpeg(imgFilename, rgb);
        i++;
    }
    hideProgressDialog();
    gSceneView->redraw();
}


/**
 * Create the Treeview items for setting up the printout for this session.
 */
void IAPrinterInkjet::buildSessionSettings()
{
    char buf[80];

    static Fl_Menu_Item lHgtMenu[] = {
        { "0.1" },
        { "0.2" },
        { "0.3" }
    };

    /** \todo FIXME: this keeps on adding Choice widgets to the tree class!
     * We must either delete the widgets when changing printer (which is hard,
     * because the scrollbars are also children of Fl_Tree), or we must
     * store the link to this widget and show and hide it accordingly (which
     * is also not obvious)
     * Or, we override Fl_Tree and write our owb clean() method?
     */
    Fl_Input_Choice *lHgt = new Fl_Input_Choice(1, 1, 60, 1, "mm");
    lHgt->align(FL_ALIGN_RIGHT);
    lHgt->labelsize(12);
    lHgt->textsize(12);
    lHgt->menu(lHgtMenu);
    sprintf(buf, "%.2f", layerHeight()); lHgt->value(buf);
    lHgt->callback(userSetLayerHeightCB, this);
    Fl_Tree_Item *it = wSessionSettings->add("Layer Height: ");
    it->widget(lHgt);
}


/**
 * The layer height was changed via the layer height chooser in the seesion
 * setting view.
 */
void IAPrinterInkjet::userSetLayerHeight(Fl_Input_Choice *w)
{
    /** \todo warn if layer height is 0, negative, or huge */
    pLayerHeight = atof(w->value());
}


