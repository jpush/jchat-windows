#pragma once

#include <memory>
#include <QEvent>
#include <QThread>
#include <QVariant>

namespace JChat
{
	//////////////////////////////////////////////////////////////////////////
	class QCallEvent : public QEvent
	{
	public:
		QCallEvent() :QEvent(static_cast<QEvent::Type>(static_type)) {}
		virtual void operator()() = 0;
		static const int static_type;
	};

	namespace _qx_core{

		class CallHandler :public QObject
		{
		public:
			CallHandler(QObject* parent = nullptr) :QObject(parent){}
			bool event(QEvent * e) override
			{
				if(e->type() == QCallEvent::static_type)
				{
					(*static_cast<QCallEvent*>(e))();
					return true;
				}
				return false;
			}
		};

		template<class Fn>
		struct CallEventT :public QCallEvent
		{
			template<class ...Args>
			CallEventT(Args && ... args) :_fn(std::forward<Args>(args)...){}
			virtual void operator() ()
			{
				try{
					_fn();
				}
				catch(...)
				{
					std::terminate();
				}
			}
			Fn _fn;
			CallHandler handler;
		};

		template<class Fn>
		inline CallEventT<std::decay_t<Fn>> * createCallEvent(Fn&& fn)
		{
			return new CallEventT<std::decay_t<Fn> >(std::forward<Fn>(fn));
		}
	}

	//
	template<class _Slot,
		class = std::enable_if_t< !std::is_member_function_pointer< std::remove_reference_t< _Slot > >::value >,
		class = std::result_of_t<_Slot() > >
		inline void post(const QObject * context, _Slot&& _slot)
	{
		auto e = _qx_core::createCallEvent([c = QPointer<const QObject>(context), slot = std::forward<_Slot>(_slot)]() mutable {
			slot();
		});
		e->handler.moveToThread(context->thread());
		QCoreApplication::postEvent(&e->handler, e);
	}

	template<class _Slot, class _Slot2,
		class = std::enable_if_t< !std::is_member_function_pointer< std::remove_reference_t< _Slot > >::value >,
		class = std::result_of_t<_Slot() >, class = std::result_of_t<_Slot2() >>
		inline void post(const QObject * context, _Slot&& _slot, _Slot2&& _slot2)
	{
		auto e = _qx_core::createCallEvent([c = QPointer<const QObject>(context),
										   slot = std::forward<_Slot>(_slot),
										   slot2 = std::forward<_Slot2>(_slot2)]() mutable
		{
			if(c){ slot(); }
			else{ slot2(); }
		});
		e->handler.moveToThread(context->thread());
		QCoreApplication::postEvent(&e->handler, e);
	}

	template<class C, class M, class = std::enable_if_t< std::is_function<M>::value >,
		class = std::enable_if_t< std::is_base_of< QObject, C >::value >, class = std::result_of_t< M(C::*(C*))  > >
		inline void post(const std::remove_reference_t< C >* receiver, M C::* pmf)
	{
		auto receiver_ = const_cast<C*>(receiver);
		auto e = _qx_core::createCallEvent([c = QPointer<const QObject>(receiver), receiver_, pmf]{ (receiver_->*pmf)(); });
		e->handler.moveToThread(receiver->thread());
		QCoreApplication::postEvent(&e->handler, e);
	}

	template<class C, class M, class _Slot2,
		class = std::enable_if_t< std::is_function<M>::value >,
		class = std::enable_if_t< std::is_base_of< QObject, C >::value >, class = std::result_of_t< M(C::*(C*))>, class = std::result_of_t<_Slot2() >>
		inline void post(const std::remove_reference_t< C >* receiver, M C::* pmf, _Slot2&& _slot2)
	{
		auto receiver_ = const_cast<C*>(receiver);
		auto e = _qx_core::createCallEvent([c = QPointer<const QObject>(receiver),
										   receiver_, pmf,
										   slot2 = std::forward<_Slot2>(_slot2)]() mutable
		{
			if(c) (receiver_->*pmf)();
			else _slot2();
		});
		e->handler.moveToThread(receiver->thread());
		QCoreApplication::postEvent(&e->handler, e);
	}

	template<class _Slot,
		class = std::enable_if_t< !std::is_member_function_pointer< std::remove_reference_t< _Slot > >::value >,
		class = std::result_of_t<_Slot() > >
		inline void post(const QThread * thread, _Slot&& _slot)
	{
		auto e = new _qx_core::CallEventT<std::decay_t<_Slot>>(std::forward<_Slot>(_slot));
		e->handler.moveToThread(const_cast<QThread*>(thread));
		QCoreApplication::postEvent(&e->handler, e);
	}


	template<class _Slot,
		class = std::enable_if_t< !std::is_member_function_pointer< std::remove_reference_t< _Slot > >::value >,
		class = std::result_of_t<_Slot() >>
		inline void dispatch(const QObject * context, _Slot&& _slot)
	{
		if(QThread::currentThread() == context->thread())
		{
			_slot();
		}
		else
		{
			post(context, std::forward<_Slot>(_slot));
		}
	}

	template<class C, class M, class = std::enable_if_t< std::is_function<M>::value >,
		class = std::enable_if_t< std::is_base_of< QObject, C >::value >, class = std::result_of_t< M(C::*(C*))  > >
		inline void dispatch(const std::remove_reference_t<C>* receiver, M C::* pmf)
	{
		auto receiver_ = const_cast<C*>(receiver);
		if(QThread::currentThread() == receiver_->thread())
		{
			(receiver_->*pmf)();
		}
		else
		{
			post(receiver, pmf);
		}
	}


}

