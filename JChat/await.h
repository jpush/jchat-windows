#pragma once
#include <optional>
#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include <QPointer>
#include <QDebug>
#include <QShowEvent>
#include <QWidget>

#include <pplx/pplxtasks.h>

#if defined(_RESUMABLE_FUNCTIONS_SUPPORTED) && _RESUMABLE_FUNCTIONS_SUPPORTED || defined(__cpp_coroutines) && __cpp_coroutines >= 201703L

#include <experimental/coroutine>
#include "Dispatch.h"
#include "pplxawait.h"

template < class T, int ResumePolicy = 0, bool ResumeOnIdle = (ResumePolicy == 1), bool ResumeOnShow = (ResumePolicy > 1) >
class Pointer : public QPointer<T>
{
	static_assert(!ResumeOnShow || std::is_base_of_v<QWidget, T>);

	template<class U>
	static auto isVisible(U u) -> decltype(u->isVisible())
	{
		return u->isVisible();
	}

	static auto isVisible(...)
	{
		return true;
	}

	class ResumeOnShowTask :QObject
	{
		std::experimental::coroutine_handle<> _h;
	public:
		ResumeOnShowTask(...) { Q_UNREACHABLE(); }

		ResumeOnShowTask(QWidget* w, std::experimental::coroutine_handle<> h) :_h(h){
			w->installEventFilter(this);
			QObject::connect(w, &QWidget::destroyed, this, [=]
			{
				this->deleteLater();
			});
		}
		~ResumeOnShowTask()
		{
			if(_h)
			{
				_h.destroy();
				_h = nullptr;
			}
		}
	protected:
		virtual bool eventFilter(QObject *watched, QEvent *event) override
		{
			if(event->type() == QEvent::Show)
			{
				auto handle = _h;
				_h = nullptr;
				auto connection = QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, [=]() mutable
				{
					handle.destroy();
				});

				QTimer::singleShot(0, QCoreApplication::instance(), [=]() mutable
				{
					QObject::disconnect(connection);
					handle();
				});
				this->deleteLater();
			}
			return false;
		}
	};

public:
	using QPointer<T>::QPointer;

	bool await_ready() const noexcept
	{
		if(ResumeOnShow || ResumeOnIdle)
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

		if constexpr(ResumeOnShow)
		{
			std::optional<QEventLoop> el;
			if(QThread::currentThread() != QCoreApplication::instance()->thread()){
				el.emplace();
			}
			QTimer::singleShot(0, QCoreApplication::instance(), [=]() mutable
			{
				QObject::disconnect(connection);
				if(this->data())
				{
					if(isVisible(this->data()))
					{
						handle();
					}
					else
					{
						new ResumeOnShowTask(this->data(), handle);
					}
				}
				else
				{
					const_cast<std::experimental::coroutine_handle<>&>(handle).destroy();
				}
			});
		}
		else if constexpr(ResumeOnIdle)
		{
			std::optional<QEventLoop> el;
			if(QThread::currentThread() != QCoreApplication::instance()->thread())	{
				el.emplace();
			}
			QTimer::singleShot(0, QCoreApplication::instance(), [=]() mutable
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
		else
		{
			JChat::post(QCoreApplication::instance(), [=]() mutable
			{
				QObject::disconnect(connection);
				if(this->data())
				{
					handle();
				}
				else
				{
					handle.destroy();
				}
			});
		}
	}

	auto resumeOnIdle() const
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

				std::optional<QEventLoop> el;
				if(QThread::currentThread() != QCoreApplication::instance()->thread()){
					el.emplace();
				}
				QTimer::singleShot(0, QCoreApplication::instance(), [=]() mutable
				{
					QObject::disconnect(connection);
					if(_context)
					{
						handle();
					}
					else
					{
						handle.destroy();
					}
				});
			}

			Pointer<T> const& _context;
		};

		return Awaitable{ *this };
	}


	auto resumeOnShow() const
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

				std::optional<QEventLoop> el;
				if(QThread::currentThread() != QCoreApplication::instance()->thread()){
					el.emplace();
				}
				QTimer::singleShot(0, QCoreApplication::instance(), [=]() mutable
				{
					QObject::disconnect(connection);
					if(_context.data())
					{
						if(isVisible(_context.data()))
						{
							handle();
						}
						else
						{
							new ResumeOnShowTask(_context.data(), handle);
						}
					}
					else
					{
						handle.destroy();
					}

				});
			}

			Pointer<T> const& _context;
		};

		return Awaitable{ *this };
	}
};

template<int ResumePolicy = 0>
struct QTrackFn
{
	template<class T, class = std::enable_if_t<std::is_base_of_v<QObject, T>>>
	Pointer<T, ResumePolicy> operator()(T* ptr) const
	{
		return ptr;
	}

	static const QTrackFn<1> resumeOnIdle;

	static const QTrackFn<2> resumeOnShow;

	template<class T, class = std::enable_if_t<std::is_base_of_v<QObject, T>>>
	friend Pointer<T, ResumePolicy> operator|(T* ptr, QTrackFn)
	{
		return ptr;
	}
};

template<int ResumePolicy>
const QTrackFn<1> QTrackFn<ResumePolicy>::resumeOnIdle;

template<int ResumePolicy>
const QTrackFn<2> QTrackFn<ResumePolicy>::resumeOnShow;

constexpr QTrackFn<> qTrack;


struct ResumeMainThread
{
	const bool delayed = false;//resumeOnIdle

	explicit ResumeMainThread(bool const delayed = false) :delayed(delayed){}

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
			std::optional<QEventLoop> el;
			if(QThread::currentThread() != QCoreApplication::instance()->thread()){
				el.emplace();
			}

			QTimer::singleShot(0, QCoreApplication::instance(), [=]()mutable
			{
				QObject::disconnect(connection);
				handle();
			});
		}
		else
		{
			JChat::post(QCoreApplication::instance(), [=]()mutable
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

			void return_void() const noexcept{}

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

template<class T>
inline T qAwait(pplx::task<T> tsk, QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents)
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

	el.exec(flags);
	if(ex){
		std::rethrow_exception(ex);
	}
	return std::move(*result);
}

inline
void qAwait(pplx::task<void> tsk, QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents)
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

	el.exec(flags);
	if(ex){
		std::rethrow_exception(ex);
	}
}
template<class Rep, class Period>
inline void qAwait(std::chrono::duration<Rep, Period> const& duration)
{
	QEventLoop el;
	QTimer::singleShot(std::chrono::milliseconds(duration).count(), [&]
	{
		el.quit();
	});
	el.exec();
}
