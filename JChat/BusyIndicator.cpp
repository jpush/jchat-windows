#include "BusyIndicator.h"

#include <QTimer>
#include <QMovie>
#include <QPainter>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <QtSvg/QSvgWidget>

namespace JChat {

	BusyIndicator::BusyIndicator(QWidget *parent)
		: QWidget(parent)
	{
		auto horizontalLayout = new QHBoxLayout(this);
		horizontalLayout->setSpacing(6);
		horizontalLayout->setContentsMargins(11, 11, 11, 11);
		horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
		horizontalLayout->setContentsMargins(0, 0, 0, 0);

		if(parent)
		{
			setStyleSheet(" background-color: rgba(255, 255, 255, 150 );    ");
			// 		
			auto svg = new QSvgWidget(this);
			svg->load(QString(":/image/resource/Double Ring.svg"));
			svg->setStyleSheet(" background-color: rgba(0, 0, 0, 0 );    ");

			horizontalLayout->addWidget(svg);
			svg->setFixedSize(48, 48);
			parent->installEventFilter(this);
			resize(parent->size());
		}
		else
		{
			this->setWindowModality(Qt::WindowModality::WindowModal);
			setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
			setStyleSheet(" background-color: rgba(0, 0, 0, 0 );    ");
			auto svg = new QSvgWidget(this);
			svg->load(QString(":/image/resource/Double Ring.svg"));
			svg->setStyleSheet(" background-color: rgba(0, 0, 0, 0 );    ");
			horizontalLayout->addWidget(svg);
			setFixedSize(48, 48);
		}
		show();
	}

	BusyIndicator::~BusyIndicator()
	{

	}

	bool BusyIndicator::eventFilter(QObject *watched, QEvent *event)
	{
		if(watched == parent() && event->type() == QEvent::Resize)
		{
			auto e = static_cast<QResizeEvent*>(event);
			resize(e->size());
		}
		return false;
	}

	void BusyIndicator::showEvent(QShowEvent *event)
	{
		auto p = this->parentWidget();
		if(p)
		{
			raise();
		}
	}

	void BusyIndicator::closeEvent(QCloseEvent *event)
	{
		event->setAccepted(!event->spontaneous());
	}

} // namespace JChat
