#include "GroupInfoDialog.h"

namespace JChat {

	GroupInfoDialog::GroupInfoDialog(ClientObjectPtr const& co, QWidget *parent)
		: QDialog(parent)
		, _co(co)
	{
		ui.setupUi(this);
	}

	GroupInfoDialog::~GroupInfoDialog()
	{

	}

	pplx::task<void> GroupInfoDialog::setGroup(Jmcpp::GroupId groupId)
	{
		auto self = this | qTrack;

		auto infoTask = _co->getCacheGroupInfo(groupId);
		auto memberTask = _co->getGroupMembers(groupId);

		auto pixmap =co_await _co->getCacheGroupAvatar(groupId);

		auto info = co_await infoTask;
		auto members = co_await memberTask;

		auto iter = std::find_if(members.begin(), members.end(), [](Jmcpp::GroupMember const& m)
		{
			return m.isOwner;
		});

		Jmcpp::UserId onwer = (iter == members.end()) ? Jmcpp::UserId{} : iter->userId;


		co_await self;

		ui.label->setPixmap(pixmap);

		ui.labelGroupName->setText(info.groupName.c_str());

		ui.labelGroupId->setText(QString(u8"»∫ID:%1").arg(info.groupId.get()));
		ui.labelOwnerName->setText(onwer.username.c_str());
		ui.labelCount->setText(QString(u8"%1»À").arg(members.size()));

		ui.textBrowser->setText(QString(u8"»∫√Ë ˆ:%1").arg(info.description.c_str()));
	}

	Q_SLOT void GroupInfoDialog::on_btnJoinGroup_clicked()
	{



	}

} // namespace JChat
