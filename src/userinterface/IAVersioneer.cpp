//
// IAVersioneer.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//


#include "IAVersioneer.h"


/**
 * \todo copy newly built FLTK binaries automatically:
 *      /Users/matt/dev/fltk-1.4.svn/build/Xcode/bin/Release/fluid.app
 * \todo add terminal view when running system commands to see output
 * \todo path setting and "release" button to create distributables
 */

void IAVersioneer::loadSettings()
{
    int v;
    char buf[2048];

    Fl_Preferences pref(Fl_Preferences::USER, "iota.matthiasm.com", "versioneer");
    pref.get("path", buf, "/Users/matt/dev/IotaSlicer/", 2046);
    wBasePath->value(buf);
    Fl_Preferences vers( pref, "version");
    vers.get("major", v, 0);
    sprintf(buf, "%d", v);
    wMajor->value(buf);
    vers.get("minor", v, 0);
    sprintf(buf, "%d", v);
    wMinor->value(buf);
    vers.get("build", v, 0);
    sprintf(buf, "%d", v);
    wBuild->value(buf);
    vers.get("class", buf, "-", 8);
    wClass->value(buf);

    const char *name = "README.md";

    char filename[2048];
    sprintf(filename, "%s/%s", wBasePath->value(), name);
    FILE *f = fopen(filename, "rb");
    if (f) {
        while (!feof(f)) {
            fgets(buf, 1024, f);
            if (feof(f)) break;
            char *a = strstr(buf, "<!--[ver-->");
            char *b = strstr(buf, "<!--]-->");
            if (a!=nullptr && b!=nullptr) {
                *b = 0;
                int maj, min, bld;
                char cls[80];
                int cc = sscanf(a+11, "v%d.%d.%d%s", &maj, &min, &bld, cls);
                if (cc>0) {
                    sprintf(buf, "%d", maj);
                    wMajor->value(buf);
                }
                if (cc>1) {
                    sprintf(buf, "%d", min);
                    wMinor->value(buf);
                }
                if (cc>2) {
                    sprintf(buf, "%d", bld);
                    wBuild->value(buf);
                }
                if (cc>3) {
                    wClass->value(cls);
                }
                break;
            }
        }
        fclose(f);
        printf("Versioneer: found current version in %s\n", filename);
    } else {
        perror(filename);
    }
}

void IAVersioneer::saveSettings()
{
    Fl_Preferences pref(Fl_Preferences::USER, "iota.matthiasm.com", "versioneer");
    pref.set("path", wBasePath->value());
    Fl_Preferences vers( pref, "version");
    vers.set("major", atoi(wMajor->value()));
    vers.set("minor", atoi(wMinor->value()));
    vers.set("build", atoi(wBuild->value()));
    vers.set("class", wClass->value());
}

void IAVersioneer::setProjectPath()
{
    Fl_Preferences pref(Fl_Preferences::USER, "iota.matthiasm.com", "versioneer");
    pref.set("path", wBasePath->value());
}

void IAVersioneer::selectProjectPath()
{
    const char *path = fl_dir_chooser("Choose the Iota Base Directory",
                                      wBasePath->value(),
                                      0);
    if (path) {
        wBasePath->value(path);
        wBasePath->do_callback();
    }
}


void IAVersioneer::majorPlus()
{
    char buf[80];
    int vv = atoi(wMajor->value());
    vv++;
    sprintf(buf, "%d", vv);
    wMajor->value(buf);
    wMinor->value("0");
    wBuild->value("0");
}


void IAVersioneer::minorPlus()
{
    char buf[80];
    int vv = atoi(wMinor->value());
    vv++;
    sprintf(buf, "%d", vv);
    wMinor->value(buf);
    wBuild->value("0");
}


void IAVersioneer::buildPlus()
{
    char buf[80];
    int vv = atoi(wBuild->value());
    vv++;
    sprintf(buf, "%d", vv);
    wBuild->value(buf);
}


void IAVersioneer::applySettingsHtml(const char *name) {
    char filename[2048];
    char out[2048];
    char buf[2048];
    sprintf(filename, "%s/%s", wBasePath->value(), name);
    strcpy(out, filename);
    strcat(out, "~");
    FILE *fOut = fopen(out, "wb");
    if (!fOut) {
        perror(out);
        return;
    }
    FILE *f = fopen(filename, "rb");
    if (f) {
        int n = 0;
        while (!feof(f)) {
            fgets(buf, 1024, f);
            if (feof(f)) break;
            char *a = strstr(buf, "<!--[ver-->");
            char *b = strstr(buf, "<!--]-->");
            if (a!=nullptr && b!=nullptr) {
                char *c = strdup(b);
                sprintf(a,
                        "<!--[ver-->v%d.%d.%d%s%s",
                        atoi(wMajor->value()),
                        atoi(wMinor->value()),
                        atoi(wBuild->value()),
                        wClass->value(),
                        c
                        );
                free(c);
                n++;
            }
            fputs(buf, fOut);
        }
        fclose(f);
        fclose(fOut);
        ::remove(filename);
        ::rename(out, filename);
        printf("Versioneer: %d replacements in %s\n", n, filename);
    } else {
        perror(filename);
    }
}

void IAVersioneer::applySettingsCpp(const char *name)
{
    char filename[2048];
    char out[2048];
    char buf[2048];
    sprintf(filename, "%s/%s", wBasePath->value(), name);
    strcpy(out, filename);
    strcat(out, "~");
    FILE *fOut = fopen(out, "wb");
    if (!fOut) {
        perror(out);
        return;
    }
    FILE *f = fopen(filename, "rb");
    if (f) {
        int n = 0;
        while (!feof(f)) {
            fgets(buf, 1024, f);
            if (feof(f)) break;
            char *a = strstr(buf, "/*[ver*/");
            char *b = strstr(buf, "/*]*/");
            if (a!=nullptr && b!=nullptr) {
                char *c = strdup(b);
                sprintf(a,
                        "/*[ver*/\"v%d.%d.%d%s\"%s",
                        atoi(wMajor->value()),
                        atoi(wMinor->value()),
                        atoi(wBuild->value()),
                        wClass->value(),
                        c
                        );
                free(c);
                n++;
            }
            fputs(buf, fOut);
        }
        fclose(f);
        fclose(fOut);
        ::remove(filename);
        ::rename(out, filename);
        printf("Versioneer: %d replacements in %s\n", n, filename);
    } else {
        perror(filename);
    }
}

void IAVersioneer::applySettingsDoxyfile(const char *name)
{
    char filename[2048];
    char out[2048];
    char buf[2048];
    sprintf(filename, "%s/%s", wBasePath->value(), name);
    strcpy(out, filename);
    strcat(out, "~");
    FILE *fOut = fopen(out, "wb");
    if (!fOut) {
        perror(out);
        return;
    }
    FILE *f = fopen(filename, "rb");
    if (f) {
        int n = 0;
        while (!feof(f)) {
            fgets(buf, 1024, f);
            if (feof(f)) break;
            if (strncmp(buf, "PROJECT_NUMBER         = ", 25)==0) {
                sprintf(buf,
                        "PROJECT_NUMBER         = %d.%d.%d%s\n",
                        atoi(wMajor->value()),
                        atoi(wMinor->value()),
                        atoi(wBuild->value()),
                        wClass->value()
                        );
                n++;
            }
            fputs(buf, fOut);
        }
        fclose(f);
        fclose(fOut);
        ::remove(filename);
        ::rename(out, filename);
        printf("Versioneer: %d replacements in %s\n", n, filename);
    } else {
        perror(filename);
    }
}


void IAVersioneer::touch(const char *name)
{
    char filename[2048];
    sprintf(filename, "%s/%s", wBasePath->value(), name);
    ::utime(filename, nullptr);
}


void IAVersioneer::updatePlatformFiles()
{
#ifdef __APPLE__
    char rev[80];
    strcpy(rev, "0000000");
    FILE *f = popen("/usr/bin/git rev-parse --short HEAD", "r");
    fgets(rev, 78, f);
    fclose(f);
    rev[7] = 0;

    char filename[2048];
    sprintf(filename, "%s/platforms/MacOS/Info.plist", wBasePath->value());
    char cmd[2*2048];
    sprintf(cmd,
            "/usr/libexec/PlistBuddy -c \"Set :CFBundleVersion %s\" %s",
            rev, filename);
    fl_system(cmd);
    sprintf(cmd,
            "/usr/libexec/PlistBuddy -c \"Set :CFBundleShortVersionString %d.%d.%d%s\" %s",
            atoi(wMajor->value()),
            atoi(wMinor->value()),
            atoi(wBuild->value()),
            wClass->value(),
            filename);
    fl_system(cmd);
#endif
}


void IAVersioneer::applySettings()
{
    fl_chdir(wBasePath->value());
    applySettingsHtml("README.md");
    applySettingsHtml("html/helpAbout.html");
    applySettingsHtml("html/helpLicenses.html");
    applySettingsHtml("html/index.html");
    applySettingsCpp("src/Iota.cpp");
    applySettingsDoxyfile("Doxyfile");
    updatePlatformFiles();
    touch("userinterface/IAGUIMain.fl");
}


void IAVersioneer::gitTag()
{
    char buf[2048];
    sprintf(buf, "cd %s; /usr/bin/git tag v%d.%d.%d%s",
            wBasePath->value(),
            atoi(wMajor->value()),
            atoi(wMinor->value()),
            atoi(wBuild->value()),
            wClass->value());
    fl_system(buf);
    sprintf(buf, "cd %s; /usr/bin/git push --tags",
            wBasePath->value());
    fl_system(buf);
}


IAVersioneer::IAVersioneer()
{
}


IAVersioneer::~IAVersioneer()
{
}


void IAVersioneer::show()
{
    if (!pDialog)
        pDialog = createWindow();
    if (pDialog) {
        loadSettings();
        pDialog->show();
    }
}



