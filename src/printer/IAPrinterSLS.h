//
//  IAPrinterSLS.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_SLS_H
#define IA_PRINTER_SLS_H


#include "printer/IAPrinter.h"


/**
 * SLS, Selective Laser Sintering, melts fine plastic powder with a laser
 * beam.
 *
 * This printer driver writes GCode files.
 *
 * No color or multimaterial support.
 */
class IAPrinterSLS : public IAPrinter
{
public:
    IAPrinterSLS(const char *name) : IAPrinter(name) { }

    // ---- direct user interaction
    virtual void userSliceSave() override;
    virtual void userSliceSaveAs() override;
    virtual void userSliceGenerateAll() override;

};


#endif /* IA_PRINTER_SLS_H */


