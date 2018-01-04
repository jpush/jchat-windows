#include "ItemWidgetInterface.h"



void JChat::ItemWidgetInterface::setDisplayName(QString const&)
{

}

void JChat::ItemWidgetInterface::setAvatarPixmap(QPixmap const&)
{

}

void JChat::ItemWidgetInterface::setDateTime(QDateTime const& dt)
{
	_dt = dt;
}

QDateTime JChat::ItemWidgetInterface::getDateTime() const
{
	if(_msg)
	{
		return QDateTime::fromMSecsSinceEpoch(_msg->time);
	}
	return _dt;
}

void JChat::ItemWidgetInterface::setProgress(int)
{

}

void JChat::ItemWidgetInterface::setComplete()
{

}

void JChat::ItemWidgetInterface::setFailed()
{

}

void JChat::ItemWidgetInterface::visibleChanged(bool isVisible)
{

}

void JChat::ItemWidgetInterface::setUnreadUserCount(size_t count)
{

}

QWidget* JChat::ItemWidgetInterface::asWidget()
{
	return  dynamic_cast<QWidget*>(this);
}

void JChat::ItemWidgetInterface::attachItem(QListWidgetItem* item, QModelIndex const& index)
{
	auto w = asWidget();
	Q_ASSERT(w);
	if(w){
		item->listWidget()->setItemWidget(item, w);
		_item = item;
		_index = index;
	}
}

QListWidgetItem* JChat::ItemWidgetInterface::item() const
{
	return _item;
}

Jmcpp::MessagePtr const& JChat::ItemWidgetInterface::getMessage() const
{
	return _msg;
}

void JChat::ItemWidgetInterface::setMessage(Jmcpp::MessagePtr const& msg)
{
	_msg = msg;
}

void JChat::ItemWidgetInterface::updateItemWidgetSize()
{
	if(_index.isValid() && _item)
	{
		auto size = _item->listWidget()->viewport()->size();
		auto sz = itemWidgetSizeHint(size);
		_item->setSizeHint(sz);
	}
}

QString JChat::ItemWidgetInterface::getUnreadCountText(size_t count) const
{
	if(!_msg)
	{
		return {};
	}
	if(_msg->conId.isGroup())
	{
		if(count)
		{
			return QString(u8R"(<html><head/><body><p><span style=" color:#0000ff;">%1人未读</span></p></body></html>)").arg(count);
		}
		else
		{
			return QString(u8R"(<html><head/><body><p><span style=" color:#9a9a9a;">全部已读</span></p></body></html>)");
		}
	}
	else if(_msg->conId.isUser())
	{
		if(count)
		{
			return QString(u8R"(<html><head/><body><p><span style=" color:#0000ff;">未读</span></p></body></html>)");
		}
		else
		{
			return QString(u8R"(<html><head/><body><p><span style=" color:#9a9a9a;">已读</span></p></body></html>)");
		}
	}
	else
	{
		return {};
	}
}

QMovie* JChat::ItemWidgetInterface::getProgressMovie()
{
	static auto mv = []
	{
		auto mv = new QMovie();
		mv->setFileName(":/image/resource/Spinner.gif");
		mv->start();
		return mv;
	}();
	return mv;
}

QPixmap JChat::ItemWidgetInterface::getFailedPixmap()
{
	static	QPixmap pixmap(u8":/image/resource/发送文件 失败.png");
	return pixmap;
}

QPixmap JChat::ItemWidgetInterface::getFailedImagePlaceHolder()
{
	static	QPixmap pixmap(u8":/image/resource/imageFailed.png");
	return pixmap;
}
