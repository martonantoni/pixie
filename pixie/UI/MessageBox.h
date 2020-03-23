#pragma once

class cMessageBox final
{
public:
	using cButtonFunction=std::function<void()>;
	struct cInitData
	{
		std::string mText;
		int mWindowWidth=400;
		int mHorizontalBorder=10;
		struct cButtonData
		{
			std::string mText;
			cButtonFunction mFunction;

			cButtonData(const std::string &Text, const cButtonFunction &Function):
				mText(Text), mFunction(Function) {}
			cButtonData(cButtonData &&)=default;
		};
		std::vector<cButtonData> mButtons;
	};
private:
	cStandardWindow mWindow;
	std::unique_ptr<cSimpleMultiSprite> mTextSprite;
	std::vector<std::unique_ptr<cPushButton>> mButtons;
	cRegisteredIDList mListeningIDs;
	cMessageBox()=default;
	void Init(const cInitData &InitData);
public:
	static void Display(const cInitData &InitData);
	static void DisplayError(const std::string &Message, const cButtonFunction & =cButtonFunction());
};
