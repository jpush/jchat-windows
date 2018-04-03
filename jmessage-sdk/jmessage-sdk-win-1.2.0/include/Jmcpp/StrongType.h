#pragma once

#include <utility>
#include <type_traits>

/// @cond ignored
namespace Jmcpp::StrongType{

	template<class T, class Tag, template< class > class ...Ops>
	class StrongAlias :public Ops< StrongAlias<T, Tag, Ops...> > ...
	{
		T _v{};
	public:
		using underlying_type = T;

		StrongAlias() = default;

		template<class U = T, class = std::enable_if_t< std::is_constructible_v<T, U&&> > >
		StrongAlias(U&& value) :_v(std::forward<U>(value)){}


		T&			get() & { return _v; }
		T const&	get() const &{ return _v; }

		T&&			get() && { return std::move(_v); }
		T const&&	get() const &&{ return std::move(_v); }

	};

	template<class >
	struct ExplictConvertible;

	template<class T, class Tag, template< class ST > class ...Ops>
	struct ExplictConvertible < StrongAlias<T, Tag, Ops...> >
	{
		using self_type = StrongAlias<T, Tag, Ops...>;

		explicit operator T const&() const & { return static_cast<const self_type &>(*this).get(); }
		explicit operator T &() &{ return static_cast<self_type&>(*this).get(); }

		explicit operator T const&&() const && { return static_cast<const self_type&&>(*this).get(); }
		explicit operator T && () && { return static_cast<self_type &&>(*this).get(); }
	};


	template<class A>
	struct ImplictConvertible;

	template<class T, class Tag, template< class ST > class ...Ops>
	struct ImplictConvertible<StrongAlias<T, Tag, Ops...> >
	{
		using self_type = StrongAlias<T, Tag, Ops...>;

		operator T const&() const & { return static_cast<const self_type &>(*this).get(); }
		operator T &() &{ return static_cast<self_type&>(*this).get(); }

		operator T const&&() const && { return static_cast<const self_type&&>(*this).get(); }
		operator T && () && { return static_cast<self_type &&>(*this).get(); }
	};


	template<class>
	struct EqualityComparable;

	template<class T, class Tag, template< class ST > class ...Ops>
	struct EqualityComparable<StrongAlias<T, Tag, Ops...> >
	{
		using self_type = StrongAlias<T, Tag, Ops...>;

		friend auto operator==(self_type const& lhs, self_type const& rhs)->decltype(std::declval<const T&>() == std::declval<const T&>())
		{
			return lhs.get() == rhs.get();
		}

		friend auto operator!=(self_type const& lhs, self_type const& rhs)->decltype(std::declval<const T&>() != std::declval<const T&>())
		{
			return lhs.get() != rhs.get();
		}
	};

	template<class>
	struct OrderingComparable;

	template<class T, class Tag, template< class ST > class ...Ops>
	struct OrderingComparable<StrongAlias<T, Tag, Ops...> >
	{
		using self_type = StrongAlias<T, Tag, Ops...>;

		friend auto operator<(self_type const& lhs, self_type const& rhs)->decltype(std::declval<const T&>() < std::declval<const T&>())
		{
			return lhs.get() < rhs.get();
		}

		friend auto operator<=(self_type const& lhs, self_type const& rhs)->decltype(std::declval<const T&>() <= std::declval<const T&>())
		{
			return lhs.get() <= rhs.get();
		}

		friend auto operator>(self_type const& lhs, self_type const& rhs)->decltype(std::declval<const T&>() > std::declval<const T&>())
		{
			return lhs.get() < rhs.get();
		}

		friend auto operator>=(self_type const& lhs, self_type const& rhs)->decltype(std::declval<const T&>() >= std::declval<const T&>())
		{
			return lhs.get() >= rhs.get();
		}
	};

}

namespace std{
	template<class T, class Tag, template< class > class ...Ops>
	struct hash<Jmcpp::StrongType::StrongAlias<T, Tag, Ops...> >{
		auto operator()(Jmcpp::StrongType::StrongAlias<T, Tag, Ops...> const& v) const noexcept{
			return hash<T>{}(v.get());
		}
	};
}

/// @endcond
