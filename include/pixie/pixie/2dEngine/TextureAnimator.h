#pragma once

class cTextureAnimator: public cPixieObjectAnimator
{
    std::variant<std::monostate, std::vector<std::string>, std::vector<tIntrusivePtr<cTexture>>> mTextures;
private:
	unsigned int mFramesPerSec;
	unsigned int mPreviousFrame=~0;
protected:
	virtual void Activated(cPixieObject &Object) override;
	virtual eAnimateResult Animate(cPixieObject &Object) override;
public:
	template<class T> cTextureAnimator(T textures, unsigned int FramesPerSec):
		cPixieObjectAnimator(DoneWhenSamePropertyAnimatorAdded|DoneWhenAffectedPropertySet|KeepsObjectAlive, cPixieObject::Property_Texture),
		mTextures(std::forward<T>(textures)), mFramesPerSec(FramesPerSec) {}

};
