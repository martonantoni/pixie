#pragma once

class cMultiSprite_Shader9Patch: public cMultiSprite
{
	bool mRectSpecifiesCenter = false;
	struct Indexes
	{
		static constexpr int topLeft = 0;
		static constexpr int top = 1;
        static constexpr int topRight = 2;
        static constexpr int left = 3;
        static constexpr int right = 4;
        static constexpr int bottomLeft = 5;
        static constexpr int bottom = 6;
        static constexpr int bottomRight = 7;
        static constexpr int center = 8; // optional
	};
	virtual void ArrangeSprites() override;
protected:
	virtual ~cMultiSprite_Shader9Patch() = default;
	struct
	{
        int mTop = -1;
        int mBottom = 0;
        int mLeft = 0;
        int mRight = 0;
        bool isInitialized() const { return mTop >= 0; }
    } mFrameWidths;
public:
	cMultiSprite_Shader9Patch(cSprites&& sprites, bool rectSpecifiesCenter) 
        : cMultiSprite(std::move(sprites)), mRectSpecifiesCenter(rectSpecifiesCenter) {}

	struct cInitData
	{
		std::string_view mShaderFolderName;
        bool mRectSpecifiesCenter = false; // if true, mRect specifies the center rect, otherwise it specifies the whole sprite rect
        cPixieWindow* mWindow = nullptr;
	};
	static std::unique_ptr<cMultiSprite_Shader9Patch> create(const cInitData& initData);
};

namespace Pixie { using cFrameSprite = cMultiSprite_Shader9Patch; }

USE_DROP_INSTEAD_DELETE_PARENT(cMultiSprite_Shader9Patch, cMultiSprite)
