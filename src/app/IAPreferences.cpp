//
//  IAPreferences.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPreferences.h"

#include "view/IAGUIMain.h"
#include <FL/Fl_Preferences.H>


/**
 * Create the preferences interface an load the last settings.
 */
IAPreferences::IAPreferences()
{
    char buf[FL_PATH_MAX];

    Fl_Preferences pPrefs(Fl_Preferences::USER, "com.matthiasm.iota", "IotaSlicer");
    buf[0] = 0;
    pPrefs.getUserdataPath(buf, sizeof(buf));
    strcat(buf, "printerDefinitions/");
    pPrinterDefinitionsPath = strdup(buf);

    Fl_Preferences main(pPrefs, "main");

    Fl_Preferences window(main, "window");

    window.get("x", pMainWindowX, -1);
    window.get("y", pMainWindowY, -1);
    window.get("w", pMainWindowW, 800);
    window.get("h", pMainWindowH, 600);

    Fl_Preferences recentFiles(main, "recentFiles");
    for (int i=0; i<pNRecentFiles; i++) {
        recentFiles.get(Fl_Preferences::Name(i), pRecentFile[i], "");
    }
    updateRecentfilesMenu();
}


/**
 * Write the current setting to the preferences file.
 */
IAPreferences::~IAPreferences()
{
    flush();
    if (pPrinterDefinitionsPath) ::free((void*)pPrinterDefinitionsPath);
}


/**
 * Write the current setting to the preferences file.
 */
void IAPreferences::flush()
{
    Fl_Preferences pPrefs(Fl_Preferences::USER, "com.matthiasm.iota", "IotaSlicer");

    Fl_Preferences main(pPrefs, "main");

    Fl_Preferences window(main, "window");

    window.set("x", wMainWindow->x());
    window.set("y", wMainWindow->y());
    window.set("w", wMainWindow->w());
    window.set("h", wMainWindow->h());

    Fl_Preferences recentFiles(main, "recentFiles");
    for (int i=0; i<pNRecentFiles; i++) {
        recentFiles.set(Fl_Preferences::Name(i), pRecentFile[i]);
    }

    pPrefs.flush();
}


/**
 * Update the main menu that shows all the recently loaded files.
 */
void IAPreferences::updateRecentfilesMenu()
{
    auto menu = wRecentFiles;
    int lastVisible = -1;
    for (int i=0; i<pNRecentFiles; i++) {
        if (pRecentFile[i] && pRecentFile[i][0]) {
            menu->label(fl_filename_name(pRecentFile[i]));
            menu->show();
            menu->flags &= ~FL_MENU_DIVIDER;
            lastVisible = i;
        } else {
            menu->label("");
            menu->hide();
        }
        menu++;
    }
    if (lastVisible>=0) {
        wRecentFiles[lastVisible].flags |= FL_MENU_DIVIDER;
    }
}


/**
 * Add another file to the list of recently opened files.
 */
void IAPreferences::addRecentFile(const char *filename)
{
    if (!filename || !filename[0])
        return;

    char buf[FL_PATH_MAX];
    fl_filename_absolute(buf, FL_PATH_MAX, filename);
    int sameAs = pNRecentFiles;
    for (int i=0; i<pNRecentFiles; i++) {
        if (strcmp(pRecentFile[i], buf)==0) {
            sameAs = i;
            break;
        }
    }
    char *first = nullptr;
    if (sameAs==pNRecentFiles) {
        first = strdup(buf);
        ::free((void*)pRecentFile[pNRecentFiles-1]);
        sameAs--;
    } else {
        first = pRecentFile[sameAs];
    }
    for (int i=sameAs; i>0; i--) {
        pRecentFile[i] = pRecentFile[i-1];
    }
    pRecentFile[0] = first;
    updateRecentfilesMenu();
    flush();
}


void IAPreferences::clearRecentFileList()
{
    for (int i=0; i<pNRecentFiles; i++) {
        if (pRecentFile[i]) {
            ::free((void*)pRecentFile[i]);
        }
        pRecentFile[i] = strdup("");
    }
    updateRecentfilesMenu();
    flush();
}


/**
 * Get a file path for storing 3d printer definitions.
 *
 * \return path to a directory in the user data area.
 */
const char *IAPreferences::printerDefinitionsPath() const
{
    return pPrinterDefinitionsPath;
}

