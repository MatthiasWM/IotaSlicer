//
//  IAPrinterFDM.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_FDM_H
#define IA_PRINTER_FDM_H


#include "printer/IAPrinter.h"

class Fl_Widget;
class Fl_Choice;
class Fl_Input_Choice;

/**
 * FDM, Fused Deposition Modeling, describes printers that extrude filament
 * to create 3D models.
 *
 * This printer driver writes GCode files.
 */
class IAPrinterFDM : public IAPrinter
{
    typedef IAPrinter super;
public:
    // ---- constructor and destructor
    IAPrinterFDM(const char *name);

    // ---- direct user interaction
    virtual void userSliceSave() override;
    virtual void userSliceSaveAs() override;
    virtual void userSliceGenerateAll() override;

    // ----
    void sliceAll();
    void saveToolpath(const char *filename = nullptr);

    virtual void buildSessionSettings() override;

    double layerHeight() { return pLayerHeight; }
    int colorMode() { return pColorMode; }

protected:
    void userSetLayerHeight(Fl_Input_Choice *w);
    void userSetColorMode(Fl_Choice *w);
    void userChangedColorMode();

private:
    static void userSetLayerHeightCB(Fl_Widget *w, void *d) {
        ((IAPrinterFDM*)d)->userSetLayerHeight((Fl_Input_Choice*)w); }
    static void userSetColorModeCB(Fl_Widget *w, void *d) {
        ((IAPrinterFDM*)d)->userSetColorMode((Fl_Choice*)w); }

    double pNozzleDiameter = 0.4;

    IASettingList pSettingList;
    /** \todo make this an enum */
    int pColorMode = 0;
//    IASettingChoice pSettingColorMode;
};


#endif /* IA_PRINTER_FDM_H */


