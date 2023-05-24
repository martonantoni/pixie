#include "StdAfx.h"

cRadioButton::cRadioButton()
{
}

void cRadioButton::Selected()
{
}

void cRadioButton::SetCheckState(bool IsChecked)
{
	if(IsChecked)
	{
		cCheckBox::SetCheckState(true);
	}
}
