#include "GroupEventItem.h"

#include "Util.h"
namespace JChat {

	GroupEventItem::GroupEventItem(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);

		connect(ui.toolButtonPass, &QToolButton::clicked, this, [=]
		{
			passClicked(_et.groupId, _et.eventId, { _et.username,_et.appkey }, { _et.fromUsername,_et.fromAppkey });
		});
		connect(ui.toolButtonReject, &QToolButton::clicked, this, [=]
		{
			rejectClicked(_et.groupId, _et.eventId, { _et.username,_et.appkey }, { _et.fromUsername,_et.fromAppkey });
		});

	}

	GroupEventItem::~GroupEventItem()
	{
	}



	None GroupEventItem::setEvent(ClientObjectPtr co, GroupEventT et)
	{
		_et = et;
		auto self = this | qTrack;

		setStatus(et);

		auto user = Jmcpp::UserId{ et.username,et.appkey };
		auto fromUser =  Jmcpp::UserId{ et.fromUsername,et.fromAppkey };

		auto groupName = co_await co->getGroupDisplayName(et.groupId);
		auto fromUserInfo = co_await co->getCacheUserInfo(fromUser);
		auto userInfo = co_await co->getCacheUserInfo(user);

		co_await self;
		if(et.isReject)
		{
			QString msg = u8"群主拒绝 %1 入群";

			auto name = co_await co->getUserDisplayName(user);

			auto info = co_await co->getCacheGroupInfo(et.groupId);

			auto avatar = co_await co->getCacheGroupAvatar(et.groupId);

			co_await self;

			ui.labelMessage->setText(msg.arg(name));

			ui.labelTitle->setText(groupName);

			ui.label->setPixmap(avatar);
		}
		else
		{

			if(et.bySelf)
			{
				auto avatar = co_await co->getCacheUserAvatar(fromUser);

				co_await self;

				QString t = u8"%1 申请加入群 %2";

				ui.labelTitle->setText(t.arg(getUserDisplayName(fromUserInfo), groupName));

				ui.labelMessage->setText(u8"附加信息：" + et.message);

				ui.label->setPixmap(avatar);
			}
			else
			{
				auto avatar = co_await co->getCacheUserAvatar(user);

				co_await self;

				QString t = u8"%1 申请加入群 %1";

				ui.labelTitle->setText(t.arg(getUserDisplayName(userInfo), groupName));

				ui.labelMessage->setText(u8"邀请人：" + getUserDisplayName(fromUserInfo));

				ui.label->setPixmap(avatar);

			}

		}
	}

} // namespace JChat
