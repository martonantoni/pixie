#pragma once

class cObjectPlacementEditor: public tSingleton<cObjectPlacementEditor>
{
public:
	enum class eAction { Selected, Unselected, Moved, Resized };
	using cNotifierFunction = std::function<void(eAction)>;
	struct cObjectData
	{
		cPixieObject *mObject;
		cNotifierFunction mNotifierFunction;
		bool mResizable=true;
		bool mMoveable=true;
	};
private:
	static const cEventDispatchers::cDispatcherRangeInfo mDispatcherRangeInfo;
	cEventDispatchers mEventDispatchers;
	class cMouseBlocker_Rect: public cMouseTarget
	{
		cRect mRect;
	public:
		void SetRect(const cRect &Rect) { mRect=Rect; }
		virtual bool IsInside_Overridable(cPoint WindowRelativePoint) const override;
	};
	cObjectData *mSelectedObject=nullptr;
	tRegisteredObjects<std::unique_ptr<cObjectData>> mObjects;
	tDataHolder<cPixieObject *> mPixieObjectHolder;
	std::unique_ptr<cMouseBlocker_Rect> mMouseBlockerTarget;
	cRegisteredIDList mListeningIDs;
	cRegisteredID mEnableEditorListeningID;
	cRegisteredID mHelpID;
	cRegisteredID mCursorID;
	cRect mEditorArea;
	enum class eDragType { None, Position, Size } mDragType = eDragType::None;
	cPoint mPrevDragPosition;
	bool mKeepAspectRatio;
	tDataHolder<cPoint> &mPointHolder=tDataHolder<cPoint>::Get();
	std::unique_ptr<cSpriteBase> mSelectedObjectBorder, mHoverObjectBorder;
	static std::unique_ptr<cSpriteBase> CreateBorderSprite(const cPixieObject &Object);
	static cRect GetObjectRect(const cPixieObject &Object);
	cObjectPlacementEditor::cObjectData *FindObject(cPoint ScreenCoordinates);
	void ResetHover();
	void UpdateBorders();
	void RegisterListeners();
	void ObjectRemoved(cPixieObject *Sprite);
	void MoveObject(cPoint Offset);
	void ChangeObjectSize(cPoint Offset);
	void OnMouseMove(const cEvent &Event);
	void OnLeftButtonDown(const cEvent &Event);
	void OnLeftButtonUp(const cEvent &Event);
	void SetSizeAsTexture();
	void ResetSelection();
	void DisplayHelp();
	void SwitchAspectRatioKeeping();
	void SetSelectedObject(cPixieObject *Object);
	std::pair<cPixieObject *, eDragType> HandleHover(cPoint ScreenCoordinates);
public:
	cObjectPlacementEditor();
	cRegisteredID AddEditableObject(cPixieObject *Object, const cNotifierFunction &NotifierFunction=cNotifierFunction());
	cRegisteredID AddEditableObject(std::unique_ptr<cObjectData> ObjectData);
	void SelectObject(cPixieObject *Object); // Object must be already added, or nullptr (nullptr means clear selection)
	bool IsEnabled() const;
	void Enable();
	bool Disable(); // returns true if it was enabled before
	void SetEditorArea(const cRect &Rect);
	cPixieObject *GetSelectedObject() const { return mSelectedObject?mSelectedObject->mObject:nullptr; }
	int GetSelectedObjectCount() const { return mSelectedObject?1:0; }
	static void DisplayNotification(const std::string &Notification);

	enum eEventType: size_t
	{
		Event_SelectionChanged = 0, Event_ObjectMoved, Event_ObjectResized
	};
	tIntrusivePtr<cEventDispatcher> GetDispatcher(size_t EventIndex) const { return mEventDispatchers[EventIndex]; }
};