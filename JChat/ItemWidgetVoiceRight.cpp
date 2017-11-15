#include "ItemWidgetVoiceRight.h"

#include <QMouseEvent>
#include <QtMultimedia/QMediaPlayer>

#include "MessageListWidget.h"
#include "UserInfoWidget.h"

static QMovie* getPlayMoive()
{
	static QMovie* mv = []
	{
		auto mv = new QMovie(":/image/resource/voice-active-me.gif");
		mv->start();
		return mv;
	}();
	return mv;
}

static QPixmap getPlayIcon()
{
	static QPixmap mv(":/image/resource/voice_me.png");
	return mv;
}

JChat::ItemWidgetVoiceRight::ItemWidgetVoiceRight(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setPlaying(false);

	ui.frameVoice->installEventFilter(this);
	connect(ui.label, &Label::clicked, this, [=]
	{
		if(_msg && _msg->sender)
		{
			auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
			UserInfoWidget::showUserInfo(listWidget->getClientObject(), _msg->sender, this->topLevelWidget());
		}
	});

}

JChat::ItemWidgetVoiceRight::~ItemWidgetVoiceRight()
{

}


void JChat::ItemWidgetVoiceRight::setVoiceUrl(QString const& url)
{
	ui.labelIcon->setProperty("url", url);
}

QSize JChat::ItemWidgetVoiceRight::itemWidgetSizeHint(QSize const& newSize)
{
	auto sz = newSize;
	sz.setHeight(ui.frameVoice->height() + 10);
	return sz;
}

void JChat::ItemWidgetVoiceRight::setAvatarPixmap(QPixmap const& pixmap)
{
	ui.label->setPixmap(pixmap);
}

void JChat::ItemWidgetVoiceRight::setComplete()
{
	ui.labelStatus->clear();
	ui.labelStatus->setScaledContents(true);
}

void JChat::ItemWidgetVoiceRight::setFailed()
{
	ui.labelStatus->setPixmap(getFailedPixmap());
}

void JChat::ItemWidgetVoiceRight::setProgress(int)
{
	ui.labelStatus->setMovie(getProgressMovie());
}

bool JChat::ItemWidgetVoiceRight::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == ui.frameVoice && event->type() == QEvent::MouseButtonRelease)
	{
		auto ev = static_cast<QMouseEvent*>(event);
		if(ev->button() == Qt::LeftButton)
		{
			if(_msg->needReceipt)
			{
				auto listWidget = static_cast<MessageListWidget*>(_item->listWidget());
				listWidget->sendMessageReceipts({ this });
				ui.labelStatus->clear();
				ui.labelStatus->setScaledContents(true);
			}

			auto url = ui.labelIcon->property("url").toString();
			if(!url.isEmpty())
			{
				QMediaPlayer* player = qApp->property("QMediaPlayer").value<QMediaPlayer*>();
				if(player)
				{
					player->stop();

					connect(player, &QMediaPlayer::currentMediaChanged, this, [=](const QMediaContent &media)
					{
						if(media != QUrl(url))
						{
							setPlaying(false);
						}
					});
					connect(player, &QMediaPlayer::stateChanged, this, [=](QMediaPlayer::State state)
					{
						if(state == QMediaPlayer::StoppedState)
						{
							setPlaying(false);
						}
					});

					setPlaying(true);
					player->setMedia(QUrl(url));
					player->play();
				}
			}
		}
	}
	return false;
}

void JChat::ItemWidgetVoiceRight::setPlaying(bool playing)
{
	if(playing)
	{
		ui.labelIcon->setMovie(getPlayMoive());
	}
	else
	{
		ui.labelIcon->setPixmap(getPlayIcon());

		QMediaPlayer* player = qApp->property("QMediaPlayer").value<QMediaPlayer*>();
		if(player)
		{
			disconnect(player, 0, this, 0);
		}
	}
}

