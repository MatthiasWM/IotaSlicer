//
//  IAPrinterFDM.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_FDM_H
#define IA_PRINTER_FDM_H


#include "printer/IAPrinter.h"


/**
 * FDM, Fused Deposition Modeling, describes printers that extrude filament
 * to create 3D models.
 *
 * This printer driver writes GCode files.
 */
class IAPrinterFDM : public IAPrinter
{
public:
    IAPrinterFDM(const char *name) : IAPrinter(name) { }

    virtual void userSliceAs() override;
    virtual void userSliceAgain() override;
    virtual void sliceAndWrite(const char *filename=nullptr) override;
};


#endif /* IA_PRINTER_FDM_H */


