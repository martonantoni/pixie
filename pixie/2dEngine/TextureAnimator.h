#pragma once

class cTextureAnimator: public cPixieObjectAnimator
{
	std::vector<std::string> mTextureNames;
private:
	unsigned int mFramesPerSec;
	unsigned int mPreviousFrame=~0;
protected:
	virtual void Activated(cPixieObject &Object) override;
	virtual eAnimateResult Animate(cPixieObject &Object) override;
public:
	template<class T> cTextureAnimator(T TextureNames, unsigned int FramesPerSec):
		cPixieObjectAnimator(DoneWhenSamePropertyAnimatorAdded|DoneWhenAffectedPropertySet|KeepsObjectAlive, cPixieObject::Property_Texture),
		mTextureNames(std::forward<T>(TextureNames)), mFramesPerSec(FramesPerSec) {}
};
