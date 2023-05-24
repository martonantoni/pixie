#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cPixieObject::~cPixieObject()
{
	KillAnimators();
}

bool cPixieObject::CheckIfChangableProperty(unsigned int PropertiesBeingChanged) const
{
	if(mAnimatorBeingExecuted&&PropertiesBeingChanged!=(PropertiesBeingChanged&mAnimatorBeingExecuted->GetAffectedProperties()))
	{
		ASSERT(false);
		return false;
	}
	if(ASSERTFALSE(mLockedProperties&PropertiesBeingChanged))
	{
		return false;
	}
	return true;
}

void cPixieObject::BindProperty(unsigned int PropertyFlags, const cResourceLocation &SharedVariableLocation)
{
	if(PropertyFlags&PropertyType_Text)
	{
		mPropertyBinders.emplace_back(std::make_unique<tPropertyBinder<std::string>>(*this, SharedVariableLocation, PropertyFlags));
	}
	else
	{
		ASSERT(NumberOfSetBits(PropertyFlags)==1);
		mPropertyBinders.emplace_back(std::make_unique<tPropertyBinder<int>>(*this, SharedVariableLocation, PropertyFlags));
	}
}

void cPixieObject::PropertiesSet(unsigned int Properties)
{
	if(mAnimatorBeingExecuted)
		return;

	if(!mAnimators2.empty())
	{
		mAnimators2.RemoveIf([Properties](auto &Animator)
		{
			return (Animator->GetFlags()&cPixieObjectAnimator::DoneWhenAffectedPropertySet)&&(Animator->GetAffectedProperties()&Properties);
		});
		if(mAnimators2.empty())
			mAnimatorID.Unregister();
	}

	PropertiesChanged(Properties);
}

void cPixieObject::AddAnimator(tIntrusivePtr<cPixieObjectAnimator> Animator)
{
	if(!mAnimators2.empty())
	{
		mAnimators2.RemoveIf([AffectedProperties=Animator->GetAffectedProperties()](auto &Animator)
		{
			return (Animator->GetFlags()&cPixieObjectAnimator::DoneWhenSamePropertyAnimatorAdded)
				&&(Animator->GetAffectedProperties()&AffectedProperties);
		});
	}
	else
	{
		mAnimatorID=thePixieObjectAnimatorManager->RegisterAnimatedObject(this);
	}
	Animator->Activated(*this);
	mAnimators2.Register(std::move(Animator));
}

void cPixieObject::KillAnimators()
{
	if(!mAnimators2.empty())
	{
		mAnimators2.clear();
		mAnimatorID.Unregister();
	}
}

void cPixieObject::RunAnimators()
{
	if(ASSERTFALSE(mAnimators2.empty()))
	{
		mAnimatorID.Unregister();
		return;
	}
	unsigned int ChangedProperties=0;

	mAnimators2.RemoveIf([&ChangedProperties, this](auto &Animator)
	{
		ChangedProperties|=Animator->GetAffectedProperties();
		this->mAnimatorBeingExecuted=Animator.get();
		bool IsDone=Animator->Animate(*this)==cPixieObjectAnimator::AnimationDone;
		this->mAnimatorBeingExecuted=nullptr;
		return IsDone;
	});
	if(mAnimators2.empty())
		mAnimatorID.Unregister();

	PropertiesChanged(ChangedProperties);
}
