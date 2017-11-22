
#include <algorithm>
#include <QCoreApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QLabel>
#include <QToolButton>
#include <QTimer>
#include <QList>
#include <QPointer>
#include <QWindow>
#include <QScreen>

#include <QBoxLayout>
#include <QDebug>
#include <QStyle>
#include <QLibrary>

#include <windows.h>
#include <qt_windows.h>

#include "MetroStyle.h"

namespace QxWin{

	namespace _qx_win
	{
		static auto titleBarHeight = 30;
		using DwmIsCompositionEnabledPtr = HRESULT(__stdcall *)(BOOL*);
		static DwmIsCompositionEnabledPtr dwmEnableCompositionPtr;
		static void initDwmFun()
		{
			dwmEnableCompositionPtr = (DwmIsCompositionEnabledPtr)QLibrary::resolve("dwmapi.dll", "DwmIsCompositionEnabled");
		}
		Q_COREAPP_STARTUP_FUNCTION(initDwmFun);

		static inline bool dwmCompositionIsEnabled()
		{
			BOOL enabled = FALSE;
			bool successed = false;
			if(dwmEnableCompositionPtr)
			{
				successed = dwmEnableCompositionPtr(&enabled) == S_OK;
			}
			return enabled && successed;
		}

		static inline bool isMinAnimate()
		{
			ANIMATIONINFO aminfo;
			aminfo.cbSize = sizeof(ANIMATIONINFO);
			::SystemParametersInfoW(SPI_GETANIMATION, sizeof(ANIMATIONINFO), &aminfo, 0);
			return aminfo.iMinAnimate;
		}

		static inline void dragMove(QWidget* w)
		{
			if(auto hwnd = ::GetCapture())
			{
				::ReleaseCapture();
			}
			//#define SC_DRAGMOVE 0xF012
			SendMessageW((HWND)w->winId(), WM_SYSCOMMAND, 0xF012, 0);
		}

		static inline void switchNormalMaximized(QWidget* w)
		{
			POINTS pt{};
			::ReleaseCapture();
			::SendMessageW((HWND)w->winId(), WM_NCLBUTTONDBLCLK, HTCAPTION, reinterpret_cast<LPARAM&>(pt));
		}

		enum class Style : DWORD
		{
			aero_borderless = /* */WS_POPUP | WS_SYSMENU /*| WS_MAXIMIZEBOX */ | WS_MINIMIZEBOX | WS_CAPTION /*  | WS_THICKFRAME  */,
			basic_borderless =/* */WS_POPUP | WS_SYSMENU /*| WS_MAXIMIZEBOX */ | WS_MINIMIZEBOX
		};

		static DWORD getStyle(bool resizeEnabled)
		{
			if(dwmCompositionIsEnabled())
			{
				return resizeEnabled ? (DWORD(_qx_win::Style::aero_borderless) | WS_THICKFRAME | WS_MAXIMIZEBOX) : (DWORD)_qx_win::Style::aero_borderless;
			}
			else
			{
				return resizeEnabled ? (DWORD(_qx_win::Style::basic_borderless) | WS_THICKFRAME | WS_MAXIMIZEBOX) : (DWORD)_qx_win::Style::basic_borderless;
			}
		}


		class BorderWidget :public QWidget
		{
		public:
			QWidget*					mainWidget;
			QColor						borderColor{ 0,100,255 };
			const qint8					resizeBorderWidth = 30;
			qint8						borderWidth = 9;
			bool						borderActived = false;
		public:
			BorderWidget(QWidget* mainWidget)
				:QWidget(mainWidget, Qt::Tool | Qt::WindowType::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus)
				, mainWidget(mainWidget)
			{
				this->setAttribute(Qt::WA_TranslucentBackground);
				this->setMouseTracking(true);
			}

			QColor getBorderColor() const
			{
				return borderActived ? borderColor.lighter() : (mainWidget->isActiveWindow() ? borderColor : Qt::gray);
			}

		protected:
			virtual bool event(QEvent *e) override
			{
				return QWidget::event(e);
			}

			virtual bool nativeEvent(const QByteArray &, void *message, long *result) override
			{
				MSG* msg = static_cast<MSG*>(message);
				switch(msg->message)
				{
					case WM_NCACTIVATE:
					{
						borderActived = msg->wParam;
						update();
						if(msg->wParam)
						{
							mainWidget->activateWindow();
						}
					}break;

					case 0x0118:
					{
						if(msg->wParam == 0xFFF8)
						{

						}
						return true;
					}break;
					default:break;
				}
				return false;
			}

			virtual void paintEvent(QPaintEvent *event) override
			{
				auto rect = this->rect();
				auto borderWidth = this->borderWidth;
				QPainter p(this);
				auto borderClr = this->getBorderColor();
				auto alphaf = borderClr.alphaF();

				{// top
					QLinearGradient l{ 0,0,0,1 };
					l.setCoordinateMode(QGradient::ObjectBoundingMode);
					auto clr = borderClr;
					clr.setAlpha(0);			l.setColorAt(0, clr);
					clr.setAlpha(1);			l.setColorAt(0.5, clr);
					clr.setAlpha(30 * alphaf);	l.setColorAt(0.9, clr);
					clr.setAlpha(255 * alphaf); l.setColorAt(1, clr);
					p.fillRect(QRect{ borderWidth,0,rect.width() - borderWidth * 2 ,borderWidth }, l);
				}


				{// left
					QLinearGradient l{ 0,0,1,0 };
					l.setCoordinateMode(QGradient::ObjectBoundingMode);
					auto clr = borderClr;
					clr.setAlpha(0);			l.setColorAt(0, clr);
					clr.setAlpha(1);			l.setColorAt(0.5, clr);
					clr.setAlpha(30 * alphaf);	l.setColorAt(0.9, clr);
					clr.setAlpha(255 * alphaf); l.setColorAt(1, clr);
					p.fillRect(QRect{ 0, borderWidth,borderWidth ,rect.height() - borderWidth * 2 }, l);
				}


				{// top left
					QRadialGradient l(1, 1, 1);
					l.setCoordinateMode(QGradient::ObjectBoundingMode);
					auto clr = borderClr;
					clr.setAlpha(255 * alphaf);	l.setColorAt(0, clr);
					clr.setAlpha(30 * alphaf);	l.setColorAt(0.1, clr);
					clr.setAlpha(1);			l.setColorAt(0.5, clr);
					clr.setAlpha(0);			l.setColorAt(1, clr);
					p.fillRect(QRect{ 0, 0,borderWidth,borderWidth }, l);
				}

				{// top right
					QRadialGradient l(0, 1, 1);
					l.setCoordinateMode(QGradient::ObjectBoundingMode);
					auto clr = borderClr;
					clr.setAlpha(255 * alphaf);	l.setColorAt(0, clr);
					clr.setAlpha(30 * alphaf);	l.setColorAt(0.1, clr);
					clr.setAlpha(1);			l.setColorAt(0.5, clr);
					clr.setAlpha(0);			l.setColorAt(1, clr);
					p.fillRect(QRect{ rect.width() - borderWidth, 0,borderWidth ,borderWidth }, l);
				}

				{// right
					QLinearGradient l{ 1,0,0,0 };
					l.setCoordinateMode(QGradient::ObjectBoundingMode);
					auto clr = borderClr;
					clr.setAlpha(0);			l.setColorAt(0, clr);
					clr.setAlpha(1);			l.setColorAt(0.5, clr);
					clr.setAlpha(30 * alphaf);	l.setColorAt(0.9, clr);
					clr.setAlpha(255 * alphaf); l.setColorAt(1, clr);
					p.fillRect(QRect{ rect.width() - borderWidth, borderWidth,borderWidth ,rect.height() - borderWidth * 2 }, l);
				}

				{// bottom right
					QRadialGradient l(0, 0, 1);
					l.setCoordinateMode(QGradient::ObjectBoundingMode);
					auto clr = borderClr;
					clr.setAlpha(255 * alphaf);	l.setColorAt(0, clr);
					clr.setAlpha(30 * alphaf);	l.setColorAt(0.1, clr);
					clr.setAlpha(1);			l.setColorAt(0.5, clr);
					clr.setAlpha(0);			l.setColorAt(1, clr);
					p.fillRect(QRect{ rect.width() - borderWidth, rect.height() - borderWidth ,borderWidth ,borderWidth }, l);
				}


				{// bottom
					QLinearGradient l{ 0,1,0,0 };
					l.setCoordinateMode(QGradient::ObjectBoundingMode);
					auto clr = borderClr;
					clr.setAlpha(0);			l.setColorAt(0, clr);
					clr.setAlpha(1);			l.setColorAt(0.5, clr);
					clr.setAlpha(30 * alphaf);	l.setColorAt(0.9, clr);
					clr.setAlpha(255 * alphaf); l.setColorAt(1, clr);
					p.fillRect(QRect{ borderWidth,rect.height() - borderWidth,rect.width() - borderWidth * 2 ,borderWidth }, l);
				}

				{// bottom left
					QRadialGradient l(1, 0, 1);
					l.setCoordinateMode(QGradient::ObjectBoundingMode);
					auto clr = borderClr;
					clr.setAlpha(255 * alphaf);	l.setColorAt(0, clr);
					clr.setAlpha(30 * alphaf);	l.setColorAt(0.1, clr);
					clr.setAlpha(1);			l.setColorAt(0.5, clr);
					clr.setAlpha(0);			l.setColorAt(1, clr);
					p.fillRect(QRect{ 0, rect.height() - borderWidth ,borderWidth ,borderWidth }, l);
				}
				p.setPen(QPen{ borderClr });
				p.drawRect((rect).adjusted(borderWidth - 1, borderWidth - 1, -borderWidth, -borderWidth));
			}

			virtual void resizeEvent(QResizeEvent *event) override
			{
				auto rect = this->rect();
				QRegion maskedRegion(rect);
				maskedRegion -= QRect(borderWidth, borderWidth, rect.width() - borderWidth * 2, rect.height() - borderWidth * 2);
				this->setMask(maskedRegion);
			}

			virtual void mousePressEvent(QMouseEvent *ev) override
			{
				if(ev->buttons() & Qt::LeftButton)
				{
					const LONG borderWidth = this->resizeBorderWidth; //in pixels
					RECT winrect;
					::GetWindowRect((HWND)this->winId(), &winrect);
					long x = ev->globalX();
					long y = ev->globalY();

					::ReleaseCapture();

					//bottom right corner
					if(x < winrect.right && x >= winrect.right - borderWidth &&
					   y < winrect.bottom && y >= winrect.bottom - borderWidth)
					{
						::SendMessageW((HWND)this->mainWidget->winId(), WM_NCLBUTTONDOWN, HTBOTTOMRIGHT, 0);
						return;
					}

					//bottom left corner
					if(x >= winrect.left && x < winrect.left + borderWidth &&
					   y < winrect.bottom && y >= winrect.bottom - borderWidth)
					{
						::SendMessageW((HWND)this->mainWidget->winId(), WM_NCLBUTTONDOWN, HTBOTTOMLEFT, 0);
						return;
					}

					//top right corner
					if(x < winrect.right && x >= winrect.right - borderWidth &&
					   y >= winrect.top && y < winrect.top + borderWidth)
					{
						::SendMessageW((HWND)this->mainWidget->winId(), WM_NCLBUTTONDOWN, HTTOPRIGHT, 0);
						return;

					}

					//top left corner
					if(x >= winrect.left && x < winrect.left + borderWidth &&
					   y >= winrect.top && y < winrect.top + borderWidth)
					{
						::SendMessageW((HWND)this->mainWidget->winId(), WM_NCLBUTTONDOWN, HTTOPLEFT, 0);
						return;
					}

					//left border
					if(x >= winrect.left && x < winrect.left + borderWidth)
					{
						::SendMessageW((HWND)this->mainWidget->winId(), WM_NCLBUTTONDOWN, HTLEFT, 0);
						return;
					}
					//right border
					if(x < winrect.right && x >= winrect.right - borderWidth)
					{
						::SendMessageW((HWND)this->mainWidget->winId(), WM_NCLBUTTONDOWN, HTRIGHT, 0);
						return;
					}
					//bottom border
					if(y < winrect.bottom && y >= winrect.bottom - borderWidth)
					{
						::SendMessageW((HWND)this->mainWidget->winId(), WM_NCLBUTTONDOWN, HTBOTTOM, 0);
						return;
					}
					//top border
					if(y >= winrect.top && y < winrect.top + borderWidth)
					{
						::SendMessageW((HWND)this->mainWidget->winId(), WM_NCLBUTTONDOWN, HTTOP, 0);
						return;
					}
				}
			}


			virtual void mouseMoveEvent(QMouseEvent *ev) override
			{
				const LONG borderWidth = this->resizeBorderWidth; //in pixels
				RECT winrect;
				::GetWindowRect((HWND)this->winId(), &winrect);
				long x = ev->globalX();
				long y = ev->globalY();


				//bottom right corner
				if(x < winrect.right && x >= winrect.right - borderWidth &&
				   y < winrect.bottom && y >= winrect.bottom - borderWidth)
				{
					this->setCursor(Qt::CursorShape::SizeFDiagCursor);
					return;
				}

				//top right corner
				if(x < winrect.right && x >= winrect.right - borderWidth &&
				   y >= winrect.top && y < winrect.top + borderWidth)
				{
					this->setCursor(Qt::CursorShape::SizeBDiagCursor);
					return;
				}

				//bottom left corner
				if(x >= winrect.left && x < winrect.left + borderWidth &&
				   y < winrect.bottom && y >= winrect.bottom - borderWidth)
				{
					this->setCursor(Qt::CursorShape::SizeBDiagCursor);
					return;
				}

				//top left corner
				if(x >= winrect.left && x < winrect.left + borderWidth &&
				   y >= winrect.top && y < winrect.top + borderWidth)
				{
					this->setCursor(Qt::CursorShape::SizeFDiagCursor);
					return;
				}

				//left border
				if(x >= winrect.left && x < winrect.left + borderWidth)
				{
					this->setCursor(Qt::CursorShape::SizeHorCursor);
					return;
				}
				//right border
				if(x < winrect.right && x >= winrect.right - borderWidth)
				{
					this->setCursor(Qt::CursorShape::SizeHorCursor);
					return;
				}
				//bottom border
				if(y < winrect.bottom && y >= winrect.bottom - borderWidth)
				{
					this->setCursor(Qt::CursorShape::SizeVerCursor);
					return;
				}
				//top border
				if(y >= winrect.top && y < winrect.top + borderWidth)
				{
					this->setCursor(Qt::CursorShape::SizeVerCursor);
					return;
				}
			}
		};

		//////////////////////////////////////////////////////////////////////////
		//
		// 
		class StyleBase::WidgetData :public QObject
		{
		public:
			StyleBase*					styleBase = nullptr;

			QWidget*					mainWidget = nullptr;
			_qx_win::BorderWidget*		border = nullptr;
			TitleBar*					titleBar = nullptr;

			std::vector< std::tuple<QPointer<QObject>, bool > > draggerAreas;

			bool						resizeEnabled = true;


			bool						titleBarEnabled = true;


			void updateStyle()
			{
				::SetWindowLongPtr((HWND)mainWidget->winId(), GWL_STYLE, getStyle(resizeEnabled));
				::SetWindowPos((HWND)mainWidget->internalWinId(), nullptr, 0, 0, 0, 0,
							   SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);

			}

			void updatContentsMargins()
			{
				auto states = mainWidget->windowState();
				if(states & Qt::WindowMaximized)
				{
					if(titleBarEnabled)
					{
						titleBar->widget()->show();
					}
					mainWidget->setContentsMargins(0, titleBarEnabled ? titleBarHeight : 0, 16, 16);// HACK
				}
				else if(mainWidget->windowState() == Qt::WindowState::WindowNoState)
				{
					if(titleBarEnabled)
					{
						titleBar->widget()->show();
					}
					mainWidget->setContentsMargins(0, titleBarEnabled ? titleBarHeight : 0, 0, 0);// HACK
				}
				else if(states & Qt::WindowFullScreen)
				{
					titleBar->widget()->hide();
					mainWidget->setContentsMargins(0, 0, 0, 0); // HACK
				}
			}

			virtual bool eventFilter(QObject *watched, QEvent *e) override
			{
				if(watched == this->border)
				{
					switch(e->type())
					{
						case QEvent::MouseMove:
						{
							if(!this->resizeEnabled)
							{
								return true;
							}
						}; break;
						case QEvent::MouseButtonPress:
						{
							if(!this->resizeEnabled)
							{
								return true;
							}
						}; break;
						default:break;
					}
				}
				else switch(e->type())
				{
					case QEvent::MouseButtonPress:
					{
						auto iter = std::find_if(draggerAreas.begin(), draggerAreas.end(), [watched](std::tuple<QPointer<QObject>, bool > const& p)
						{
							return  std::get<0>(p) == watched;
						});

						if(iter != end(draggerAreas))
						{
							styleBase->dragMove();
						}
					}break;
					case QEvent::MouseButtonDblClick:
					{
						QMouseEvent* ev = static_cast<QMouseEvent*>(e);
						if(ev->button() != Qt::LeftButton)
						{
							break;
						}
						auto iter = std::find_if(draggerAreas.begin(), draggerAreas.end(), [watched](std::tuple<QPointer<QObject>, bool > const& p)
						{
							return std::get<0>(p) == watched;
						});

						if(iter != end(draggerAreas) && std::get<1>(*iter))
						{
							styleBase->switchNormalMaximized();
						}
					}break;

					default:
						break;
				}
				return false;
			}
		};
	}

	//////////////////////////////////////////////////////////////////////////

	namespace _qx_win
	{
		static const QString titleBarQSS = QString::fromWCharArray(LR"(
			QWidget
			{
				font-family: "微软雅黑";
			}

			QLabel
			{
				font-size:10pt;
			}

			QToolButton 
			{ 
				background: none;
				border:none;
				border-radius: 0px;
				padding:0px;
				min-height: 30px;
				min-width: 30px;
			}

			QToolButton:hover
			{ 
				background-color: gray;
			}

			#qxWinTitleBarIcon
			{
				background: none;
			}
		)");
	}

	class TitleBar::Impl
	{
	public:
		QWidget*				widget = nullptr;
		_qx_win::StyleBase*	styleBase = nullptr;

		QWidget*				titleBarWidget = nullptr;
		QHBoxLayout*			layout = nullptr;
		QToolButton				*mini, *maxi, *close;
	};

	TitleBar::TitleBar(_qx_win::StyleBase* styleBase)
		:QObject(styleBase->_p->mainWidget)
		, _p(std::make_unique<Impl>())
	{
		_p->widget = styleBase->_p->mainWidget;
		_p->styleBase = styleBase;
		_p->widget->installEventFilter(this);

		_p->titleBarWidget = new QWidget(_p->widget);
		_p->titleBarWidget->installEventFilter(this);

		_p->titleBarWidget->setStyleSheet(_qx_win::titleBarQSS);


		_p->layout = new QHBoxLayout(_p->titleBarWidget);
		_p->layout->setContentsMargins(0, 0, 0, 0);
		_p->layout->setSpacing(0);

		auto icon = new QToolButton(_p->titleBarWidget);
		icon->setObjectName("qxWinTitleBarIcon");
		//icon->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
		icon->setFixedSize(_qx_win::titleBarHeight, _qx_win::titleBarHeight);

		icon->setIconSize({ _qx_win::titleBarHeight - 8,_qx_win::titleBarHeight - 8 });
		icon->setIcon(_p->widget->windowIcon());

		icon->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
		_p->layout->addWidget(icon);

		auto title = new QLabel(_p->titleBarWidget);
		title->setObjectName("qxWinTitleBarTitle");
		title->setText(_p->widget->windowTitle());
		_p->layout->addWidget(title);
		_p->layout->addItem(new QSpacerItem(200, 40, QSizePolicy::Expanding, QSizePolicy::Minimum));

		{
			auto mini = new QToolButton(_p->titleBarWidget);
			mini->setObjectName("qxWinTitleBarMinButton");
			mini->setMouseTracking(true);
			mini->setIcon(_p->titleBarWidget->style()->standardPixmap(QStyle::SP_TitleBarMinButton));
			mini->setFixedSize(_qx_win::titleBarHeight + 5, _qx_win::titleBarHeight);
			_p->layout->addWidget(_p->mini = mini);
			connect(mini, &QToolButton::clicked, this, [=]
			{
				_p->widget->setWindowState(_p->widget->windowState() | Qt::WindowState::WindowMinimized);
			});
		}

		{
			auto maxi = new QToolButton(_p->titleBarWidget);
			maxi->setObjectName("qxWinTitleBarMaxButton");
			auto pixmap = QStyle::SP_TitleBarMaxButton;
			if(_p->widget->windowState() & Qt::WindowState::WindowMaximized)
			{
				pixmap = QStyle::SP_TitleBarNormalButton;
			}

			maxi->setIcon(_p->titleBarWidget->style()->standardPixmap(pixmap));
			maxi->setFixedSize(_qx_win::titleBarHeight + 5, _qx_win::titleBarHeight);
			_p->layout->addWidget(_p->maxi = maxi);

			connect(maxi, &QToolButton::clicked, this, [=]
			{
				_qx_win::switchNormalMaximized(_p->widget);
			});
		}

		{
			auto close = new QToolButton(_p->titleBarWidget);
			close->setObjectName("qxWinTitleBarCloseButton");
			close->setIcon(_p->titleBarWidget->style()->standardPixmap(QStyle::SP_TitleBarCloseButton));
			close->setFixedSize(_qx_win::titleBarHeight + 5, _qx_win::titleBarHeight);
			_p->layout->addWidget(_p->close = close);

			connect(close, &QToolButton::clicked, this, [=]
			{
				_p->widget->close();
			});
		}

		connect(_p->widget, &QWidget::windowIconChanged, icon, &QToolButton::setIcon);
		connect(_p->widget, &QWidget::windowTitleChanged, title, &QLabel::setText);
	}

	TitleBar::~TitleBar()
	{

	}

	QWidget* TitleBar::widget() const
	{
		return _p->titleBarWidget;
	}

	void TitleBar::onResizeEnableChanged(bool resizeEnable)
	{
		_p->maxi->setVisible(resizeEnable);
	}

	void TitleBar::setMinButtonVisible(bool visible)
	{
		_p->mini->setVisible(visible);
	}

	bool TitleBar::minButtonVisible() const
	{
		return _p->mini->isVisible();
	}

	void TitleBar::setMaxButtonVisible(bool visible)
	{
		_p->maxi->setVisible(visible);
	}

	bool TitleBar::maxButtonVisible() const
	{
		return _p->maxi->isVisible();
	}

	void TitleBar::setCloseButtonVisible(bool visible)
	{
		_p->close->setVisible(visible);
	}

	bool TitleBar::closeButtonVisible() const
	{
		return _p->close->isVisible();
	}

	bool TitleBar::eventFilter(QObject *watched, QEvent *e)
	{
		if(watched == _p->widget)
		{
			switch(e->type())
			{
				case QEvent::WindowStateChange:
				{
					auto pixmap = QStyle::SP_TitleBarMaxButton;
					if(_p->widget->windowState() & Qt::WindowState::WindowMaximized)
					{
						pixmap = QStyle::SP_TitleBarNormalButton;
					}
					_p->maxi->setIcon(_p->titleBarWidget->style()->standardPixmap(pixmap));
				}break;
				default:
					break;
			}
		}
		else if(_p->titleBarWidget == watched)
		{
			switch(e->type())
			{
				case QEvent::MouseButtonPress:
				{
					QMouseEvent* ev = static_cast<QMouseEvent*>(e);

					if(ev->button() == Qt::LeftButton)
					{
						_p->styleBase->dragMove();
						return true;
					}
				}break;
				case QEvent::MouseButtonDblClick:
				{
					QMouseEvent* ev = static_cast<QMouseEvent*>(e);

					if(ev->button() == Qt::LeftButton)
					{
						_qx_win::switchNormalMaximized(_p->widget);
						return true;
					}
				}break;
				default:
					break;
			}
		}
		return false;
	}


	namespace _qx_win
	{
		StyleBase::StyleBase(QWidget* mainWidget, bool resizeEnabled, bool titleBarEnabled)
			: _p(std::make_unique<WidgetData>())
		{
			_p->styleBase = this;
			_p->mainWidget = mainWidget;
			_p->titleBar = new TitleBar(this);

			_p->resizeEnabled = resizeEnabled;
			_p->titleBarEnabled = titleBarEnabled;

			_p->titleBar->onResizeEnableChanged(_p->resizeEnabled);

			if(!_p->titleBarEnabled)
			{
				_p->titleBar->widget()->hide();
			}

			//_p->titleBar->setMinimumHeight(titleBarHeight);
			//_p->titleBar->setMaximumHeight(titleBarHeight);

			_p->border = new _qx_win::BorderWidget(mainWidget);
			_p->border->installEventFilter(_p.get());
			_p->updatContentsMargins();
		}

		StyleBase::~StyleBase()
		{

		}

		void StyleBase::setResizeEnabled(bool enable)
		{
			if(_p->resizeEnabled != enable)
			{
				_p->resizeEnabled = enable;
				_p->titleBar->onResizeEnableChanged(enable);
				_p->updateStyle();
			}
		}

		bool StyleBase::resizeEnabled() const
		{
			return _p->resizeEnabled;
		}

		QColor const& StyleBase::borderColor() const
		{
			return _p->border->borderColor;
		}

		void StyleBase::setBorderColor(QColor const& color)
		{
			_p->border->borderColor = color;
			_p->border->update();
		}

		void StyleBase::dragMove() const
		{
			//#hack 全屏不允许拖动
			if(_p->mainWidget->windowState() & Qt::WindowFullScreen)
			{
				return;
			}

			_qx_win::dragMove(_p->mainWidget);
		}

		void StyleBase::switchNormalMaximized() const
		{
			_qx_win::switchNormalMaximized(_p->mainWidget);
		}

		void StyleBase::setTitleBarEnabled(bool enable)
		{
			_p->titleBarEnabled = enable;
			_p->titleBar->widget()->setVisible(enable);

			if(_p->titleBarEnabled)
			{
				int titleBarWidth = _p->mainWidget->width();
				WINDOWPLACEMENT wp;
				wp.length = sizeof(WINDOWPLACEMENT);
				::GetWindowPlacement((HWND)_p->mainWidget->winId(), &wp);
				if(wp.showCmd == SW_SHOWMAXIMIZED)
				{
					titleBarWidth -= 16;
				}
				_p->titleBar->widget()->resize(titleBarWidth, _qx_win::titleBarHeight);
			}

			_p->updatContentsMargins();
		}

		bool StyleBase::titleBarEnabled() const
		{
			return _p->titleBarEnabled;
		}

		TitleBar* StyleBase::titleBar() const
		{
			return _p->titleBar;
		}

		void StyleBase::addDragger(QWidget* widget, bool maximizedByDoubleClick /*= false*/)
		{
			if(widget)
			{
				if(widget == _p->border){
					return;
				}
				_p->draggerAreas.push_back(std::make_tuple(widget, maximizedByDoubleClick));
				widget->installEventFilter(_p.get());

				auto last = std::remove_if(_p->draggerAreas.begin(), _p->draggerAreas.end(), [](std::tuple<QPointer<QObject>, bool >  const& p){
					return !std::get<0>(p);
				});

				_p->draggerAreas.erase(last, _p->draggerAreas.end());
			}
		}

		void StyleBase::removeDragger(QWidget* widget)
		{
			if(widget)
			{
				widget->removeEventFilter(_p.get());
				auto last = std::remove_if(_p->draggerAreas.begin(), _p->draggerAreas.end(), [widget](std::tuple<QPointer<QObject>, bool >  const& p){
					return widget == std::get<0>(p);
				});

				_p->draggerAreas.erase(last, _p->draggerAreas.end());
			}
		}

		//////////////////////////////////////////////////////////////////////////
		bool StyleBase::_eventImpl(QEvent * e)
		{
			switch(e->type())
			{
				case QEvent::WinIdChange:
				{
					_p->updateStyle();
				}break;

				case QEvent::Show:
				{
					if(!(_p->mainWidget->windowState() & (Qt::WindowMaximized | Qt::WindowFullScreen | Qt::WindowMinimized)))
					{
						if(isMinAnimate())
						{
							_p->border->hide();// HACK
							QTimer::singleShot(200, _p->border, [=]{_p->border->show();	});
						}
						else
						{
							_p->border->show();
						}
					}
				}break;
				case QEvent::Hide:
				{
					_p->border->hide();
				}break;
				case QEvent::WindowStateChange:
				{
					_p->updatContentsMargins();

					if(_p->mainWidget->windowState() & Qt::WindowMaximized)
					{
						_p->border->hide();
					}
					else if(_p->mainWidget->windowState() & Qt::WindowFullScreen)
					{
						_p->border->hide();
					}
					else if(_p->mainWidget->windowState() == Qt::WindowState::WindowMinimized)
					{
						_p->border->hide();
					}
					else if(_p->mainWidget->windowState() == Qt::WindowState::WindowNoState)
					{
						QWindowStateChangeEvent* ev = static_cast<QWindowStateChangeEvent*>(e);
						if(ev->oldState() & (Qt::WindowMaximized | Qt::WindowFullScreen | Qt::WindowState::WindowMinimized))
						{
							if(isMinAnimate())
							{
								_p->border->hide();// HACK
								QTimer::singleShot(200, _p->border, [=]{_p->border->show();	});
							}
							else
							{
								_p->border->show();
							}
						}
					}
				}break;
				case QEvent::ActivationChange:
				{
					_p->border->update();
				}break;
				case QEvent::Close:
				{
					_p->border->close();
				}break;
				default:
					break;
			}

			return false;
		}

		bool StyleBase::_nativeEventImpl(const QByteArray &, void *message, long *result)
		{
			MSG* msg = static_cast<MSG*>(message);
			switch(msg->message)
			{
				case WM_DWMCOMPOSITIONCHANGED:
				{
					_p->updateStyle();
					_p->mainWidget->setVisible(!_p->mainWidget->isVisible());//hack: DWM混合模式变化后，不显示
					_p->mainWidget->setVisible(!_p->mainWidget->isVisible());
				}break;
				case WM_STYLECHANGING:
				{
					if(msg->wParam == GWL_STYLE)
					{
						//STYLESTRUCT * ss = reinterpret_cast<STYLESTRUCT*>(msg->lParam);
						//ss->styleNew |= _p->getStyle();
					}
				}break;

				case WM_STYLECHANGED:
				{
					if(msg->wParam == GWL_STYLE)
					{
						//STYLESTRUCT * ss = reinterpret_cast<STYLESTRUCT*>(msg->lParam);
					}
				}break;

				case WM_WINDOWPOSCHANGING:
				{
					auto lpos = reinterpret_cast<WINDOWPOS*>(msg->lParam);
					WINDOWPLACEMENT wp;
					wp.length = sizeof(WINDOWPLACEMENT);
					::GetWindowPlacement(msg->hwnd, &wp);
					if(wp.showCmd == SW_SHOWMAXIMIZED)
					{
						RECT rect;
						rect.left = lpos->x;
						rect.top = lpos->y;
						rect.right = lpos->x + lpos->cx;
						rect.bottom = lpos->y + lpos->cy;
						HMONITOR hScreen = MonitorFromRect(&rect, MONITOR_DEFAULTTONEAREST);

						MONITORINFO info;
						info.cbSize = sizeof(MONITORINFO);
						if(GetMonitorInfo(hScreen, &info))
						{
							if(!(lpos->flags & SWP_NOMOVE))
							{
								//lpos->x = info.rcWork.left;
								//lpos->y = info.rcWork.top;
							}
							if(!(lpos->flags & SWP_NOSIZE))
							{
								lpos->cx = info.rcWork.right - info.rcWork.left;
								lpos->cy = info.rcWork.bottom - info.rcWork.top;
							}
						}
					}
				}break;
				case WM_WINDOWPOSCHANGED:
				{
					auto lpos = reinterpret_cast<WINDOWPOS*>(msg->lParam);
					_p->border->resize(lpos->cx + _p->border->borderWidth * 2, lpos->cy + _p->border->borderWidth * 2);
					_p->border->move(lpos->x - _p->border->borderWidth, lpos->y - _p->border->borderWidth);

					if(_p->titleBarEnabled)
					{
						int titleBarWidth = lpos->cx;
						WINDOWPLACEMENT wp;
						wp.length = sizeof(WINDOWPLACEMENT);
						::GetWindowPlacement(msg->hwnd, &wp);
						if(wp.showCmd == SW_SHOWMAXIMIZED)
						{
							titleBarWidth -= 16;
						}
						_p->titleBar->widget()->resize(titleBarWidth, _qx_win::titleBarHeight);
					}
				}break;
				case WM_NCCALCSIZE:
				{
					WINDOWPLACEMENT wp;
					wp.length = sizeof(WINDOWPLACEMENT);
					::GetWindowPlacement(msg->hwnd, &wp);
					if(wp.showCmd == SW_SHOWMAXIMIZED)
					{
						if(msg->wParam)
						{
							auto lpP = reinterpret_cast <LPNCCALCSIZE_PARAMS>(msg->lParam);
							auto p = &lpP->rgrc[0];
							p->left += 8;
							p->top += 8;
							p->right -= 8;
							p->bottom -= 8;
							//*result = WVR_REDRAW;
							return true;
						}
					}
					*result = 0;
					return true;
				}break;
				case WM_NCACTIVATE:
				{
					if(!_qx_win::dwmCompositionIsEnabled())
					{
						*result = 1;
						return true;
					}
				}break;
				case WM_NCHITTEST:
				{
					*result = HTCLIENT;
					return true;
				}break;

			#if 1
				case WM_SYSKEYDOWN:
				{
					if(msg->wParam == VK_SPACE)
					{
						*result = 0;
						return true;
					}
					return false;
				}
			#endif
				default:
					break;
			}

			return false;
		}
	}
}