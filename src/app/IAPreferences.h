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

    void updateRecentfilesMenu();
    void addRecentFile(const char *filename);
    void clearRecentFileList();
    const char *printerDefinitionsPath() const;

    /** main window position, or -1 if undefined. */
    int pMainWindowX = -1;
    /** main window position, or -1 if undefined. */
    int pMainWindowY = -1;
    /** main window width. */
    int pMainWindowW = 800;
    /** main window height. */
    int pMainWindowH = 600;

    /** Name and path of the last STL file that was read.
     * \todo we should remember 10 files and put them in a menu.
     */
    char pLastLoadFilename[FL_PATH_MAX] = { 0 };
    
    static const int pNRecentFiles = 10;
    char *pRecentFile[pNRecentFiles] = { 0 };
    char *pPrinterDefinitionsPath = nullptr;
};


#endif /* IA_PREFERENCES_H */


