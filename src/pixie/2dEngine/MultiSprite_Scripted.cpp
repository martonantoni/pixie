#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

#include "pixie/pixie/2dEngine/MultiSprite_Scripted.h"

void cMultiSprite_Scripted::ArrangeSprites()
{
    if (!mShaderScript || mShaderScript->mMetaData.isNil())
        return;
    cRect usedRect = GetRect();
    if (mLastSize == usedRect.size())
    {
        // only need to set positions
        for (auto&& [name, descriptor] : mDescriptors)
        {
            mSprites[descriptor.mIndex]->SetPosition(
                descriptor.mOffset.x + usedRect.left(),
                descriptor.mOffset.y + usedRect.top());
        }
        return;
    }
    auto& state = mShaderScript->mMetaData.state();
    cLuaObject spriteDescriptorsTable = state.createTable();
    for (auto&& [name, descriptor] : mDescriptors)
    {
        cLuaObject spriteDescriptor = state.createTable();
        spriteDescriptor.set("pos", state.createTable());
        spriteDescriptor.set("size", state.createTable());
        spriteDescriptorsTable.set(name, std::move(spriteDescriptor));
    }
    cLuaObject sizeTable = state.createTable();
    sizeTable.set("x", static_cast<int>(usedRect.width()));
    sizeTable.set("y", static_cast<int>(usedRect.height()));
    mShaderScript->mMetaData.get("arrange_sprites").call(spriteDescriptorsTable, sizeTable);
    for (auto&& [name, descriptor] : mDescriptors)
    {
        cLuaObject spriteDescriptorTable = spriteDescriptorsTable.get(name);
        cPoint posOffset = cPoint{ spriteDescriptorTable.get("pos").get<int>("x"), spriteDescriptorTable.get("pos").get<int>("y") };
        descriptor.mOffset = posOffset;
        mSprites[descriptor.mIndex]->SetPosition(
            posOffset.x + usedRect.left(),
            posOffset.y + usedRect.top());
        mSprites[descriptor.mIndex]->SetSize(
            spriteDescriptorTable.get("size").get<int>("x"),
            spriteDescriptorTable.get("size").get<int>("y"));
    }
    mLastSize = static_cast<cPoint>(usedRect.size());
}

cMultiSprite_Scripted::cMultiSprite_Scripted(const std::string& mainShader)
    : cMultiSprite()
{
    mShaderScript = theShaderManager->shaderScript(mainShader); 
    auto spriteDescriptors = mShaderScript->mMetaData.get("create_sprites").call<cLuaObject>();
    for (auto&& [name, descriptor] : spriteDescriptors)
    {
        cSpriteDescriptor spriteDescriptor;
        spriteDescriptor.mOffset = cPoint{ 0, 0 };
        spriteDescriptor.mIndex = static_cast<int>(mSprites.size());

        auto sprite = std::make_unique<cSprite>();
        sprite->setShader(descriptor.get<std::string>("shader"));
        sprite->SetZOrder(100 + descriptor.get<int>("z_order", 0));
        mSprites.emplace_back(std::move(sprite));

        mDescriptors[name.toString()] = std::move(spriteDescriptor);
    }
#ifdef _DEBUG
    mMessageListeners.listen("pixie.shaders_reloaded", [this]()
        {
            mLastSize = cPoint{ -1, -1 };
            ArrangeSprites();
        });
#endif
}

