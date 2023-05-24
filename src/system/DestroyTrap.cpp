#include "StdAfx.h"

cDestroyTrap::~cDestroyTrap()
{
	while(!mGuards.empty())
		mGuards.pop_front()->mTriggered=true;
}