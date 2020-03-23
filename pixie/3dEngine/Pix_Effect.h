#pragma once

class cEffectHolder
{
	ID3DXEffect *Effect;
public:
	cEffectHolder(const std::string &EffectName);
	virtual ~cEffectHolder();
	operator ID3DXEffect *() const { return Effect; }
};
