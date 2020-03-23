#pragma once

template<class T> class tSingleton
{
public:
	tSingleton()=default;
	tSingleton(const tSingleton &)=delete;
	tSingleton &operator=(const tSingleton &)=delete;
	static T &Get()
	{
		static T *Instance=NULL;
		return Instance?*Instance:*(Instance=new T);
	}
};