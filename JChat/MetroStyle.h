#pragma once

#include <memory>
#include <type_traits>
#include <QObject>

#define QXWIN_EXPORT

QT_BEGIN_NAMESPACE
QT_FORWARD_DECLARE_CLASS(QByteArray);
QT_FORWARD_DECLARE_CLASS(QEvent);
QT_FORWARD_DECLARE_CLASS(QColor);
QT_FORWARD_DECLARE_CLASS(QWidget);
QT_END_NAMESPACE

namespace QxWin
{
	class QXWIN_EXPORT TitleBar;

	namespace _qx_win
	{
		class QXWIN_EXPORT StyleBase;
		class StyleBase
		{
			class WidgetData;
			std::unique_ptr<WidgetData> _p;
		public:
			void	setResizeEnabled(bool b);
			bool	resizeEnabled() const;

			QColor const& 	borderColor() const;
			void			setBorderColor(QColor const& color);

			void  dragMove() const;
			void  switchNormalMaximized() const;


			void	setTitleBarEnabled(bool enable);
			bool	titleBarEnabled() const;

			TitleBar* titleBar() const;

			void	addDragger(QWidget*, bool maximizedByDoubleClick = false);
			void	removeDragger(QWidget*);
		protected:
			StyleBase(QWidget* mainWidget, bool resizeEnabled, bool titleBarEnabled);
			virtual ~StyleBase();

			bool _eventImpl(QEvent * e);
			bool _nativeEventImpl(const QByteArray &eventType, void *message, long *result);

			friend class TitleBar;
		};
	}

	class TitleBar :public QObject
	{
		Q_OBJECT

			Q_PROPERTY(bool minButtonVisible READ minButtonVisible WRITE setMinButtonVisible)
			Q_PROPERTY(bool maxButtonVisible READ maxButtonVisible WRITE setMaxButtonVisible)
			Q_PROPERTY(bool closeButtonVisible READ closeButtonVisible WRITE setCloseButtonVisible)
	public:

		void setMinButtonVisible(bool);
		bool minButtonVisible() const;

		void setMaxButtonVisible(bool);
		bool maxButtonVisible() const;

		void setCloseButtonVisible(bool);
		bool closeButtonVisible() const;

	private:
		TitleBar(_qx_win::StyleBase* windowImpl);
		~TitleBar();
		QWidget*		widget() const;
		void			onResizeEnableChanged(bool resizeEnable);
		virtual bool	eventFilter(QObject *watched, QEvent *event) override;

		class Impl;
		std::unique_ptr<Impl> _p;
		friend _qx_win::StyleBase;
	};

	template<class T, bool ResizeEnabled = true, bool TitleBarEnabled = true>
	class MetroStyle : public T, _qx_win::StyleBase
	{
		static_assert(std::is_base_of< QWidget, T >::value, "T must be a subclass of QWidget");

		static_assert(!std::is_base_of<_qx_win::StyleBase, T>::value, "");

		using base_type = T;
	public:

	#if defined(_MSC_VER)
		MetroStyle(QWidget* parent = 0, Qt::WindowFlags f = Qt::WindowFlags())
			:base_type(parent, f | ((f&Qt::WindowType_Mask) ? Qt::WindowType(0) : Qt::Window) | Qt::FramelessWindowHint)
			, StyleBase(this, ResizeEnabled, TitleBarEnabled)
		{

		}
	#else
		MetroStyle(QWidget* parent = 0, Qt::WindowFlags f = Qt::WindowFlags())
			: base_type(parent, f), StyleBase(this, ResizeEnabled, TitleBarEnabled)
		{

		}

	#endif // _MSC_VER

		~MetroStyle(){}

		using StyleBase::setTitleBarEnabled;
		using StyleBase::titleBarEnabled;

		using StyleBase::setResizeEnabled;
		using StyleBase::resizeEnabled;

		using StyleBase::borderColor;
		using StyleBase::setBorderColor;

		using StyleBase::dragMove;
		using StyleBase::switchNormalMaximized;

		using StyleBase::titleBar;

		using StyleBase::addDragger;
		using StyleBase::removeDragger;

	protected:
		bool event(QEvent * e)
		{
			if(this->_eventImpl(e))
			{
				return true;
			}
			return base_type::event(e);
		}

		virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result) override
		{
			if(this->_nativeEventImpl(eventType, message, result))
			{
				return true;
			}
			return base_type::nativeEvent(eventType, message, result);
		}
	};
}