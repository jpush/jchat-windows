#include "MemberItemWidget.h"

#include <QMouseEvent>
namespace JChat {

	MemberItemWidget::MemberItemWidget(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);
		setContextMenuPolicy(Qt::CustomContextMenu);

		ui.labelAvatar->installEventFilter(this);
	}

	MemberItemWidget::~MemberItemWidget()
	{

	}

	void MemberItemWidget::setOwner(bool b)
	{
		_isOwner = b;
		ui.labelOwner->setVisible(b);
	}

	void MemberItemWidget::setSilent(bool b)
	{
		_isSilent = b;
		ui.labelSlient->setVisible(b);
	}

	bool MemberItemWidget::eventFilter(QObject *watched, QEvent *event)
	{
		if(watched == ui.labelAvatar && event->type() == QEvent::MouseButtonPress)
		{
			auto ev = static_cast<QMouseEvent*>(event);

			if(ev->button() == Qt::LeftButton)
			{
				emit memberInfoClicked();
			}
		}

		return false;
	}

} // namespace JChat
