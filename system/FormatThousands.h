#pragma once

inline std::string FormatThousands(int Value)
{
	if(Value>=0)
		return Value<1000?fmt::sprintf("%d",Value):Value<1000000?fmt::sprintf("%d,%.03d",Value/1000,Value%1000):fmt::sprintf("%d,%.03d,%.03d",Value/1000000,(Value/1000)%1000,Value%1000);
	else
	{
		int AbsValue=std::abs(Value);
		return AbsValue<1000?fmt::sprintf("-%d",AbsValue):AbsValue<1000000?fmt::sprintf("-%d,%.03d",AbsValue/1000,AbsValue%1000):fmt::sprintf("-%d,%.03d,%.03d",AbsValue/1000000,(AbsValue/1000)%1000,AbsValue%1000);
	}
}

inline std::string FormatThousands64(__int64 Value)
{
	if(!Value)
		return "0";
	__int64 AbsValue=Abs(Value);
	std::string Result;
	for(;;)
	{
		__int64 NextAbsValue=AbsValue/1000;
		if(NextAbsValue)
			Result=fmt::sprintf("%03d",AbsValue%1000)+(Result.length()?","+Result:Result);
		else
		{
			Result=fmt::sprintf("%d",AbsValue%1000)+(Result.length()?","+Result:Result);
			break;
		}
		AbsValue=NextAbsValue;
	}
	return Value>=0?Result:fmt::sprintf("-%s",Result);
}

