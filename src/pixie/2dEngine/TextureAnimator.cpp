#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

void cTextureAnimator::Activated(cPixieObject &Object)
{
}

cPixieObjectAnimator::eAnimateResult cTextureAnimator::Animate(cPixieObject &Object)
{
	auto Elapsed=gFrameTime-GetStartTime();
	auto Frame=(Elapsed*mFramesPerSec)/1000;
	if(Frame!=mPreviousFrame)
	{
		mPreviousFrame=Frame;
		int numberOfTextures = std::visit([&Object, Frame, this](auto&& textures) -> int
			{
                if constexpr (std::is_same_v<std::monostate, std::decay_t<decltype(textures)>>)
                    return 0;
				else if constexpr (std::is_same_v<std::vector<std::string>, std::decay_t<decltype(textures)>>)
				{
                    std::string nextTextureName = textures[std::min<size_t>(textures.size() - 1, Frame)];
					Object.SetStringProperty(cPixieObject::Property_Texture, nextTextureName);
                    return textures.size();
				}
				else if constexpr (std::is_same_v<std::vector<tIntrusivePtr<cTexture>>, std::decay_t<decltype(textures)>>)
				{
					return textures.size();
				}
				return 0;
            }, mTextures);
//		Object.SetStringProperty(cPixieObject::Property_Texture, mTextureNames[std::min<size_t>(mTextureNames.size()-1, Frame)]);
		if(Frame>= numberOfTextures)
			return cPixieObjectAnimator::AnimationDone;
	}
	return cPixieObjectAnimator::AnimationActive;
}
