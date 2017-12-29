#pragma once

#ifndef _PPLXAWAIT_H
#define _PPLXAWAIT_H

#if defined(_MSC_VER)

#include <pplawait.h>

#else

#include <experimental/coroutine>
#include <pplx/pplxtasks.h>
#include <memory>
#include <type_traits>
#include <utility>

namespace std
{
	namespace experimental
	{
		template <typename _Ty, typename... _Whatever>
		struct coroutine_traits< ::pplx::task<_Ty>, _Whatever...>
		{
			struct promise_type
			{
				auto get_return_object() const
				{
					return ::pplx::create_task(_M_tce);
				}

				suspend_never initial_suspend() const { return {}; }

				suspend_never final_suspend() const { return {}; }

				void return_value(const _Ty &_Val)
				{
					_M_tce.set(_Val);
				}

				void unhandled_exception()
				{
					_M_tce.set_exception(std::current_exception());
				}
			private:
				::pplx::task_completion_event<_Ty> _M_tce;
			};
		};

		template <typename... _Whatever>
		struct coroutine_traits< ::pplx::task<void>, _Whatever...>
		{
			struct promise_type
			{
				auto get_return_object() const
				{
					return ::pplx::create_task(_M_tce);
				}

				suspend_never initial_suspend() const { return {}; }

				suspend_never final_suspend() const { return {}; }

				void return_void()
				{
					_M_tce.set();
				}

				void unhandled_exception()
				{
					_M_tce.set_exception(std::current_exception());
				}
			private:
				::pplx::task_completion_event<void> _M_tce;
			};
		};
	}
}

namespace pplx
{
	template<class T>
	auto operator co_await(task<T> const& t)
	{
		struct Awaitable
		{
			bool await_ready() const
			{
				return _task.is_done();
			}

			void await_suspend(std::experimental::coroutine_handle<void> _ResumeCb)
			{
				_task.then([_ResumeCb](const task<T>&) mutable
				{
					_ResumeCb();
				});
			}

			auto await_resume()
			{
				return _task.get();
			}

			task<T> const& _task;
		};

		return Awaitable{ t };
	}

}

#endif

#endif // _PPLXAWAIT_H
