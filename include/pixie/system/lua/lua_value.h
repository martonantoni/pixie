#pragma once

class cLuaValue final
{
public:
    enum class IsRecursive { Yes, No };
    using cKey = std::variant<std::reference_wrapper<const std::string>, const char*, int>;
    struct cFunctionMustExist {};
private:
    std::shared_ptr<cLuaScript> mScript;
    int mReference = LUA_NOREF;
    bool mIsGlobalTable = false;
    template<class T> static void push(lua_State* L, const T& value);
    template<class T, class... Ts> static void push(lua_State* L, const T& value, const Ts&... values);
    template<class T> static T pop(cLuaScript& script, lua_State* L);
    std::shared_ptr<cConfig> toConfig_topTable(lua_State* L, IsRecursive isRecursive) const;
    void copy_(const cLuaValue& src);
    class cStateWithSelfCleanup;
    cStateWithSelfCleanup retrieveSelf() const; // returns nullptr on error, self will be at -1 on stack if successful
public:

    cLuaValue() = default;
    cLuaValue(std::shared_ptr<cLuaScript> script, int reference, bool isGlobalTable);
    cLuaValue(cLuaValue&& src);
    cLuaValue(const cLuaValue& src);
    ~cLuaValue();
    cLuaValue& operator=(cLuaValue&& src);
    cLuaValue& operator=(const cLuaValue& src);
    cLuaValue subTable(const std::string& key) const; // creates a new table if it doesn't exist
    int arraySize() const; // returns the length of the array, returns 0 if the value is not an array
// when the value is a table, accessing an element:
    template<class T = cLuaValue> std::optional<T> tryGet(const std::string& key) const;
    template<class T = cLuaValue, class D = T> T get(const std::string& key, D&& defaultValue) const;
    template<class T = cLuaValue> T get(const std::string& key) const;
    template<class T = cLuaValue> T get(int index) const; // array access. index >= 1
    template<class T> void set(const std::string& key, const T& value);
    template<class R, class... Args, class C> void registerFunction(const std::string& key, const C&& func);
    //template<class... Args> std::vector<cLuaValue> callFunction(const std::string& key, const Args&... args);
    //template<class... Args> std::vector<cLuaValue> callFunction(const std::string& key, const cFunctionMustExist&, const Args&... args);
    //template<class... Args> std::vector<cLuaValue> callMemberFunction(const std::string& key, const Args&... args);
    //template<class... Args> std::vector<cLuaValue> callMemberFunction(const std::string& key, const cFunctionMustExist&, const Args&... args);
    template<class T> bool isType(const std::string& key) const;
    void remove(const std::string& key);
    bool has(const std::string& key) const;
    template<class C> 
        requires std::is_invocable_v<C, const std::string&, const cLuaValue&> ||
                 std::is_invocable_v<C, const cLuaValue&>
    void forEach(const C& callable) const; 
// operating on the value itself:
    int toInt() const;
    double toDouble() const;
    bool isNumber() const;
    bool isString() const;
    std::string toString() const;
    bool isFunction() const;
    bool isTable() const;
    template<class C> void visit(C&& callable) const;
    template<class... Args> std::vector<cLuaValue> call(const Args&... args);
    template<class... Args> std::vector<cLuaValue> callMember(const std::string& functionName, const Args&... args);

    cLuaScript& script() { return *mScript; }
    const cLuaScript& script() const { return *mScript; }
// if the value is a table, we can create a config from it:
    std::shared_ptr<cConfig> toConfig(IsRecursive isRecursive = IsRecursive::Yes) const;
};

class cLuaValue::cStateWithSelfCleanup
{
    lua_State* mState = nullptr;
public:
    cStateWithSelfCleanup() = default;
    cStateWithSelfCleanup(lua_State* L) : mState(L) {}
    cStateWithSelfCleanup(cStateWithSelfCleanup&& src) : mState(src.mState) { src.mState = nullptr; }
    cStateWithSelfCleanup& operator=(cStateWithSelfCleanup&& src) { mState = src.mState; src.mState = nullptr; return *this; }
    cStateWithSelfCleanup(const cStateWithSelfCleanup&) = delete;
    cStateWithSelfCleanup& operator=(const cStateWithSelfCleanup&) = delete;
    ~cStateWithSelfCleanup() { if(mState) lua_pop(mState, 1); }
    lua_State* operator->() { return mState; }
    operator lua_State* () { return mState; }
    operator bool() { return mState != nullptr; }
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
    else if constexpr (std::is_same_v<T, cLuaValue*>)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, value->mReference);
    }
    else
    {
        ASSERT(false);
        // error
    }
}

template<class T, class... Ts> void cLuaValue::push(lua_State* L, const T& value, const Ts&... values)
{
    push(L, value);
    push(L, values...);
}

template<class T>
void cLuaValue::set(const std::string& key, const T& value)
{
    if(auto L = retrieveSelf())
    {
        lua_pushstring(L, key.c_str()); // Push the variable name onto the Lua stack
        push(L, value);
        lua_settable(L, -3); // Set the value in the table using the variable name
    }
}

template<class C> void cLuaValue::visit(C&& callable) const
{
    if(auto L = retrieveSelf())
    {
        switch (lua_type(L, -1))
        {
            case LUA_TNUMBER:
                if (lua_isinteger(L, -1))
                {
                    callable(static_cast<int>(lua_tointeger(L, -1)));
                }
                else
                {
                    callable(static_cast<double>(lua_tonumber(L, -1)));
                }
                break;
            case LUA_TSTRING:
                callable(std::string(lua_tostring(L, -1)));
                break;
            case LUA_TBOOLEAN:
                callable(lua_toboolean(L, -1) != 0);
                break;
            case LUA_TTABLE:
                callable(*this);
                break;
            default:
                callable(std::monostate());
                break;
        }
    }
    else
    {
        callable(std::monostate());
    }
}

template<class T>
T cLuaValue::pop(cLuaScript& script, lua_State* L)
{
    if constexpr (std::is_same_v<T, cLuaValue>)
    {
        return cLuaValue(script.shareSelf(), luaL_ref(L, LUA_REGISTRYINDEX), false);
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
    else if constexpr (std::is_same_v<T, bool>)
    {
        if (lua_isboolean(L, -1))
        {
            return lua_toboolean(L, -1) != 0;
        }
    }
    cLuaScript::error(L, "pop: type mismatch");
    return T{};
}

template<class T> std::optional<T> cLuaValue::tryGet(const std::string& key) const
{
    if(auto L = retrieveSelf())
    {
        lua_pushstring(L, key.c_str()); // Push the variable name onto the Lua stack
        lua_gettable(L, -2); // Get the value from the table using the variable name
        if(lua_isnil(L, -1))
        {
            lua_pop(L, 1); // Pop the nil value
            return {};
        }
        return pop<T>(*mScript, L);
    }
    return {};
}

template<class T, class D> T cLuaValue::get(const std::string& key, D&& defaultValue) const
{
    std::optional<T> value = tryGet<T>(key);
    return value ? *value : std::forward<D>(defaultValue);
}

template<class T> T cLuaValue::get(const std::string& key) const
{
    if (auto L = retrieveSelf())
    {
        lua_pushstring(L, key.c_str()); // Push the variable name onto the Lua stack
        lua_gettable(L, -2); // Get the value from the table using the variable name
        if (lua_isnil(L, -1))
        {
            lua_pop(L, 1); // Pop the nil value
            throw std::runtime_error("key not found");
        }
        return pop<T>(*mScript, L);
    }
    throw std::runtime_error("non-value");
}

template<class T> T cLuaValue::get(int index) const
{
    if (auto L = retrieveSelf())
    {
        lua_rawgeti(L, -1, index);
        return pop<T>(*mScript, L);
    }
    return {};
}

template<class T>
bool cLuaValue::isType(const std::string& variableName) const
{
    if (auto L = retrieveSelf())
    {
        lua_pushstring(L, variableName.c_str()); // Push the variable name onto the Lua stack
        lua_gettable(L, -2); // Get the value from the table using the variable name
        int type = lua_type(L, -1);
        bool result = false;
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
        else if constexpr (std::is_same_v<T, bool>)
        {
            result = type == LUA_TBOOLEAN;
        }

        lua_pop(L, 1); // Pop the value (table will be automatically popped when the function returns)
        return result;
    }
    return false;
}

template<class... Args> std::vector<cLuaValue> cLuaValue::call(const Args&... args)
{
    if (!mScript || mReference == LUA_NOREF)
    {
        throw std::runtime_error("not a valid lua object");
    }
    lua_State* L = mScript->state();
    int startSize = lua_gettop(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    if (!lua_isfunction(L, -1))
    {
        throw std::runtime_error("not a function");
    }
    int oldSize = lua_gettop(L);
    if constexpr (sizeof...(args) > 0)
    {
        push(L, args...);
    }
    int status = lua_pcall(L, sizeof...(args), LUA_MULTRET, 0);
    if (status != 0) 
    {
        const char* errorMessage = lua_tostring(L, -1);
        printf("Lua error: %s\n", errorMessage);
        lua_pop(L, lua_gettop(L));
        ASSERT(false);
        return {};
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

template<class... Args> std::vector<cLuaValue> cLuaValue::callMember(const std::string& functionName, const Args&... args)
{
    return get(functionName).call(*this, args...);
}

template<class R, class... Args, class C>
void cLuaValue::registerFunction(const std::string& key, const C&& func)
{
    if (auto L = retrieveSelf())
    {
        lua_pushstring(L, key.c_str()); // Push the variable name onto the Lua stack

        using FuncType = std::function<R(Args...)>;

        struct cCallableHolder : public cLuaScript::cUserDataBase
        {
            C mCallable;
            cLuaScript& mScript;
            cCallableHolder(cLuaScript& script, const C& callable)
                : mCallable(callable)
                , mScript(script) {}
            virtual ~cCallableHolder() = default;
        };
        cCallableHolder* holder = mScript->pushNewUserData<cCallableHolder>(*mScript, func);

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
        lua_pushcclosure(L, cFunction, 1); // the closure will have the mScript as an upvalue
        // the 1 means that the closure will have 1 upvalue

        lua_settable(L, -3); // Set the value in the table using the variable name
    }
}

template<class C> 
    requires std::is_invocable_v<C, const std::string&, const cLuaValue&> ||
             std::is_invocable_v<C, const cLuaValue&>
void cLuaValue::forEach(const C& callable) const
{
    if (auto L = retrieveSelf())
    {
        lua_pushnil(L); // Push the first key onto the stack
        while (lua_next(L, -2) != 0) // key at -2, value at -1
        {
            std::string key;
            bool skip = false;
            if (lua_type(L, -2) == LUA_TNUMBER)
            {
                key = std::to_string(static_cast<int>(lua_tointeger(L, -2) - 1));
            }
            else
            {
                key = cLuaScript::valueToString(L, -2);
                skip = mIsGlobalTable && cLuaScript::isGlobalInternalElement(key);
            }
            if (!skip)
            {
                cLuaValue value = pop<cLuaValue>(*mScript, L);
                if constexpr (std::is_invocable_v<C, const std::string&, const cLuaValue&>)
                {
                    callable(key, value);
                }
                else if constexpr (std::is_invocable_v<C, const cLuaValue&>)
                {
                    callable(value);
                }
            }
            else
            {
                lua_pop(L, 1); // Pop the value, but leave the key for the next iteration
            }
        }
    }
}
