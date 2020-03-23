#pragma once

class cRadioButton: public cCheckBox
{
	virtual void Selected();
public:
	cRadioButton();
	virtual void SetCheckState(bool IsChecked);
};