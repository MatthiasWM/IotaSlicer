//
//  IAPrinterFDMBelt.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_FDM_BELT_H
#define IA_PRINTER_FDM_BELT_H


#include "printer/IAPrinterFDM.h"


/**
 * FDM Belt Printer print with a bridge at a 45 deg angle onto an endless belt.
 *
 * This printer driver writes GCode files.
 */
class IAPrinterFDMBelt : public IAPrinterFDM
{
public:
    IAPrinterFDMBelt(const char *name) : IAPrinterFDM(name) { }
};


#endif /* IA_PRINTER_FDM_BELT_H */


