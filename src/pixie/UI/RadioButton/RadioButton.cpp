#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
#include "pixie/pixie/ui/RadioButton/RadioButton.h"

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
