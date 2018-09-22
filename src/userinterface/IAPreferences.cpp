//
//  IAPreferences.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAPreferences.h"

#include "userinterface/IAGUIMain.h"
#include <FL/Fl_Preferences.H>


/**
 * Create the preferences interface an load the last settings.
 */
IAPreferences::IAPreferences()
{
    Fl_Preferences pPrefs(Fl_Preferences::USER, "com.matthiasm.iota", "IotaSlicer");

    Fl_Preferences main(pPrefs, "main");

    Fl_Preferences window(main, "window");

    window.get("x", pMainWindowX, -1);
    window.get("y", pMainWindowY, -1);
    window.get("w", pMainWindowW, 800);
    window.get("h", pMainWindowH, 600);

    Fl_Preferences filenames(main, "filenames");

    filenames.get("lastLoad", pLastLoadFilename, "", sizeof(pLastLoadFilename));
}


/**
 * Write the current setting to the preferences file.
 */
IAPreferences::~IAPreferences()
{
    flush();
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

    Fl_Preferences filenames(main, "filenames");

    filenames.set("lastLoad", pLastLoadFilename);

    pPrefs.flush();
}

