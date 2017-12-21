#include "RoomInfoWidget.h"
#include <QPainter>
#include <QResizeEvent>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QMessageBox>
#include <QMenu>
#include <QMouseEvent>

#include "BusyIndicator.h"
#include "MainWidget.h"
namespace JChat {


	RoomInfoWidget::RoomInfoWidget(ClientObjectPtr const& co, Jmcpp::RoomId roomId, QWidget *parent)
		: QWidget(parent)
		, _co(co)
		, _roomId(roomId)
	{
		ui.setupUi(this);
		ui.labelMembers->hide();
		connect(qApp, &QApplication::focusChanged, this, [=](QWidget *old, QWidget *now)
		{
			if(!isVisible() || !now){ return; }
			while(now){
				if(now == this){ return; }
				now = now->parentWidget();
			}
			close();
		});

		auto effect = new QGraphicsDropShadowEffect(this);
		effect->setOffset(-2, 0);
		effect->setBlurRadius(10);
		this->setGraphicsEffect(effect);
		connect(ui.btnClose, &QToolButton::clicked, this, &RoomInfoWidget::close);
		parent->installEventFilter(this);


		updateInfo();
	}

	RoomInfoWidget::~RoomInfoWidget()
	{
	}

	Q_SLOT void RoomInfoWidget::close()
	{
		if(active)
		{
			return;
		}
		active = true;
		raise();
		auto parent = parentWidget();
		auto prarentWidth = parent->width();

		QPropertyAnimation *animnow = new QPropertyAnimation(this, "pos", this);
		animnow->setDuration(300);
		animnow->setEasingCurve(QEasingCurve::Type::InOutCubic);
		animnow->setStartValue(QPoint(prarentWidth - width(), 0));
		animnow->setEndValue(QPoint(prarentWidth + 30, 0));
		QObject::connect(animnow, &QPropertyAnimation::finished, this, [=]{
			animnow->deleteLater();
			lower();
			QWidget::close();
			active = false;
		});
		animnow->start();
	}

	Q_SLOT void RoomInfoWidget::on_btnLeaveRoom_clicked()
	{
		auto co = _co;
		auto roomId = _roomId;

		try
		{
			BusyIndicator busy(this->topLevelWidget());
			qAwait(co->leaveRoom(roomId));
			co->setCurrentRoomId({});
			auto mw = MainWidget::getCurrentMainWidget();
			if(mw)
			{
				mw->switchToRoomPage(roomId);
			}
		}
		catch(std::runtime_error& e)
		{
		}
	}

	None RoomInfoWidget::updateInfo()
	{
		auto self = this | qTrack;
		auto co = _co;
		auto roomId = _roomId;

		auto roomInfo = co_await co->getRoomInfo(roomId);

		co_await self;

		ui.labelRoomName->setText(QString(u8"聊天室名称: %1").arg(roomInfo.roomName.data()));

		ui.labelRoomId->setText(QString(u8"聊天室ID: %1").arg(roomId.get()));
		ui.labelMembers->setText(QString(u8"聊天室成员: %1人").arg(roomInfo.currentMemberCount));

		ui.textBrowser->setText(QString(u8"%1").arg(roomInfo.description.c_str()));

	}

	void RoomInfoWidget::showEvent(QShowEvent *event)
	{
		if(active)
		{
			return;
		}

		active = true;
		auto parent = parentWidget();
		auto prarentWidth = parent->width();

		resize(width(), parent->height());

		setFocus();
		raise();

		QPropertyAnimation *animnow = new QPropertyAnimation(this, "pos", this);

		animnow->setDuration(300);
		animnow->setEasingCurve(QEasingCurve::Type::InOutCubic);
		animnow->setStartValue(QPoint(prarentWidth + 30, 0));
		animnow->setEndValue(QPoint(prarentWidth - width(), 0));
		QObject::connect(animnow, &QPropertyAnimation::finished, this, [=]{
			animnow->deleteLater();
			active = false;
		});
		animnow->start();
	}

	void RoomInfoWidget::paintEvent(QPaintEvent *event)
	{
		QStyleOption opt;
		opt.init(this);
		QPainter p(this);
		style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	}

} // namespace JChat
