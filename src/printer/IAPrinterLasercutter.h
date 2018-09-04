//
//  IAPrinterLaser.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_LASERCUTTER_H
#define IA_PRINTER_LASERCUTTER_H


#include "printer/IAPrinter.h"


/**
 * The Lasercutter driver generates slices that can be cut and clued together.
 *
 * This printer driver generates DXF files for laser engravers.
 *
 * No color or multimaterial support.
 */
class IAPrinterLasercutter : public IAPrinter
{
public:
    IAPrinterLasercutter() { }
};


#endif /* IA_PRINTER_LASERCUTTER_H */


