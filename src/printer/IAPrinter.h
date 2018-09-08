//
//  IAPrinter.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PRINTER_H
#define IA_PRINTER_H


#include "geometry/IAVector3d.h"

#include <vector>

class Fl_Menu_Item;


/**
 * Base class to manage different types of 3D printers.
 */
class IAPrinter
{
public:
    IAPrinter(const char *name);
    virtual ~IAPrinter();
    virtual void draw();

    void loadSettings();
    void saveSettings();
    
    void setName(const char *name);
    const char *name();
    void setOutputPath(const char *name);
    const char *outputPath();

    virtual void userSliceAs();
    virtual void userSliceAgain();
    virtual void sliceAndWrite(const char *filename=nullptr);

    virtual void buildSessionSettings();

    IAVector3d pBuildVolume = { 214.0, 214.0, 230.0 };
    IAVector3d pBuildVolumeMin = { 0.0, 0.0, 0.0 };
    IAVector3d pBuildVolumeMax = { 214.0, 214.0, 230.0 };
    double pBuildVolumeRadius = 200.0; // sphere that contains the entire centered build volume

protected:
    bool queryOutputFilename(const char *title,
                             const char *filter,
                             const char *extension);

    bool pFirstWrite = true;

private:
    char *pName = nullptr;

    char *pOutputPath = nullptr;
};


/**
 * Manage a list of printers.
 */
class IAPrinterList
{
public:
    IAPrinterList(Fl_Menu_Item *printermenu);
    ~IAPrinterList();
    bool add(IAPrinter *printer);
    IAPrinter *defaultPrinter();
    void userSelectsPrinter(IAPrinter *p);

private:
    void buildMenuArray();
    void buildSessionSettings(IAPrinter *p);
    static void userSelectsPrinterCB(Fl_Menu_Item*, void *p);

    Fl_Menu_Item *pMenuItem = nullptr;
    Fl_Menu_Item *pMenuArray = nullptr;

    std::vector<IAPrinter *> pPrinterList;
};


#endif /* IA_PRINTER_H */


