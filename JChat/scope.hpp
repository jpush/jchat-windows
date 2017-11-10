#pragma once

#include <limits>
#include <type_traits>
#include <exception>

namespace cpp
{
	// contribution by (c) Eric Niebler 2016, slightly adapted by Peter Sommerlad
	namespace __scope {
		namespace hidden{
			template<typename T>
			constexpr std::conditional_t<
				(!std::is_nothrow_move_assignable<T>::value &&
				 std::is_copy_assignable<T>::value),
				T const &,
				T &&>
				_move_assign_if_noexcept(T &x) noexcept
			{
				return std::move(x);
			}
		}
		template<typename T>
		class _box
		{
			T value;
			_box(T const &t) noexcept(noexcept(T(t)))
				: value(t)
			{}
			_box(T &&t) noexcept(noexcept(T(std::move_if_noexcept(t))))
				: value(std::move_if_noexcept(t))
			{}

		public:
			template<typename TT, class U,
				typename = std::enable_if_t<std::is_constructible<T, TT>::value>>
				explicit _box(TT &&t, U &&guard) noexcept(noexcept(_box((T &&)t)))
				: _box((T &&)t)
			{
				guard.release();
			}
			T &get() noexcept
			{
				return value;
			}
			T const &get() const noexcept
			{
				return value;
			}
			T &&move() noexcept
			{
				return std::move(value);
			}
			void reset(T const &t) noexcept(noexcept(value = t))
			{
				value = t;
			}
			void reset(T &&t) noexcept(noexcept(value = hidden::_move_assign_if_noexcept(t)))
			{
				value = hidden::_move_assign_if_noexcept(t);
			}

		};

		template<typename T>
		class _box<T &>
		{
			std::reference_wrapper<T> value;
		public:
			template<typename TT, class U,
				typename = std::enable_if_t<std::is_convertible<TT, T &>::value>>
				_box(TT &&t, U &&guard) noexcept(noexcept(static_cast<T &>((TT &&)t)))
				: value(static_cast<T &>(t))
			{
				guard.release();
			}
			T &get() const noexcept
			{
				return value.get();
			}
			T &move() const noexcept
			{
				return get();
			}
			void reset(T &t) noexcept
			{
				value = std::ref(t);
			}
		};

		template<bool B>
		using _bool = std::integral_constant<bool, B>;

	}


	namespace __scope {
		template<typename T>
		using _is_nothrow_movable =
			_bool<std::is_nothrow_move_constructible<T>::value &&
			std::is_nothrow_move_assignable<T>::value>;

		template<typename T>
		constexpr bool _is_nothrow_movable_v = _is_nothrow_movable<T>::value;

		//template<typename T>
		//using _is_nothrow_movable =
		//    std::integral_constant<bool,
		//        std::is_nothrow_move_constructible<T>::value &&
		//        std::is_nothrow_move_assignable<T>::value>;

		// the following is never ODR used. it is only relevant for obtaining a noexcept value
		template<typename T>
		T for_noexcept_on_copy_construction(const T &t) noexcept(noexcept(T(t)))
		{
			return t;
		}

	} // detail

}

namespace cpp
{
	namespace __scope{

		struct _{ void operator()() noexcept{} };

		struct on_exit_policy
		{
		protected:
			constexpr static bool noexcept_ = true;
			bool execute_ = true;
			bool should_execute() noexcept
			{
				return execute_;
			}
		public:
			void release() noexcept
			{
				execute_ = false;
			}
		};


		struct on_fail_policy
		{
		protected:
			constexpr static bool noexcept_ = true;
			int ec_ = std::uncaught_exceptions();
			bool should_execute() noexcept
			{
				return ec_ < std::uncaught_exceptions();
			}
		public:
			void release() noexcept
			{
				ec_ = (std::numeric_limits<int>::max)();
			}

		};

		struct on_success_policy
		{
		protected:
			constexpr static bool noexcept_ = false;
			int ec_ = std::uncaught_exceptions();
			bool should_execute() noexcept
			{
				return ec_ >= std::uncaught_exceptions();
			}
		public:
			void release() noexcept
			{
				ec_ = -1;
			}
		};
	}

	template<class EF, class Policy = __scope::on_exit_policy>
	class basic_scope_exit;

	//PS: It would be nice if just the following would work in C++17
	//PS: however, we need a real class for template argument deduction
	//PS: and a deduction guide, because the ctors are partially instantiated
	//template<class EF>
	//using scope_exit = basic_scope_exit<EF, detail::on_exit_policy>;

	template<class EF>
	struct scope_exit : basic_scope_exit<EF, __scope::on_exit_policy>{
		using basic_scope_exit<EF, __scope::on_exit_policy>::basic_scope_exit;
	};

	//template <class EF>
	//scope_exit(EF &&ef)->scope_exit<std::decay_t<EF>>;

	//template<class EF>
	//using scope_fail = basic_scope_exit<EF, detail::on_fail_policy>;

	template<class EF>
	struct scope_fail : basic_scope_exit<EF, __scope::on_fail_policy>{
		using basic_scope_exit<EF, __scope::on_fail_policy>::basic_scope_exit;
	};

	//template <class EF>
	//scope_fail(EF &&ef)->scope_fail<std::decay_t<EF>>;

	//template<class EF>
	//using scope_success = basic_scope_exit<EF, detail::on_success_policy>;

	template<class EF>
	struct scope_success : basic_scope_exit<EF, __scope::on_success_policy>{
		using basic_scope_exit<EF, __scope::on_success_policy>::basic_scope_exit;
	};

	//template <class EF>
	//scope_success(EF &&ef)->scope_success<std::decay_t<EF>>;



	namespace __scope{
		// DETAIL:
		template<class Policy, class EF>
		auto _make_guard(EF &&ef)
		{
			return basic_scope_exit<std::decay_t<EF>, Policy>(std::forward<EF>(ef));
		}
		struct _empty_scope_exit
		{
			void release() noexcept {}
		};

	}

	// Requires: EF is Callable
	// Requires: EF is nothrow MoveConstructible OR CopyConstructible
	template<class EF, class Policy /*= on_exit_policy*/>
	class basic_scope_exit : Policy
	{
		__scope::_box<EF> exit_function;

		static auto _make_failsafe(std::true_type, const void *)
		{
			return __scope::_empty_scope_exit{};
		}
		template<typename Fn>
		static auto _make_failsafe(std::false_type, Fn *fn)
		{
			return basic_scope_exit<Fn &, Policy>(*fn);
		}
		template<typename EFP>
		using _ctor_from = std::is_constructible<__scope::_box<EF>, EFP, __scope::_empty_scope_exit>;
		template<typename EFP>
		using _noexcept_ctor_from = __scope::_bool<std::is_nothrow_constructible < __scope::_box<EF>, EFP, __scope::_empty_scope_exit> ::value >;
	public:
		template<typename EFP, typename = std::enable_if_t<_ctor_from<EFP>::value>>
		explicit basic_scope_exit(EFP &&ef) noexcept(_noexcept_ctor_from<EFP>::value)
			: exit_function((EFP &&)ef, _make_failsafe(_noexcept_ctor_from<EFP>{}, &ef))
		{}
		basic_scope_exit(basic_scope_exit &&that) noexcept(noexcept(__scope::_box<EF>(that.exit_function.move(), that)))
			: Policy(that), exit_function(that.exit_function.move(), that)
		{}
		~basic_scope_exit() noexcept(Policy::noexcept_ || noexcept(exit_function.get()()))
		{
			if(this->should_execute())
				exit_function.get()();
		}
		basic_scope_exit(const basic_scope_exit &) = delete;
		basic_scope_exit &operator=(const basic_scope_exit &) = delete;
		basic_scope_exit &operator=(basic_scope_exit &&) = delete;

		using Policy::release;
	};

	template<class EF, class Policy>
	void swap(basic_scope_exit<EF, Policy> &, basic_scope_exit<EF, Policy> &) = delete;

	template<class EF>
	auto make_scope_exit(EF &&ef)
		noexcept(std::is_nothrow_constructible<std::decay_t<EF>, EF>::value)
	{
		return scope_exit<std::decay_t<EF>>(std::forward<EF>(ef));
	}

	template<class EF>
	auto make_scope_fail(EF &&ef)
		noexcept(std::is_nothrow_constructible<std::decay_t<EF>, EF>::value)
	{
		return scope_fail<std::decay_t<EF>>(std::forward<EF>(ef));
	}

	template<class EF>
	auto make_scope_success(EF &&ef)
		noexcept(std::is_nothrow_constructible<std::decay_t<EF>, EF>::value)
	{
		return scope_success<std::decay_t<EF>>(std::forward<EF>(ef));
	}

	namespace __scope
	{
		struct scope_on_exit{};
		struct scope_on_fail{};
		struct scope_on_success{};

		template<class FN>
		auto operator+(scope_on_exit, FN&& fn)
		{
			return make_scope_exit(std::forward<FN>(fn));
		}

		template<class FN>
		auto operator+(scope_on_fail, FN&& fn)
		{
			return make_scope_fail(std::forward<FN>(fn));;
		}
		template<class FN>
		auto operator+(scope_on_success, FN&& fn)
		{
			return make_scope_success(std::forward<FN>(fn));
		}
	}

}


namespace cpp
{
	template<typename R, typename D>
	class unique_resource
	{
		static_assert(std::is_nothrow_move_constructible_v<R> ||
					  std::is_copy_constructible_v<R>,
					  "resource must be notrhow_move_constructible or copy_constructible");
		static_assert(std::is_nothrow_move_constructible_v<D> ||
					  std::is_copy_constructible_v<D>,
					  "deleter must be notrhow_move_constructible or copy_constructible");

		static const unique_resource &this_; // never ODR used! Just for getting no_except() expr

		__scope::_box<R> resource;
		__scope::_box<D> deleter;
		bool execute_on_destruction = true;

		static constexpr auto is_nothrow_delete_v = __scope::_bool<noexcept(std::declval<D &>()(std::declval<R &>()))>::value;

		static constexpr auto is_nothrow_swappable_v = __scope::_bool<is_nothrow_delete_v &&
			__scope::_is_nothrow_movable_v<R> &&
			__scope::_is_nothrow_movable_v<D>>::value;

	public:
		template<typename RR, typename DD,
			typename = std::enable_if_t<std::is_constructible<__scope::_box<R>, RR, __scope::_empty_scope_exit>::value &&
			std::is_constructible<__scope::_box<D>, DD, __scope::_empty_scope_exit>::value>>
			explicit unique_resource(RR &&r, DD &&d)
			noexcept(noexcept(__scope::_box<R>((RR &&)r, __scope::_empty_scope_exit{})) &&
					 noexcept(__scope::_box<D>((DD &&)d, __scope::_empty_scope_exit{})))
			: resource((RR &&)r, make_scope_exit([&]{ d(r); }))
			, deleter((DD &&)d, make_scope_exit([&, this]{ d(get()); }))
		{}
		unique_resource(unique_resource &&that)
			noexcept(noexcept(__scope::_box<R>(that.resource.move(), __scope::_empty_scope_exit{})) &&
					 noexcept(__scope::_box<D>(that.deleter.move(), __scope::_empty_scope_exit{})))
			: resource(that.resource.move(), __scope::_empty_scope_exit{})
			, deleter(that.deleter.move(),
					  make_scope_exit([&, this]{ that.get_deleter()(get()); that.release(); }))
			, execute_on_destruction(std::exchange(that.execute_on_destruction, false))
		{}

		unique_resource &operator=(unique_resource &&that)
			noexcept(is_nothrow_delete_v &&
					 std::is_nothrow_move_assignable<R>::value &&
					 std::is_nothrow_move_assignable<D>::value)
		{
			static_assert(std::is_nothrow_move_assignable<R>::value ||
						  std::is_copy_assignable<R>::value,
						  "The resource must be nothrow-move assignable, or copy assignable");
			static_assert(std::is_nothrow_move_assignable<D>::value ||
						  std::is_copy_assignable<D>::value,
						  "The deleter must be nothrow-move assignable, or copy assignable");
			if(&that == this)
				return *this;
			reset();
			if(std::is_nothrow_move_assignable<__scope::_box<R>>::value)
			{
				deleter = _move_assign_if_noexcept(that.deleter);
				resource = _move_assign_if_noexcept(that.resource);
			}
			else if(std::is_nothrow_move_assignable<__scope::_box<D>>::value)
			{
				resource = _move_assign_if_noexcept(that.resource);
				deleter = _move_assign_if_noexcept(that.deleter);
			}
			else
			{
				resource = _as_const(that.resource);
				deleter = _as_const(that.deleter);
			}
			execute_on_destruction = std::exchange(that.execute_on_destruction, false);
			return *this;
		}
		~unique_resource() //noexcept(is_nowthrow_delete_v)
		{
			reset();
		}
		void swap(unique_resource &that) noexcept(is_nothrow_swappable_v)
		{
			if(is_nothrow_swappable_v)
			{
				using std::swap;
				swap(execute_on_destruction, that.execute_on_destruction);
				swap(resource.get(), that.resource.get());
				swap(deleter.get(), that.deleter.get());
			}
			else
			{
				auto tmp = std::move(*this);
				*this = std::move(that);
				that = std::move(tmp);
			}
		}
		void reset()
			noexcept(is_nothrow_delete_v)
		{
			if(execute_on_destruction)
			{
				execute_on_destruction = false;
				get_deleter()(get());
			}
		}
		template<typename RR>
		auto reset(RR &&r)
			noexcept(is_nothrow_delete_v && noexcept(resource.reset((RR &&)r)))
			-> decltype(resource.reset((RR &&)r), void())
		{
			auto &&guard = make_scope_exit([&, this]{ get_deleter()(r); });
			reset();
			resource.reset((RR &&)r);
			execute_on_destruction = true;
			guard.release();
		}
		void release() noexcept
		{
			execute_on_destruction = false;
		}
		decltype(auto) get() const noexcept
		{
			return resource.get();
		}
		decltype(auto) get_deleter() noexcept
		{
			return deleter.get();
		}
		decltype(auto) get_deleter() const noexcept
		{
			return deleter.get();
		}
		auto operator->() const noexcept(noexcept(__scope::for_noexcept_on_copy_construction(this_.get())))
		{
			return get();
		}
		decltype(auto) operator*() const noexcept
		{
			return *get();
		}

		unique_resource& operator=(const unique_resource &) = delete;
		unique_resource(const unique_resource &) = delete;

	};


	template<typename R, typename D>
	void swap(unique_resource<R, D> &lhs, unique_resource<R, D> &rhs)
		noexcept(__scope::_is_nothrow_movable_v<R> && __scope::_is_nothrow_movable_v<D>)
	{
		lhs.swap(rhs);
	}
	//template<typename R, typename D>
	//unique_resource(R &&r, D &&d)->unique_resource<std::decay_t<R>, std::decay_t<D>>;

	template<typename R, typename D>
	auto make_unique_resource(R &&r, D &&d)
		noexcept(noexcept(unique_resource<std::decay_t<R>, std::decay_t<D>>{
		std::forward<R>(r), std::forward<D>(d)}))
	{
		return unique_resource<std::decay_t<R>, std::decay_t<D>>{
			std::forward<R>(r), std::forward<D>(d)};
	}
		//template<typename R, typename D>
		//unique_resource(std::reference_wrapper<R> r, D &&d)->unique_resource<R &, std::decay_t<D>>; // should need to unwrap, but how?

		template<typename R, typename D>
		auto make_unique_resource(std::reference_wrapper<R> r, D &&d)
			noexcept(noexcept(unique_resource<R &, std::decay_t<D>>{r.get(), std::forward<D>(d)}))
		{
			return unique_resource<R &, std::decay_t<D>>{r.get(), std::forward<D>(d)};
		}

		template<typename R, typename D, typename S>
		auto make_unique_resource_checked(R &&r, const S &invalid, D &&d)
			noexcept(noexcept(make_unique_resource(std::forward<R>(r), std::forward<D>(d))))
		{
			bool must_release = bool(r == invalid);
			auto ur = make_unique_resource(std::forward<R>(r), std::forward<D>(d));
			if(must_release)
				ur.release();
			return ur;
		}
}

#ifdef __INTELLISENSE__

#define SCOPE_EXIT(x) ::cpp::scope_exit<::cpp::__scope::_> x{::cpp::__scope::_{}};
#define SCOPE_FAIL(x) ::cpp::scope_fail<::cpp::__scope::_> x{::cpp::__scope::_{}};
#define SCOPE_SUCCESS(x) ::cpp::scope_success<::cpp::__scope::_> x{::cpp::__scope::_{}};

#else

#define __CPP__ELEM_SIZE_PREFIX__CPP__ELEM_SIZE_POSTFIX ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,0
#define __CPP__ELEM_SIZE_I(__p0,__p1,__p2,__p3,__p4,__p5,__p6,__p7,__p8,__p9,__p10,__p11,__p12,__p13,__p14,__p15,__p16,__p17,__p18,__p19,__p20,__p21,__p22,__p23,__p24,__p25,__p26,__p27,__p28,__p29,__p30,__p31,__n,...) __n
#define __CPP__ELEM_SIZE_II(__args) __CPP__ELEM_SIZE_I __args
#define __CPP__ELEM_SIZE(...) __CPP__ELEM_SIZE_II(( __CPP__ELEM_SIZE_PREFIX_ ## __VA_ARGS__ ## _CPP__ELEM_SIZE_POSTFIX,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0))

#define __CPP_SCOPE_CAT2(a,b) a##b
#define __CPP_SCOPE_CAT(a,b) __CPP_SCOPE_CAT2(a,b)

#define __CPP_SCOPE_GET_ELEM_1( _1 ,... )  _1
#define __CPP_SCOPE_GET_ELEM_2( _1 ,_2 ,... )  _2
//#define __CPP_SCOPE_GET_ELEM_3( _1 ,_2 ,_3 ,... )  _3

#define __CPP_SCOPE_GET_ELEM(N,ELEMS) __CPP_SCOPE_CAT(__CPP_SCOPE_GET_ELEM_, N) ELEMS
#define __CPP_SCOPE_GET_LAST(...) __CPP_SCOPE_GET_ELEM( __CPP__ELEM_SIZE(__VA_ARGS__) ,  (__VA_ARGS__) )

#define __CPP_SCOPE_EXIT_VARIABLE(...)		__CPP_SCOPE_VARIABLE_NAME(___CPP_SCOPE_EXIT_STATE) ,__VA_ARGS__
#define __CPP_SCOPE_FAIL_VARIABLE(...)		__CPP_SCOPE_VARIABLE_NAME(___CPP_SCOPE_FAIL_STATE) ,__VA_ARGS__
#define __CPP_SCOPE_SUCCESS_VARIABLE(...)	__CPP_SCOPE_VARIABLE_NAME(___CPP_SCOPE_SUCCESS_STATE) ,__VA_ARGS__

#if defined(__COUNTER__)
#define __CPP_SCOPE_VARIABLE_NAME(x) __CPP_SCOPE_CAT(x,__COUNTER__)
#else
#define __CPP_SCOPE_VARIABLE_NAME(x) __CPP_SCOPE_CAT(x,__LINE__)
#endif // __COUNTER__

#define SCOPE_EXIT(...) \
			auto __CPP_SCOPE_GET_LAST(__CPP_SCOPE_EXIT_VARIABLE(__VA_ARGS__)) = ::cpp::__scope::scope_on_exit{} +[&]() noexcept

#define SCOPE_FAIL(...) \
			auto __CPP_SCOPE_GET_LAST(__CPP_SCOPE_FAIL_VARIABLE(__VA_ARGS__)) = ::cpp::__scope::scope_on_fail{} +[&]() noexcept

#define SCOPE_SUCCESS(...) \
			auto __CPP_SCOPE_GET_LAST(__CPP_SCOPE_SUCCESS_VARIABLE(__VA_ARGS__)) = ::cpp::__scope::scope_on_success{} +[&]()

#endif
