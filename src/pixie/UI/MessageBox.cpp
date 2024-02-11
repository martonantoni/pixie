#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/Editor/i_Editor.h"

void cMessageBox::Init(const cInitData &InitData)
{
	bool EditorWasEnabled=cObjectPlacementEditor::Get().Disable();

	int HorizontalBorder=InitData.mHorizontalBorder, WindowWidth=InitData.mWindowWidth;

	cTextRenderer TextRenderer("messagebox_text");
	TextRenderer.setDefaultColor("messagebox_text");
	TextRenderer.setMaxWidth(WindowWidth-HorizontalBorder*2);
	mTextSprite=std::make_unique<cSimpleMultiSprite>(TextRenderer.render(InitData.mText), cSimpleMultiSprite::eBasePosition::Zero);

	int TextVerticalEnd=mTextSprite->GetHeight()+40;

	cStandardWindow::cInitData WindowInitData;
	WindowInitData.mParentWindow=&thePixieDesktop;
	WindowInitData.mZOrder=10'000;
	WindowInitData.mPlacement=thePixieDesktop.GetClientRect().GetAlignedRect({ 0,0,WindowWidth,TextVerticalEnd+110 }, eHorizontalAlign::Center, eVerticalAlign::Center);
	WindowInitData.mFlags=cPixieWindow::Flag_IsModal;
	mWindow.Init(WindowInitData);


	mTextSprite->SetWindow(&mWindow);
	mTextSprite->SetPosition(HorizontalBorder, 40);
	mTextSprite->Show();

	if(InitData.mButtons.empty())
		return;

	int ButtonSumWidth=0;
	for(auto &ButtonRequest: InitData.mButtons)
	{
		cPushButton::cInitData ButtonInitData;
		ButtonInitData.mParentWindow=&mWindow;
		ButtonInitData.mTitle=ButtonRequest.mText;
		ButtonInitData.mPlacement= { 0,TextVerticalEnd+35,150,50 };
		auto Button=std::make_unique<cPushButton>();
		Button->Init(ButtonInitData);
		mListeningIDs.emplace_back(Button->GetDispatcher(cPushButton::Event_Activated)->RegisterListener(
			[FunctionToCall=ButtonRequest.mFunction, EditorWasEnabled, this](auto &)
		{
			if(EditorWasEnabled)
				cObjectPlacementEditor::Get().Enable();
			delete this;
			if(FunctionToCall)
				FunctionToCall();
		}));
		ButtonSumWidth+=Button->GetPlacement().Width();
		mButtons.emplace_back(std::move(Button));
	}
	if(mButtons.size()>=2)
	{
		int ButtonGap=(WindowInitData.mPlacement.Width()-HorizontalBorder*2-ButtonSumWidth)/(int(mButtons.size())-1);
		int ButtonX=HorizontalBorder;
		for(auto &Button: mButtons)
		{
			Button->SetPosition({ ButtonX, Button->GetPlacement().Top() });
			ButtonX+=ButtonGap+Button->GetPlacement().Width();
		}
	}
	else
	{
		mButtons.front()->SetPosition({ (WindowInitData.mPlacement.Width()-ButtonSumWidth)/2,mButtons.front()->GetPlacement().Top() });
	}
}

void cMessageBox::Display(const cInitData &InitData)
{
	(new cMessageBox)->Init(InitData);
}

void cMessageBox::DisplayError(const std::string &Message, const cButtonFunction &ButtonFunction)
{
	cMessageBox::cInitData MessageBoxInit;
	MessageBoxInit.mText=Message;
	MessageBoxInit.mButtons.emplace_back("OK"s, ButtonFunction);
	cMessageBox::Display(MessageBoxInit);
}
