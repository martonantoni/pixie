#pragma warning(disable: 4800) // forcing value to BOOL 'true' or 'false' (performance warning)
#pragma warning(disable: 4244)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list -- MakeFunctor there..
#pragma warning(disable: 4267) // size_t -> int conversion warning

#define _CRT_SECURE_NO_WARNINGS
#define WINVER 0x0A00  // Windows 10
#define _WIN32_WINNT 0x0A00
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define NOMINMAX

#include <WinSock2.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#undef min
#undef max
#define ALL(container) container.begin(), container.end()

#include <cstdint>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <numeric>
#include <deque>
#include <memory>
#include <ranges>
#include <source_location>
#include <functional>
#include <variant>
#include <array>
#include <random>
#include <iterator>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <filesystem>
#include <type_traits>
#include <typeindex>
#include <concepts>
#include <optional>
#include <any>

#include "fmt/core.h"
#include "fmt/printf.h"

using namespace std::string_literals; // enables s-suffix for std::string literals

//#ifndef _DEBUG
//#define NO_CUSTOM_CURSORS
//#endif

using std::make_unique;
using std::unique_ptr;

#include "stdExtension.h"

#define USE_DROP_INSTEAD_DELETE(ClassName) \
	namespace std \
	{ \
		template<> class default_delete<ClassName> \
		{ \
		public: \
			void operator()(ClassName *Object) const \
			{ \
				Object->Drop(); \
			} \
		}; \
	}

#define USE_DROP_INSTEAD_DELETE_PARENT(ClassName, ParentClassName) \
	namespace std \
	{ \
		template<> class default_delete<ClassName>: public default_delete<ParentClassName> \
		{ \
		public: \
			void operator()(ClassName *Object) const \
			{ \
				Object->Drop(); \
			} \
		}; \
	}

template<class T>
constexpr T sum(T x) { return x; }
template<class T, class... Tail>
constexpr T sum(T x, Tail... tail)
{
    return x + sum(tail...);
}

template<class T> auto ExtractPtrsFromSmartPtrs(T &SmartPtrs)
{
	std::vector<decltype(SmartPtrs.front().get())> RawPointers;
	RawPointers.reserve(SmartPtrs.size());
	for(auto &SmartPtr: SmartPtrs)
		RawPointers.emplace_back(SmartPtr.get());
	return RawPointers;
}

template<class T, class P> auto ExtractPtrsFromSmartPtrs_const_if(T &SmartPtrs, P Condition)
{
	std::vector<typename std::add_const<typename std::remove_reference<decltype(*SmartPtrs.front().get())>::type>::type *> RawPointers;
	RawPointers.reserve(SmartPtrs.size());
	for(auto &SmartPtr: SmartPtrs)
		if(Condition(*SmartPtr))
			RawPointers.emplace_back(SmartPtr.get());
	return RawPointers;
}

template<class T> class tIgnoreCopy: public T
{
public:
	tIgnoreCopy(const tIgnoreCopy &) {}
	tIgnoreCopy &operator=(const tIgnoreCopy &) {}

	tIgnoreCopy()=default;
	tIgnoreCopy(tIgnoreCopy &&)=default;
	tIgnoreCopy &operator=(tIgnoreCopy &&)=default;
};

using QWORD = uint64_t;
using ll = long long;
using cIntVector = std::vector<int>;
using cIndexVector = std::vector<size_t>;

class cLog;

#define OWNERSHIP
#ifndef INOUT
	#define INOUT
#endif

#pragma warning(error:4061)

enum class eCallbackType 
{ 
	Normal, 
	Wait, 
	NoImmediate, 
	WithNext 
};

class cBoolGuard
{
	bool &mGuardedValue;
public:
	cBoolGuard(bool &Guarded): mGuardedValue(Guarded) { mGuardedValue=true; }
	~cBoolGuard() { mGuardedValue=false; }
};

template<class T> class tCachedValue
{
	mutable T mValue;
	mutable unsigned int mCacheValidityCounter=~0u;
public:
	template<class U>
	T GetValue(int CacheValidityCounter, U CalculatorFunction) const
	{
		if(CacheValidityCounter!=mCacheValidityCounter)
		{
			mCacheValidityCounter=CacheValidityCounter;
			mValue=CalculatorFunction();
		}
		return mValue;
	}
};

template<class T> class Finalizer final
{
public:
    Finalizer(T&& toBeCalled)
        : mToBeCalled(std::forward<T>(toBeCalled)) {}

    ~Finalizer() {
		mToBeCalled();
    }
private:
    T mToBeCalled;
};

template<class T> Finalizer<T> finalizer(T&& toBeCalled)
{
	return Finalizer<T>(std::forward<T>(toBeCalled));
}

#define FINALLY(callable) auto finalizer##__LINE__ = finalizer(callable);

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

template<typename... Args>
constexpr bool NoneOfType()
{
    if constexpr (sizeof...(Args) == 0) 
	{
        return true;
    }
    else
	{
        return (!std::is_same_v<Args, int> && ...);
    }
}

#include "x64Common.h"
#include "DebugAssert.h"
#include "CommonInclude.h"
#include "utf8.h"
#include "DefaultValue.h"
#include "IntrusiveList.h"
#include "IntrusivePtr.h"
#include "DestroyTrap.h"
#include "RegisteredID.h"
#include "RegisteredObjects.h"
#include "singleton.h"
#include "DataHolder.h"
#include "CallableList2.h"
#include "Factory.h"
#include "MemoryStream.h"
#include "FormatThousands.h"
#include "StringList.h"
#include "ResourceLocation.h"
#include "LocatableResource.h"
#include "SharedValue/i_SharedValue.h"

#include "ProgramDirector.h"
#include "ProgramArguments.h"

#include "FileHandling/i_FileHandling.h"

#include "TodayString.h"
#include "ProgramTitle.h"

#include "Thread/i_thread.h"

#include "DropGuarded.h"
#include "Configuration/i_configuration.h"
#include "lua/i_lua.h"

#include "CommonUtilityFunctions.h"

#include "Log/i_log.h"
#include "EventSystem/i_EventSystem.h"

#include "Random.h"

#include "Point.h"

class cLogPerformance_Guard
{
	std::chrono::time_point<std::chrono::high_resolution_clock> mStartTime=std::chrono::high_resolution_clock::now();
	const char *mName;
public:
	cLogPerformance_Guard(const char *Name): mName(Name) {}
	~cLogPerformance_Guard()
	{
		auto EndTime=std::chrono::high_resolution_clock::now();
		auto Elapsed=std::chrono::duration_cast<std::chrono::nanoseconds>(EndTime-mStartTime);
		MainLog->Log("Elapsed %llu ns in %s\n", Elapsed.count(), mName);
	}
};

class cPrintfPerformance_Guard
{
    std::chrono::time_point<std::chrono::high_resolution_clock> mStartTime = std::chrono::high_resolution_clock::now();
    const char* mName;
public:
	cPrintfPerformance_Guard(const char* Name) : mName(Name) {}
    ~cPrintfPerformance_Guard()
    {
        auto EndTime = std::chrono::high_resolution_clock::now();
        auto Elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(EndTime - mStartTime);
        printf("Elapsed %llu ms in %s\n", Elapsed.count() / 1'000'000, mName);
    }
};


#include "network/i_Network.h"