#pragma once

class cMouseEventEmitterTarget;

class cTextLinkHelper final
{
public:
	enum class eAction { Click, Hover_Enter, Hover_Leave };
	using cLinkHandler = std::function<void(eAction, const cRect &LinkRect)>;
private:
	struct cLinkData
	{
		std::vector<cSpriteBase *> mSprites;
		std::vector<cColor> mOriginalSpriteColors;
		std::unique_ptr<cMouseEventEmitterTarget> mTarget;
		cLinkHandler mLinkHandler;
		cRect mBoundingRect;
		cRegisteredIDList mListeners;
	};
	std::vector<cLinkData> mLinks;
	void SetLinkColor(size_t LinkIndex, const std::string &ColorName);
	void NotifyHandler(size_t LinkIndex, eAction Action);
	void OnMoveInsideTarget(size_t LinkIndex);
	void OnMoveOutsideTarget(size_t LinkIndex);
	void OnClickTarget(size_t LinkIndex);
public:
	struct cInitData
	{
		const cTextRenderer::cRenderInfo &mRenderInfo;
		const cMultiSpriteBase &mTextSprite;
		using cLinkHandlers=std::vector<cLinkHandler>;
		cLinkHandlers mLinkHandlers;
		cPixieWindow *mWindow=nullptr;
		cPoint mTextPosition;
		cInitData(const cTextRenderer::cRenderInfo &RenderInfo, const cMultiSpriteBase &TextSprite):
			mRenderInfo(RenderInfo), mTextSprite(TextSprite) {}
	};
	cTextLinkHelper(const cInitData &InitData);
	~cTextLinkHelper();

	void SetWindow(cPixieWindow *Window);
	void SetTextPosition(cPoint Position);
};