#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"

/*
Keyboard commands:

F1: help
F6: enable editor   <--- need to use this FIRST
F7: disable editor
F9: keep aspect ration on/off

cursor keys: move selected item
A/S/D/W: resize selected item
T: set selected item's size to the same as its texture's size

*/

const cEventDispatchers::cDispatcherRangeInfo cObjectPlacementEditor::mDispatcherRangeInfo=
{
	Event_SelectionChanged, { "selection_changed", "object_moved", "object_resized" }
};

cObjectPlacementEditor::cObjectPlacementEditor()
{
	mEventDispatchers.Init(cEventDispatcher::GetGlobalDispatcher("object_placement_editor"));
	mEventDispatchers.AddEvents(mDispatcherRangeInfo);
	mEventDispatchers[Event_SelectionChanged]->SetCoalescing();

	auto &KeyboardServer=cKeyboardServer::Get();
	mEnableEditorListeningID=KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_F6)->RegisterListener([this](auto &Event)
	{ Enable(); });
	mHelpID=KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_F1)->RegisterListener([this](auto &Event)
	{ DisplayHelp(); });
	mEditorArea=thePixieDesktop.GetClientRect();
}

void cObjectPlacementEditor::RegisterListeners()
{
	auto &KeyboardServer=cKeyboardServer::Get();
	mListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_Left)->RegisterListener([this](auto &Event)
	{ MoveObject(cPoint { -1, 0 }); }));
	mListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_Right)->RegisterListener([this](auto &Event)
	{ MoveObject(cPoint { 1, 0 });  }));
	mListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_Up)->RegisterListener([this](auto &Event)
	{ MoveObject(cPoint { 0, -1 });  }));
	mListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_Down)->RegisterListener([this](auto &Event)
	{ MoveObject(cPoint { 0, 1 });  }));

	mListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_A)->RegisterListener([this](auto &Event)
	{ ChangeObjectSize(cPoint { -1, 0 });  }));
	mListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_D)->RegisterListener([this](auto &Event)
	{ ChangeObjectSize(cPoint { 1, 0 });  }));
	mListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_W)->RegisterListener([this](auto &Event)
	{ ChangeObjectSize(cPoint { 0, -1 });  }));
	mListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_S)->RegisterListener([this](auto &Event)
	{ ChangeObjectSize(cPoint { 0, 1 });  }));

	mListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_T)->RegisterListener([this](auto &Event)
	{ SetSizeAsTexture(); }));
	mListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_F9)->RegisterListener([this](auto &Event)
	{ SwitchAspectRatioKeeping(); }));

	mListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_Esc)->RegisterListener([this](auto &Event)
	{ ResetSelection(); }));
	mListeningIDs.emplace_back(KeyboardServer.GetDispatcher(cKeyboardServer::Keyboard_KeyDown_F7)->RegisterListener([this](auto &Event)
	{ Disable(); }));

	auto &MouseServer=cMouseServer::Get();
	mListeningIDs.push_back(MouseServer.GetDispatcher(cMouseServer::Event_Move)->
		RegisterListener([this](const auto &Event) { OnMouseMove(Event); }));
	mListeningIDs.push_back(MouseServer.GetDispatcher(cMouseServer::Event_LeftButtonDown)->
		RegisterListener([this](const auto &Event) { OnLeftButtonDown(Event); }));
	mListeningIDs.push_back(MouseServer.GetDispatcher(cMouseServer::Event_LeftButtonUp)->
		RegisterListener([this](const auto &Event) { OnLeftButtonUp(Event); }));
}

bool cObjectPlacementEditor::IsEnabled() const
{
	return !!mMouseBlockerTarget;
}

void cObjectPlacementEditor::Enable()
{
	if(IsEnabled())
		return;
	RegisterListeners();
	cMouseBlocker::cInitData MouseBlockerInit;
	MouseBlockerInit.mFlags=cMouseTarget::Flag_IgnoreBoundingBoxForIsInside;
	MouseBlockerInit.mParentWindow=&thePixieDesktop.GetTopLayer();
	MouseBlockerInit.mZOrder=100'000;
	mMouseBlockerTarget=std::make_unique<cMouseBlocker_Rect>();
	mMouseBlockerTarget->SetRect(mEditorArea);
	mMouseBlockerTarget->Init(MouseBlockerInit);
	DisplayNotification("Editor enabled");
}

bool cObjectPlacementEditor::Disable()
{
	if(!IsEnabled())
		return false;
	ResetSelection();
	mCursorID.Unregister();
	mMouseBlockerTarget.reset();
	mHoverObjectBorder.reset();
	mListeningIDs.clear();
	DisplayNotification("Editor disabled");
	return true;
}

void cObjectPlacementEditor::SetEditorArea(const cRect &Rect)
{
	mEditorArea=Rect;
	if(mMouseBlockerTarget)
		mMouseBlockerTarget->SetRect(mEditorArea);
}

void cObjectPlacementEditor::DisplayHelp()
{
	bool WasEnabled=!!mMouseBlockerTarget;
	if(WasEnabled)
		Disable();
	cMessageBox::cInitData MessageBoxConfig;
	MessageBoxConfig.mWindowWidth=340;
	MessageBoxConfig.mButtons.emplace_back("Thanks",
		[this, WasEnabled]()
	{
		if(WasEnabled)
			Enable();
	});
	MessageBoxConfig.mText=
		"{|}{f:messagebox_title}{c:messagebox_title}Object Placement Editor{c}{f}\n\n"
		"{c:messagebox_title}F1{c}: this help screen\n"
		"{c:messagebox_title}F6{c}: enable editor\n"
		"{c:messagebox_title}F7{c}: disable editor\n\n"
		"{c:messagebox_title}F9{c}: keep aspect ratio on/off\n"
		"{f:messagebox_italic}If there is an item selected:{f}\n"
		"{c:messagebox_title}cursor keys{c}: move item\n"
		"{c:messagebox_title}A,S,D,W{c}: resize item\n"
		"{c:messagebox_title}T{c}: set size to texture's size\n"
		"{c:messagebox_title}ESC{c}: clear selection\n";
	cMessageBox::Display(MessageBoxConfig);
}

void cObjectPlacementEditor::MoveObject(cPoint Offset)
{
	if(!mSelectedObject)
		return;
	cPixieObject::cPropertyValues PositionValues;
	mSelectedObject->mObject->GetProperty(cPixieObject::Property_Position, PositionValues);
	cPoint Position=PositionValues.ToPoint();
	Position+=Offset;
	mSelectedObject->mObject->SetProperty(cPixieObject::Property_Position, Position);
	mEventDispatchers[Event_ObjectMoved]->PostEvent(cEvent(mPixieObjectHolder.StoreData(mSelectedObject->mObject)));
	if(mSelectedObject->mNotifierFunction)
		mSelectedObject->mNotifierFunction(eAction::Moved);
	UpdateBorders();
}

void cObjectPlacementEditor::ChangeObjectSize(cPoint Offset)
{
	if(!mSelectedObject)
		return;
	cPixieObject::cPropertyValues SizeValues;
	mSelectedObject->mObject->GetProperty(cPixieObject::Property_Size, SizeValues);
	cPoint Size=SizeValues.ToPoint();
	if(mKeepAspectRatio)
		Size.RatioKeeping_SetX(Size.x+Offset.x);
	else
		Size+=Offset;
	mSelectedObject->mObject->SetProperty(cPixieObject::Property_Size, Size);
	mEventDispatchers[Event_ObjectResized]->PostEvent(cEvent(mPixieObjectHolder.StoreData(mSelectedObject->mObject)));
	if(mSelectedObject->mNotifierFunction)
		mSelectedObject->mNotifierFunction(eAction::Resized);
	UpdateBorders();
}

void cObjectPlacementEditor::SetSizeAsTexture()
{
	if(!mSelectedObject)
	{
		DisplayNotification("Error: no object selected");
		return;
	}
	cPixieObject::cPropertyValues Size;
	if(mSelectedObject->mObject->GetProperty(cPixieObject::Property_TextureSize, Size))
	{
		mSelectedObject->mObject->SetProperty(cPixieObject::Property_Size, Size);
		mEventDispatchers[Event_ObjectResized]->PostEvent(cEvent(mPixieObjectHolder.StoreData(mSelectedObject->mObject)));
		if(mSelectedObject->mNotifierFunction)
			mSelectedObject->mNotifierFunction(eAction::Resized);
		UpdateBorders();
	}
}

void cObjectPlacementEditor::UpdateBorders()
{
	if(ASSERTFALSE(!mSelectedObject||!mSelectedObject->mObject))
		return;
	cRect Rect=GetObjectRect(*mSelectedObject->mObject);
	auto UpdateBorder=[Rect](auto &Sprite)
	{
		if(Sprite)
		{
			Sprite->SetRect(Rect);
		}
	};
	UpdateBorder(mSelectedObjectBorder);
	UpdateBorder(mHoverObjectBorder);
}

void cObjectPlacementEditor::ObjectRemoved(cPixieObject *Object)
{
	if(mSelectedObject&&mSelectedObject->mObject==Object)
	{
		ResetSelection();
	}
}

cRect cObjectPlacementEditor::GetObjectRect(const cPixieObject &Object)
{
	cPixieObject::cPropertyValues PositionValues, PositionOffsetValues, SizeValues;
	Object.GetProperty(cPixieObject::Property_ScreenPosition, PositionValues);
	Object.GetProperty(cPixieObject::Property_PositionOffset, PositionOffsetValues);
	Object.GetProperty(cPixieObject::Property_Size, SizeValues);
	cRect Rect(PositionValues.ToPoint(), SizeValues.ToPoint());
	Rect.Move(PositionOffsetValues.ToPoint());
	return Rect;
}

std::unique_ptr<cSpriteBase> cObjectPlacementEditor::CreateBorderSprite(const cPixieObject &Object)
{
	auto BorderSprite=std::make_unique<cMultiSprite_1PixRect>("editor_hover"s, cMultiSprite_1PixRect::eOnlyBorder::Yes);
	BorderSprite->SetWindow(&thePixieDesktop.GetTopLayer());
	BorderSprite->SetRect(GetObjectRect(Object));
	BorderSprite->SetZOrder(100'000);
	BorderSprite->Show();
	return BorderSprite;
}

std::pair<cPixieObject *, cObjectPlacementEditor::eDragType> cObjectPlacementEditor::HandleHover(cPoint ScreenCoordinates)
{
	auto OverObjectData=FindObject(ScreenCoordinates);
	if(!OverObjectData)
	{
		ResetHover();
		return { nullptr, eDragType::None };
	}
	auto OverObject=OverObjectData->mObject;
	mHoverObjectBorder=CreateBorderSprite(*OverObject);
	cPoint RightBottom=mHoverObjectBorder->GetRect().BottomRight();
	cRect SizingGrabRect(RightBottom-cPoint { 10, 10 }, { 10, 10 });
	eDragType DragType=eDragType::None;
	if(OverObjectData->mResizable&&SizingGrabRect.IsPointInside(ScreenCoordinates))
	{
		mCursorID=cMouseCursorServer::Get().SetCursor("ed_scale");
		DragType=eDragType::Size;
	}
	else
	{
		mCursorID=cMouseCursorServer::Get().SetCursor("ed_move");
		DragType=eDragType::Position;
	}
	return { OverObject, DragType };
}

void cObjectPlacementEditor::ResetHover()
{
	mCursorID.Unregister();
	mHoverObjectBorder.reset();
}

void cObjectPlacementEditor::OnMouseMove(const cEvent& event)
{
    cPoint screenCoordinates = cMouseServer::point(event);
    switch (mDragType)
    {
    case eDragType::None:
        if (mEditorArea.IsPointInside(screenCoordinates))
            HandleHover(screenCoordinates);
        else
            ResetHover();
        break;
    case eDragType::Position:
        MoveObject(screenCoordinates - mPrevDragPosition);
        break;
    case eDragType::Size:
        ChangeObjectSize(screenCoordinates - mPrevDragPosition);
        break;
    }
    mPrevDragPosition = screenCoordinates;
}

cObjectPlacementEditor::cObjectData *cObjectPlacementEditor::FindObject(cPoint ScreenCoordinates)
{
	auto FoundObjectData=mObjects.FindIf([ScreenCoordinates](auto &ObjectData)
	{
		auto Object=ObjectData->mObject;
		if(ASSERTFALSE(!Object))
			return false;
		return GetObjectRect(*Object).IsPointInside(ScreenCoordinates);
	});
	return FoundObjectData?FoundObjectData->get():nullptr;
}

void cObjectPlacementEditor::OnLeftButtonDown(const cEvent& event)
{
    cPoint screenCoordinates = cMouseServer::point(event);
	if(!mEditorArea.IsPointInside(screenCoordinates))
		return;
	mSelectedObjectBorder.reset();
	SetSelectedObject(nullptr);
	cPixieObject *FoundObject;
	eDragType DragType;
	std::tie(FoundObject, DragType)=HandleHover(screenCoordinates);
	if(FoundObject)
	{
		SetSelectedObject(FoundObject);
		mDragType=DragType;
		mPrevDragPosition=screenCoordinates;
		mHoverObjectBorder.reset();
	}
}

void cObjectPlacementEditor::SetSelectedObject(cPixieObject *Object)
{
	auto ObjectData=Object?mObjects.FindIf([Object](auto &ObjectData) { return ObjectData->mObject==Object; }):nullptr;
	if((ObjectData&&ObjectData->get()==mSelectedObject)||(!ObjectData&&!mSelectedObject))
		return;
	if(mSelectedObject&&mSelectedObject->mNotifierFunction)
		mSelectedObject->mNotifierFunction(eAction::Unselected);
	if(!ObjectData)
	{
		mSelectedObject=nullptr;
	}
	else
	{
		mSelectedObject=ObjectData->get();
		if(mSelectedObject&&mSelectedObject->mNotifierFunction)
			mSelectedObject->mNotifierFunction(eAction::Selected);
	}
	mEventDispatchers[Event_SelectionChanged]->PostEvent();
}

void cObjectPlacementEditor::OnLeftButtonUp(const cEvent& event)
{
	mDragType=eDragType::None;
    cPoint screenCoordinates = cMouseServer::point(event);
	if(!mEditorArea.IsPointInside(screenCoordinates))
		return;
	auto ObjectData=FindObject(screenCoordinates);
	SelectObject(ObjectData?ObjectData->mObject:nullptr);
}

void cObjectPlacementEditor::SelectObject(cPixieObject *Object)
{
	if(!IsEnabled())
		return;
	SetSelectedObject(Object);
	if(mSelectedObject)
	{
		mSelectedObjectBorder=CreateBorderSprite(*mSelectedObject->mObject);
	}
}

void cObjectPlacementEditor::ResetSelection()
{
	SetSelectedObject(nullptr);
	mDragType=eDragType::None;
	mSelectedObjectBorder.reset();
}

cRegisteredID cObjectPlacementEditor::AddEditableObject(std::unique_ptr<cObjectData> ObjectData)
{
	auto Object=ObjectData->mObject;
	return CreateWrappedRegisteredID(mObjects.Register(std::move(ObjectData)), [Object, this]() { ObjectRemoved(Object); });
}

cRegisteredID cObjectPlacementEditor::AddEditableObject(cPixieObject *Object, const cNotifierFunction &NotifierFunction/* =cNotifierFunction() */)
{
	auto ObjectData=std::make_unique<cObjectData>();
	ObjectData->mObject=Object;
	ObjectData->mNotifierFunction=NotifierFunction;
	return AddEditableObject(std::move(ObjectData));
}

void cObjectPlacementEditor::SwitchAspectRatioKeeping()
{
	mKeepAspectRatio=!mKeepAspectRatio;
	DisplayNotification(mKeepAspectRatio?"Aspect ratio keeping: ON"s:"Aspect ratio keeping: OFF"s);
}

void cObjectPlacementEditor::DisplayNotification(const std::string &Notification)
{
	cTextRenderer TextRenderer("main_menu_button");
	auto TextSprite=std::make_unique<cSimpleMultiSprite>(TextRenderer.RenderText(Notification), cSimpleMultiSprite::eBasePosition::Zero);
	TextSprite->SetWindow(&thePixieDesktop.GetTopLayer());
	TextSprite->SetZOrder(100000);
	TextSprite->SetPosition(0, 0);
	TextSprite->SetRGBColor(~0u);
	TextSprite->Show();
	::BlendObjectAlpha(*TextSprite, 255, 2000);
}

bool cObjectPlacementEditor::cMouseBlocker_Rect::IsInside_Overridable(cPoint WindowRelativePoint) const
{
	return mRect.IsPointInside(WindowRelativePoint);
}
