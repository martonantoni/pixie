#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

void cTextureBindings::setSlot(int index, tIntrusivePtr<cTexture> texture)
{
    using cBinding = decltype(mBindings)::value_type;
    auto it = std::ranges::lower_bound(mBindings, index, {}, &cBinding::first);
    if (it != mBindings.end() && it->first == index)
    {
        if (texture)
            it->second = texture;
        else
            mBindings.erase(it);
    }
    else if (texture)
    {
        mBindings.emplace(it, index, texture);
    }
}

void cTextureBindings::merge(const cTextureBindings& other)
{
    for (auto&& [index, texture] : other.mBindings)
    {
        setSlot(index, texture);
    }
}
