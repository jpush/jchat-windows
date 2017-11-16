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


		disconnect(ui.label, &Label::clicked, nullptr, nullptr);
		disconnect(ui.labelMessage, &Label::linkActivated, nullptr, nullptr);
		disconnect(ui.labelTitle, &Label::linkActivated, nullptr, nullptr);

		setStatus(et);

		auto user = Jmcpp::UserId{ et.username,et.appkey };
		auto fromUser = Jmcpp::UserId{ et.fromUsername,et.fromAppkey };

		auto groupId = et.groupId;

		auto groupName = co_await co->getGroupDisplayName(et.groupId);
		auto fromUserInfo = co_await co->getCacheUserInfo(fromUser);
		auto userInfo = co_await co->getCacheUserInfo(user);

		co_await self;
		if(et.isReject)
		{
			QString msg = u8R"(群主拒绝 <a href=""><span style="text-decoration: none; color:#0000ff;">%1</span></a> 入群)";

			auto name = getUserDisplayName(userInfo);

			auto info = co_await co->getCacheGroupInfo(et.groupId);

			auto avatar = co_await co->getCacheGroupAvatar(et.groupId);

			co_await self;

			ui.labelMessage->setText(msg.arg(name));

			connect(ui.labelMessage, &QLabel::linkActivated, this, [=]
			{
				infoClicked(user);
			});

			ui.labelTitle->setText(QString(u8R"(<a href=""><span style="text-decoration: none; color:#0000ff;">%1</span></a>)").arg(groupName));

			connect(ui.labelTitle, &QLabel::linkActivated, this, [=](QString const& link)
			{
				groupInfoClicked(groupId);
			});

			ui.label->setPixmap(avatar);
			connect(ui.label, &Label::clicked, this, [=]
			{
				groupInfoClicked(groupId);
			}, Qt::UniqueConnection);
		}
		else
		{

			if(et.bySelf)
			{
				auto avatar = co_await co->getCacheUserAvatar(fromUser);

				co_await self;

				QString t = u8R"(<a href="1"><span style="text-decoration: none; color:#0000ff;">%1</span></a> 申请加入群 <a href="2"><span style="text-decoration: none; color:#0000ff;">%2</span></a>)";

				ui.labelTitle->setText(t.arg(getUserDisplayName(fromUserInfo), groupName));

				connect(ui.labelTitle, &QLabel::linkActivated, this, [=](QString const& link)
				{
					if(link == "1")
					{
						infoClicked(fromUser);
					}
					else
					{
						groupInfoClicked(groupId);
					}
				});


				ui.labelMessage->setText(u8"附加信息：" + et.message);

				ui.label->setPixmap(avatar);

				connect(ui.label, &Label::clicked, this, [=]
				{
					infoClicked(fromUser);
				});
			}
			else
			{
				auto avatar = co_await co->getCacheUserAvatar(user);

				co_await self;

				QString t = u8R"(<a href="1"><span style="text-decoration: none; color:#0000ff;">%1</span></a> 申请加入群 <a href="2"><span style="text-decoration: none; color:#0000ff;">%2</span></a>)";

				ui.labelTitle->setText(t.arg(getUserDisplayName(userInfo), groupName));

				connect(ui.labelTitle, &QLabel::linkActivated, this, [=](QString const& link)
				{
					if(link == "1")
					{
						infoClicked(user);
					}
					else
					{
						groupInfoClicked(groupId);
					}
				});


				ui.labelMessage->setText(u8"邀请人：" + getUserDisplayName(fromUserInfo));

				ui.label->setPixmap(avatar);

				connect(ui.label, &Label::clicked, this, [=]
				{
					infoClicked(user);
				});

			}

		}
	}

} // namespace JChat
