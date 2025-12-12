#pragma once

template<class T> concept cLuaAssignable = 
    std::is_same_v<std::decay_t<T>, cLuaObject> ||
    std::is_same_v<T, int> ||
    std::is_same_v<T, double> ||
    std::convertible_to<T, std::string_view> ||
    std::is_same_v<T, bool>;

template<class T> concept cLuaRetrievable =
    std::is_same_v<T, cLuaObject> ||
    std::is_same_v<T, int> ||
    std::is_same_v<T, double> ||
    std::is_same_v<T, std::string> ||
    std::is_same_v<T, bool>;

struct cLuaReturnVector {};

template<class T> concept cLuaReturnable =
    std::is_same_v<T, cLuaReturnVector> ||
    cLuaRetrievable<T>;

class cLuaObject final
{
public:
    enum class IsRecursive { Yes, No };
    using cKey = std::variant<std::string_view, int, cLuaObject>;
    struct cFunctionMustExist {};
    class iterator;

private:
    friend class cLuaState;
    std::shared_ptr<cLuaState> mState;
    int mReference = LUA_NOREF;
    bool mIsGlobalTable = false;
    template<class T> static void push(lua_State* L, const T& value);
    template<class T, class... Ts> static void push(lua_State* L, const T& value, const Ts&... values);
    template<class T> static T pop(cLuaState& state, lua_State* L);
    void copy_(const cLuaObject& src);
    static void retrieveItem(lua_State* L, const cKey& key);
    static void pushKey(lua_State* L, const cKey& key);
    class cStateWithSelfCleanup;
    cStateWithSelfCleanup retrieveSelf() const; // returns nullptr on error, self will be at -1 on stack if successful
    static int luaErrorFunction(lua_State* L);
    cLuaObject(std::shared_ptr<cLuaState> state, int reference, bool isGlobalTable);

public:
    cLuaObject() = default;
    cLuaObject(std::shared_ptr<cLuaState> state): mState(std::move(state)) {}
    cLuaObject(cLuaObject&& src);
    cLuaObject(const cLuaObject& src);
    ~cLuaObject();
    template<cLuaAssignable T> cLuaObject& operator=(T&& value);
    cLuaObject& operator=(cLuaObject&& src) { return operator=<cLuaObject&&>(std::move(src)); }
    cLuaObject& operator=(const cLuaObject& src) { return operator=<const cLuaObject&>(src); }
    cLuaObject subTable(const std::string& key) const; // creates a new table if it doesn't exist
    int arraySize() const; // returns the length of the array, returns 0 if the value is not an array

/////////////////////////////////////////////////////
// when the value is a table, accessing element(s):
    template<cLuaRetrievable T = cLuaObject> std::optional<T> tryGet(const cKey& key) const;
    template<cLuaRetrievable T = cLuaObject, class D = T> T get(const cKey& key, D&& defaultValue) const;
    template<cLuaRetrievable T = cLuaObject> T get(const cKey& key) const;
    void set(const cKey& key, const cLuaAssignable auto& value);
    template<class C> requires cCallableSignature<C>::available
    void registerFunction(const cKey& key, const C& func);
    void remove(const cKey& key);
    bool has(const cKey& key) const;
    template<class C> 
        requires std::is_invocable_v<C, const std::string&, const cLuaObject&> ||
                 std::is_invocable_v<C, const cLuaObject&>
    void forEach(const C& callable) const;  // old method, prefer begin()/end() for iteration
    iterator begin() const;
    iterator end() const;
/////////////////////////////////////////////////////
// operating on the value itself:
    template<cLuaRetrievable T> bool isType() const;
    int toInt() const;
    double toDouble() const;
    bool isNumber() const;
    bool isString() const;
    std::string toString() const;
    bool isFunction() const;
    bool isTable() const;
    bool isNil() const;
    template<class C> auto visit(const C& callable) const;
    template<cLuaReturnable... ReturnTs, cLuaAssignable... Args> auto call(const Args&... args);
    template<cLuaReturnable... ReturnTs, cLuaAssignable... Args> auto callMember(const cKey& functionKey, const Args&... args);

    cLuaState& state() { return *mState; }
    const cLuaState& state() const { return *mState; }
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

class cLuaObject::iterator
{
    cLuaObject mTable;
    cLuaObject mKey; 
    cLuaObject mValue;
    bool mEnd = true;
    void next();

public:
    using iterator_category = std::input_iterator_tag;
    using value_type = std::pair<cLuaObject, cLuaObject>;
    using difference_type = std::ptrdiff_t;
    using reference = value_type;
    using pointer = void;

    iterator() = default;
    explicit iterator(const cLuaObject& table);

    iterator& operator++();
    iterator operator++(int);
    value_type operator*() const { return { mKey, mValue }; }
    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const { return !(*this == other); }
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

void cLuaObject::set(const cKey& key, const cLuaAssignable auto& value)
{
    if(auto L = retrieveSelf())
    {
        pushKey(L, key);
        push(L, value);
        lua_settable(L, -3); // Table: -3, key: -2, value: -1, pops key and value
    }
}

template<class C> auto cLuaObject::visit(const C& callable) const
{
    if(auto L = retrieveSelf())
    {
        switch (lua_type(L, -1))
        {
            case LUA_TNUMBER:
                if (lua_isinteger(L, -1))
                {
                    return callable(static_cast<int>(lua_tointeger(L, -1)));
                }
                else
                {
                    return callable(static_cast<double>(lua_tonumber(L, -1)));
                }
                break;
            case LUA_TSTRING:
                return callable(std::string(lua_tostring(L, -1)));
                break;
            case LUA_TBOOLEAN:
                return callable(lua_toboolean(L, -1) != 0);
                break;
            case LUA_TTABLE:
                return callable(*this);
                break;
            default:
                return callable(std::monostate());
                break;
        }
    }
    else
    {
        return callable(std::monostate());
    }
}

template<class T>
T cLuaObject::pop(cLuaState& state, lua_State* L)
{
    if constexpr (std::is_same_v<T, cLuaObject>)
    {
        return cLuaObject(state.shareSelf(), luaL_ref(L, LUA_REGISTRYINDEX), false);
    }
    else
    {
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
                return std::string(lua_tostring(L, -1));
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
        else
        {
            static_assert(false);
        }
    }
    cLuaState::error(L, "pop: type mismatch");
}

template<cLuaRetrievable T> std::optional<T> cLuaObject::tryGet(const cKey& key) const
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

template<cLuaRetrievable T, class D> T cLuaObject::get(const cKey& key, D&& defaultValue) const
{
    std::optional<T> value = tryGet<T>(key);
    return value ? *value : std::forward<D>(defaultValue);
}

template<cLuaRetrievable T> T cLuaObject::get(const cKey& key) const
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

template<cLuaRetrievable T>
bool cLuaObject::isType() const
{
    if (auto L = retrieveSelf())
    {
        int type = lua_type(L, -1);
        bool result = false;
        if constexpr (std::is_same_v<T, int>)
        {
            return type == LUA_TNUMBER && lua_isinteger(L, -1);
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            return type == LUA_TNUMBER;
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            return type == LUA_TSTRING && lua_isstring(L, -1);
        }
        else if constexpr (std::is_same_v<T, cLuaObject>)
        {
            return type == LUA_TTABLE;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            return type == LUA_TBOOLEAN;
        }
    }
    return false;
}

template<cLuaReturnable... ReturnTs, cLuaAssignable... Args> auto cLuaObject::call(const Args&... args)
{
    if (!mState || mReference == LUA_NOREF)
    {
        throw std::runtime_error("not a valid lua object");
    }
    cLuaStackGuard guard(*mState);
    lua_State* L = mState->state();

    lua_pushcfunction(L, luaErrorFunction); // Push error handler

    lua_rawgeti(L, LUA_REGISTRYINDEX, mReference); // Retrieve the table from the registry
    if (!lua_isfunction(L, -1))
    {
        throw std::runtime_error("not a function");
    }
    if constexpr (sizeof...(args) > 0)
    {
        push(L, args...);
    }
    int status = lua_pcall(L, sizeof...(args), LUA_MULTRET, guard.oldTop() + 1);
    if (status != 0) 
    {
        const char* errorMessage = lua_tostring(L, -1);
        throw cLuaException(std::format("lua error: {}", errorMessage));
    }
    if constexpr (sizeof...(ReturnTs) == 0)
    {
        return;
    }
    else if constexpr (sizeof...(ReturnTs) == 1)
    {
        using FirstType = std::tuple_element_t<0, std::tuple<ReturnTs...>>;
        if constexpr (std::is_same_v<FirstType, void>)
        {
            return;
        }
        else if constexpr(std::is_same_v<FirstType, cLuaReturnVector>)
        {
            std::vector<cLuaObject> returnValues;
            auto numberOfReturnValues = lua_gettop(L) - guard.oldTop() - 1; // -1 for error handler
            returnValues.resize(numberOfReturnValues);
            for (int i = 0; i < numberOfReturnValues; ++i)
            {
                returnValues[numberOfReturnValues - i - 1] = 
                    cLuaObject(mState, luaL_ref(L, LUA_REGISTRYINDEX), false);
            }
            return returnValues;
        }
        else
        {
            auto returnValue = pop<FirstType>(*mState, L);
            return returnValue;
        }
    }
    else
    {
        auto returnValues = std::make_tuple(pop<ReturnTs>(*mState, L)...);
        return returnValues;
    }
}

template<cLuaReturnable... ReturnTs, cLuaAssignable... Args>
auto cLuaObject::callMember(const cKey& functionKey, const Args&... args)
{
    return get(functionKey).call<ReturnTs...>(*this, args...);
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

template<cLuaAssignable T> cLuaObject& cLuaObject::operator=(T&& value)
{
    if constexpr (std::is_same_v<std::decay_t<T>, cLuaObject>)
    {
        if (&value == this)
        {
            return *this;
        }
        if (mReference != LUA_NOREF)
        {
            luaL_unref(mState->state(), LUA_REGISTRYINDEX, mReference);
        }
        mReference = value.mReference;
        if constexpr (std::is_rvalue_reference_v<T>)
        {
            value.mReference = LUA_NOREF;
            mState = std::move(value.mState);
        }
        else
        {
            copy_(value);
        }
        mIsGlobalTable = value.mIsGlobalTable;
    }
    else
    {
        if (!mState)
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
    }
    return *this;
}

template<class C> requires cCallableSignature<C>::available
void cLuaObject::registerFunction(const cKey& key, const C& func)
{
    using Signature = cCallableSignature<C>;
    if (auto L = retrieveSelf())
    {
        if (!lua_istable(L, -1))
        {
            throw std::runtime_error("not a table");
        }
        pushKey(L, key);

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

        lua_CFunction cFunction = [](lua_State* L) -> int
            {
                cCallableHolder* holder = static_cast<cCallableHolder*>(lua_touserdata(L, lua_upvalueindex(1)));

                if constexpr (Signature::numberOfArguments > 0)
                {
                    using ARGS = Signature::DecayedArguments;
                    ARGS arguments = [&]<size_t... Indices>(std::index_sequence<Indices...>)
                    {
                        return std::make_tuple(pop<std::tuple_element_t<Indices, ARGS>>(holder->mState, L)...);
                    }(std::make_index_sequence<std::tuple_size_v<ARGS>>{});

                    if constexpr (std::is_same_v<typename Signature::ReturnType, void>)
                    {
                        std::apply([&](auto&&... args) 
                            { 
                                holder->mCallable(std::move(args)...); 
                            }, arguments);
                        return 0;
                    }
                    else
                    {
                        auto result = std::apply([&](auto&&... args) 
                            { 
                                return holder->mCallable(std::move(args)...); 
                            }, arguments);
                        push(L, result);
                        return 1;
                    }
                }
                else
                {
                    if constexpr (std::is_same_v<typename Signature::ReturnType, void>)
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
