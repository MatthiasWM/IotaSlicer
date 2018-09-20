//
//  IASetting.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_SETTING_H
#define IA_SETTING_H


#include <vector>
#include <functional>


struct Fl_Menu_Item;
class Fl_Input_Choice;
class Fl_Choice;
class Fl_Tree_Item;



class IASetting
{
public:
    IASetting();
    virtual ~IASetting();
    virtual void build() { }

    // write to preferences
    // read from preferences
    // FIXME: what actually happens whe the tree is cleared? Tree-Items deletd? Widget stay in Group? ???

    Fl_Menu_Item *dup(Fl_Menu_Item const*);
};


/**
 * Manage a setting that appears in a tree view.
 */
class IASettingFloatChoice : public IASetting
{
public:
    IASettingFloatChoice(const char *path, double &value, std::function<void()>&& cb, Fl_Menu_Item *menu);
    virtual ~IASettingFloatChoice() override;
    virtual void build() override;

    static void wCallback(Fl_Input_Choice *w, IASettingFloatChoice *d);

    char *pPath = nullptr;
    double &pValue;
    Fl_Menu_Item *pMenu = nullptr;
    std::function<void()> pCallback;
    void *pUserData = nullptr;
    Fl_Tree_Item *pTreeItem = nullptr;
    Fl_Input_Choice *pWidget = nullptr;
};


/**
 * Manage a setting that appears in a tree view.
 */
class IASettingChoice : public IASetting
{
public:
    IASettingChoice(const char *path, int &value, std::function<void()>&& cb, Fl_Menu_Item *menu);
    virtual ~IASettingChoice() override;
    virtual void build() override;

    static void wCallback(Fl_Choice *w, IASettingChoice *d);

    char *pPath = nullptr;
    int &pValue;
    Fl_Menu_Item *pMenu = nullptr;
    std::function<void()> pCallback;
    void *pUserData = nullptr;
    Fl_Tree_Item *pTreeItem = nullptr;
    Fl_Choice *pWidget = nullptr;
};


typedef std::vector<IASetting*> IASettingList;


#endif /* IA_SETTING_H */


