#pragma once

class cFocusHandler: public cRegistrationHandler
{
	unsigned int mIDCounter=0;
	cDialogItem *mFocusedItem=nullptr;
public:
	cRegisteredID AcquireFocus(cDialogItem *Item);
	virtual void Unregister(const cRegisteredID &RegisteredID, eCallbackType CallbackType=eCallbackType::Wait) override;
};

extern cFocusHandler theFocusHandler;