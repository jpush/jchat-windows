#include "GroupEventWidget.h"

namespace JChat {

	GroupEventWidget::GroupEventWidget(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);
	}

	GroupEventWidget::~GroupEventWidget()
	{
	}

	void GroupEventWidget::setClientObject(ClientObjectPtr const& co)
	{
		_co = co;

		connect(_co.get(), &ClientObject::userLogined, this, [=]
		{
			loadFromDB();
		});


	}

	void GroupEventWidget::loadFromDB()
	{

	}

} // namespace JChat
