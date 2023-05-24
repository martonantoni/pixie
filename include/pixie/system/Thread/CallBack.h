#pragma once

namespace ThreadCallbackHelper
{
	typedef void (*CallbackHelperFunction)(char *&MessageBuffer);

	template<typename... Ts> using parameters_tuple=std::tuple<typename std::remove_reference<Ts>::type...>;

	template<typename... Ts> inline void ToStream(char *&Stream, Ts &&...Objects)
	{
		if(sizeof...(Ts)!=0)
		{
			typedef parameters_tuple<Ts...> tuple_type;
			new (Stream) tuple_type(std::forward<Ts>(Objects)...);
			Stream+=sizeof(tuple_type);
		}
	}

	template<typename BASE, typename FUNCTION, typename... PARS> inline void CallbackHelper(char *&MessageBuffer)
	{
		std::tuple<BASE *, FUNCTION> &Target=*(std::tuple<BASE *, FUNCTION> *)MessageBuffer;
		MessageBuffer+=sizeof(Target);
		typedef parameters_tuple<PARS...> tuple_type;
		tuple_type &Parameters=*(tuple_type *)MessageBuffer;
		if(sizeof...(PARS)!=0)
		{
			MessageBuffer+=sizeof(Parameters);
		}
		invoke_obj_move(std::get<BASE *>(Target), std::get<FUNCTION>(Target), Parameters);
		Parameters.tuple_type::~tuple();
	}
}

template<typename BaseClass,typename FunctionType, typename... PARS> 
	void CallBack(cThread *Thread,eCallbackType Type,BaseClass *Base,FunctionType Function, PARS &&...Parameters)
{
	if(Thread->IsInThread()&&Type!=eCallbackType::NoImmediate)
	{
		(Base->*Function)(std::forward<PARS>(Parameters)...);
		return;
	}
	auto HelperToUse=&ThreadCallbackHelper::CallbackHelper<BaseClass,FunctionType,PARS...>;
	char *Buffer=Thread->LockCallbackBuffer(sizeof(HelperToUse)+sizeof(std::tuple<BaseClass*,FunctionType>)+(sizeof...(PARS)==0?0:sizeof(ThreadCallbackHelper::parameters_tuple<PARS...>)));
	*(decltype(HelperToUse) *)Buffer=HelperToUse;
	Buffer+=sizeof(HelperToUse);
	ThreadCallbackHelper::ToStream<BaseClass *,FunctionType>(Buffer, std::forward<BaseClass *>(Base), std::forward<FunctionType>(Function));
	ThreadCallbackHelper::ToStream(Buffer, std::forward<PARS>(Parameters)...);
 	Thread->ReleaseCallbackBuffer(Type);
}
