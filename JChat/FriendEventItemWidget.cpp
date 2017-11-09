#include "FriendEventItemWidget.h"
#include <QMouseEvent>

#include "Util.h"
namespace JChat {

	FriendEventItemWidget::FriendEventItemWidget(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);

		ui.label->installEventFilter(this);


		connect(ui.toolButtonPass, &QToolButton::clicked, this, [=]
		{
			Q_EMIT passClicked(_userId, _id);

		});
		connect(ui.toolButtonReject, &QToolButton::clicked, this, [=]
		{
			Q_EMIT rejectClicked(_userId, _id);
		});

	}

	FriendEventItemWidget::~FriendEventItemWidget()
	{

	}

	None FriendEventItemWidget::setEvent(ClientObjectPtr co, FriendEventT et)
	{
		auto userId = Jmcpp::UserId{ et.username ,et.appkey };
		_userId = userId;
		_id = et.id;

		auto self = this | qTrack;

		Jmcpp::UserInfo info;
		info.userId = userId;
		try
		{
			info = co_await co->getCacheUserInfo(userId);
		}
		catch(...)
		{
		}

		auto name = getUserDisplayName(info);

		co_await self;
		setStatus(et.isOutgoing, et.status);

		if(et.isOutgoing)
		{
			setTitleLabel(name);

			auto text = et.status == et.undone ? u8"等待对方验证"
				: et.status == et.passed ? u8"同意了您的好友请求"
				: u8"拒绝了您的好友请求";

			setStatusLabel(text);

		}
		else
		{
			auto text = QString(u8"%1 请求添加您为好友").arg(name);
			setTitleLabel(text);

			text = et.status == et.undone ? u8""
				: et.status == et.passed ? u8"已同意"
				: u8"已拒绝";

			setStatusLabel(text);
		}

		setMessageLabel(et.message);

		auto pixmap = co_await co->getCacheUserAvatar(userId);
		co_await self;
		setPixmap(pixmap);
	}

	bool FriendEventItemWidget::eventFilter(QObject *watched, QEvent *event)
	{
		if(event->type() == QEvent::MouseButtonRelease)
		{
			auto ev = static_cast<QMouseEvent*>(event);
			if(ev->button() == Qt::LeftButton)
			{
				Q_EMIT infoClicked(_userId, _id);
			}
		}
		return false;
	}

} // namespace JChat
