#include "ElidedLabel.h"

#include <QResizeEvent>
#include <QPainter>
namespace JChat {

	ElidedLabel::ElidedLabel(QWidget *parent)
		: QLabel(parent)
	{
	}

	ElidedLabel::~ElidedLabel()
	{
	}

	Q_SLOT void ElidedLabel::setText(const QString & txt)
	{
		QLabel::setText(txt);
		cacheElidedText(geometry().width());
	}

	void ElidedLabel::resizeEvent(QResizeEvent* e)
	{
		QLabel::resizeEvent(e);
		cacheElidedText(e->size().width());
	}

	void ElidedLabel::paintEvent(QPaintEvent* e)
	{
		if(elideMode_ == Qt::ElideNone) {
			QLabel::paintEvent(e);
		}
		else {
			QPainter p(this);
			p.drawText(0, 0,
					   geometry().width(),
					   geometry().height(),
					   alignment(), cachedElidedText
			);
		}
	}

	void ElidedLabel::cacheElidedText(int w)
	{
		cachedElidedText = fontMetrics().elidedText(text(), elideMode_, w, Qt::TextShowMnemonic);
	}

} // namespace JChat
