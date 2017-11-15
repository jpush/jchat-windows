#include "Label.h"

#include <QMouseEvent>
namespace JChat {

	Label::Label(QWidget *parent)
		: QLabel(parent)
	{
	}

	Label::~Label()
	{
	}

	bool Label::event(QEvent * e)
	{
		if(e->type() == QEvent::MouseButtonRelease)
		{
			auto ev = static_cast<QMouseEvent*>(e);
			if(ev->button() == Qt::LeftButton)
			{
				Q_EMIT clicked();
			}
		}
		return QLabel::event(e);
	}

} // namespace JChat
