#pragma once

class cTooltipDisplayer final: public tSingleton<cTooltipDisplayer>
{
public:
	struct cBaseRequest
	{
		cRect mBaseRegion;
		int mWidth=-1; // -1: automatic
		int mZOrder=4000;
		enum class eDirection { UpAndRight, Right, Above, Bellow, Left } mPrefferedDirection=eDirection::UpAndRight;
	};
	struct cTooltipRequest: public cBaseRequest
	{
		cTextRenderer mTextRenderer { theFontManager.GetFont("tooltip") };
		std::string mText;
		bool operator==(const cTooltipRequest &Other) { return Other.mText==mText&&Other.mBaseRegion==mBaseRegion; }
		cTooltipRequest() { mTextRenderer.setDefaultColor("tooltip"); }
	};
private:
	tRegisteredObjects<std::unique_ptr<cSimpleMultiSprite>> mTooltips;
public:
	cTooltipDisplayer() {}
	cRegisteredID DisplayTooltip(const cTooltipRequest &Request);

	static std::unique_ptr<cSimpleMultiSprite> CreateTooltipSprites(const cTooltipRequest &Request);
	static std::unique_ptr<cSimpleMultiSprite> CreateTooltipSprites(const cBaseRequest &Request, std::unique_ptr<cSimpleMultiSprite> TextSprites);
};