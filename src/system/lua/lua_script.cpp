#include "StdAfx.h"

const char* cLuaScript::userDataMetaTableName = "destructed_user_data";
std::vector<std::string> cLuaScript::globalTableInternalElements;

void cLuaScript::staticInit()
{
    auto script = std::make_shared<cLuaScript>();
    auto L = script->L;
    lua_pushglobaltable(L);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        // key: -2, value: -1
        std::string key = valueToString(L, -2);
        globalTableInternalElements.emplace_back(key);
        lua_pop(L, 1); // pop the value, keep the key
    }
    lua_pop(L, 1); // global table
    std::sort(ALL(globalTableInternalElements));
}

bool cLuaScript::isGlobalInternalElement(const std::string& key)
{
    return std::binary_search(ALL(globalTableInternalElements), key);
}

void cLuaScript::dumpStack(lua_State* L)
{
    printf("\n---- STACK START ----\n");
    int top = lua_gettop(L);
    for (int i = 1; i <= top; i++) 
    {
        printf("%d\t%s\t", i, luaL_typename(L, i));
        switch (lua_type(L, i))
        {
        case LUA_TNUMBER:
            printf("%g\n", lua_tonumber(L, i));
            break;
        case LUA_TSTRING:
            printf("%s\n", lua_tostring(L, i));
            break;
        case LUA_TBOOLEAN:
            printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
            break;
        case LUA_TNIL:
            printf("%s\n", "nil");
            break;
        case LUA_TTABLE:
        {
            printf("table:\n");
            lua_pushnil(L);
            while (lua_next(L, i) != 0) {
                // At this point, the key is at index -2 and the value is at index -1 on the stack

                // Check if the key is the first one
                if (lua_type(L, -2) == LUA_TSTRING || lua_type(L, -2) == LUA_TNUMBER) {
                    const char* key = lua_tostring(L, -2); // Get the key as a string
                    const char* value = lua_tostring(L, -1); // Get the value as a string

                    // Print the key-value pair
                    printf("key-value pair: %s = %s\n", key, value);

                }

                // Pop the value, but keep the key for the next iteration
                lua_pop(L, 1);
            }
            break;
        }

        default:
            printf("%p\n", lua_topointer(L, i));
            break;
        }
    }
    printf("---- STACK END ----\n");
    fflush(stdout);
}

int cLuaScript::panicHandler(lua_State* L) 
{
    const char* errorMessage = lua_tostring(L, -1); // Get the error message from the Lua stack
    printf("LUA PANIC: %s\n", errorMessage); // Print the error message
    return 0; // Return 0 to exit the application
}

cLuaScript::cLuaScript()
{
    L = luaL_newstate();
    luaL_openlibs(L);
    lua_atpanic(L, panicHandler);

    lua_getglobal(L, "package");
    lua_pushstring(L, "./?.lua");
    lua_setfield(L, -2, "path");
    lua_pop(L, 1); // Pop the package table off the stack


    luaL_newmetatable(L, userDataMetaTableName);
    lua_pushcfunction(L, gcUserData);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

cLuaScript::cLuaScript(lua_State* l)
    : L(l)
    , mIsOwningState(false)
{
}

cLuaScript::cLuaScript(cLuaScript&& src)
    : L(src.L)
    , mIsOwningState(src.mIsOwningState)
{
    src.L = nullptr;
}

cLuaScript& cLuaScript::operator=(cLuaScript&& src)
{
    if (&src == this)
        return *this;
    cLuaScript toDiscard(std::move(*this));
    std::swap(src.L, L);
    mIsOwningState = src.mIsOwningState;
    return *this;
}

cLuaScript::~cLuaScript()
{
    if (mIsOwningState && L)
    {
        lua_close(L);
    }
}

void cLuaScript::executeFile(const cPath& scriptPath)
{
    if (luaL_dofile(L, scriptPath.c_str()))
    {
        const char* errorMessage = lua_tostring(L, -1);
        printf("LUA Error: %s\n", errorMessage);
        fflush(stdout);
        DebugBreak();
        // handle error
    }
}

void cLuaScript::executeString(const std::string& script)
{
    if (luaL_dostring(L, script.c_str())) 
    {
        const char* errorMessage = lua_tostring(L, -1);
        printf("LUA Error: %s\n", errorMessage);
        fflush(stdout);
        DebugBreak();
        // Handle the error, if any
    }
}

int cLuaScript::gcUserData(lua_State* L)
{
    cUserDataBase* obj = static_cast<cUserDataBase*>(lua_touserdata(L, 1));
    obj->~cUserDataBase();
    return 0;
}

cLuaValue cLuaScript::globalTable()
{
    lua_pushglobaltable(L);
    return cLuaValue{ shared_from_this(), luaL_ref(L, LUA_REGISTRYINDEX), true };
}

cLuaValue cLuaScript::createTable()
{
    lua_newtable(L);
    return cLuaValue{ shared_from_this(), luaL_ref(L, LUA_REGISTRYINDEX), true };
}

int cLuaScript::stackSize() const
{
    return lua_gettop(L);
}

std::shared_ptr<cConfig> cLuaScript::stringToConfig(const std::string& scriptText)
{
    auto script = std::make_shared<cLuaScript>();
    script->executeString(scriptText);
    return script->globalTable().toConfig();
}

std::shared_ptr<cConfig> cLuaScript::fileToConfig(const cPath& scriptPath)
{
    auto script = std::make_shared<cLuaScript>();
    script->executeFile(scriptPath);
    return script->globalTable().toConfig();
}

void cLuaScript::error(lua_State* L, const std::string& message)
{
    lua_Debug ar;
    lua_getstack(L, 1, &ar);
    lua_getinfo(L, "Sl", &ar);
    std::string errorMessage = message + " at " + ar.short_src + ":" + std::to_string(ar.currentline);
    printf("%s\n", errorMessage.c_str());
    dumpStack(L);
    throw std::runtime_error(errorMessage);
}

std::string cLuaScript::configToScript(const cConfig& config, const std::string& ident)
{
    if (ident.empty() && config.isArray())
    {
        throw std::runtime_error("array on global level");
    }
    std::string script;
    config.visit([newLine = ident.empty() ? "\n"s : ",\n"s, &script, ident]
        (auto key, auto value)
        {
        // print out the key (it can be int or std::string)
            if constexpr (std::is_same_v<decltype(key), int>)
            {
                script += ident + "[" + std::to_string(key + 1) + "] = ";
            }
            else if constexpr (std::is_same_v<decltype(key), std::string>)
            {
                script += ident + key + " = ";
            }
            // print out the value(it can be int, double, bool, std::string, std::shared_ptr<cConfig>)
            if constexpr (std::is_same_v<decltype(value), int>)
            {
                script += std::to_string(value) + newLine;
            }
            else if constexpr (std::is_same_v<decltype(value), double>)
            {
                script += std::to_string(value) + newLine;
            }
            else if constexpr (std::is_same_v<decltype(value), bool>)
            {
                script += (value ? "true" : "false") + newLine;
            }
            else if constexpr (std::is_same_v<decltype(value), std::string>)
            {
                script += "\"" + value + "\"" + newLine;
            }
            else if constexpr (std::is_same_v<decltype(value), std::shared_ptr<cConfig>>)
            {
                script += "\n{\n";
                script += configToScript(*value, ident + "  ");
                script += ident + "}" + newLine;
            }
        });
    return script;
}