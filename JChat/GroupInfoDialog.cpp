#include "GroupInfoDialog.h"

#include <QInputDialog>
#include <QMessageBox>
namespace JChat {

	GroupInfoDialog::GroupInfoDialog(ClientObjectPtr const& co,Jmcpp::GroupId groupId, QWidget *parent)
		: QDialog(parent)
		, _co(co)
		,_groupId(groupId)
	{
		ui.setupUi(this);
	}

	GroupInfoDialog::~GroupInfoDialog()
	{

	}

	pplx::task<void> GroupInfoDialog::setGroup(Jmcpp::GroupId groupId)
	{
		_groupId = groupId;
		auto self = this | qTrack;

		auto infoTask = _co->getCacheGroupInfo(groupId);
		auto memberTask = _co->getGroupMembers(groupId);

		auto pixmap = co_await _co->getCacheGroupAvatar(groupId);

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

		ui.labelGroupId->setText(QString(u8"群ID:%1").arg(info.groupId.get()));
		ui.labelOwnerName->setText(onwer.username.c_str());
		ui.labelCount->setText(QString(u8"%1人").arg(members.size()));

		ui.textBrowser->setText(QString(u8"群描述:%1").arg(info.description.c_str()));
	}

	Q_SLOT void GroupInfoDialog::on_btnJoinGroup_clicked()
	{

		bool ok = false;
		auto text = QInputDialog::getMultiLineText(this, "", u8"请填写验证信息", {}, &ok);
		if(!ok)
		{
			return;
		}

		try
		{
			qAwait(_co->joinGroup(_groupId, text.toStdString()));
			QMessageBox::information(this, "", u8"入群申请已发送，请等待审核", QMessageBox::Ok);
		}
		catch(std::runtime_error& e)
		{
			QMessageBox::warning(this, "", e.what(), QMessageBox::Ok);
		}
	}

} // namespace JChat
