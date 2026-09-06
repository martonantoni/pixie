#pragma once

inline std::string FormatThousands(int Value)
{
	if (Value >= 0)
		return Value < 1000 ? std::format("{}", Value) : Value < 1000000 ? std::format("{},{}", Value / 1000, Value % 1000) : std::format("{},{},{}", Value / 1000000, (Value / 1000) % 1000, Value % 1000);
	else
	{
		int AbsValue = std::abs(Value);
		return AbsValue < 1000 ? std::format("-{}", AbsValue) : AbsValue < 1000000 ? std::format("-{},{}", AbsValue / 1000, AbsValue % 1000) : std::format("-{},{},{}", AbsValue / 1000000, (AbsValue / 1000) % 1000, AbsValue % 1000);
	}
}

inline std::string FormatThousands64(__int64 Value)
{
	if (!Value)
		return "0";
	__int64 AbsValue = std::abs(Value);
	std::string Result;
	for (;;)
	{
		__int64 NextAbsValue = AbsValue / 1000;
		if (NextAbsValue)
			Result = std::format("{:03}", AbsValue % 1000) + (Result.length() ? "," + Result : Result);
		else
		{
			Result = std::format("{}", AbsValue % 1000) + (Result.length() ? "," + Result : Result);
			break;
		}
		AbsValue = NextAbsValue;
	}
	return Value >= 0 ? Result : std::format("-{}", Result);
}

