#include "ItemWidgetVoiceLeft.h"

#include <QMouseEvent>
#include <QtMultimedia/QMediaPlayer>

#include "MessageListWidget.h"

static QMovie* getPlayMoive()
{
	static QMovie* mv = []
	{
		auto mv = new QMovie(":/image/resource/voice-active-you.gif");
		mv->start();
		return mv;
	}();
	return mv;
}

static QPixmap getPlayIcon()
{
	static QPixmap mv(":/image/resource/voice_you.png");
	return mv;
}

JChat::ItemWidgetVoiceLeft::ItemWidgetVoiceLeft(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setPlaying(false);

	ui.frameVoice->installEventFilter(this);
}

JChat::ItemWidgetVoiceLeft::~ItemWidgetVoiceLeft()
{

}


void JChat::ItemWidgetVoiceLeft::setVoiceUrl(QString const& url)
{
	ui.labelIcon->setProperty("url", url);
}

QSize JChat::ItemWidgetVoiceLeft::itemWidgetSizeHint(QSize const& newSize)
{
	auto sz = newSize;
	sz.setHeight(ui.frameVoice->height() + 30);
	return sz;
}

void JChat::ItemWidgetVoiceLeft::setDisplayName(QString const& name)
{
	ui.labelName->setText(name);
}

void JChat::ItemWidgetVoiceLeft::setAvatarPixmap(QPixmap const& pixmap)
{
	ui.label->setPixmap(pixmap);
}

void JChat::ItemWidgetVoiceLeft::setComplete()
{
	if(_msg->needReceipt)
	{
		static QPixmap icon(u8":/image/resource/ÓïÒôÎ´¶Á.png");
		ui.labelStatus->setPixmap(icon);
		ui.labelStatus->setScaledContents(false);
	}
	else
	{
		ui.labelStatus->clear();
		ui.labelStatus->setScaledContents(true);
	}
}

void JChat::ItemWidgetVoiceLeft::setFailed()
{
	ui.labelStatus->setPixmap(getFailedPixmap());
}

void JChat::ItemWidgetVoiceLeft::setProgress(int)
{
	ui.labelStatus->setMovie(getProgressMovie());
}

bool JChat::ItemWidgetVoiceLeft::eventFilter(QObject *watched, QEvent *event)
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

void JChat::ItemWidgetVoiceLeft::setPlaying(bool playing)
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

