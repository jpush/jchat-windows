#include "ItemWidgetCenter.h"

namespace JChat {

	ItemWidgetCenter::ItemWidgetCenter(QWidget *parent)
		: QWidget(parent)
	{
		ui.setupUi(this);
	}

	ItemWidgetCenter::~ItemWidgetCenter()
	{
	}

	JChat::ItemWidgetInterface::ItemFlags ItemWidgetCenter::flags()
	{
		return ItemFlag::notice;
	}

	QSize ItemWidgetCenter::itemWidgetSizeHint(QSize const& newSize)
	{
		auto sz = newSize;
		sz.setHeight(20);
		return sz;
	}

} // namespace JChat
