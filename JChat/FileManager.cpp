#include "FileManager.h"
#include <QPainter>
#include <QResizeEvent>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QMessageBox>
#include <QApplication>

JChat::FileManager::FileManager(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
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
	connect(ui.btnClose, &QToolButton::clicked, this, &FileManager::close);
	parent->installEventFilter(this);


	auto btnGroup = new QButtonGroup(this);
	btnGroup->addButton(ui.btnShowDoc);
	btnGroup->addButton(ui.btnShowImage);
	btnGroup->addButton(ui.btnShowMusic);
	btnGroup->addButton(ui.btnShowOther);
	btnGroup->addButton(ui.btnShowVideo);
}

JChat::FileManager::~FileManager()
{
}

Q_SLOT void 
JChat::FileManager::close()
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

bool JChat::FileManager::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == parentWidget() && event->type() == QEvent::Resize)
	{
		auto ev = static_cast<QResizeEvent*>(event);
		move(ev->size().width() - width(), 0);
		resize(width(), ev->size().height());
	}


	return false;
}

void JChat::FileManager::showEvent(QShowEvent *event)
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

