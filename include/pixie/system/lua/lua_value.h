#pragma once

template<class T> concept cAssignableToLuaValue = 
    std::is_same_v<std::decay_t<T>, int> ||
    std::is_same_v<std::decay_t<T>, double> ||
    std::convertible_to<std::decay_t<T>, std::string_view> ||
    std::is_same_v<std::decay_t<T>, bool>;

class cLuaObject final
{
public:
    enum class IsRecursive { Yes, No };
    using cKey = std::variant<std::string_view, int, cLuaObject>;
    struct cFunctionMustExist {};
private:
    std::shared_ptr<cLuaState> mState;
    int mReference = LUA_NOREF;
    bool mIsGlobalTable = false;
    template<class T> static void push(lua_State* L, const T& value);
    template<class T, class... Ts> static void push(lua_State* L, const T& value, const Ts&... values);
    template<class T> static T pop(cLuaState& state, lua_State* L);
    std::shared_ptr<cConfig> toConfig_topTable(lua_State* L, IsRecursive isRecursive) const;
    void copy_(const cLuaObject& src);
    static void retrieveItem(lua_State* L, const cKey& key);
    static void pushKey(lua_State* L, const cKey& key);
    class cStateWithSelfCleanup;
    cStateWithSelfCleanup retrieveSelf() const; // returns nullptr on error, self will be at -1 on stack if successful
public:

    cLuaObject() = default;
    cLuaObject(std::shared_ptr<cLuaState> state, int reference, bool isGlobalTable);
    cLuaObject(std::shared_ptr<cLuaState> state): mState(std::move(state)) {}
    cLuaObject(cLuaObject&& src);
    cLuaObject(const cLuaObject& src);
    ~cLuaObject();
    template<cAssignableToLuaValue T> cLuaObject& operator=(T&& value);
    cLuaObject& operator=(cLuaObject&& src);
    cLuaObject& operator=(const cLuaObject& src);
    cLuaObject subTable(const std::string& key) const; // creates a new table if it doesn't exist
    int arraySize() const; // returns the length of the array, returns 0 if the value is not an array
// when the value is a table, accessing an element:
    template<class T = cLuaObject> std::optional<T> tryGet(const cKey& key) const;
    template<class T = cLuaObject, class D = T> T get(const cKey& key, D&& defaultValue) const;
    template<class T = cLuaObject> T get(const cKey& key) const;    
    template<class T> void set(const cKey& key, const T& value);
    template<class R, class... Args, class C> void registerFunction(const cKey& key, const C&& func);
    template<class T> bool isType(const cKey& key) const;
    void remove(const cKey& key);
    bool has(const cKey& key) const;
    template<class C> 
        requires std::is_invocable_v<C, const std::string&, const cLuaObject&> ||
                 std::is_invocable_v<C, const cLuaObject&>
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
    struct ReturnVector {};
    template<class... ReturnTs, class... Args> auto call(const Args&... args);
    template<class... ReturnTs, class... Args> auto callMember(const cKey& functionKey, const Args&... args);

    cLuaState& state() { return *mState; }
    const cLuaState& state() const { return *mState; }
// if the value is a table, we can create a config from it:
    std::shared_ptr<cConfig> toConfig(IsRecursive isRecursive = IsRecursive::Yes) const;
};

class cLuaObject::cStateWithSelfCleanup
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
    void release() { mState = nullptr; }
};


template<class T> void cLuaObject::push(lua_State* L, const T& value)
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
    else if constexpr (std::convertible_to<T, std::string_view>)
    {
        lua_pushstring(L, std::string_view(value).data()); 
    }   
    else if constexpr (std::is_same_v<T, cLuaObject>)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, value.mReference);
    }
    else if constexpr (std::is_same_v<T, cLuaObject*>)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, value->mReference);
    }
    else
    {
        ASSERT(false);
        // error
    }
}

template<class T, class... Ts> void cLuaObject::push(lua_State* L, const T& value, const Ts&... values)
{
    push(L, value);
    push(L, values...);
}

template<class T>
void cLuaObject::set(const cKey& key, const T& value)
{
    if(auto L = retrieveSelf())
    {
        pushKey(L, key);
        push(L, value);
        lua_settable(L, -3); // Table: -3, key: -2, value: -1, pops key and value
    }
}

template<class C> void cLuaObject::visit(C&& callable) const
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
T cLuaObject::pop(cLuaState& state, lua_State* L)
{
    if constexpr (std::is_same_v<T, cLuaObject>)
    {
        return cLuaObject(state.shareSelf(), luaL_ref(L, LUA_REGISTRYINDEX), false);
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
    else if constexpr (std::is_same_v<T, cLuaState>)
    {
        return cLuaState(L);
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
        if (lua_isboolean(L, -1))
        {
            return lua_toboolean(L, -1) != 0;
        }
    }
    cLuaState::error(L, "pop: type mismatch");
    return T{};
}

template<class T> std::optional<T> cLuaObject::tryGet(const cKey& key) const
{
    if(auto L = retrieveSelf())
    {
        retrieveItem(L, key);
        if(lua_isnil(L, -1))
        {
            lua_pop(L, 1); // Pop the nil value
            return {};
        }
        return pop<T>(*mState, L);
    }
    return {};
}

template<class T, class D> T cLuaObject::get(const cKey& key, D&& defaultValue) const
{
    std::optional<T> value = tryGet<T>(key);
    return value ? *value : std::forward<D>(defaultValue);
}

template<class T> T cLuaObject::get(const cKey& key) const
{
    if (auto L = retrieveSelf())
    {
        retrieveItem(L, key);
        if (lua_isnil(L, -1))
        {
            lua_pop(L, 1); // Pop the nil value
            throw std::runtime_error("key not found");
        }
        return pop<T>(*mState, L);
    }
    throw std::runtime_error("non-value");
}

template<class T>
bool cLuaObject::isType(const cKey& key) const
{
    if (auto L = retrieveSelf())
    {
        retrieveItem(L, key);
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
        else if constexpr (std::is_same_v<T, cLuaObject>)
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

template<class... ReturnTs, class... Args> auto cLuaObject::call(const Args&... args)
{
    if (!mState || mReference == LUA_NOREF)
    {
        throw std::runtime_error("not a valid lua object");
    }
    lua_State* L = mState->state();
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
        lua_pop(L, startSize - lua_gettop(L));
        throw std::runtime_error(std::format("lua error: {}", errorMessage));
    }
    if constexpr (sizeof...(ReturnTs) == 0)
    {
        lua_pop(L, lua_gettop(L) - startSize);
        return;
    }
    else if constexpr (sizeof...(ReturnTs) == 1)
    {
        using FirstType = std::tuple_element_t<0, std::tuple<ReturnTs...>>;
        if constexpr (std::is_same_v<FirstType, void>)
        {
            lua_pop(L, lua_gettop(L) - startSize);
            return;
        }
        else if constexpr(std::is_same_v<FirstType, ReturnVector>)
        {
            std::vector<cLuaObject> returnValues;
            auto numberOfReturnValues = lua_gettop(L);
            returnValues.resize(numberOfReturnValues);
            for (int i = lua_gettop(L); i >= 1; --i) // at the bottom of the stack, our table is.
            {
                returnValues[i - 1] = cLuaObject(mState, luaL_ref(L, LUA_REGISTRYINDEX), false);
            }
            lua_pop(L, lua_gettop(L) - startSize);
            return returnValues;
        }
        else
        {
            auto returnValue = pop<FirstType>(*mState, L);
            lua_pop(L, lua_gettop(L) - startSize);
            return returnValue;
        }
    }
    else
    {
        auto returnValues = std::make_tuple(pop<ReturnTs>(*mState, L)...);
        return returnValues;
    }
}

template<class... ReturnTs, class... Args>
auto cLuaObject::callMember(const cKey& functionKey, const Args&... args)
{
    return get(functionKey).call<ReturnTs...>(*this, args...);
}

template<class R, class... Args, class C>
void cLuaObject::registerFunction(const cKey& key, const C&& func)
{
    if (auto L = retrieveSelf())
    {
        if (!lua_istable(L, -1))
        {
            throw std::runtime_error("not a table");
        }
        pushKey(L, key);

        using FuncType = std::function<R(Args...)>;

        struct cCallableHolder : public cLuaState::cUserDataBase
        {
            C mCallable;
            cLuaState& mState;
            cCallableHolder(cLuaState& state, const C& callable)
                : mCallable(callable)
                , mState(state) {}
            virtual ~cCallableHolder() = default;
        };
        cCallableHolder* holder = mState->pushNewUserData<cCallableHolder>(*mState, func);

        // Create a C function wrapper that calls the callable object
        lua_CFunction cFunction = [](lua_State* L) -> int
            {
                cCallableHolder* holder = static_cast<cCallableHolder*>(lua_touserdata(L, lua_upvalueindex(1)));

                if constexpr (sizeof...(Args) > 0)
                {
                    auto arguments = std::make_tuple(pop<std::remove_reference<Args>::type>(holder->mState, L)...);
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
        lua_pushcclosure(L, cFunction, 1); // the closure will have the mState as an upvalue
        // the 1 means that the closure will have 1 upvalue

        lua_settable(L, -3); // Set the value in the table using the variable name
    }
    else
    {
        throw std::runtime_error("not a valid lua object");
    }
}

template<class C> 
    requires std::is_invocable_v<C, const std::string&, const cLuaObject&> ||
             std::is_invocable_v<C, const cLuaObject&>
void cLuaObject::forEach(const C& callable) const
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
                key = cLuaState::valueToString(L, -2);
                skip = mIsGlobalTable && cLuaState::isGlobalInternalElement(key);
            }
            if (!skip)
            {
                cLuaObject value = pop<cLuaObject>(*mState, L);
                if constexpr (std::is_invocable_v<C, const std::string&, const cLuaObject&>)
                {
                    callable(key, value);
                }
                else if constexpr (std::is_invocable_v<C, const cLuaObject&>)
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

template<cAssignableToLuaValue T> cLuaObject& cLuaObject::operator=(T&& value)
{
    if(!mState)
    {
        throw std::runtime_error("not a valid lua object");
    }
    if (mReference != LUA_NOREF)
    {
        luaL_unref(mState->state(), LUA_REGISTRYINDEX, mReference);
    }
    if constexpr (std::is_same_v<std::decay_t<T>, int>)
    {
        lua_pushinteger(mState->state(), value);
    }
    else if constexpr (std::is_same_v<std::decay_t<T>, bool>)
    {
        lua_pushboolean(mState->state(), value);
    }
    else if constexpr (std::is_same_v<std::decay_t<T>, double>)
    {
        lua_pushnumber(mState->state(), value);
    }
    else if constexpr (std::convertible_to<std::decay_t<T>, std::string_view>)
    {
        lua_pushstring(mState->state(), std::string_view(value).data());
    }
    mReference = luaL_ref(mState->state(), LUA_REGISTRYINDEX);
    return *this;
}
