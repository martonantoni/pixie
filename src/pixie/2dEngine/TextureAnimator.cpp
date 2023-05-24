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
		Object.SetStringProperty(cPixieObject::Property_Texture, mTextureNames[std::min<size_t>(mTextureNames.size()-1, Frame)]);
		if(Frame>=mTextureNames.size())
			return cPixieObjectAnimator::AnimationDone;
	}
	return cPixieObjectAnimator::AnimationActive;
}
