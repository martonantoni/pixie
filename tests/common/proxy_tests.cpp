#include <gtest/gtest.h>

#include "pixie/system/CommonInclude_System.h"

/*
template<class T, size_t Size> class tSharableArrayBase
{
protected:
    struct cControlBlock final
    {
        std::atomic<size_t> mRefCount = 1;
        std::array<T, Size> mArray;
    };
    cControlBlock* mControlBlock = nullptr;
    const T& operator[](size_t index) const
    {
        return mControlBlock->mArray[index];
    }
    operator bool() const
    {
        return mControlBlock != nullptr;
    }
    bool operator!() const
    {
        return mControlBlock == nullptr;
    }
};

template<class T, size_t Size> class tShareableArray final : protected tSharableArrayBase<T, Size>
{
    using cControlBlock = typename tSharableArrayBase<T, Size>::cControlBlock;
    friend class tShareableArray<const T, Size>;
public:
    tShareableArray();
    ~tShareableArray();
    tShareableArray(tShareableArray&& other);
    tShareableArray& operator=(tShareableArray&& other);
    tShareableArray(const tShareableArray& other) = delete;
    tShareableArray& operator=(const tShareableArray& other) = delete;

    using tSharableArrayBase<T, Size>::operator[];
    using tSharableArrayBase<T, Size>::operator bool;
    using tSharableArrayBase<T, Size>::operator!;

    T& operator[](size_t index)
    {
        return this->mControlBlock->mArray[index];
    }
    auto range() 
    {         
        return std::views::all(this->mControlBlock->mArray);
    }
};

template<class T, size_t Size> tShareableArray<T, Size>::tShareableArray()
{
    this->mControlBlock = new cControlBlock;
}

template<class T, size_t Size> tShareableArray<T, Size>::~tShareableArray()
{
    delete this->mControlBlock;
}

template<class T, size_t Size> tShareableArray<T, Size>::tShareableArray(tShareableArray&& other)
{
    this->mControlBlock = other.mControlBlock;
    other.mControlBlock = nullptr;
}

template<class T, size_t Size> tShareableArray<T, Size>& tShareableArray<T, Size>::operator=(tShareableArray&& other)
{
    if(this == &other)
    {
        return *this;
    }
    tShareableArray<T, Size> toDiscard(std::move(*this));
    std::swap(this->mControlBlock, other.mControlBlock);
    return *this;
}

template<class T, size_t Size> class tShareableArray<const T, Size> final : protected tSharableArrayBase<T, Size>
{
    using cControlBlock = typename tSharableArrayBase<T, Size>::cControlBlock;
public:
    tShareableArray(const tShareableArray& other)
    {
        this->mControlBlock = other.mControlBlock;
        this->mControlBlock->mRefCount++;
    }
    tShareableArray(tShareableArray&& other)
    {
        this->mControlBlock = other.mControlBlock;
        other.mControlBlock = nullptr;
    }
    tShareableArray(tShareableArray<T, Size>&& other)
    {
        this->mControlBlock = other.mControlBlock;
        other.mControlBlock = nullptr;
    }
    ~tShareableArray()
    {
        this->mControlBlock->mRefCount--;
        if(this->mControlBlock->mRefCount == 0)
        {
            delete this->mControlBlock;
        }
    }
    tShareableArray& operator=(const tShareableArray& other)
    {
        if(this == &other)
        {
            return *this;
        }
        tShareableArray<T, Size> toDiscard(std::move(*this));
        this->mControlBlock = other.mControlBlock;
        this->mControlBlock->mRefCount++;
        return *this;
    }
    tShareableArray& operator=(tShareableArray&& other)
    {
        if(this == &other)
        {
            return *this;
        }
        tShareableArray<T, Size> toDiscard(std::move(*this));
        std::swap(this->mControlBlock, other.mControlBlock);
        return *this;
    }
    tShareableArray& operator=(tShareableArray<T, Size>&& other)
    {
        if(this == &other)
        {
            return *this;
        }
        tShareableArray<T, Size> toDiscard(std::move(*this));
        std::swap(this->mControlBlock, other.mControlBlock);
        return *this;
    }

    using tSharableArrayBase<T, Size>::operator[];
    using tSharableArrayBase<T, Size>::operator bool;
    using tSharableArrayBase<T, Size>::operator!;

    auto range() const
    {
        return std::views::all(this->mControlBlock->mArray);
    }
};

TEST(ShareableArray, access)
{
    tShareableArray<int, 5> testedArray;
    testedArray[0] = 10;
    tShareableArray<const int, 5> testedConstArray(std::move(testedArray));
    for(auto& element : testedConstArray.range())
    {
        element = 1;
    }
}
*/
namespace ProxyTests
{

TEST(Proxy, value)
{
    int testedValue = 50;
    auto testedProxy = tProxy<int, int,
        [](auto& value) { return value; },
        [](auto& value, auto newValue) { value = newValue; } > (testedValue);

    EXPECT_EQ(testedValue, 50);
    EXPECT_EQ(testedProxy, 50);
}

TEST(Proxy, assignment)
{
    int testedValue = 50;
    auto testedProxy = tProxy<int, int,
        [](auto& value) { return value; },
        [](auto& value, auto newValue) { value = newValue; } > (testedValue);

    EXPECT_EQ(testedValue, 50);
    testedProxy = 60;
    EXPECT_EQ(testedValue, 60);
    testedProxy = 70;
    EXPECT_EQ(testedValue, 70);
}

TEST(Proxy, compound_assignment)
{
    std::pair<int, int> testedValue(50, 60);
    auto testedProxy = tProxy < std::pair<int, int>, int,
        [](auto& value) { return value.first; },
        [](auto& value, auto newValue) { value.first = newValue; } > (testedValue);

    EXPECT_EQ(testedValue.first, 50);
    testedProxy += 10;
    EXPECT_EQ(testedValue.first, 60);
    testedProxy -= 20;
    EXPECT_EQ(testedValue.first, 40);
    testedProxy *= 2;
    EXPECT_EQ(testedValue.first, 80);
    testedProxy /= 4;
    EXPECT_EQ(testedValue.first, 20);
}

TEST(Proxy, comparison)
{
    int testedValue = 50;
    auto testedProxy = tProxy<int, int,
        [](auto& value) { return value; },
        [](auto& value, auto newValue) { value = newValue; } > (testedValue);

    EXPECT_EQ(testedProxy, 50);
    EXPECT_EQ(testedProxy, 50);
    EXPECT_NE(testedProxy, 60);
    EXPECT_NE(testedProxy, 40);
}

} // namespace ProxyTests

