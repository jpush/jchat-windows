#include "ItemWidgetImageLeft.h"

#include <QDesktopServices>
#include <QMouseEvent>
#include "Util.h"
#include "MessageListWidget.h"
#include "UserInfoWidget.h"

namespace JChat
{
	ItemWidgetImageLeft::ItemWidgetImageLeft(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);

		ui.labelImage->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
		connect(ui.labelImage, &QLabel::customContextMenuRequested, this, [=](QPoint const& pt)
		{
			auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
			auto menu = listWidget->createMenu(this);
			menu->exec(ui.labelImage->mapToGlobal(pt));
		});

		ui.labelImage->installEventFilter(this);

	
		connect(ui.label, &Label::clicked, this, [=]
		{
			if(_msg && _msg->sender)
			{
				auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
				UserInfoWidget::showUserInfo(listWidget->getClientObject(), _msg->sender, this->topLevelWidget());
			}
		});

	}

	ItemWidgetImageLeft::~ItemWidgetImageLeft()
	{
	}

	void ItemWidgetImageLeft::setImageHolder(QSize const& imageSize)
	{
		auto sz = getImageThumbnailSize(imageSize);

		ui.labelImage->setScaledContents(true);
		ui.labelImage->setFixedSize(sz + QSize(10, 10));
	}

	void ItemWidgetImageLeft::setFilePath(QString const& filePath)
	{
		ui.labelImage->setProperty("filepath", filePath);
	}

	void ItemWidgetImageLeft::setImage(QPixmap const& img)
	{
		auto sz = img.size();
		sz = getImageThumbnailSize(sz);
		auto image = img.scaledToWidth(sz.width(), Qt::TransformationMode::SmoothTransformation);

		if(image.isNull())
		{
			setFailed();
			return;
		}

		ui.labelImage->setPixmap(image);
		ui.labelImage->setScaledContents(true);
		ui.labelImage->setFixedSize(sz + QSize(10, 10));
	}

	void ItemWidgetImageLeft::setMovie(QMovie *mv, QSize const& sz)
	{
		auto newSz = getImageThumbnailSize(sz);
		mv->setScaledSize(newSz);

		mv->setParent(this);
		ui.labelImage->setMovie(mv);
		if(this->isVisible())
		{
			mv->start();
		}

		ui.labelImage->setScaledContents(true);
		ui.labelImage->setFixedSize(newSz + QSize(10, 10));
	}

	QSize ItemWidgetImageLeft::itemWidgetSizeHint(QSize const& newSize)
	{
		auto sz = newSize;
		sz.setHeight(ui.labelImage->height() + 30);
		return sz;
	}

	void ItemWidgetImageLeft::setAvatarPixmap(QPixmap const& pixmap)
	{
		ui.label->setPixmap(pixmap);
	}

	void ItemWidgetImageLeft::setDisplayName(QString const& name)
	{
		ui.labelName->setText(name);
	}

	void ItemWidgetImageLeft::setComplete()
	{
		ui.labelStatus->clear();

	}

	void ItemWidgetImageLeft::setFailed()
	{
		//ui.labelStatus->setPixmap(getFailedPixmap());
		setImage(getFailedImagePlaceHolder());
	}

	void ItemWidgetImageLeft::setProgress(int)
	{
		ui.labelStatus->setMovie(getProgressMovie());
	}

	void ItemWidgetImageLeft::visibleChanged(bool isVisible)
	{
		auto mv = ui.labelImage->movie();
		if(mv)
		{
			mv->setPaused(!isVisible);
		}
	}

	bool ItemWidgetImageLeft::eventFilter(QObject *watched, QEvent *event)
	{
		if(watched == ui.labelImage && event->type() == QEvent::MouseButtonRelease)
		{
			auto ev = static_cast<QMouseEvent*>(event);
			if(ev->button() == Qt::LeftButton)
			{
				auto filepath = ui.labelImage->property("filepath").toString();
				if(!filepath.isEmpty())
				{
					QDesktopServices::openUrl(QUrl::fromLocalFile(filepath));
				}
			}
		}

		return false;
	}

}