namespace std
{
	template<class IT, class P> auto reversed_find(IT i1, IT i2, const P &par) // returns i1 if not found!
	{
		auto i=std::find(std::reverse_iterator<IT>(i1), std::reverse_iterator<IT>(i2), par);
		return i==std::reverse_iterator<IT>(i2)?i1:(i+1).base();
	}
	template<class IT, class P> auto reversed_find_if(IT i1, IT i2, const P &par) // returns i1 if not found!
	{
		auto i=std::find_if(std::reverse_iterator<IT>(i1), std::reverse_iterator<IT>(i2), par);
		return i==std::reverse_iterator<IT>(i2)?i1:(i+1).base();
	}
	template<class IT, class P> auto reversed_find_if_not(IT i1, IT i2, const P &par) // returns i1 if not found!
	{
		auto i=std::find_if_not(std::reverse_iterator<IT>(i1), std::reverse_iterator<IT>(i2), par);
		return i==std::reverse_iterator<IT>(i2)?i1:(i+1).base();
	}

}

template<class T>
T &as_non_const(const T &o)
{
	return const_cast<T &>(o);
}

template<class T, class P>
auto best_score_element(const T &Container, P ScoreCalculator)
{
	auto i=begin(Container);
	auto iend=end(Container);
	if(i==iend)
		return iend;
	auto BestScore=ScoreCalculator(*i);
	auto Best=i;
	for(++i; i!=iend; ++i)
	{
		auto Score=ScoreCalculator(*i);
		if(Score>BestScore)
		{
			BestScore=Score;
			Best=i;
		}
	}
	return Best;
}

template<class T>
auto AtEnd(T Function)
{
	struct cDummy
	{
		T Function;
		cDummy(T Function): Function(Function) {}
		cDummy(cDummy &&)=default;
		cDummy(const cDummy &)=delete;
		~cDummy() { Function(); }
	};
	return cDummy(Function);
}

constexpr size_t SumOfSizeof()
{
	return 0;
}

template<typename T, typename... R>
constexpr size_t SumOfSizeof(const T &t, const R &...r)
{
	return sizeof(t)+SumOfSizeof(r...);
}

template<class C1, class C2, class P>
void for_both(C1 &Container1, C2 &Container2, P Function)
{
	auto i1=begin(Container1);
	auto i2=begin(Container2);
	while(i1!=end(Container1)&&i2!=end(Container2))
	{
		Function(*i1, *i2);
		++i1;
		++i2;
	}
}

namespace std
{
	template<> struct hash<std::pair<std::string, std::string>>
	{
		size_t operator()(std::pair<std::string, std::string> const &sp) const
		{
			return std::hash<std::string>{}(sp.first)^std::hash<std::string>{}(sp.second);
		}
	};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

// from: http://stackoverflow.com/questions/12042824/how-to-write-a-type-trait-is-container-or-is-vector

template<typename T, typename _ = void>
struct is_container: std::false_type {};

template<typename... Ts>
struct is_container_helper {};

template<typename T>
struct is_container<
	T,
	std::conditional_t<
	false,
	is_container_helper<
	typename T::value_type,
	typename T::size_type,
	typename T::allocator_type,
	typename T::iterator,
	typename T::const_iterator,
	decltype(std::declval<T>().size()),
	decltype(std::declval<T>().begin()),
	decltype(std::declval<T>().end()),
	decltype(std::declval<T>().cbegin()),
	decltype(std::declval<T>().cend())
	>,
	void
	>
> : public std::true_type {};


///////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename Func, typename Tup, std::size_t... index>
decltype(auto) invoke_helper(Func&& func, Tup&& tup, std::index_sequence<index...>)
{
	return func(std::get<index>(std::forward<Tup>(tup))...);
}

template<typename Func, typename Tup>
decltype(auto) invoke(Func&& func, Tup&& tup)
{
	constexpr auto Size = std::tuple_size<typename std::decay<Tup>::type>::value;
	return invoke_helper(std::forward<Func>(func),
		std::forward<Tup>(tup),
		std::make_index_sequence<Size>{});
}

template<typename Func, typename Tup, std::size_t... index>
decltype(auto) invoke_move_helper(Func&& func, Tup&& tup, std::index_sequence<index...>)
{
	return func(std::get<index>(std::move<Tup>(tup))...);
}

template<typename Func, typename Tup>
decltype(auto) invoke_move(Func&& func, Tup&& tup)
{
	constexpr auto Size = std::tuple_size<typename std::decay<Tup>::type>::value;
	return invoke_move_helper(std::forward<Func>(func),
		std::forward<Tup>(tup),
		std::make_index_sequence<Size>{});
}

template<typename Base, typename Func, typename Tup, std::size_t... index>
decltype(auto) invoke_obj_helper(Base *base, Func&& func, Tup&& tup, std::index_sequence<index...>)
{
	return (base->*func)(std::get<index>(std::forward<Tup>(tup))...);
}

template<typename Base, typename Func, typename Tup>
decltype(auto) invoke_obj(Base *base, Func&& func, Tup&& tup)
{
	constexpr auto Size = std::tuple_size<typename std::decay<Tup>::type>::value;
	return invoke_obj_helper(base, std::forward<Func>(func),
		std::forward<Tup>(tup),
		std::make_index_sequence<Size>{});
}

template<typename Base, typename Func, typename Tup, std::size_t... index>
decltype(auto) invoke_obj_move_helper(Base *base, Func&& func, Tup&& tup, std::index_sequence<index...>)
{
	return (base->*func)(std::get<index>(std::move<Tup>(tup))...);
}

template<typename Base, typename Func, typename Tup>
decltype(auto) invoke_obj_move(Base *base, Func&& func, Tup&& tup)
{
	constexpr auto Size = std::tuple_size<typename std::decay<Tup>::type>::value;
	return invoke_obj_move_helper(base, std::forward<Func>(func),
		std::forward<Tup>(tup),
		std::make_index_sequence<Size>{});
}

/*
template <typename F, typename Tuple, size_t... I>
auto apply_impl(F&& f, Tuple&& t, std::integer_sequence<size_t, I...>)
-> decltype(std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...)) {
	return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}

template <typename F, typename Tuple>
auto apply(F&& f, Tuple&& t)
-> decltype(apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
	std::make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>())) {
	return apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
		std::make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>());
}
*/