//
//  IALua.h
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#ifndef IA_LUA_H
#define IA_LUA_H


struct lua_State;


class IALua
{
public:
    IALua();
    ~IALua();
    int dostring(const char *cmd);

    static int quit(lua_State *L);

    lua_State *L = nullptr;
};


#endif /* IA_LUA_H */
