#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
void cDialogItem::Init(const cPixieInitData &InitData)
{
	mIsEnabled.BindTo(InitData.mBindTo, "is_enabled");
	mIsVisible.BindTo(InitData.mBindTo, "is_visible");
	mText.BindTo(InitData.mBindTo, "text");
	mTooltipText.BindTo(InitData.mBindTo, "tooltip");
	mListeningIDs.emplace_back(mIsEnabled.GetDispatcher()->RegisterListener([this](auto &) { DialogItemStateChanged(); }));
	mListeningIDs.emplace_back(mIsVisible.GetDispatcher()->RegisterListener([this](auto &) { DialogItemStateChanged(); }));
	mListeningIDs.emplace_back(mText.GetDispatcher()->RegisterListener([this](auto &) { PropertiesChanged(Property_Text); DialogItemStateChanged(); }));
	mListeningIDs.emplace_back(mTooltipText.GetDispatcher()->RegisterListener([this](auto &) { TooltipTextChanged(); }));
	cMouseTarget::Init(InitData);
}

void cDialogItem::SetEnabled(bool IsEnabled)
{
	if(mIsEnabled==IsEnabled)
		return;
	mIsEnabled=IsEnabled;
	DialogItemStateChanged();
}

void cDialogItem::SetText(const std::string &Text)
{
	if(mText==Text)
		return;
	mText=Text;
	PropertiesChanged(Property_Text);
	DialogItemStateChanged();
}

void cDialogItem::SetVisible(bool IsVisible)
{
	if(mIsVisible==IsVisible)
		return;
	mIsVisible=IsVisible;
	DialogItemStateChanged();
}

bool cDialogItem::SetStringProperty(unsigned int PropertyFlags, const std::string &Value)
{
	if(!CheckIfChangableProperty(PropertyFlags))
		return false;
	if(PropertyFlags==Property_Text)
	{
		SetText(Value);
		return true;
	}
	return cPixieObject::SetStringProperty(PropertyFlags, Value);
}

void cDialogItem::LostFocus()
{
	mFocusID.Unregister();
	OnLostFocus();
	DialogItemStateChanged();
}

void cDialogItem::SetFocus()
{
	mFocusID=theFocusHandler.AcquireFocus(this);
	OnFocused();
	DialogItemStateChanged();
}

void cDialogItem::TooltipTextChanged()
{
}

void cDialogItem::OnMouseMove(cPoint ScreenCoords, bool IsInside)
{
	if(IsInside)
	{
		cTooltipDisplayer::cTooltipRequest Request;
		Request.mText=mTooltipText;
		Request.mBaseRegion=GetScreenPlacement();
		Request.mPrefferedDirection=cTooltipDisplayer::cTooltipRequest::eDirection::UpAndRight;
		if(!mTooltipID.IsValid()&&!mTooltipText.GetValue().empty())
		{
			mTooltipID=cTooltipDisplayer::Get().DisplayTooltip(Request);
		}
	}
	else
	{
		mTooltipID.Unregister();
	}
}

/// ---------------------------------------------------------------------------------------------------------------------------------------

void cDialogItems::AddPushButton(cPixieWindow &Window, tIntrusivePtr<cConfig> Config)
{
	cPushButton::cInitData InitData(Config);
	InitData.mParentWindow=&Window;
	auto Button=std::make_unique<cPushButton>();
	Button->Init(InitData);
	mItems.emplace_back(std::move(Button));
}

void cDialogItems::AddTextField(cPixieWindow &Window, tIntrusivePtr<cConfig> Config)
{
	cTextField::cInitData InitData(Config);
	InitData.mParentWindow=&Window;
	auto TextField=std::make_unique<cTextField>();
	TextField->Init(InitData);
	mItems.emplace_back(std::move(TextField));
}

void cDialogItems::AddEditField(cPixieWindow &Window, tIntrusivePtr<cConfig> Config)
{
	cEditField::cInitData InitData(Config);
	InitData.mParentWindow=&Window;
	auto EditField=std::make_unique<cEditField>();
	EditField->Init(InitData);
	mItems.emplace_back(std::move(EditField));
}

void cDialogItems::Init(cPixieWindow &Window, const cConfig &Config)
{
	auto NumberOfItems=Config.GetSubConfigCount();
	mItems.reserve(NumberOfItems);
	for(int i=0; i<NumberOfItems; ++i)
	{
		auto SubConfig=Config.GetSubConfig(i);
		if(ASSERTFALSE(!SubConfig))
			continue;
		auto TypeName=SubConfig->GetString("control_type");
		if(TypeName=="pushbutton")
		{
			AddPushButton(Window, SubConfig);
		}
		else if(TypeName=="textfield")
		{
			AddTextField(Window, SubConfig);
		}
		else if(TypeName=="edit"||TypeName=="editfield")
		{
			AddEditField(Window, SubConfig);
		}
		else
		{
			ASSERT(false); // unknown control_type
			continue;
		}
		auto &NewItem=mItems.back();
		if(SubConfig->GetBool("grab_focus", false))
		{
			NewItem->SetFocus();
		}
	}
}



