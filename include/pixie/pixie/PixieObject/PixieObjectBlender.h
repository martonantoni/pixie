#pragma once

class cGeneralPixieObjectBlender: public cPixieObjectAnimator
{
public:
	struct cRequest
	{
		unsigned int mStartTime=gFrameTime;
		unsigned int mBlendTime;
		unsigned int mAffectedProperties;
		cPixieObject::cPropertyValues mTargetValues;
		bool mKeepObjectAlive=true;
		cRequest() {}
		cRequest(unsigned int AffectedProperties, const cPixieObject::cPropertyValues &TargetValues, unsigned int BlendTime):
			mAffectedProperties(AffectedProperties), mTargetValues(TargetValues), mBlendTime(BlendTime) {}
	};
private:
	cRequest mRequest;
	cPixieObject::cPropertyValues mStartValues;
public:
	cGeneralPixieObjectBlender(const cRequest &Request);
	virtual ~cGeneralPixieObjectBlender()=default;
	virtual void Activated(cPixieObject &Object) override;
	virtual eAnimateResult Animate(cPixieObject &Object) override;

	static tIntrusivePtr<cPixieObjectAnimator> BlendObject(cPixieObject &Object, const cPixieObject::cPropertyValues &TargetValues, unsigned int AffectedProperties, unsigned int BlendTime, bool KeepObjectAlive=true);
};


#define BLEND_SPRITE_FUNCTION(FunctionNameExtension,AffectedProperties) \
	inline tIntrusivePtr<cPixieObjectAnimator> BlendObject##FunctionNameExtension(cPixieObject &Sprite,const cPixieObject::cPropertyValues &TargetValues,int BlendTime) \
	{ return cGeneralPixieObjectBlender::BlendObject(Sprite,TargetValues,AffectedProperties,BlendTime); } \
	inline tIntrusivePtr<cPixieObjectAnimator> BlendObject##FunctionNameExtension##_NoKeepAlive(cPixieObject &Sprite, const cPixieObject::cPropertyValues &TargetValues, int BlendTime) \
	{ return cGeneralPixieObjectBlender::BlendObject(Sprite,TargetValues,AffectedProperties,BlendTime,false); } 

BLEND_SPRITE_FUNCTION(Position, cPixieObject::Property_Position);
BLEND_SPRITE_FUNCTION(PositionOffset, cPixieObject::Property_PositionOffset);
BLEND_SPRITE_FUNCTION(Size, cPixieObject::Property_Size);
BLEND_SPRITE_FUNCTION(Rect, cPixieObject::Property_Rect);
BLEND_SPRITE_FUNCTION(CenterAndHSize, cPixieObject::Property_CenterAndHSize);
BLEND_SPRITE_FUNCTION(Alpha, cPixieObject::Property_Alpha);
BLEND_SPRITE_FUNCTION(RGBColor, cPixieObject::Property_Color);
BLEND_SPRITE_FUNCTION(Center, cPixieObject::Property_Center);

