//
// IAVersioneer.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_VERSIONEER_H
#define IA_VERSIONEER_H


#include "userinterface/IAGUIMain.h"


class IAVersioneer : public IAVersioneerDialog
{
public:
    IAVersioneer();
    virtual ~IAVersioneer();
    void show();

protected:
    virtual void saveSettings() override;
    virtual void setProjectPath() override;
    virtual void selectProjectPath() override;
    virtual void majorPlus() override;
    virtual void minorPlus() override;
    virtual void buildPlus() override;
    virtual void setFLTKPath() override;
    virtual void selectFLTKPath() override;
    virtual void updateFLTK() override;
    virtual void setArchivePath() override;
    virtual void selectArchivePath() override;
    virtual void createArchive() override;
    virtual void gitTag() override;
    virtual void applySettings() override;

    void loadSettings();
    void applySettingsDoxyfile(const char *name);
    void applySettingsCpp(const char *name);
    void applySettingsHtml(const char *name);
    void touch(const char *name);
    void updatePlatformFiles();

    void system(const char *);
    void systemf(const char *, ...);

    Fl_Double_Window *pDialog = nullptr;
};


#endif /* IA_VERSIONEER_H */
