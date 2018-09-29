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
class Fl_Tree;
class Fl_Tree_Item;


class IASetting
{
public:
    typedef enum { kProperty, kSetting } Type;
    IASetting(const char *path, const char *label);
    virtual ~IASetting();
    virtual void build(Fl_Tree*, Type) { }

    // write to preferences
    // read from preferences
    // FIXME: what actually happens whe the tree is cleared? Tree-Items deletd? Widget stay in Group? ???

    Fl_Menu_Item *dup(Fl_Menu_Item const*);

    char *pPath = nullptr;
    char *pLabel = nullptr;
    Fl_Tree_Item *pTreeItem = nullptr;
};


class IAFLLabel;

/**
 * Manage a setting that appears in a tree view.
 */
class IASettingLabel : public IASetting
{
public:
    IASettingLabel(const char *path, const char *label);
    virtual ~IASettingLabel() override;
    virtual void build(Fl_Tree*, Type) override;

    IAFLLabel *pWidget = nullptr;
};


class IAFLFloat;

/**
 * Manage a setting that appears in a tree view.
 */
class IASettingFloat : public IASetting
{
public:
    IASettingFloat(const char *path, const char *label, double &value,
                         const char *unit, std::function<void()>&& cb);
    virtual ~IASettingFloat() override;
    virtual void build(Fl_Tree*, Type) override;

    static void wCallback(IAFLFloat *w, IASettingFloat *d);

    double &pValue;
    char *pUnit = nullptr;
    std::function<void()> pCallback;
    IAFLFloat *pWidget = nullptr;
};


class IAFLFloatChoice;

/**
 * Manage a setting that appears in a tree view.
 */
class IASettingFloatChoice : public IASetting
{
public:
    IASettingFloatChoice(const char *path, const char *label, double &value,
                         const char *unit, std::function<void()>&& cb,
                         Fl_Menu_Item *menu);
    virtual ~IASettingFloatChoice() override;
    virtual void build(Fl_Tree*, Type) override;

    static void wCallback(IAFLFloatChoice *w, IASettingFloatChoice *d);

    double &pValue;
    char *pUnit = nullptr;
    std::function<void()> pCallback;
    Fl_Menu_Item *pMenu = nullptr;
    IAFLFloatChoice *pWidget = nullptr;
};


class IAFLChoice;

/**
 * Manage a setting that appears in a tree view.
 */
class IASettingChoice : public IASetting
{
public:
    IASettingChoice(const char *path, const char *label, int &value,
                    std::function<void()>&& cb, Fl_Menu_Item *menu);
    virtual ~IASettingChoice() override;
    virtual void build(Fl_Tree*, Type) override;

    static void wCallback(IAFLChoice *w, IASettingChoice *d);

    int &pValue;
    Fl_Menu_Item *pMenu = nullptr;
    std::function<void()> pCallback;
    IAFLChoice *pWidget = nullptr;
};


typedef std::vector<IASetting*> IASettingList;


#endif /* IA_SETTING_H */


