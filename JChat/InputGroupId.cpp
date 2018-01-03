#include "InputGroupId.h"

#include <QMessageBox>

#include "BusyIndicator.h"
#include "GroupInfoDialog.h"

namespace JChat {

	InputGroupId::InputGroupId(ClientObjectPtr const& co, QWidget *parent)
		: QDialog(parent)
		, _co(co)

	{
		ui.setupUi(this);
	}

	InputGroupId::~InputGroupId()
	{
	}


	Q_SLOT void InputGroupId::on_btnOK_clicked()
	{
		bool ok = false;
		Jmcpp::GroupId groupId = ui.lineEdit->text().toLongLong(&ok);
		if(!ok)
		{
			QMessageBox::warning(this, "", u8"群组不存在", QMessageBox::Ok);
			return;
		}

		try
		{
			auto dialog = std::make_unique<GroupInfoDialog>(_co, groupId, this);
			dialog->setAttribute(Qt::WA_DeleteOnClose);
			dialog->setModal(true);

			BusyIndicator busy(this);
			auto info = qAwait(_co->getCacheGroupInfo(groupId));

			if(!info.isPublic)
			{
				//QMessageBox::warning(this, "", u8"群组不存在", QMessageBox::Ok);
				//return;
			}

			qAwait(dialog->setGroup(groupId));

			dialog->show();

			dialog.release();
		}
		catch(std::runtime_error& e)
		{
			QMessageBox::warning(this, "", u8"群组不存在", QMessageBox::Ok);
		}
	}

} // namespace JChat
