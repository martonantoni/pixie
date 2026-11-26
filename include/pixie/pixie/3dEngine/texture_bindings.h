#pragma once

struct cTextureBindings
{
    std::vector<std::pair<int, tIntrusivePtr<cTexture>>> mBindings;
    bool operator==(const cTextureBindings& other) const = default;
    bool operator!=(const cTextureBindings& other) const = default;
    void setSlot(int index, tIntrusivePtr<cTexture> texture);
    void merge(const cTextureBindings& other);
};