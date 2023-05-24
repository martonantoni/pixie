#pragma once

class cDestroyTrapGuard;

class cDestroyTrap
{
	typedef tIntrusiveList<cDestroyTrapGuard> cGuards;
	cGuards mGuards;
public:
	~cDestroyTrap();
	void Register(cDestroyTrapGuard *Guard) { mGuards.push_back(Guard); }
	void Unregister(cDestroyTrapGuard *Guard) { mGuards.remove(Guard); }
};

class cDestroyTrapGuard: public tIntrusiveListItemBase<cDestroyTrapGuard>
{
	cDestroyTrap &mTrap;
	bool mTriggered=false;
	friend class cDestroyTrap;
public:
	cDestroyTrapGuard(const cDestroyTrapGuard &) = delete;
	cDestroyTrapGuard(cDestroyTrap &Trap)
		: mTrap(Trap)
	{
		mTrap.Register(this);
	}
	~cDestroyTrapGuard()
	{
		if(!mTriggered)
			mTrap.Unregister(this);
	}
	bool Triggered() const
	{
		return mTriggered;
	}
};
