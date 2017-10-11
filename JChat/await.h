#pragma once

#include <windows.h>

#include <optional>
#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include <QPointer>
#include <QDebug>

#include <pplx/pplxtasks.h>


#if defined(_RESUMABLE_FUNCTIONS_SUPPORTED) && _RESUMABLE_FUNCTIONS_SUPPORTED

#include <experimental/coroutine>
#include "Dispatch.h"
#include <pplawait.h>

template<class T>
class Pointer : public QPointer<T>
{
public:
	using QPointer::QPointer;

	bool await_ready() const noexcept
	{
		return QThread::currentThread() == QCoreApplication::instance()->thread();
	}

	void await_resume() const noexcept{}

	void await_suspend(std::experimental::coroutine_handle<> handle) const
	{
		auto connection = QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [=]() mutable
		{
			handle.destroy();
		});
		JChat::post(QCoreApplication::instance(), [=]()
		{
			QObject::disconnect(connection);

			if(this->data())
			{
				handle();
			}
			else
			{
				const_cast<std::experimental::coroutine_handle<>&>(handle).destroy();
			}
		});
	}

	auto delayed() const
	{
		struct Awaitable
		{
			bool await_ready() const noexcept
			{
				return false;
			}

			void await_resume() const noexcept{}

			void await_suspend(std::experimental::coroutine_handle<> handle) const
			{
				auto connection = QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [=]() mutable
				{
					handle.destroy();
				});
				QTimer::singleShot(0, QCoreApplication::instance(), [=]
				{
					QObject::disconnect(connection);
					if(_context)
					{
						handle();
					}
					else
					{
						const_cast<std::experimental::coroutine_handle<>&>(handle).destroy();
					}
				});
			}

			Pointer<T> const& _context;
		};

		return Awaitable{ *this };
	}
};

struct QTrackFn
{
	template<class T, class = std::enable_if_t<std::is_base_of_v<QObject, T>>>
	Pointer<T> operator()(T* ptr) const
	{
		return ptr;
	}

	template<class T, class = std::enable_if_t<std::is_base_of_v<QObject, T>>>
	friend Pointer<T> operator|(T* ptr, QTrackFn)
	{
		return ptr;
	}
};

constexpr QTrackFn qTrack;


struct ResumeMainThread
{
	const bool delayed = false;

	bool await_ready() const noexcept
	{
		if(delayed)
		{
			return false;
		}

		return QThread::currentThread() == QCoreApplication::instance()->thread();
	}

	void await_resume() const noexcept{}

	void await_suspend(std::experimental::coroutine_handle<> handle) const
	{
		auto connection = QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [=]() mutable
		{
			handle.destroy();
		});
		if(delayed)
		{
			QTimer::singleShot(0, QCoreApplication::instance(), [=]
			{
				QObject::disconnect(connection);
				handle();
			});
		}
		else
		{
			JChat::post(QCoreApplication::instance(), [=]()
			{
				QObject::disconnect(connection);
				handle();
			});
		}
	}
};

struct ResumeBackground
{
	bool await_ready() const noexcept
	{
		return QThread::currentThread() != QCoreApplication::instance()->thread();
	}

	void await_resume() const noexcept{}

	void await_suspend(std::experimental::coroutine_handle<> handle) const
	{
		struct Task : public QRunnable
		{
			std::experimental::coroutine_handle<> _handle;
			Task(std::experimental::coroutine_handle<> handle) :_handle(handle){}
			void run() override
			{
				_handle();
			}
		};
		QThreadPool::globalInstance()->start(new Task{ handle });
	}
};


struct ResumeAfter
{
	explicit ResumeAfter(std::chrono::milliseconds const& duration) noexcept
		:_duration(duration){}

	~ResumeAfter()
	{
		if(_timer)
		{
			CloseThreadpoolTimer(_timer);
		}
	}

	bool await_ready() const noexcept
	{
		return _duration.count() <= 0;
	}

	void await_suspend(std::experimental::coroutine_handle<> handle)
	{
		_timer = CreateThreadpoolTimer(callback, handle.address(), nullptr);
		if(!_timer)
		{
			throw std::system_error(std::make_error_code(std::errc::resource_unavailable_try_again));
		}
		int64_t relative_count = -_duration.count() * 10000;
		SetThreadpoolTimer(_timer, reinterpret_cast<PFILETIME>(&relative_count), 0, 0);
	}

	void await_resume() const noexcept{	}

private:
	static void __stdcall callback(PTP_CALLBACK_INSTANCE, void * context, PTP_TIMER) noexcept
	{
		std::experimental::coroutine_handle<>::from_address(context)();
	}

	PTP_TIMER					_timer{};
	std::chrono::milliseconds	_duration;
};



struct FireAndForget{};
using None = FireAndForget;

namespace std::experimental {
	template <typename ... Args>
	struct coroutine_traits<FireAndForget, Args ...>
	{
		struct promise_type
		{
			FireAndForget get_return_object() const noexcept
			{
				return{};
			}

			void return_void() const noexcept	{	}

			suspend_never initial_suspend() const noexcept	{ return{}; }

			suspend_never final_suspend() const noexcept{ return{}; }

			void unhandled_exception()
			{
				try
				{
					std::rethrow_exception(std::current_exception());
				}
				catch(Jmcpp::ServerException& e)
				{
					qWarning() << "unhandled_exception:" << typeid(e).name() << e.what();
				}
				catch(std::system_error& e)
				{
					qWarning() << "unhandled_exception:" << typeid(e).name() << e.code().value() << e.what();
				}
				catch(std::runtime_error& e)
				{
					qWarning() << "unhandled_exception:" << typeid(e).name() << e.what();
				}
				catch(std::bad_weak_ptr& e)
				{
					qWarning() << "unhandled_exception:" << typeid(e).name() << e.what();
				}
				catch(...)
				{
					qWarning() << "unhandled_exception:" << "Unknown Exception!!!";
				}
			}
		};
	};
}


#endif

template<class T, class ... Fn>
T qAwait(pplx::task<T> tsk, Fn&&... fn)
{
	QEventLoop el;
	std::optional<T> result;
	std::exception_ptr ex;
	tsk.then([&result, &ex, &el](pplx::task<T> t){
		try
		{
			result = t.get();
		}
		catch(...)
		{
			ex = std::current_exception();
		}
		if(el.isRunning()){
			el.quit();
		}
		else{
			QMetaObject::invokeMethod(&el, "quit", Qt::QueuedConnection);
		}
	});

	auto invokeFns = { 0, (void(fn()),0)... };
	el.exec();
	if(ex){
		std::rethrow_exception(ex);
	}
	return std::move(*result);
}


template<class ... Fn>
void qAwait(pplx::task<void> tsk, Fn&&... fn)
{
	QEventLoop el;
	std::exception_ptr ex;
	tsk.then([&ex, &el](pplx::task<void> t){
		try
		{
			t.get();
		}
		catch(...)
		{
			ex = std::current_exception();
		}
		if(el.isRunning()){
			el.quit();
		}
		else{
			QMetaObject::invokeMethod(&el, "quit", Qt::QueuedConnection);
		}
	});

	auto invokeFns = { 0, (void(fn()),0)... };
	el.exec();
	if(ex){
		std::rethrow_exception(ex);
	}
}


inline void qAwait(std::chrono::milliseconds const& ms)
{
	QEventLoop el;
	QTimer::singleShot(ms.count(), [&]
	{
		el.quit();
	});
	el.exec();
}