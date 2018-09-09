//
//  IAPreferences.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_PREFERENCES_H
#define IA_PREFERENCES_H


#include <FL/filename.H>


class Fl_Preferences;


/**
 * User preferences.
 */
class IAPreferences
{
public:
    IAPreferences();
    ~IAPreferences();
    void flush();

    /** main window position, or -1 if undefined. */
    int pMainWindowX = -1;
    /** main window position, or -1 if undefined. */
    int pMainWindowY = -1;
    /** main window width. */
    int pMainWindowW = 800;
    /** main window height. */
    int pMainWindowH = 600;

    /** \todo obsolete. Every printer driver saves its own "last file". */
    char pLastGCodeFilename[FL_PATH_MAX] = { 0 };
    /** Name and path of the last STL file that was read.
     * \todo we should remember 10 files and put them in a menu.
     */
    char pLastLoadFilename[FL_PATH_MAX] = { 0 };
};


#endif /* IA_PREFERENCES_H */


