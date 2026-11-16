#pragma once

class cMultiSprite_Scripted: public cMultiSprite
{
	std::shared_ptr<cShaderFileScript> mShaderScript;
    struct cSpriteDescriptor
    {
		cPoint mOffset;
        int mIndex = -1; // in mSprites
    };
	cPoint mLastSize{ -1, -1 };
	std::unordered_map<std::string, cSpriteDescriptor> mDescriptors;
#ifdef _DEBUG
    cMessageListeners mMessageListeners;
#endif

	virtual void ArrangeSprites() override;
protected:
	virtual ~cMultiSprite_Scripted() = default;
public:
	cMultiSprite_Scripted(const std::string& mainShader);
};

USE_DROP_INSTEAD_DELETE_PARENT(cMultiSprite_Scripted, cMultiSprite)
