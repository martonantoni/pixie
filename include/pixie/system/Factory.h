#pragma once

template<class T, class ...Ps>
class tFactory: public tSingleton<tFactory<T, Ps...>>
{
public:
	using cCreatorFunction = std::function<std::unique_ptr<T>(Ps ...)>;
private:
	std::unordered_map<std::string, cCreatorFunction> mCreators;
public:
	template<class SubT>
	static std::unique_ptr<T> CreatorFunction(Ps ...ps)
	{
		return std::make_unique<SubT>(std::forward<Ps>(ps)...);
	}
	std::unique_ptr<T> Create(const std::string &Name, Ps ...ps) const
	{
		auto i=mCreators.find(Name);
		if(ASSERTFALSE(i==mCreators.end()))
			return nullptr;
		return i->second(std::forward<Ps>(ps)...);
	}
	void RegisterCreator(const std::string &Name, const cCreatorFunction &CreatorFunction)
	{
		auto &CreatorInMap=mCreators[Name];
		ASSERT(!CreatorInMap);
		CreatorInMap=CreatorFunction;
	}
};

#define REGISTER_INTO_FACTORY(Factory, Class, Name) \
	struct cFactoryRegistratorFor##Class \
	{ \
		cFactoryRegistratorFor##Class() \
		{ \
			Factory::Get().RegisterCreator(Name, &Factory::CreatorFunction<Class> ); \
		} \
	} FactoryRegistratorInstanceFor##Class;
