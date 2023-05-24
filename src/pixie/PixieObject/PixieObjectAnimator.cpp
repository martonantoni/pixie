#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cPixieObjectAnimator::cPixieObjectAnimator(unsigned int Flags, unsigned int AffectedProperties)
: mFlags(Flags)
, mAffectedProperties(AffectedProperties)
{
}

cPixieObjectAnimator::~cPixieObjectAnimator()
{
	if(mDoneFunction)
		theLogicServer.ExecuteOnce(std::move(mDoneFunction));
}

void cPixieObjectAnimator::SetDoneFunction(const std::function<void()> &DoneFunction)
{
	mDoneFunction=DoneFunction;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cPixieObjectAnimatorManager *thePixieObjectAnimatorManager=NULL;

cPixieObjectAnimatorManager::cPixieObjectAnimatorManager()
{
	thePixieObjectAnimatorManager=this;
	mLogicID=theLogicServer.AddLogic([this]() { OnLogic(); }, /*cLogicServer::DefaultLogicOrder*/400);
}

cPixieObjectAnimatorManager::~cPixieObjectAnimatorManager()
{
	thePixieObjectAnimatorManager=NULL;
}

void cPixieObjectAnimatorManager::OnLogic()
{
	mAnimatedObjects.ForEach([](cPixieObject *Object) { Object->RunAnimators(); });
}

cRegisteredID cPixieObjectAnimatorManager::RegisterAnimatedObject(cPixieObject *Object)
{
	return mAnimatedObjects.Register(Object);
}