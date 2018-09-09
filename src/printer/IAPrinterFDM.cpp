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

    if (!Iota.pMachineToolpath)
        Iota.pMachineToolpath = new IAMachineToolpath();
    else
        Iota.pMachineToolpath->clear();
    double hgt = Iota.pMesh->pMax.z() - Iota.pMesh->pMin.z();
    // initial height determines stickiness to bed

    double zMin = layerHeight() * 0.7;
    double zLayerHeight = layerHeight();
#if 0
    double zMax = hgt;
#else
    double zMax = 2;
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

        Iota.gMeshSlice.changeZ(z);
        Iota.gMeshSlice.clear();
        Iota.gMeshSlice.generateRim(Iota.pMesh);
        Iota.gMeshSlice.tesselateLidFromRim();
        Iota.gMeshSlice.drawFlat(false, 1, 1, 1);

        uint8_t *rgb = Iota.gMeshSlice.pColorbuffer->getRawImageRGB();

        // create an outline for this slice image
        IAToolpath *tp0 = new IAToolpath(z);
        Iota.gMeshSlice.pFramebuffer->traceOutline(tp0, z);
        // reduce the slice image by this toolpath to make the next shell fit
        // the physical model size
        Iota.gMeshSlice.pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp0->drawFlat(4);
        Iota.gMeshSlice.pFramebuffer->unbindFromRendering();
        delete tp0; // we no longer need this toolpath

        // draw the first shell for this slice image
        IAToolpath *tp1 = new IAToolpath(z);
        Iota.gMeshSlice.pFramebuffer->traceOutline(tp1, z);
        Iota.gMeshSlice.pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp1->drawFlat(4); /** \todo width depends on nozzle width! */
        Iota.gMeshSlice.pFramebuffer->unbindFromRendering();

        // draw the second shell for this slice image
        IAToolpath *tp2 = new IAToolpath(z);
        Iota.gMeshSlice.pFramebuffer->traceOutline(tp2, z);
        Iota.gMeshSlice.pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp2->drawFlat(4);
        Iota.gMeshSlice.pFramebuffer->unbindFromRendering();

        // draw the third shell for this slice image
        IAToolpath *tp3 = new IAToolpath(z);
        Iota.gMeshSlice.pFramebuffer->traceOutline(tp3, z);
        Iota.gMeshSlice.pFramebuffer->bindForRendering();
        glDisable(GL_DEPTH_TEST);
        glColor3f(0.0, 0.0, 0.0);
        tp3->drawFlat(4);
        Iota.gMeshSlice.pFramebuffer->unbindFromRendering();

        // remaining image is either a lid, a floor, or an infill
        /** \todo look at the layers above to find out if this is a lid
            \todo look at the layers below to find out if this is a floor
            \todo look at what is support structure for the layers above
         */

        // Now whatever is still here will be infill
        Iota.gMeshSlice.pFramebuffer->bindForRendering();
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
        Iota.gMeshSlice.pFramebuffer->unbindFromRendering();
        IAToolpath *infill = new IAToolpath(z);
        Iota.gMeshSlice.pFramebuffer->traceOutline(infill, z); // set the parameters so that we get sharp edges

        IAToolpath *tp = Iota.pMachineToolpath->createLayer(z);
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
    Iota.pMachineToolpath->saveGCode(filename);
    hideProgressDialog();
    zSlider1->value(0.0);
    zSlider1->do_callback();
    gSceneView->redraw();
}


/**
 * Create the Treeview items for setting up the printout for this session.
 */
void IAPrinterFDM::buildSessionSettings()
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




