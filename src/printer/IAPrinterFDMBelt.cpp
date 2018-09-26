//
//  IAPrinter.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPrinterFDMBelt.h"



#if 0

/*
 this is only here to remind us how we printed color, so when the day comes
 to reimplement multi-nozzle and mixed-nozzle printing, we may find some
 ideas here
 */

/**
 * Virtual, implement the slicer for the given machine here.
 */
void IAPrinterFDM::sliceAndWrite(const char *filename)
{
    if (!filename)
        filename = outputPath();

    if (!pMachineToolpath)
        pMachineToolpath = new IAMachineToolpath(this);
    else
        pMachineToolpath->clear();
    double hgt = Iota.pMesh->pMax.z() - Iota.pMesh->pMin.z();
    // initial height determines stickiness to bed

    double zMin = layerHeight() * 0.7;
    double zLayerHeight = layerHeight();
    double zMax = hgt;

    showProgressDialog();
    char buf[1024];
    strcpy(buf, "Genrating slices");
    wProgressText->copy_label(buf);
    wProgressValue->value(0);
    int i = 0, n = (int)((zMax-zMin)/zLayerHeight);




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
    tp0->drawFlat(pNozzleDiameter);
    gSlice.pFramebuffer->unbindFromRendering();
    delete tp0; // we no longer need this toolpath

    // draw the first shell for this slice image
    IAToolpath *tp1 = new IAToolpath(z);
    gSlice.pFramebuffer->traceOutline(tp1, z);
    gSlice.pFramebuffer->bindForRendering();
    glDisable(GL_DEPTH_TEST);
    glColor3f(0.0, 0.0, 0.0);
    tp1->drawFlat(2.0*pNozzleDiameter); /** \todo width depends on nozzle width! */
    gSlice.pFramebuffer->unbindFromRendering();

    // draw the second shell for this slice image
    IAToolpath *tp2 = new IAToolpath(z);
    gSlice.pFramebuffer->traceOutline(tp2, z);
    gSlice.pFramebuffer->bindForRendering();
    glDisable(GL_DEPTH_TEST);
    glColor3f(0.0, 0.0, 0.0);
    tp2->drawFlat(2.0*pNozzleDiameter);
    gSlice.pFramebuffer->unbindFromRendering();

    // draw the third shell for this slice image
    IAToolpath *tp3 = new IAToolpath(z);
    gSlice.pFramebuffer->traceOutline(tp3, z);
    gSlice.pFramebuffer->bindForRendering();
    glDisable(GL_DEPTH_TEST);
    glColor3f(0.0, 0.0, 0.0);
    tp3->drawFlat(2.0*pNozzleDiameter);
    gSlice.pFramebuffer->unbindFromRendering();

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
        // creating a black-and-white toolpath
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



pMachineToolpath->saveGCode(filename);
hideProgressDialog();
zSlider1->value(0.0);
zSlider1->do_callback();
gSceneView->redraw();
}


#endif
