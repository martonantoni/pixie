#pragma once
/*
class cAlongPathPixieObjectBlender: public cPixieObjectAnimator
{
public:
    using cPath = std::vector<std::pair<int, cPixieObject::cPropertyValues>>;
    struct cRequest
	{
        unsigned int mStartTime = gFrameTime;
        unsigned int mAffectedProperties;
        cPath mPath; // time, target
		bool mKeepObjectAlive=true;
		cRequest() {}
		cRequest(unsigned int AffectedProperties, cPath path):
			mAffectedProperties(AffectedProperties), mPath(std::move(path)) {}
	};
private:
	cRequest mRequest;
	cPixieObject::cPropertyValues mStartValues;
    virtual eAnimateResult Animate(cPixieObject& Object) override;
    virtual void Activated(cPixieObject& Object) override;
public:
    cAlongPathPixieObjectBlender(const cRequest &Request);
	virtual ~cAlongPathPixieObjectBlender()=default;

	static void BlendObject(cPixieObject &Object, cPath path, unsigned int AffectedProperties, bool KeepObjectAlive=true);
};


#define BLEND_SPRITE_FUNCTION(FunctionNameExtension,AffectedProperties) \
	inline void BlendObject##FunctionNameExtension_AlongPath(cPixieObject &Sprite, cAlongPathPixieObjectBlender::cPath path) \
	{ \
        cAlongPathPixieObjectBlender::BlendObject(Sprite,std::move(path),AffectedProperties); \
    } \
	inline void BlendObject##FunctionNameExtension##_AlongPath_NoKeepAlive(cPixieObject &Sprite, cAlongPathPixieObjectBlender::cPath path) \
	{ \
        cAlongPathPixieObjectBlender::BlendObject(Sprite,std::move(path),AffectedProperties,false); \
    } 

BLEND_SPRITE_FUNCTION(Position, cPixieObject::Property_Position);
BLEND_SPRITE_FUNCTION(PositionOffset, cPixieObject::Property_PositionOffset);
BLEND_SPRITE_FUNCTION(Size, cPixieObject::Property_Size);
BLEND_SPRITE_FUNCTION(Rect, cPixieObject::Property_Rect);
BLEND_SPRITE_FUNCTION(CenterAndHSize, cPixieObject::Property_CenterAndHSize);
BLEND_SPRITE_FUNCTION(Alpha, cPixieObject::Property_Alpha);
BLEND_SPRITE_FUNCTION(RGBColor, cPixieObject::Property_Color);
BLEND_SPRITE_FUNCTION(Center, cPixieObject::Property_Center);

*/