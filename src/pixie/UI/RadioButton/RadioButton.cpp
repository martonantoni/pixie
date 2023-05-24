#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
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
