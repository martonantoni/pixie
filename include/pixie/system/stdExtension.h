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


template <typename Tuple, typename... Ts>
using tuple_prepend_t = decltype(std::tuple_cat(std::declval<std::tuple<Ts...>>(), std::declval<Tuple>()));

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

