#pragma once

template<typename ...Ts> class tFunctions: protected tRegisteredObjects<std::function<void(Ts...)>>
{
public:
	void Call(Ts ...Args)
	{
		this->ForEach([&](auto &Function) { Function(Args...); });
	}
	void operator()(Ts ...Args) { Call(Args...); }
	using tRegisteredObjects<std::function<void(Ts...)>>::Register;
};