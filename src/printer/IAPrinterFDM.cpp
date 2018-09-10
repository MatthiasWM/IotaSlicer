//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinterFDM.h"

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
 */
void IAPrinterFDM::userSliceAs()
{
    if (queryOutputFilename("Save toolpath as GCode", "*.gcode", ".gcode")) {
        sliceAndWrite();
    }
}


/**
 * Virtual, implement the slicer for the given machine here.
 */
void IAPrinterFDM::sliceAndWrite(const char *filename)
{
    if (!filename)
        filename = outputPath();

    if (!pMachineToolpath)
        pMachineToolpath = new IAMachineToolpath();
    else
        pMachineToolpath->clear();
    double hgt = Iota.pMesh->pMax.z() - Iota.pMesh->pMin.z();
    // initial height determines stickiness to bed

    double zMin = layerHeight() * 0.7;
    double zLayerHeight = layerHeight();
#if 0
    double zMax = hgt;
#else
    zLayerHeight = 1.0;
    double zMax = hgt + zLayerHeight;
#endif

    showProgressDialog();
    char buf[1024];
    strcpy(buf, "Genrating slices");
    wProgressText->copy_label(buf);
    wProgressValue->value(0);
    int i = 0, n = (int)((zMax-zMin)/zLayerHeight);


#if 1

    // create a slice for each layer
    IASlice **sliceList = (IASlice**)calloc(n+1, sizeof(IASlice*));
    for (double z=zMin; z<zMax; z+=zLayerHeight) {
        IASlice *slc = sliceList[i] = new IASlice();
        sprintf(buf, "Building shell for layer %d of %d (%d%%)", i, n, i*50/n);
        wProgressText->copy_label(buf);
        wProgressValue->value(i*100/n);
        bool abort = updateProgressDialog();
        if (abort) break;

        slc->changeZ(z);
        slc->clear();
        slc->generateRim(Iota.pMesh);
        slc->tesselateLidFromRim();
        slc->drawFlat(false, 1, 1, 1);

        // create an outline for this slice image
        IAToolpath *tp0 = new IAToolpath(z);
        slc->pFramebuffer->traceOutline(tp0, z);
        // reduce the slice image by this toolpath to make the next shell fit
        // the physical model size
        slc->pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp0->drawFlat(4);
        slc->pFramebuffer->unbindFromRendering();
        delete tp0; // we no longer need this toolpath

        // draw the first shell for this slice image
        IAToolpath *tp1 = new IAToolpath(z);
        slc->pFramebuffer->traceOutline(tp1, z);
        slc->pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp1->drawFlat(4); /** \todo width depends on nozzle width! */
        slc->pFramebuffer->unbindFromRendering();

        // draw the second shell for this slice image
        IAToolpath *tp2 = new IAToolpath(z);
        slc->pFramebuffer->traceOutline(tp2, z);
        slc->pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp2->drawFlat(4);
        slc->pFramebuffer->unbindFromRendering();

        // draw the third shell for this slice image
        IAToolpath *tp3 = new IAToolpath(z);
        slc->pFramebuffer->traceOutline(tp3, z);
        slc->pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp3->drawFlat(4);
        slc->pFramebuffer->unbindFromRendering();

        IAToolpath *tp = pMachineToolpath->createLayer(z);
        tp->add(*tp3);
        tp->add(*tp2);
        tp->add(*tp1);
        delete tp1;
        delete tp2;
        delete tp3;
        i++;
    }

    i=0;
    for (double z=zMin; z<zMax-zLayerHeight; z+=zLayerHeight) {
        IAToolpath *tp = pMachineToolpath->findLayer(z);

        IASlice *slc = sliceList[i];
        sprintf(buf, "Building shell for layer %d of %d (%d%%)", i, n, i*50/n);
        wProgressText->copy_label(buf);
        wProgressValue->value(i*100/n);
        bool abort = updateProgressDialog();
        if (abort) break;

        {
            IAFramebuffer fb(slc->pFramebuffer);
            if (i==24) fb.saveAsJpeg("/Users/matt/aaa_010.jpg");
            /*if (i<n)*/ fb.logicAndNot(sliceList[i+1]->pFramebuffer);
            if (i==24) sliceList[i+1]->pFramebuffer->saveAsJpeg("/Users/matt/aaa_005.jpg");
            if (i==24) fb.saveAsJpeg("/Users/matt/aaa_020.jpg");
            // Now whatever is still here is the lid
            fb.bindForRendering();
            glDisable(GL_DEPTH_TEST);
            glColor3f(0.0, 0.0, 0.0);
            // draw spaces so the infill gets spread out nicely
            double wdt = Iota.pCurrentPrinter->pBuildVolumeMax.x();
            double hgt = Iota.pCurrentPrinter->pBuildVolumeMax.y();
            // generate a lid
            double infillWdt = 0.3;
            for (double j=0; j<wdt; j+=infillWdt*2) {
                glBegin(GL_POLYGON);
                glVertex2f(j+infillWdt, 0);
                glVertex2f(j, 0);
                glVertex2f(j, hgt);
                glVertex2f(j+infillWdt, hgt);
                glEnd();
            }
            fb.unbindFromRendering();
            IAToolpath *infill = new IAToolpath(z);
            fb.traceOutline(infill, z); // set the parameters so that we get sharp edges
            fb.clear();
            tp->add(*infill);
            delete infill;
        }

        {
            // generate a diagonal infill
            IAFramebuffer fb(slc->pFramebuffer);
            if (i<n) fb.logicAnd(sliceList[i+1]->pFramebuffer);
            fb.bindForRendering();
            glDisable(GL_DEPTH_TEST);
            glColor3f(0.0, 0.0, 0.0);
            // draw spaces so the infill gets spread out nicely
            double wdt = Iota.pCurrentPrinter->pBuildVolumeMax.x();
            double hgt = Iota.pCurrentPrinter->pBuildVolumeMax.y();
            double infillWdt = 3;
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
            fb.unbindFromRendering();
            IAToolpath *infill = new IAToolpath(z);
            fb.traceOutline(infill, z); // set the parameters so that we get sharp edges
            fb.clear();
            tp->add(*infill);
            delete infill;
        }
        // add to this toolpath
        i++;
    }

    i=0;
    for (double z=zMin; z<zMax; z+=zLayerHeight) {
        IASlice *slc = sliceList[i];
        delete slc;
        i++;
    }
    free((void*)sliceList);
#if 0
        // remaining image is either a lid, a floor, or an infill
        /** \todo look at the layers above to find out if this is a lid
         \todo look at the layers below to find out if this is a floor
         \todo look at what is support structure for the layers above
         */

        /*
         How do we find a lid?

         A single layer lid is the AND NOT operation between this layer and the
         layer above this one. Everything in this layer that is not the lid is
         then the infill.

         A multi layer lid is the AND NOT operation between this layer and
         the AND operation of multiple layers above this one.

         A bottom lid is the same as a top lib, but with the layers below. A
         general lid is then the current pattern AND NOT the AND operation of
         all relevant layers below or above.

         Again the remaining part is the infill, or, to put it more
         mathematically, the infill is the AND operation of all layers
         involved.
         */

        /*
         How do we find the support structure pattern?

         There are two supports needed: triangles that are flatter than 45 deg
         from z need support, and "icicles", hanging structures need a support
         with a minimum diameter. Icicles are vertices that are lower than all
         vertices of all connected triangles.

         Icicles and angled triangles throw a volumetric shadow down. They
         go all the way down to z=0, unless we find a system to only project
         them onto geometry below instead.

         Support can be rendered onto a finished slice, but it must not disturb
         anything that was already rendered, and it must not be rendered above
         the current z height (*1). Other than that, it is a simple projection
         along the z axis.

         *1) by slightly modifying the z height, we generate a layer between
         the support and the model that is less compressed and less sticky.
         This may help a lot with support removal.
         *2) support should not touch the model sideways. This can be acheived
         by redering on bigger circumference and subtracting it befor3
         tracing and filling.
         */

        // Now whatever is still here will be infill
        gSlice.pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        // draw spaces so the infill gets spread out nicely
        double wdt = Iota.pCurrentPrinter->pBuildVolumeMax.x();
        double hgt = Iota.pCurrentPrinter->pBuildVolumeMax.y();
#if 0 // generate a lid
        double infillWdt = 0.3;
        for (double j=0; j<wdt; j+=infillWdt*2) {
            glBegin(GL_POLYGON);
            glVertex2f(j+infillWdt, 0);
            glVertex2f(j, 0);
            glVertex2f(j, hgt);
            glVertex2f(j+infillWdt, hgt);
            glEnd();
        }
#else   // generate a diagonal infill
        double infillWdt = 3;
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
#endif
        gSlice.pFramebuffer->unbindFromRendering();
        IAToolpath *infill = new IAToolpath(z);
        gSlice.pFramebuffer->traceOutline(infill, z); // set the parameters so that we get sharp edges

        IAToolpath *tp = pMachineToolpath->createLayer(z);
        if (colorMode()==0) {
            tp->add(*tp3);
            tp->add(*tp2);
            tp->add(*tp1);
        } else {
            IAToolpath *b1 = new IAToolpath(z);
            IAToolpath *w1 = new IAToolpath(z);
            IAToolpath *b2 = new IAToolpath(z);
            IAToolpath *w2 = new IAToolpath(z);
            tp2->colorize(rgb, b1, w1);
            tp3->colorize(rgb, b2, w2);
            tp->pList.push_back(new IAToolpathExtruder(0));
            tp->add(*w2);
            tp->add(*w1);
            tp->pList.push_back(new IAToolpathExtruder(1));
            tp->add(*b2);
            tp->add(*b1);
            delete b1;
            delete w1;
            delete b2;
            delete w2;
        }
        tp->add(*infill);
        delete tp1;
        delete tp2;
        delete tp3;
        delete infill;
        free(rgb);
        i++;
    }
#endif

#else


    for (double z=zMin; z<zMax; z+=zLayerHeight) {
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

        // create an outline for this slice image
        IAToolpath *tp0 = new IAToolpath(z);
        gSlice.pFramebuffer->traceOutline(tp0, z);
        // reduce the slice image by this toolpath to make the next shell fit
        // the physical model size
        gSlice.pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp0->drawFlat(4);
        gSlice.pFramebuffer->unbindFromRendering();
        delete tp0; // we no longer need this toolpath

        // draw the first shell for this slice image
        IAToolpath *tp1 = new IAToolpath(z);
        gSlice.pFramebuffer->traceOutline(tp1, z);
        gSlice.pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp1->drawFlat(4); /** \todo width depends on nozzle width! */
        gSlice.pFramebuffer->unbindFromRendering();

        // draw the second shell for this slice image
        IAToolpath *tp2 = new IAToolpath(z);
        gSlice.pFramebuffer->traceOutline(tp2, z);
        gSlice.pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp2->drawFlat(4);
        gSlice.pFramebuffer->unbindFromRendering();

        // draw the third shell for this slice image
        IAToolpath *tp3 = new IAToolpath(z);
        gSlice.pFramebuffer->traceOutline(tp3, z);
        gSlice.pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp3->drawFlat(4);
        gSlice.pFramebuffer->unbindFromRendering();

        // remaining image is either a lid, a floor, or an infill
        /** \todo look at the layers above to find out if this is a lid
            \todo look at the layers below to find out if this is a floor
            \todo look at what is support structure for the layers above
         */

        /*
         How do we find a lid?

         A single layer lid is the AND NOT operation between this layer and the
         layer above this one. Everything in this layer that is not the lid is
         then the infill.

         A multi layer lid is the AND NOT operation between this layer and
         the AND operation of multiple layers above this one.

         A bottom lid is the same as a top lib, but with the layers below. A
         general lid is then the current pattern AND NOT the AND operation of
         all relevant layers below or above.

         Again the remaining part is the infill, or, to put it more
         mathematically, the infill is the AND operation of all layers
         involved.
         */

        /*
         How do we find the support structure pattern?

         There are two supports needed: triangles that are flatter than 45 deg
         from z need support, and "icicles", hanging structures need a support
         with a minimum diameter. Icicles are vertices that are lower than all
         vertices of all connected triangles.

         Icicles and angled triangles throw a volumetric shadow down. They
         go all the way down to z=0, unless we find a system to only project
         them onto geometry below instead.

         Support can be rendered onto a finished slice, but it must not disturb
         anything that was already rendered, and it must not be rendered above
         the current z height (*1). Other than that, it is a simple projection
         along the z axis.

         *1) by slightly modifying the z height, we generate a layer between
             the support and the model that is less compressed and less sticky.
             This may help a lot with support removal.
         *2) support should not touch the model sideways. This can be acheived
             by redering on bigger circumference and subtracting it befor3
             tracing and filling.
         */

        // Now whatever is still here will be infill
        gSlice.pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        // draw spaces so the infill gets spread out nicely
        double wdt = Iota.pCurrentPrinter->pBuildVolumeMax.x();
        double hgt = Iota.pCurrentPrinter->pBuildVolumeMax.y();
#if 0 // generate a lid
        double infillWdt = 0.3;
        for (double j=0; j<wdt; j+=infillWdt*2) {
            glBegin(GL_POLYGON);
            glVertex2f(j+infillWdt, 0);
            glVertex2f(j, 0);
            glVertex2f(j, hgt);
            glVertex2f(j+infillWdt, hgt);
            glEnd();
        }
#else   // generate a diagonal infill
        double infillWdt = 3;
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
#endif
        gSlice.pFramebuffer->unbindFromRendering();
        IAToolpath *infill = new IAToolpath(z);
        gSlice.pFramebuffer->traceOutline(infill, z); // set the parameters so that we get sharp edges

        IAToolpath *tp = pMachineToolpath->createLayer(z);
        if (colorMode()==0) {
            tp->add(*tp3);
            tp->add(*tp2);
            tp->add(*tp1);
        } else {
            uint8_t *rgb = gSlice.pColorbuffer->getRawImageRGB();
            IAToolpath *b1 = new IAToolpath(z);
            IAToolpath *w1 = new IAToolpath(z);
            IAToolpath *b2 = new IAToolpath(z);
            IAToolpath *w2 = new IAToolpath(z);
            tp2->colorize(rgb, b1, w1);
            tp3->colorize(rgb, b2, w2);
            tp->pList.push_back(new IAToolpathExtruder(0));
            tp->add(*w2);
            tp->add(*w1);
            tp->pList.push_back(new IAToolpathExtruder(1));
            tp->add(*b2);
            tp->add(*b1);
            delete b1;
            delete w1;
            delete b2;
            delete w2;
            free(rgb);
        }
        tp->add(*infill);
        delete tp1;
        delete tp2;
        delete tp3;
        delete infill;
        i++;
    }


#endif


    pMachineToolpath->saveGCode(filename);
    hideProgressDialog();
    zSlider1->value(0.0);
    zSlider1->do_callback();
    gSceneView->redraw();
}


/**
 * Create the Treeview items for setting up the printout for this session.
 *
 * \todo number of extrusion in the shell
 * \todo number of layers for lids and bottoms
 * \todo density for infills
 */
void IAPrinterFDM::buildSessionSettings()
{
    char buf[80];

    static Fl_Menu_Item lHgtMenu[] = {
        { "0.1" },
        { "0.2" },
        { "0.3" }
    };

    /** \bug this keeps on adding Choice widgets to the tree class!
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

    /**
     \todo The color settings should be determined by the number of extruders,
     extruder types (mixing, mono), and by the number of filemanets (color,
     fill, support)
     */
    static Fl_Menu_Item colorModeMenu[] = {
        { "monochrome", 0, nullptr, (void*)0 },
        { "dual color", 0, nullptr, (void*)1 }
    };

    Fl_Choice *colorMode = new Fl_Choice(1, 1, 120, 1);
    colorMode->textsize(12);
    colorMode->menu(colorModeMenu);
    colorMode->value(0);
    colorMode->callback(userSetColorModeCB, this);
    it = wSessionSettings->add("Color: ");
    it->widget(colorMode);

}


/**
 * The layer height was changed via the layer height chooser in the seesion
 * setting view.
 */
void IAPrinterFDM::userSetLayerHeight(Fl_Input_Choice *w)
{
    /** \todo warn if layer height is 0, negative, or huge */
    pLayerHeight = atof(w->value());
}


/**
 * The color mode was changed via the color mode chooser in the session
 * settings view.
 */
void IAPrinterFDM::userSetColorMode(Fl_Choice *w)
{
    Fl_Menu_Item const* mi = w->mvalue();
    if (mi) {
        pColorMode = (int)(fl_intptr_t)(mi->user_data());
    }
}




