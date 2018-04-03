#pragma once

#include <exception>
#include <type_traits>
#include <utility>
#include <limits>

namespace cpp
{
	// contribution by (c) Eric Niebler 2016, slightly adapted by Peter Sommerlad
	namespace __scope {
		namespace hidden{
			template<typename T>
			constexpr std::conditional_t<
				std::is_nothrow_move_assignable_v<T>,
				T &&,
				T const &>
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
				typename = std::enable_if_t<std::is_constructible_v<T, TT>>>
				explicit _box(TT &&t, U &&guard) noexcept(noexcept(_box((T &&)t)))
				: _box(std::forward<TT>(t))
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
				typename = std::enable_if_t<std::is_convertible_v<TT, T &>>>
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
	}
}

namespace cpp
{
	namespace __scope{

		struct _{ void operator()() const noexcept{} };

		struct on_exit_policy
		{
		protected:
			constexpr static bool noexcept_ = true;
			bool execute_ = true;
			bool should_execute() const noexcept
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
			bool should_execute() const noexcept
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
			bool should_execute() const noexcept
			{
				return ec_ >= std::uncaught_exceptions();
			}
		public:
			void release() noexcept
			{
				ec_ = -1;
			}
		};

		template<class EF, class Policy = __scope::on_exit_policy>
		class basic_scope_exit;
	}


	//PS: It would be nice if just the following would work in C++17
	//PS: however, we need a real class for template argument deduction
	//PS: and a deduction guide, because the ctors are partially instantiated
	//template<class EF>
	//using scope_exit = basic_scope_exit<EF, detail::on_exit_policy>;

	template<class EF>
	struct scope_exit :__scope::basic_scope_exit<EF, __scope::on_exit_policy>{
		using __scope::basic_scope_exit<EF, __scope::on_exit_policy>::basic_scope_exit;
	};

	//template <class EF>
	//scope_exit(EF)->scope_exit<EF>;

	//template<class EF>
	//using scope_fail = basic_scope_exit<EF, detail::on_fail_policy>;

	template<class EF>
	struct scope_fail :__scope::basic_scope_exit<EF, __scope::on_fail_policy>{
		using __scope::basic_scope_exit<EF, __scope::on_fail_policy>::basic_scope_exit;
	};

	//template <class EF>
	//scope_fail(EF)->scope_fail<EF>;

	template<class EF>
	struct scope_success :__scope::basic_scope_exit<EF, __scope::on_success_policy>{
		using __scope::basic_scope_exit<EF, __scope::on_success_policy>::basic_scope_exit;
	};

	//template <class EF>
	//scope_success(EF)->scope_success<EF>;

	namespace __scope{

		struct _empty_scope_exit
		{
			void release() const noexcept {}
		};

		// Requires: EF is Callable
		// Requires: EF is nothrow MoveConstructible OR CopyConstructible
		template<class EF, class Policy /*= on_exit_policy*/>
		class basic_scope_exit : Policy
		{
			static_assert(std::is_invocable_v<EF>, "scope guard must be callable");
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
			using _noexcept_ctor_from = std::bool_constant<std::is_nothrow_constructible < __scope::_box<EF>, EFP, __scope::_empty_scope_exit> ::value >;
		public:
		#if defined(__INTELLISENSE__)
			template<typename EFP>
			explicit basic_scope_exit(EFP &&ef){}
		#else
			template<typename EFP, typename = std::enable_if_t<_ctor_from<EFP>::value>>
			explicit basic_scope_exit(EFP &&ef) noexcept(_noexcept_ctor_from<EFP>::value)
				: exit_function(std::forward<EFP>(ef), _make_failsafe(_noexcept_ctor_from<EFP>{}, &ef))
			{}
		#endif
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
	}

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
		static_assert((std::is_move_constructible_v<R> && std::is_nothrow_move_constructible_v<R>) ||
					  std::is_copy_constructible_v<R>,
					  "resource must be nothrow_move_constructible or copy_constructible");
		static_assert((std::is_move_constructible_v<R> && std::is_nothrow_move_constructible_v<D>) ||
					  std::is_copy_constructible_v<D>,
					  "deleter must be nothrow_move_constructible or copy_constructible");

		static const unique_resource &this_; // never ODR used! Just for getting no_except() expr

		__scope::_box<R> resource;
		__scope::_box<D> deleter;
		bool execute_on_destruction = true;

		static constexpr auto is_nothrow_delete_v = std::bool_constant<noexcept(std::declval<D &>()(std::declval<R &>()))>::value;

	public://private
		template<typename RR, typename DD,
			typename = std::enable_if_t<std::is_constructible_v<__scope::_box<R>, RR, __scope::_empty_scope_exit> &&
			std::is_constructible_v<__scope::_box<D>, DD, __scope::_empty_scope_exit>>>
			unique_resource(RR &&r, DD &&d, bool should_run)
			noexcept(noexcept(__scope::_box<R>(std::forward<RR>(r), __scope::_empty_scope_exit{})) &&
					 noexcept(__scope::_box<D>(std::forward<DD>(d), __scope::_empty_scope_exit{})))
			: resource(std::forward<RR>(r), make_scope_exit([&] {if(should_run) d(r); }))
			, deleter(std::forward<DD>(d), make_scope_exit([&, this] {if(should_run) d(get()); }))
			, execute_on_destruction(should_run)
		{

		}

		//template<class MR, class MD, class S>
		//friend unique_resource<std::decay_t<MR>, std::decay_t<MD>>
		//	make_unique_resource_checked(MR &&r, const S &invalid, MD &&d)
		//	noexcept(std::is_nothrow_constructible_v<std::decay_t<MR>, MR> &&
		//			 std::is_nothrow_constructible_v<std::decay_t<MD>, MD>);

	public:
		template<typename RR, typename DD,
			typename = std::enable_if_t<std::is_constructible<__scope::_box<R>, RR, __scope::_empty_scope_exit>::value &&
			std::is_constructible<__scope::_box<D>, DD, __scope::_empty_scope_exit>::value >
		>
			explicit unique_resource(RR &&r, DD &&d)
			noexcept(noexcept(__scope::_box<R>(std::forward<RR>(r), __scope::_empty_scope_exit{})) &&
					 noexcept(__scope::_box<D>(std::forward<DD>(d), __scope::_empty_scope_exit{})))
			: resource(std::forward<RR>(r), make_scope_exit([&] {d(r); }))
			, deleter(std::forward<DD>(d), make_scope_exit([&, this] {d(get()); }))
		{}
		unique_resource(unique_resource&& that)
			noexcept(noexcept(__scope::_box<R>(that.resource.move(), __scope::_empty_scope_exit{})) &&
					 noexcept(__scope::_box<D>(that.deleter.move(), __scope::_empty_scope_exit{})))
			: resource(that.resource.move(), __scope::_empty_scope_exit{})
			, deleter(that.deleter.move(), make_scope_exit([&, this] { that.get_deleter()(get()); that.release(); }))
			, execute_on_destruction(std::exchange(that.execute_on_destruction, false))
		{ }

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
			if constexpr (std::is_nothrow_move_assignable_v<__scope::_box<R>>)
				if constexpr (std::is_nothrow_move_assignable_v<__scope::_box<D>>)
				{
					resource = std::forward<__scope::_box<R>>(that.resource);
					deleter = std::forward<__scope::_box<D>>(that.deleter);
				}
				else
				{
					deleter = std::as_const(that.deleter);
					resource = std::forward<__scope::_box<R>>(that.resource);
				}
			else
				if constexpr (std::is_nothrow_move_assignable_v<__scope::_box<D>>)
				{
					resource = std::as_const(that.resource);
					deleter = std::forward<__scope::_box<D>>(that.deleter);
				}
				else
				{
					resource = std::as_const(that.resource);
					deleter = std::as_const(that.deleter);
				}
			execute_on_destruction = std::exchange(that.execute_on_destruction, false);
			return *this;
		}
		~unique_resource()
		{
			reset();
		}

		void reset() noexcept
		{
			if(execute_on_destruction)
			{
				execute_on_destruction = false;
				get_deleter()(get());
			}
		}
		template<typename RR>
		auto reset(RR &&r)
			noexcept(noexcept(resource.reset(std::forward<RR>(r))))
			-> decltype(resource.reset(std::forward<RR>(r)), void())
		{
			auto &&guard = make_scope_fail([&, this]{ get_deleter()(r); });
			reset();
			resource.reset(std::forward<RR>(r));
			execute_on_destruction = true;
		}
		void release() noexcept
		{
			execute_on_destruction = false;
		}
		decltype(auto) get() const noexcept
		{
			return resource.get();
		}
		decltype(auto) get_deleter() const noexcept
		{
			return deleter.get();
		}
		template<typename RR = R, class = std::enable_if_t<std::is_pointer_v<RR> >>
		decltype(auto) operator->() const noexcept
		{
			return get();
		}
		template<typename RR = R>
		auto operator*() const noexcept
			-> std::enable_if_t<std::is_pointer_v<RR> && !std::is_void_v<std::remove_pointer_t<RR>>,
			std::add_lvalue_reference_t<std::remove_pointer_t<R>>>
		{
			return *get();
		}

		unique_resource& operator=(const unique_resource &) = delete;
		unique_resource(const unique_resource &) = delete;

	};

	//template<typename R, typename D>
	//unique_resource(R, D)->unique_resource<R, D>;

	template<class R, class D>
	auto make_unique_resource(R &&r, D &&d)
		noexcept(std::is_nothrow_constructible_v<std::decay_t<R>, R> &&
				 std::is_nothrow_constructible_v<std::decay_t<D>, D>)
	{
		return unique_resource<std::decay_t<R>, std::decay_t<D>>{std::forward<R>(r), std::forward<D>(d)};
	}


	template<class R, class D, class S>
	unique_resource<std::decay_t<R>, std::decay_t<D>>
		make_unique_resource_checked(R &&r, const S &invalid, D &&d)
		noexcept(std::is_nothrow_constructible_v<std::decay_t<R>, R>&&
				 std::is_nothrow_constructible_v<std::decay_t<D>, D>)
	{
		return { std::forward<R>(r), std::forward<D>(d), !bool(r == invalid) };
	}
}

#if defined( __INTELLISENSE__ )

#define SCOPE_EXIT(...) ::cpp::scope_exit<::cpp::__scope::_> __VA_ARGS__{::cpp::__scope::_{}};
#define SCOPE_FAIL(...) ::cpp::scope_fail<::cpp::__scope::_> __VA_ARGS__{::cpp::__scope::_{}};
#define SCOPE_SUCCESS(...) ::cpp::scope_success<::cpp::__scope::_> __VA_ARGS__{::cpp::__scope::_{}};

#else

#define __CPP_SCOPE_CAT2(a,b) a##b
#define __CPP_SCOPE_CAT(a,b) __CPP_SCOPE_CAT2(a,b)

#ifdef _MSC_VER

#define __CPP__ELEM_SIZE_PREFIX__CPP__ELEM_SIZE_POSTFIX ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,0
#define __CPP__ELEM_SIZE_I(__p0,__p1,__p2,__p3,__p4,__p5,__p6,__p7,__p8,__p9,__p10,__p11,__p12,__p13,__p14,__p15,__p16,__p17,__p18,__p19,__p20,__p21,__p22,__p23,__p24,__p25,__p26,__p27,__p28,__p29,__p30,__p31,__n,...) __n
#define __CPP__ELEM_SIZE_II(__args) __CPP__ELEM_SIZE_I __args
#define __CPP__ELEM_SIZE(...) __CPP__ELEM_SIZE_II(( __CPP__ELEM_SIZE_PREFIX_ ## __VA_ARGS__ ## _CPP__ELEM_SIZE_POSTFIX,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0))


#define __CPP_SCOPE_GET_ELEM(N,ELEMS) __CPP_SCOPE_CAT(__CPP_SCOPE_GET_ELEM_, N) ELEMS
#define __CPP_SCOPE_GET_ELEM_1( _1 ,... )  _1
#define __CPP_SCOPE_GET_ELEM_2( _1 ,_2 ,... )  _2
#define __CPP_SCOPE_GET_ELEM_3( _1 ,_2 ,_3 ,... )  _3

#define __CPP_SCOPE_GET_LAST(...) __CPP_SCOPE_GET_ELEM( __CPP__ELEM_SIZE(__VA_ARGS__) ,  (__VA_ARGS__) )

#define __CPP_SCOPE_EXIT_VARIABLE(...)		__CPP_SCOPE_VARIABLE_NAME(___CPP_SCOPE_EXIT_STATE) ,__VA_ARGS__
#define __CPP_SCOPE_FAIL_VARIABLE(...)		__CPP_SCOPE_VARIABLE_NAME(___CPP_SCOPE_FAIL_STATE) ,__VA_ARGS__
#define __CPP_SCOPE_SUCCESS_VARIABLE(...)	__CPP_SCOPE_VARIABLE_NAME(___CPP_SCOPE_SUCCESS_STATE) ,__VA_ARGS__

#else

#define __CPP__ELEM_SIZE_I(_10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) N
#define __CPP__ELEM_SIZE(...) __CPP__ELEM_SIZE_I(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)


#define __CPP_SCOPE_GET_ELEM(N, ...) __CPP_SCOPE_CAT(__CPP_SCOPE_GET_ELEM_, N)(__VA_ARGS__)
#define __CPP_SCOPE_GET_ELEM_0(_0, ...) _0
#define __CPP_SCOPE_GET_ELEM_1(_0, _1, ...) _1
#define __CPP_SCOPE_GET_ELEM_2(_0, _1, _2, ...) _2
#define __CPP_SCOPE_GET_ELEM_3(_0, _1, _2, _3, ...) _3

#define __CPP_SCOPE_GET_LAST(...) __CPP_SCOPE_GET_ELEM(__CPP__ELEM_SIZE(__VA_ARGS__), _, __VA_ARGS__ ,,,,,,,,,,,)


#define __CPP_SCOPE_EXIT_VARIABLE(...)		__CPP_SCOPE_VARIABLE_NAME(___CPP_SCOPE_EXIT_STATE) ,##__VA_ARGS__
#define __CPP_SCOPE_FAIL_VARIABLE(...)		__CPP_SCOPE_VARIABLE_NAME(___CPP_SCOPE_FAIL_STATE) ,##__VA_ARGS__
#define __CPP_SCOPE_SUCCESS_VARIABLE(...)	__CPP_SCOPE_VARIABLE_NAME(___CPP_SCOPE_SUCCESS_STATE) ,##__VA_ARGS__

#endif


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
