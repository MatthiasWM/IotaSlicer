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
    IAPrinterFDM();
    IAPrinterFDM(IAPrinterFDM const& src);
    virtual ~IAPrinterFDM() override;
    IAPrinterFDM &operator=(IAPrinterFDM&) = delete;
    virtual IAPrinter *clone() const override;
    virtual const char *type() const override { return "IAPrinterFDM"; }

    // ---- direct user interaction
    virtual void userSliceSave() override;
    virtual void userSliceSaveAs() override;
    virtual void userSliceGenerateAll() override;

    virtual void purgeSlicesAndCaches() override;
    virtual void drawPreview(double lo, double hi) override;
    
    virtual void initializePrinterProperties() override;
    virtual void initializeSceneSettings() override;

    // ----
    void sliceAll();
    void saveToolpath(const char *filename = nullptr);

    int colorMode() { return pColorMode; }
    double nozzleDiameter() { return pNozzleDiameter; }
    double filamentDiameter() { return 1.75; }

protected:
    void userChangedColorMode();
    void userChangedNumLids();
    void userChangedLidType();
    void userChangedNumShells();
    void userChangedInfillDensity();
    void userChangedSkirt();
    void userChangedNozzleDiameter();

private:

    /// the toolpath for the entire scene for vector based machines
    IAMachineToolpath pMachineToolpath = IAMachineToolpath(this);

    // make sure that the values below are cloned!
    double pNozzleDiameter = 0.4;
    int pColorMode = 0;
    int pNumShells = 3;
    int pNumLids = 2;
    int pLidType = 0; // 0=zigzag, 1=concentric
    double pInfillDensity = 20.0; // %
    int pHasSkirt = 1; // prime line around perimeter
};


class IALayerMapFDM;

/**
 * Layers foor FDM printers contain all data from slice to toolpath.
 */
class IALayerFDM
{
public:
    IALayerFDM(IALayerMapFDM*, double z, double height);
    ~IALayerFDM();
    void createToolpath();
    void createInfill();
    void createLid();
    void createBottom();
    void createShells();
    void createSLice();
private:
    IASlice *pSlice;
    IAFramebuffer *pFBSlice;
    IAFramebuffer *pFBCore; // slice without shell
};


class IALayerMapFDM
{
    IALayerMapFDM(IAPrinterFDM*);
    ~IALayerMapFDM();
    std::map<IALayerFDM, double> pMap;
};


#endif /* IA_PRINTER_FDM_H */


