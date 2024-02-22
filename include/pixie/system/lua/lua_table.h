#pragma once

class cLuaValue final
{
public:
    enum class IsRecursive { Yes, No };
    using cKey = std::variant<std::reference_wrapper<const std::string>, const char*, int>;
private:
    std::shared_ptr<cLuaScript> mScript;
    int mReference = LUA_NOREF;
    bool mIsGlobalTable = false;
    template<class T> static void push(lua_State* L, const T& value);
    template<class T, class... Ts> static void push(lua_State* L, const T& value, const Ts&... values);
    template<class T> static T pop(std::shared_ptr<cLuaScript> script, lua_State* L);
    tIntrusivePtr<cConfig> toConfig_topTable(lua_State* L, IsRecursive isRecursive) const;
    void copy_(const cLuaValue& src);
    static void retriveWithKey(lua_State* L, cKey key);
public:

    cLuaValue() = default;
    cLuaValue(std::shared_ptr<cLuaScript> script, int reference, bool isGlobalTable);
    cLuaValue(cLuaValue&& src);
    cLuaValue(const cLuaValue& src);
    ~cLuaValue();
    cLuaValue& operator=(cLuaValue&& src);
    cLuaValue& operator=(const cLuaValue& src);
    cLuaValue subTable(const std::string& key) const; // creates a new table if it doesn't exist
// when the value is a table, accessing an element:
    template<class T> T get(const std::string& key) const;
    template<class T> T get(int index) const; // array access. index >= 1
    template<class T> void set(const std::string& key, const T& value);
    template<class R, class... Args, class C> void registerFunction(const std::string& key, const C&& func);
    template<class... Args> std::vector<cLuaValue> callFunction(const std::string& key, Args... args);
    template<class T> bool isType(const std::string& key) const;
// operating on the value itself:
    int toInt() const;
    std::string toString() const;
    bool isFunction() const;
    template<class... Args> std::vector<cLuaValue> call(Args... args);

    cLuaScript& script() { return *mScript; }
    const cLuaScript& script() const { return *mScript; }
// if the value is a table, we can create a config from it:
    tIntrusivePtr<cConfig> toConfig(IsRecursive isRecursive = IsRecursive::Yes) const;
};

template<class T> void cLuaValue::push(lua_State* L, const T& value)
{
    if constexpr (std::is_same_v<T, int>)
    {
        lua_pushinteger(L, value);
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
        lua_pushboolean(L, value);
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        lua_pushnumber(L, value);
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        lua_pushstring(L, value.c_str()); 
    }   
    else if constexpr (std::is_same_v<T, cLuaValue>)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, value.mReference);
    }
    else
    {
        // error
    }
}

template<class T, class... Ts> void cLuaValue::push(lua_State* L, const T& value, const Ts&... values)
{
    push(L, value);
    push(L, values...);
}

template <typename T>
void cLuaValue::set(const std::string& key, const T& value)
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return;
    }
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    lua_pushstring(L, key.c_str()); // Push the variable name onto the Lua stack

    push(L, value);

    lua_settable(L, -3); // Set the value in the table using the variable name
    lua_pop(L, 1); // Pop the table from the stack
}

template<class T>
T cLuaValue::pop(std::shared_ptr<cLuaScript> script, lua_State* L)
{
    if constexpr (std::is_same_v<T, cLuaValue>)
    {
        return cLuaValue(std::move(script), luaL_ref(L, LUA_REGISTRYINDEX), false);
    }
    FINALLY([&]() { lua_pop(L, 1); });
    if constexpr (std::is_same_v<T, int>)
    {
        if (lua_isinteger(L, -1))
        {
            return static_cast<T>(lua_tointeger(L, -1));
        }
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        if (lua_isnumber(L, -1))
        {
            return static_cast<T>(lua_tonumber(L, -1));
        }
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        if (lua_isstring(L, -1))
        {
            return static_cast<T>(lua_tostring(L, -1));
        }
    }
    else if constexpr (std::is_same_v<T, cLuaScript>)
    {
        return cLuaScript(L);
    }

    // handle error
    return T{};
}

template<typename T> T cLuaValue::get(const std::string& key) const
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return {};
    }
    lua_State* L = mScript->state();

    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    lua_pushstring(L, key.c_str()); // Push the variable name onto the Lua stack
    lua_gettable(L, -2); // Get the value from the table using the variable name

    FINALLY([L]() { lua_pop(L, 1); }); 
    return pop<T>(mScript, L);
}

template<class T> T cLuaValue::get(int index) const
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return {};
    }
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry

    lua_rawgeti(L, -1, index);
    FINALLY([L]() { lua_pop(L, 1); });
    return pop<T>(mScript, L);
}


template <typename T>
bool cLuaValue::isType(const std::string& variableName) const
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return false;
    }
    lua_State* L = mScript->state();
    bool result = false;

    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    lua_pushstring(L, variableName.c_str()); // Push the variable name onto the Lua stack
    cLuaScript::dumpStack(L);
    lua_gettable(L, -2); // Get the value from the table using the variable name
    int type = lua_type(L, -1);

    if constexpr (std::is_same_v<T, int>)
    {
        result = type == LUA_TNUMBER && lua_isinteger(L, -1);
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        result = type == LUA_TNUMBER;
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        result = type == LUA_TSTRING && lua_isstring(L, -1);
    }
    else if constexpr (std::is_same_v<T, cLuaValue>)
    {
        result = type == LUA_TTABLE;
    }

    lua_pop(L, 2); // Pop the value and the table from the stack
    return result;
}

template<class... Args> std::vector<cLuaValue> cLuaValue::call(Args... args)
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return {};
    }
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    if (!lua_isfunction(L, -1))
    {
        // handle error
        return {};
    }
    if constexpr (sizeof...(args) > 0)
    {
        push(L, args...);
    }
    int status = lua_pcall(L, sizeof...(args), LUA_MULTRET, 0);
    if (status != 0) 
    {
        const char* errorMessage = lua_tostring(L, -1);
        printf("Lua error: %s\n", errorMessage);
        // Handle the error, such as logging or displaying the error message
        // ...
    }
    std::vector<cLuaValue> returnValues;
    returnValues.reserve(lua_gettop(L));
    for (int i = lua_gettop(L); i >= 1; --i) // at the bottom of the stack, our table is.
    {
        returnValues.emplace_back(mScript, luaL_ref(L, LUA_REGISTRYINDEX), false);
    }
    lua_pop(L, lua_gettop(L));
    return returnValues;
}

template<class... Args> std::vector<cLuaValue> cLuaValue::callFunction(const std::string& key, Args... args)
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return {};
    }
    cLuaValue function = get<cLuaValue>(key);
    return function.call(args...);
}

template<class R, class... Args, class C>
void cLuaValue::registerFunction(const std::string& key, const C&& func)
{
    if (!mScript || mReference == LUA_NOREF)
    {
        return;
    }
    lua_State* L = mScript->state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    lua_pushstring(L, key.c_str()); // Push the variable name onto the Lua stack

    using FuncType = std::function<R(Args...)>;

    struct cCallableHolder : public cLuaScript::cUserDataBase
    {
        C mCallable;
        std::shared_ptr<cLuaScript> mScript;
        cCallableHolder(std::shared_ptr<cLuaScript> script, const C& callable) 
            : mCallable(callable)
            , mScript(std::move(script)) {}
        virtual ~cCallableHolder() = default;
    };
    cCallableHolder* holder = mScript->pushNewUserData<cCallableHolder>(mScript, func);

    // Create a C function wrapper that calls the callable object
    lua_CFunction cFunction = [](lua_State* L) -> int
    {
        cCallableHolder* holder = static_cast<cCallableHolder*>(lua_touserdata(L, lua_upvalueindex(1)));

        if constexpr (sizeof...(Args) > 0)
        {
            auto arguments = std::make_tuple(pop<std::remove_reference<Args>::type>(holder->mScript, L)...);
            if constexpr (std::is_same_v<R, void>)
            {
                std::apply(holder->mCallable, arguments);
                return 0;
            }
            else
            {
                auto result = std::apply(holder->mCallable, arguments);
                push(L, result);
                return 1;
            }
        }
        else
        {
            if constexpr (std::is_same_v<R, void>)
            {
                (holder->mCallable)();
                return 0;
            }
            else
            {
                auto result = (holder->mCallable)();
                push(L, result);
                return 1;
            }
        }
    };
    lua_pushcclosure(L, cFunction, 1);

    lua_settable(L, -3); // Set the value in the table using the variable name
    lua_pop(L, 1); // Pop the table from the stack
}